
#include "param.h"
#include "systm.h"
#include "user.h"
#include "inode.h"
#include "file.h"
#include "reg.h"

/*
 * Max allowable buffering per pipe.
 * This is also the max size of the
 * file created to implement the pipe.
 * If this size is bigger than 4096,
 * pipes will be implemented in LARG
 * files, which is probably not good.
 */
#define	PIPSIZ	4096

/*
 * The sys-pipe entry.
 * Allocate an inode on the root device.
 * Allocate 2 file structures.
 * Put it all together with flags.
 */
void
pipe()
{
	register struct inode *ip;
	register struct file *rf, *wf;
	int r;

	ip = ialloc(rootdev);
	if(ip == NULL)
		return;
	rf = falloc();
	if(rf == NULL) {
		iput(ip);
		return;
	}
	r = u.u_ar0[R0];
	wf = falloc();
	if(wf == NULL) {
		rf->f_count = 0;
		u.u_ofile[r] = NULL;
		iput(ip);
		return;
	}
	u.u_ar0[R1] = u.u_ar0[R0];
	u.u_ar0[R0] = r;
	wf->f_flag  = FWRITE|FPIPE;
	wf->f_inode = ip;
	rf->f_flag  = FREAD|FPIPE;
	rf->f_inode = ip;
	ip->i_count = 2;
	ip->i_flag  = IACC|IUPD;
	ip->i_mode  = IALLOC;
}

/*
 * Read call directed to a pipe.
 */
readp(fp)
	register struct file *fp;
{
	register struct inode *ip;

	ip = fp->f_inode;
loop:
	/* Very conservative locking */
	plock(ip);

	/* If the head (read) has caught up with
	 * the tail (write), reset both to 0.
	 */
	if(fp->f_offset == ip->i_size) {
		ip->i_size = 0;
		fp->f_offset = 0;
		if(ip->i_mode&IWRITE) {
			ip->i_mode &= ~IWRITE;
			wakeup(ip+1);
		}
		/* If there are not both reader and
		 * writer active, return without
		 * satisfying read.
		 */
		prele(ip);
		if(ip->i_count < 2)
			return;
		ip->i_mode |= IREAD;
		sleep(ip+2, PPIPE);
		goto loop;
	}

	/* Read and return */
	u.u_offset = fp->f_offset;
	readi(ip);
	fp->f_offset = u.u_offset;
	prele(ip);
}

/*
 * Write call directed to a pipe.
 */
writep(fp)
	register struct file *fp;
{
	register struct inode *ip;
	unsigned c;

	ip = fp->f_inode;
	c = u.u_count;

loop:
	/* If all done, return */
	plock(ip);
	if(c == 0) {
		prele(ip);
		u.u_count = 0;
		return;
	}

	/* If there are not both read and
	 * write sides of the pipe active,
	 * return error and signal too.
	 */
	if(ip->i_count < 2) {
		prele(ip);
		u.u_error = EPIPE;
		psignal(u.u_procp, SIGPIPE);
		return;
	}

	/* If the pipe is full,
	 * wait for reads to deplete
	 * and truncate it.
	 */
	if(ip->i_size == PIPSIZ) {
		ip->i_mode |= IWRITE;
		prele(ip);
		sleep(ip+1, PPIPE);
		goto loop;
	}

	/* Write what is possible and
	 * loop back.
	 */
	u.u_offset = ip->i_size;
	u.u_count = min(c, (int)(PIPSIZ - u.u_offset));
	c -= u.u_count;
	writei(ip);
	prele(ip);
	if(ip->i_mode&IREAD) {
		ip->i_mode &= ~IREAD;
		wakeup(ip+2);
	}
	goto loop;
}

/*
 * Lock a pipe.
 * If its already locked,
 * set the WANT bit and sleep.
 */
plock(ip)
	register struct inode *ip;
{
	while(ip->i_flag&ILOCK) {
		ip->i_flag |= IWANT;
		sleep(ip, PPIPE);
	}
	ip->i_flag |= ILOCK;
}

/*
 * Unlock a pipe.
 * If WANT bit is on, wakeup.
 * This routine is also used
 * to unlock inodes in general.
 */
prele(ip)
	register struct inode *ip;
{
	ip->i_flag &= ~ILOCK;
	if(ip->i_flag&IWANT) {
		ip->i_flag &= ~IWANT;
		wakeup(ip);
	}
}
