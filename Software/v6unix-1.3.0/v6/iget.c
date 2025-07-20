
#include "param.h"
#include "systm.h"
#include "user.h"
#include "inode.h"
#include "filsys.h"
#include "buf.h"


/*
 * Look up an inode by device,inumber.
 * If it is in core (in the inode structure),
 * honor the locking protocol.
 * If it is not in core, read it in from the
 * specified device.
 * If the inode is mounted on, perform
 * the indicated indirection.
 * In all cases, a pointer to a locked
 * inode structure is returned.
 *
 * printk warning: no inodes -- if the inode
 *	structure is full
 * panic: no imt -- if the mounted file
 *	system is not in the mount table.
 *	"cannot happen"
 */
struct inode *
iget(dev, ino)
	int dev;
	int ino;
{
	register struct inode *p;
	struct inode *freeip;
	struct buf   *bp;
	register struct mount *mp;
	register struct dinode *dp;
	register int i;

loop:
	freeip = NULL;
	for(p = &inode[0]; p < &inode[NINODE]; p++) {
		if(dev==p->i_dev && ino==p->i_number) {
			if((p->i_flag&ILOCK) != 0) {
				p->i_flag |= IWANT;
				sleep(p, PINOD);
				goto loop;
			}
			if((p->i_flag&IMOUNT) != 0) {
				for(mp = &mount[0]; mp < &mount[NMOUNT]; mp++)
					if(mp->m_inodp == p) {
						dev = mp->m_dev;
						ino = ROOTINO;
						goto loop;
					}
				panic("mount table failure");
			}
			p->i_count++;
			p->i_flag |= ILOCK;
			return(p);
		}
		if(freeip==NULL && p->i_count==0)
			freeip = p;
	}
	if((p = freeip) == NULL) {
		printk("Inode table overflow\n");
		u.u_error = ENFILE;
		return(NULL);
	}
	p->i_dev    = dev;
	p->i_number = ino;
	p->i_flag   = ILOCK;
	p->i_count++;
	p->i_lastr  = -1;

	bp = bread(dev, (ino+31)>>4);
	if (bp->b_flags&B_ERROR) {
		brelse(bp);
		iput(p);
		return(NULL);
	}

	/* Copy disk format to kernel format */
	dp = (struct dinode*)(bp->b_addr + 32 * ((ino + 31) & 017));
	p->i_mode  = dp->d_mode;
	p->i_nlink = dp->d_nlink;
	p->i_uid   = dp->d_uid;
	p->i_gid   = dp->d_gid;
	p->i_size  = ((unsigned long)dp->d_size0<<16) + dp->d_size1;
	for(i=0; i<8; i++)
		p->i_addr[i] = dp->d_addr[i];

	brelse(bp);
	return(p);
}

/*
 * Decrement reference count of
 * an inode structure.
 * On the last reference,
 * write the inode out and if necessary,
 * truncate and deallocate the file.
 */
void
iput(rp)
	register struct inode *rp;
{
	if(rp->i_count == 1) {
		rp->i_flag |= ILOCK;
		if(rp->i_nlink <= 0) {
			itrunc(rp);
			rp->i_mode = 0;
			ifree(rp->i_dev, rp->i_number);
		}
		iupdat(rp, &time);
		prele(rp);
		rp->i_flag = 0;
		rp->i_number = 0;
	}
	rp->i_count--;
	prele(rp);
}

/*
 * Check accessed and update flags on
 * an inode structure.
 * If either is on, update the inode
 * with the corresponding dates
 * set to the argument tm.
 */
void
iupdat(p, tm)
	struct inode *p;
	unsigned long *tm;
{
	register struct buf *bp;
	register struct dinode *dp;
	int i;

	if((p->i_flag&(IUPD|IACC)) != 0) {
		if(getfs(p->i_dev)->s_ronly)
			return;
		i = p->i_number+31;
		bp = bread(p->i_dev, i>>4);
		dp = (struct dinode*) (bp->b_addr + 32 * (i & 017));
		/* Copy kernel format to disk format */
		dp->d_mode   = p->i_mode;
		dp->d_nlink  = p->i_nlink;
		dp->d_uid    = p->i_uid;
		dp->d_gid    = p->i_gid;
		dp->d_size0  = p->i_size>>16;
		dp->d_size1  = p->i_size;
		for(i=0; i<8; i++) dp->d_addr[i] = p->i_addr[i];
		if(p->i_flag&IACC)
			dp->d_atime = time;
		if(p->i_flag&IUPD)
			dp->d_mtime = *tm;
		bwrite(bp);
	}
}

/*
 * Free all the disk blocks associated
 * with the specified inode structure.
 * The blocks of the file are removed
 * in reverse order. This FILO
 * algorithm will tend to maintain
 * a contiguous free list much longer
 * than FIFO.
 */
void
itrunc(p)
	register struct inode *p;
{
	struct buf *bp, *dp;
	register int *cp, *ep, *ap;

	if((p->i_mode&(IFCHR&IFBLK)) != 0)
		return;

	for(ap = &p->i_addr[7]; ap >= &p->i_addr[0]; ap--) {
		if(*ap == 0)
			continue;
		/* if each address is an indirect block ... */
		if((p->i_mode&ILARG) != 0) {
			bp = bread(p->i_dev, *ap);
			/* ... free its 512 pointers */
			cp = (int*) (bp->b_addr + 510);
			for(; cp >= (int*) bp->b_addr; cp--)
				if(*cp) {
					/* if this is the double indirect block ... */
					if(ap == &p->i_addr[7]) {
						dp = bread(p->i_dev, *cp);
						/* free its 512 indirect blocks */
						ep = (int*) (bp->b_addr + 510);
						for(; ep >= (int*) dp->b_addr; ep--)
						if(*ep)
							free(p->i_dev, *ep);
						brelse(dp);
					}
				free(p->i_dev, *cp);
				}
			brelse(bp);
		}
		free(p->i_dev, *ap);
		*ap = 0;
	}
	p->i_mode &= ~ILARG;
	p->i_size = 0;
	p->i_flag |= IUPD;
}

/*
 * Make a new file.
 */
struct inode *
maknode(mode)
	int mode;
{
	register struct inode *ip;

	ip = ialloc(u.u_pdir->i_dev);
	if (ip==NULL)
		return(NULL);
	ip->i_flag |= IACC|IUPD;
	ip->i_mode = mode | IALLOC;
	ip->i_nlink = 1;
	ip->i_uid = u.u_uid;
	ip->i_gid = u.u_gid;
	wdir(ip);
	return(ip);
}

/*
 * Write a directory entry with
 * parameters left as side effects
 * to a call to namei.
 */
void
wdir(ip)
	struct inode *ip;
{
	u.u_dent.u_ino = ip->i_number;
	memcpy(&u.u_dent.u_name[0], &u.u_dbuf[0], DIRSIZ);
	u.u_count = DIRSIZ + 2;
	u.u_base = (char*) &u.u_dent;
	u.u_segflg++;
	writei(u.u_pdir);
	u.u_segflg--;
	iput(u.u_pdir);
}
