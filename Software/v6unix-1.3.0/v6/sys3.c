
#include "param.h"
#include "systm.h"
#include "reg.h"
#include "buf.h"
#include "filsys.h"
#include "user.h"
#include "inode.h"
#include "file.h"
#include "conf.h"


/*
 * The basic routine for fstat and stat:
 * get the inode and pass appropriate parts back.
 */
void
stat1(ip, ub)
	struct inode *ip;
	int *ub;
{
	register int i, *cp;
	register struct buf *bp;

	iupdat(ip, &time);
	bp = bread(ip->i_dev, (ip->i_number + 31) >> 4);
	cp = &(ip->i_dev);
	for(i=0; i<14; i++) {
		if(i==4) cp++; /* FIXME skip gid & filler for now */
		suword(ub++, *cp++);
	}
	cp = (int*) (bp->b_addr + 32 * ((ip->i_number + 31) & 017) + 24);
	for(i=0; i<4; i++) {
		suword(ub++, *cp++);
	}
	brelse(bp);
	u.u_ar0[R0] = 0;
}

/*
 * the fstat system call.
 */
void
fstat()
{
	register struct file *fp;

	fp = getf(u.u_arg[0]);
	if(fp == NULL)
		return;
	stat1(fp->f_inode, u.u_arg[1]);
}

/*
 * the stat system call.
 */
void
stat()
{
	register struct inode *ip;
	extern int uchar();

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	stat1(ip, u.u_arg[1]);
	iput(ip);
}

/*
 * the dup system call.
 */
void
dup()
{
	register int i;
	register struct file *fp;

	fp = getf(u.u_arg[0]);
	if(fp == NULL)
		return;
	if ((i = ufalloc()) < 0)
		return;
	u.u_ofile[i] = fp;
	fp->f_count++;
}

/*
 * the dup2 system call.
 */
void
dup2()
{
	register int i = u.u_arg[0];
	register int j = u.u_arg[1];
	register struct file *fp;

	if ( (fp = getf(j)) != NULL)
		closef(fp);
	if ( (fp = getf(i)) == NULL || (unsigned) j >= NOFILE)
		return;
	u.u_ar0[R0] = j;
	u.u_ofile[j] = fp;
	fp->f_count++;
}

/*
 * the mount system call.
 */
void
smount()
{
	int d;
	register struct inode *ip;
	register struct mount *mp, *smp;
	extern int uchar();

	d = getmdev();
	if(u.u_error)
		return;
	u.u_dirp = (char*) u.u_arg[1];
	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	if(ip->i_count!=1 || (ip->i_mode&(IFBLK&IFCHR))!=0)
		goto out;
	smp = NULL;
	for(mp = &mount[0]; mp < &mount[NMOUNT]; mp++) {
		if(mp->m_bufp != NULL) {
			if(d == mp->m_dev)
				goto out;
		} else
		if(smp == NULL)
			smp = mp;
	}
	if(smp == NULL)
		goto out;
	(*bdevsw[MAJOR(d)].d_open)(d, !u.u_arg[2]);
	if(u.u_error)
		goto out;
	mp = (struct mount*) bread(d, 1);
	if(u.u_error) {
		brelse((struct buf*)mp);
		goto out1;
	}
	smp->m_inodp = ip;
	smp->m_dev = d;
	smp->m_bufp = getblk(NODEV);
	memcpy(smp->m_bufp->b_addr, ((struct buf*)mp)->b_addr, 512);
	smp = (struct mount*)smp->m_bufp->b_addr;
	((struct filsys*)smp)->s_ilock = 0;
	((struct filsys*)smp)->s_flock = 0;
	((struct filsys*)smp)->s_ronly = u.u_arg[2] & 1;
	brelse((struct buf*)mp);
	ip->i_flag |= IMOUNT;
	prele(ip);
	return;

out:
	u.u_error = EBUSY;
out1:
	iput(ip);
}

/*
 * the umount system call.
 */
void
sumount()
{
	int d;
	register struct inode *ip;
	register struct mount *mp;

	update();
	d = getmdev();
	if(u.u_error)
		return;
	for(mp = &mount[0]; mp < &mount[NMOUNT]; mp++)
		if(mp->m_bufp!=NULL && d==mp->m_dev)
			goto found;
	u.u_error = EINVAL;
	return;

found:
	for(ip = &inode[0]; ip < &inode[NINODE]; ip++)
		if(ip->i_number!=0 && d==ip->i_dev) {
			u.u_error = EBUSY;
			return;
		}
	(*bdevsw[MAJOR(d)].d_close)(d, 0);
	ip = mp->m_inodp;
	ip->i_flag &= ~IMOUNT;
	iput(ip);
	ip = (struct inode*)mp->m_bufp;
	mp->m_bufp = NULL;
	brelse((struct buf*)ip);
}

/*
 * Common code for mount and umount.
 * Check that the user's argument is a reasonable
 * thing on which to mount, and return the device number if so.
 */
getmdev()
{
	register int d;
	register struct inode *ip;
	extern int uchar();

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	if((ip->i_mode&IFMT) != IFBLK)
		u.u_error = ENOTBLK;
	d = ip->i_addr[0];
	if(MAJOR(ip->i_addr[0]) >= nblkdev)
		u.u_error = ENXIO;
	iput(ip);
	return(d);
}
