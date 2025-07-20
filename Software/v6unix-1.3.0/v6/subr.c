
#include "param.h"
#include "conf.h"
#include "inode.h"
#include "user.h"
#include "buf.h"
#include "systm.h"

/*
 * Bmap defines the structure of file system storage
 * by returning the physical block number on a device given the
 * inode and the logical block number in a file.
 * When convenient, it also leaves the physical
 * block number of the next block of the file in rablock
 * for use in read-ahead.
 */
int
bmap(ip, bn)
	struct inode *ip;
	unsigned int bn;
{
	register struct buf *bp;
	register int *bap, nb;
	struct buf *nbp;
	int d, i;

	d = ip->i_dev;
	if(bn & 0174000) {
		u.u_error = EFBIG;
		return(0);
	}
	if((ip->i_mode&ILARG) == 0) {
		/*
		 * small file algorithm
		 */
		if((bn & ~7) != 0) {
			/*
			 * convert small to large
			 */
			if ((bp = alloc(d)) == NULL)
				return(NULL);
			bap = (int*) bp->b_addr;
			for(nb=0; nb<8; nb++) {
				*bap++ = ip->i_addr[nb];
				ip->i_addr[nb] = 0;
			}
			ip->i_addr[0] = bp->b_blkno;
			bdwrite(bp);
			ip->i_mode |= ILARG;
			goto large;
		}
		nb = ip->i_addr[bn];
		if(nb == 0 && (bp = alloc(d)) != NULL) {
			bdwrite(bp);
			nb = bp->b_blkno;
			ip->i_addr[bn] = nb;
			ip->i_flag |= IUPD;
		}
		rablock = 0;
		if (bn<7)
			rablock = ip->i_addr[bn+1];
		return(nb);
	}

	/*
	 * large file algorithm
	 */
large:
	i = bn>>8;
	if(bn & 0174000)
		i = 7;
	if((nb=ip->i_addr[i]) == 0) {
		ip->i_flag |= IUPD;
		if ((bp = alloc(d)) == NULL)
			return(NULL);
		ip->i_addr[i] = bp->b_blkno;
	} else
		bp = bread(d, nb);
	bap = (int*) bp->b_addr;

	/*
	 * "huge" fetch of double indirect block
	 */
	if(i == 7) {
		i = ((bn>>8) & 0377) - 7;
		if((nb=bap[i]) == 0) {
			if((nbp = alloc(d)) == NULL) {
				brelse(bp);
				return(NULL);
			}
			bap[i] = nbp->b_blkno;
			bdwrite(bp);
		} else {
			brelse(bp);
			nbp = bread(d, nb);
		}
		bp = nbp;
		bap = (int*) bp->b_addr;
	}

	/*
	 * normal indirect fetch
	 */
	i = bn & 0377;
	if((nb=bap[i]) == 0 && (nbp = alloc(d)) != NULL) {
		nb = nbp->b_blkno;
		bap[i] = nb;
		bdwrite(nbp);
		bdwrite(bp);
	} else
		brelse(bp);
	rablock = 0;
	if(i < 255)
		rablock = bap[i+1];
	return(nb);
}

/*
 * Routine which sets a user error; placed in
 * illegal entries in the bdevsw and cdevsw tables.
 */
nodev()
{

	u.u_error = ENODEV;
}

/*
 * Null routine; placed in insignificant entries
 * in the bdevsw and cdevsw tables.
 */
nulldev()
{
}

void *
memcpy (pto, pfrom, nbytes)
	void *pto, *pfrom;
	unsigned int nbytes;
{
	register char *to, *from;
	register unsigned bytes;

	to = (char*) pto;
	from = (char*) pfrom;
	bytes = nbytes;

	while (bytes-- > 0)
		*to++ = *from++;
	return pto;
}
