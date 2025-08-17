/*
 * General disk routines for Unix
 */
#include "param.h"
#include "buf.h"
#include "user.h"
#include "mmu.h"

struct devtab cftab;
struct buf cfbuf;

int bkread();
int bkwrite();

extern daddr_t rootmax;

/* For a CF Card the disk strategy is very minimalistic: link the block
 * into the service queue, call fdstart which unlinks the block again
 * and performs a synchroneous read or write.
 */
 
void cfstart()
{
	register struct buf *bp;
	int rc;
	char m1, m2, *a;
	register unsigned int n, count;

	 /* Unlink immediately; there is always 0 or 1 request in the queue */
	if ((bp = cftab.d_actf) == 0)
		return;
	cftab.d_actf = bp->av_forw;

	/* Read or write the buffer to the CF Card. Normal I/O to a kernel
	 * disk buffer is easy, direct I/O to user space (swapping, "raw" disk
	 * access) requires juggling the page map.
	 */
	if(bp->b_flags & B_PHYS) {
		count = -bp->b_wcount;
		rc = 0;
		/* Perform I/O in blocks of up to one page (4KB). Map the user
		 * space block into kernel memory in page 12 and possibly 13.
		 */
		while(count>0 && rc==0) {
			n = min(2048,count);
#ifdef TI_CTX
			m1 = mmu[12];
			m2 = mmu[13];
			mmu[12] = physpage(bp->b_addr, bp->b_slot);
			mmu[13] = physpage(bp->b_addr + (n<<1), bp->b_slot);
			a = REMAP(bp->b_addr, 12);
#else
			dskslot (bp->b_slot);
			a = bp->b_addr;
#endif
			if(bp->b_flags & B_READ)
			  rc = bkread(bp->b_dev, bp->b_blkno, a, n);
			else
			  rc = bkwrite(bp->b_dev, bp->b_blkno, a, n);
#ifdef TI_CTX
			mmu[12] = m1;
			mmu[13] = m2;
#endif
			bp->b_blkno += 8;
			bp->b_addr  += 4096;
			count -= n;
		}
	} else {
#ifndef TI_CTX
	        dskslot (0);
#endif
		if(bp->b_flags & B_READ)
		 rc = bkread(bp->b_dev, bp->b_blkno, bp->b_addr, -bp->b_wcount);
		else
		 rc = bkwrite(bp->b_dev, bp->b_blkno, bp->b_addr,-bp->b_wcount);
	}
	if(rc) 
		bp->b_flags |= B_ERROR;
	iodone(bp);
}

void cfstrategy( bp )
	register struct buf *bp;
{
	unsigned int max = rootmax + SWPBLKS;
	int s;

	if (bp->b_blkno >= max) {
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}

	bp->av_forw = 0;
	/* link buffer to end of service queue */
	s = spl7();
	if (cftab.d_actf == 0)
		cftab.d_actf = bp;
	else
		cftab.d_actl->av_forw = bp;

	cftab.d_actl = bp;
	/* fdtab.d_active is always 0 for now */
	if (cftab.d_active == 0)
		cfstart();
	splx(s);
}

/* Physical I/O access to the disk */

void
cfread(dev)
{

	physio(cfstrategy, &cfbuf, dev, B_READ);
}

void
cfwrite(dev)
{

	physio(cfstrategy, &cfbuf, dev, B_WRITE);
}

