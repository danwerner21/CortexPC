
#include "param.h"
#include "user.h"
#include "buf.h"
#include "conf.h"
#include "systm.h"
#include "proc.h"

/*
 * This is the set of buffers proper, whose heads
 * were declared in buf.h.  There can exist buffer
 * headers not pointing here that are used purely
 * as arguments to the I/O routines to describe
 * I/O to be done-- e.g. swbuf, just below, for
 * swapping.
 */
char	buffers[NBUF][514];
struct	buf	swbuf;

/*
 * Pick up the device's error number and pass it to the user;
 * if there is an error but the number is 0 set a generalized
 * code.  Actually the latter is always true because devices
 * don't yet return specific errors.
 */
void
geterror(bp)
	register struct buf *bp;
{
	if (bp->b_flags&B_ERROR)
		if ((u.u_error = bp->b_error)==0)
			u.u_error = EIO;
}

/*
 * The following several routines allocate and free
 * buffers with various side effects.  In general the
 * arguments to an allocate routine are a device and
 * a block number, and the value is a pointer to
 * to the buffer header; the buffer is marked "busy"
 * so that no on else can touch it.  If the block was
 * already in core, no I/O need be done; if it is
 * already busy, the process waits until it becomes free.
 * The following routines allocate a buffer:
 *	getblk
 *	bread
 * Eventually the buffer must be released, possibly with the
 * side effect of writing it out, by using one of
 *	bwrite
 *	bdwrite
 *	brelse
 */

/*
 * Read in (if necessary) the block and return a buffer pointer.
 */
struct buf *
bread(dev, blkno)
	int dev;
	unsigned int blkno;
{
	register struct buf *rbp;

	rbp = getblk(dev, blkno);
	if (rbp->b_flags&B_DONE)
		return(rbp);
	rbp->b_flags |= B_READ;
	rbp->b_wcount = -256;
	rbp->b_slot = 0;
	(*bdevsw[MAJOR(rbp->b_dev)].d_strategy)(rbp);
	iowait(rbp);
	return(rbp);
}

/*
 * Write the buffer, waiting for completion.
 * Then release the buffer.
 */
void
bwrite(bp)
	register struct buf *bp;
{
	register int flag;

	flag = bp->b_flags;
	bp->b_flags &= ~(B_READ | B_DONE | B_ERROR | B_DELWRI);
	bp->b_wcount = -256;
	bp->b_slot = 0;
	(*bdevsw[MAJOR(bp->b_dev)].d_strategy)(bp);
	if ((flag&B_ASYNC) == 0) {
		iowait(bp);
		brelse(bp);
	} else if ((flag&B_DELWRI)==0)
		geterror(bp);
}

/*
 * Release the buffer, marking it so that if it is grabbed
 * for another purpose it will be written out before being
 * given up (e.g. when writing a partial block where it is
 * assumed that another write for the same block will soon follow).
 */
void
bdwrite(bp)
	register struct buf *bp;
{
	bp->b_flags |= B_DELWRI | B_DONE;
	brelse(bp);
}

/*
 * release the buffer, with no I/O implied.
 */
void
brelse(bp)
	struct buf *bp;
{
	register struct buf *rbp, **backp;
	register int sps;

	rbp = bp;
	if (rbp->b_flags&B_WANTED)
		wakeup(rbp);
	if (bfreelist.b_flags&B_WANTED) {
		bfreelist.b_flags &= ~B_WANTED;
		wakeup(&bfreelist);
	}
	if (rbp->b_flags&B_ERROR)
		SETMINOR(rbp->b_dev, -1);  /* no assoc. on error */
	backp = &bfreelist.av_back;
	sps = spl7();
	rbp->b_flags &= ~(B_WANTED|B_BUSY|B_ASYNC);
	(*backp)->av_forw = rbp;
	rbp->av_back = *backp;
	*backp = rbp;
	rbp->av_forw = &bfreelist;
	splx(sps);
}

/*
 * Unlink a buffer from the available list and mark it busy.
 * (internal interface)
 */
void
notavail(bp)
	register struct buf *bp;
{
	register int sps;

	sps = spl7();
	bp->av_back->av_forw = bp->av_forw;
	bp->av_forw->av_back = bp->av_back;
	bp->b_flags |= B_BUSY;
	splx(sps);
}

/*
 * Assign a buffer for the given block.  If the appropriate
 * block is already associated, return it; otherwise search
 * for the oldest non-busy buffer and reassign it.
 * When a 512-byte area is wanted for some random reason
 * (e.g. during exec, for the user arglist) getblk can be called
 * with device NODEV to avoid unwanted associativity.
 */
struct buf *
getblk(dev, blkno)
{
	register struct buf *bp;
	register struct devtab *dp;

	if(dev >= 0 && MAJOR(dev) >= nblkdev)
		panic("blkdev");

loop:
	if (dev < 0)
		dp = (struct devtab*) &bfreelist;
	else {
		dp = bdevsw[MAJOR(dev)].d_tab;
		if(dp == NULL)
			panic("devtab");
		for (bp=dp->b_forw; bp != (struct buf *)dp; bp = bp->b_forw) {
			if (bp->b_blkno!=blkno || bp->b_dev!=dev)
				continue;
			spl7();
			if (bp->b_flags&B_BUSY) {
				bp->b_flags |= B_WANTED;
				sleep(bp, PRIBIO);
				spl0();
				goto loop;
			}
			spl0();
			notavail(bp);
			return(bp);
		}
	}
	spl7();
	if (bfreelist.av_forw == &bfreelist) {
		bfreelist.b_flags |= B_WANTED;
		sleep(&bfreelist, PRIBIO);
		spl0();
		goto loop;
	}
	spl0();
	notavail(bp = bfreelist.av_forw);
	if (bp->b_flags & B_DELWRI) {
		bp->b_flags |= B_ASYNC;
		bwrite(bp);
		goto loop;
	}
	bp->b_flags = B_BUSY | B_RELOC;
	bp->b_back->b_forw = bp->b_forw;
	bp->b_forw->b_back = bp->b_back;
	bp->b_forw = dp->b_forw;
	bp->b_back = (struct buf*) dp;
	dp->b_forw->b_back = bp;
	dp->b_forw = bp;
	bp->b_dev = dev;
	bp->b_blkno = blkno;
	return(bp);
}

/*
 * Wait for I/O completion on the buffer; return errors
 * to the user.
 */
void
iowait(bp)
	register struct buf *bp;
{
	spl7();
	while ((bp->b_flags&B_DONE)==0)
		sleep(bp, PRIBIO);
	spl0();
	geterror(bp);
}

/*
 * Mark I/O complete on a buffer, release it if I/O is asynchronous,
 * and wake up anyone waiting for it.
 */
void
iodone(bp)
	register struct buf *bp;
{
	bp->b_flags |= B_DONE;
	if (bp->b_flags&B_ASYNC && !(bp->b_flags & B_TAPE))
		brelse(bp);
	else {
		bp->b_flags &= ~B_WANTED;
		wakeup(bp);
	}
}

/*
 * Initialize the buffer I/O system by freeing
 * all buffers and setting all device buffer lists to empty.
 */
void
binit()
{
	register struct buf *bp;
	register struct devtab *dp;
	register int i;
	register struct bdevsw *bdp;

	bfreelist.b_forw = bfreelist.b_back =
	    bfreelist.av_forw = bfreelist.av_back = &bfreelist;
	for (i=0; i<NBUF; i++) {
		bp = &buf[i];
		bp->b_dev  = -1;
		bp->b_addr = buffers[i];
		bp->b_back = &bfreelist;
		bp->b_forw = bfreelist.b_forw;
		bfreelist.b_forw->b_back = bp;
		bfreelist.b_forw = bp;
		bp->b_flags = B_BUSY;
		brelse(bp);
	}
	i = 0;
	for (bdp = bdevsw; bdp->d_open; bdp++) {
		dp = bdp->d_tab;
		if(dp) {
			dp->b_forw = (struct buf*)dp;
			dp->b_back = (struct buf*)dp;
		}
		i++;
	}
	nblkdev = i;
}

/*
 * make sure all write-behind blocks
 * on dev (or NODEV for all)
 * are flushed out.
 * (from umount and update)
 */
void
bflush(dev)
{
	register struct buf *bp;

loop:
	spl7();
	for (bp = bfreelist.av_forw; bp != &bfreelist; bp = bp->av_forw) {
		if (bp->b_flags&B_DELWRI && (dev == NODEV||dev==bp->b_dev)) {
			bp->b_flags |= B_ASYNC;
			notavail(bp);
			bwrite(bp);
			goto loop;
		}
	}
	spl0();
}

/*
 * swap I/O
 */
int
swap(blkno, memslot, rdflg)
{
	register int *fp;

	fp = &swbuf.b_flags;
	spl7();
	while (*fp&B_BUSY) {
		*fp |= B_WANTED;
		sleep(fp, PSWP);
	}
	*fp = B_BUSY | B_PHYS | rdflg;
	swbuf.b_dev = swapdev;
	swbuf.b_wcount = -0x7800; /* words, i.e. 60KB */
	swbuf.b_blkno = blkno;
	swbuf.b_addr = 0x000;
	swbuf.b_slot = memslot;
	(*bdevsw[swapdev>>8].d_strategy)(&swbuf);
	spl7();
	while((*fp&B_DONE)==0)
		sleep(fp, PSWP);
	if (*fp&B_WANTED)
		wakeup(fp);
	spl0();
	*fp &= ~(B_BUSY|B_WANTED);
	return(*fp&B_ERROR);
}

/*
 * Raw I/O. The arguments are
 * - the strategy routine for the device
 * - a buffer, which will always be a special buffer header owned
 *   exclusively by the device for this purpose
 * - the device number
 * - a read/write flag
 * Essentially all the work is computing physical addresses and
 * validating them.
 */
physio(strat, bp, dev, rw)
	register struct buf *bp;
	int (*strat)();
{
	register unsigned int base, end;

	base = (unsigned int)(u.u_base);
	end = base + u.u_count;

	/*
	 * Check for bad buffer, count not a multiple of 512, address
	 * wraparound and offset not a multiple of 512.
	 */
	if (base&1 || base < 0x1000 || end >=0xf000 || base >= end)
		goto bad;
	if (!(bp->b_flags & B_TAPE) && (u.u_count&0x1ff || u.u_offset&0x1ff))
		goto bad;

	spl7();
	while (bp->b_flags&B_BUSY) {
		bp->b_flags |= B_WANTED;
		sleep(bp, PRIBIO);
	}
	bp->b_flags = B_BUSY | B_PHYS | rw;
	bp->b_dev   = dev;
	/*
	 * Compute physical address by simulating
	 * the segmentation hardware.
	 */
	bp->b_addr   = u.u_base;
	swbuf.b_slot = u.u_procp->p_slot;	
	bp->b_slot   = u.u_procp->p_slot;	
	bp->b_blkno  = u.u_offset>>9;
	bp->b_wcount = -((u.u_count>>1) & 077777);
	bp->b_error  = 0;
	u.u_procp->p_flag |= SLOCK;
	(*strat)(bp);
	spl7();
	while ((bp->b_flags&B_DONE) == 0)
		sleep(bp, PRIBIO);
	u.u_procp->p_flag &= ~SLOCK;
	if (bp->b_flags&B_WANTED)
		wakeup(bp);
	spl0();
	bp->b_flags &= ~(B_BUSY|B_WANTED);
	geterror(bp);
	return;

    bad:
	u.u_error = EFAULT;
}
