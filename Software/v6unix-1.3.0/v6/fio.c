
#include "param.h"
#include "user.h"
#include "filsys.h"
#include "file.h"
#include "conf.h"
#include "inode.h"
#include "reg.h"

/*
 * Convert a user supplied
 * file descriptor into a pointer
 * to a file structure.
 * Only task is to check range
 * of the descriptor.
 */
struct file *
getf(f)
	register int f;
{
	register struct file *fp;

	if(f<0 || f>=NOFILE)
		goto bad;
	fp = u.u_ofile[f];
	if(fp != NULL)
		return(fp);
bad:
	u.u_error = EBADF;
	return(NULL);
}

/*
 * Internal form of close.
 * Decrement reference count on
 * file structure and call closei
 * on last closef.
 * Also make sure the pipe protocol
 * does not constipate.
 */
void
closef(fp)
	register struct file *fp;
{
	register struct inode *ip;

	if(fp->f_flag&FPIPE) {
		ip = fp->f_inode;
		ip->i_mode &= ~(IREAD|IWRITE);
		wakeup(ip+1);
		wakeup(ip+2);
	}
	if(fp->f_count <= 1)
		closei(fp->f_inode, fp->f_flag&FWRITE);
	fp->f_count--;
}

/*
 * Decrement reference count on an
 * inode due to the removal of a
 * referencing file structure.
 * On the last closei, switchout
 * to the close entry point of special
 * device handler.
 * Note that the handler gets called
 * on every open and only on the last
 * close.
 */
void
closei(ip, rw)
	register struct inode *ip;
{
	register struct inode *rip;
	register int dev, maj;

	dev = ip->i_addr[0];
	maj = MAJOR(ip->i_addr[0]);
	if(ip->i_count <= 1)
		switch(ip->i_mode&IFMT) {
		case IFCHR:
			(*cdevsw[maj].d_close)(dev, rw);
			break;
		case IFBLK:
			(*bdevsw[maj].d_close)(dev, rw);
		}
	iput(ip);
}

/*
 * openi called to allow handler
 * of special files to initialize and
 * validate before actual IO.
 * Called on all sorts of opens
 * and also on mount.
 */
void
openi(ip, rw)
	register struct inode *ip;
{
	register int dev, maj;

	dev = ip->i_addr[0];
	maj = MAJOR(ip->i_addr[0]);
	switch(ip->i_mode&IFMT) {
	case IFCHR:
		if(maj >= nchrdev)
			goto bad;
		(*cdevsw[maj].d_open)(dev, rw);
		break;
	case IFBLK:
		if(maj >= nblkdev)
			goto bad;
		(*bdevsw[maj].d_open)(dev, rw);
	}
	return;

bad:
	u.u_error = ENXIO;
}

/*
 * Check mode permission on inode pointer.
 * Mode is READ, WRITE or EXEC.
 * In the case of WRITE, the
 * read-only status of the file
 * system is checked.
 * Also in WRITE, prototype text
 * segments cannot be written.
 * The mode is shifted to select
 * the owner/group/other fields.
 * The super user is granted all
 * permissions except for EXEC where
 * at least one of the EXEC bits must
 * be on.
 */
int
access(ip, mode)
	register struct inode *ip;
	register int mode;
{
	if(mode == IWRITE) {
		if(getfs(ip->i_dev)->s_ronly != 0) {
			u.u_error = EROFS;
			return(1);
		}
		if(ip->i_flag & ITEXT) {
			u.u_error = ETXTBSY;
			return(1);
		}
	}
	if(u.u_uid == 0) {
		if(mode == IEXEC && (ip->i_mode & 
			(IEXEC | (IEXEC>>3) | (IEXEC>>6))) == 0)
				goto bad;
		return(0);
	}
	if(u.u_uid != ip->i_uid) {
		mode >>= 3;
		if(u.u_gid != ip->i_gid)
			mode >>= 3;
	}
	if((ip->i_mode & mode) != 0)
		return(0);

bad:
	u.u_error = EACCES;
	return(1);
}

/*
 * Look up a pathname and test if
 * the resultant inode is owned by the
 * current user.
 * If not, try for super-user.
 * If permission is granted,
 * return inode pointer.
 */
struct inode*
owner()
{
	register struct inode *ip;
	extern int uchar();

	if ((ip = namei(uchar, 0)) == NULL)
		return(NULL);
	if(u.u_uid == ip->i_uid)
		return(ip);
	if (suser())
		return(ip);
	iput(ip);
	return(NULL);
}

/*
 * Test if the current user is the
 * super user.
 */
int
suser()
{

	if(u.u_uid == 0)
		return(1);
	u.u_error = EPERM;
	return(0);
}

/*
 * Allocate a user file descriptor.
 */
int
ufalloc()
{
	register int i;

	for (i=0; i<NOFILE; i++)
		if (u.u_ofile[i] == NULL) {
			u.u_ar0[R0] = i;
			return(i);
		}
	u.u_error = EMFILE;
	return(-1);
}

/*
 * Allocate a user file descriptor
 * and a file structure.
 * Initialize the descriptor
 * to point at the file structure.
 *
 * no file -- if there are no available
 * 	file structures.
 */
struct file *
falloc()
{
	register struct file *fp;
	register int i;

	i = ufalloc();
	if (i < 0)
		return(NULL);
	for (fp = &file[0]; fp < &file[NFILE]; fp++)
		if (fp->f_count==0) {
			u.u_ofile[i] = fp;
			fp->f_count++;
			fp->f_offset = 0;
			fp->f_inode = NULL;
			return(fp);
		}
	printk("no file\n");
	u.u_error = ENFILE;
	return(NULL);
}
