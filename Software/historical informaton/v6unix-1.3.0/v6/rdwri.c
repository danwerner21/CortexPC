
#include "param.h"
#include "inode.h"
#include "user.h"
#include "buf.h"
#include "conf.h"
#include "systm.h"

/*
 * Return the unsigned minimum
 * of the 2 arguments.
 */
int
min(a, b)
	unsigned a, b;
{
	if(a < b)
		return(a);
	return(b);
}

/*
 * Move 'an' bytes at byte location
 * &bp->b_addr[o] to/from (flag) the
 * user/kernel area starting at u.base.
 * Update all the arguments by the number
 * of bytes moved.
 */
void
iomove(kdata, n, flag)
	char *kdata;
	register int n, flag;
{
	if (u.u_segflg) {
		/* copy within kernel space */
		if (flag==B_WRITE)
			memcpy(kdata, u.u_base, n);
		else
			memcpy(u.u_base, kdata, n);
	} else {
		/* copy a block from/to user space */
		if (flag==B_WRITE)
			copyin(kdata, u.u_base, n);
		else
			copyout(u.u_base, kdata, n);		
	}
	u.u_base   += n;
	u.u_offset += n;
	u.u_count  -= n;
}

/*
 * Read the file corresponding to
 * the inode pointed at by the argument.
 * The actual read arguments are found
 * in the variables:
 *	u_base		core address for destination
 *	u_offset	byte offset in file
 *	u_count		number of bytes to read
 *	u_segflg	read to kernel/user
 */
void
readi(aip)
	struct inode *aip;
{
	struct buf *bp;
	unsigned int lbn, bn, on;
	register int dn, n;
	register struct inode *ip;

	ip = aip;
	if(u.u_count == 0)
		return;
	ip->i_flag |= IACC;
	if((ip->i_mode&IFMT) == IFCHR) {
		int major = MAJOR(ip->i_addr[0]);
		(*cdevsw[major].d_read)(ip->i_addr[0]);
		return;
	}

	do {
		lbn = bn = u.u_offset >> 9;
		on = (int) u.u_offset & 0777;
		n = min(512-on, u.u_count);
		if ((ip->i_mode & IFMT) != IFBLK) {
			if (ip->i_size <= u.u_offset)
				return;
			if (ip->i_size < u.u_offset + n)
				n = ip->i_size - u.u_offset;
			bn = bmap(ip, lbn);
			if (bn == 0)
				return;
			dn = ip->i_dev;
		} else {
			dn = ip->i_addr[0];
		}
		bp = bread(dn, bn);
		iomove(bp->b_addr + on, n, B_READ);
		brelse(bp);
	} while(u.u_error==0 && u.u_count!=0);
}

/*
 * Write the file corresponding to
 * the inode pointed at by the argument.
 * The actual write arguments are found
 * in the variables:
 *	u_base		core address for source
 *	u_offset	byte offset in file
 *	u_count		number of bytes to write
 *	u_segflg	write to kernel/user
 */
void
writei(aip)
	struct inode *aip;
{
	struct buf *bp;
	unsigned int n, on, bn;
	register int dn;
	register struct inode *ip;

	ip = aip;
	ip->i_flag |= IACC|IUPD;
	if((ip->i_mode&IFMT) == IFCHR) {
		(*cdevsw[MAJOR(ip->i_addr[0])].d_write)(ip->i_addr[0]);
		return;
	}
	if (u.u_count == 0)
		return;

	do {
		bn = u.u_offset >> 9;
		on = (int) u.u_offset & 0777;
		n = min(512-on, u.u_count);
		if((ip->i_mode&IFMT) != IFBLK) {
			if ((bn = bmap(ip, bn)) == 0)
				return;
			dn = ip->i_dev;
		} else
			dn = ip->i_addr[0];
		if(n == 512)
			bp = getblk(dn, bn); else
			bp = bread(dn, bn);
		iomove(bp->b_addr + on, n, B_WRITE);
		if(u.u_error != 0)
			brelse(bp);
		else {
			if (((int) u.u_offset & 0777) == 0)
				bwrite(bp);
			else
				bdwrite(bp);
		}
		if (ip->i_size < u.u_offset &&
		    (ip->i_mode & (IFBLK & IFCHR)) == 0)
			ip->i_size = u.u_offset;
		ip->i_flag |= IUPD;
	} while(u.u_error==0 && u.u_count!=0);
}
