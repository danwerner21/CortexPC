
#include "param.h"
#include "systm.h"
#include "user.h"
#include "proc.h"
#include "reg.h"
#include "file.h"
#include "inode.h"

/*
 * common code for read and write calls:
 * check permissions, set base, count, and offset,
 * and switch out to readi, writei, or pipe code.
 */
void
rdwr(mode)
	register int mode;
{
	register struct file *fp;

	fp = getf(u.u_arg[0]);
	if(fp == NULL)
		return;
	if((fp->f_flag&mode) == 0) {
		u.u_error = EBADF;
		return;
	}
	if (fp->f_flag & FNET) {
	   if (mode == FREAD)
	   	readn(fp);
	   else
	   	writen(fp);
	   return;
	}

	u.u_base = (char*) u.u_arg[1];
	u.u_count = u.u_arg[2];
	u.u_segflg = 0;
	if(fp->f_flag&FPIPE) {
		if(mode==FREAD)
			readp(fp);
		else
			writep(fp);
	} else {
		u.u_offset = fp->f_offset;
		if(mode==FREAD)
			readi(fp->f_inode);
		else
			writei(fp->f_inode);
		fp->f_offset += u.u_arg[2] - u.u_count;
	}
	u.u_ar0[R0] = u.u_arg[2] - u.u_count;
}

/*
 * read system call
 */
void
read()
{
	rdwr(FREAD);
}

/*
 * write system call
 */
void
write()
{
	rdwr(FWRITE);
}


/*
 * common code for open and creat.
 * Check permissions, allocate an open file structure,
 * and call the device open routine if any.
 */
void
open1(ip, mode, trf)
	register struct inode *ip;
	register int mode;
{
	register struct file *fp;
	int i;

	if(trf != 2) {
		if(mode&FREAD)
			access(ip, IREAD);
		if(mode&FWRITE) {
			access(ip, IWRITE);
			if((ip->i_mode&IFMT) == IFDIR)
				u.u_error = EISDIR;
		}
	}
	if(u.u_error)
		goto out;
	if(trf)
		itrunc(ip);
	prele(ip);
	if ((fp = falloc()) == NULL)
		goto out;
	fp->f_flag = mode&(FREAD|FWRITE);
	fp->f_inode = ip;
	i = u.u_ar0[R0];
	openi(ip, mode&FWRITE);
	if(u.u_error == 0)
		return;
	u.u_ofile[i] = NULL;
	fp->f_count--;

out:
	iput(ip);
}

/*
 * open system call
 */
void
open()
{
	register struct inode *ip;
	extern int uchar();

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	u.u_arg[1]++;
	open1(ip, u.u_arg[1], 0);
}

/*
 * creat system call
 */
void
creat()
{
	register struct inode *ip;
	extern int uchar();

	ip = namei(&uchar, 1);
	if(ip == NULL) {
		if(u.u_error)
			return;
		ip = maknode(u.u_arg[1]&07777&(~ISVTX));
		if (ip==NULL)
			return;
		open1(ip, FWRITE, 2);
	} else
		open1(ip, FWRITE, 1);
}

/*
 * close system call
 */
void
close()
{
	register struct file *fp;

	fp = getf(u.u_arg[0]);
	if(fp == NULL)
		return;
	u.u_ofile[u.u_arg[0]] = NULL;
	if (fp->f_flag & FNET)
		closen(fp, u.u_arg[0]);
	else
		closef(fp);
}

/*
 * lseek system call
 */
void
lseek()
{
	long n;
	register struct file *fp;
	register int t;

	fp = getf(u.u_arg[0]);
	if (fp == NULL)
		return;
	if(fp->f_flag&FPIPE) {
		u.u_error = ESPIPE;
		return;
	}
	t = u.u_arg[3];
	n = *(long*) &u.u_arg[1];

	switch (t) {
	default:
	case 0:
		fp->f_offset = n;
		break;
	case 1:
		fp->f_offset += n;
		break;
	case 2:
		fp->f_offset = n + fp->f_inode->i_size;
		break;
	}
	*(long*)(&u.u_ar0[R0]) = fp->f_offset;
}

/*
 * link system call
 */
void
link()
{
	register struct inode *ip, *xp;
	extern int uchar();

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	if(ip->i_nlink >= 127) {
		u.u_error = EMLINK;
		goto out;
	}
	if((ip->i_mode&IFMT)==IFDIR && !suser())
		goto out;

	/* unlock to avoid possibly hanging the namei */
	ip->i_flag &= ~ILOCK;
	u.u_dirp = (char*) u.u_arg[1];
	xp = namei(&uchar, 1);
	if(xp != NULL) {
		u.u_error = EEXIST;
		iput(xp);
	}
	if(u.u_error)
		goto out;

	/* check link is one same disk */
	if(u.u_pdir->i_dev != ip->i_dev) {
		iput(u.u_pdir);
		u.u_error = EXDEV;
		goto out;
	}
	
	/* update link directory & update target inode */
	wdir(ip);
	ip->i_nlink++;
	ip->i_flag |= IUPD;

out:
	iput(ip);
	u.u_ar0[R0] = 0;
}

/*
 * mknod system call
 */
void
mknod()
{
	register struct inode *ip;
	extern int uchar();

	if(suser()) {
		ip = namei(&uchar, 1);
		if(ip != NULL) {
			u.u_error = EEXIST;
			goto out;
		}
	}
	if(u.u_error)
		return;
	ip = maknode(u.u_arg[1]);
	if (ip==NULL)
		return;
	ip->i_addr[0] = u.u_arg[2];

out:
	iput(ip);
}

/*
 * sleep system call
 * not to be confused with the sleep internal routine.
 */
void
sslep()
{
	unsigned long d;
	register int s;

	s = spl7();
	d = time;
	d += (unsigned int)u.u_arg[0];

	while(d > time) {
		u.u_procp->p_tout = d;
		sleep(&u.u_procp->p_tout, PSLEP);
	}
	splx(s);
}
