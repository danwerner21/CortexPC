/*
 * General TI-990 disk routines
 */
#include "param.h"
#include "buf.h"
#include "user.h"
#include "proc.h"
#include "conf.h"
#include "mmu.h"

#define MAXPARTITION 16
#define MAXDISK  4

/*
** Command word control bits
*/

/* Status word 0 - status */

#define OLBIT	0x8000		/* Off Line */
#define NRBIT	0x4000		/* Not Ready */
#define WPBIT	0x2000		/* Write Protect */
#define USBIT	0x1000		/* UnSafe */
#define ECBIT	0x0800		/* End of Cylinder */
#define SIBIT	0x0400		/* Seek Incomplete */
#define OSABIT	0x0200		/* Offset Active */
#define PCBIT	0x0100		/* Unsafe reason */

/* Command word 1 - command */

#define STBEBIT	0x2000		/* Strobe Early */
#define STBLBIT	0x1000		/* Strobe Late */
#define TIHBIT	0x0800		/* Transfer Inhibit */
#define OSBIT	0x0080		/* Head Offset */
#define OSFBIT	0x0040		/* Head Offset Forward */

/* Status word 7  - error */

#define IDLEBIT	0x8000		/* Idle */
#define COMPBIT	0x4000		/* Complete */
#define ERRBIT	0x2000		/* Error */
#define INTBIT	0x1000		/* Interrupt enable */
#define LOCKBIT	0x0800		/* Lockout */
#define RETBIT	0x0400		/* Retry */
#define ECCBIT	0x0200		/* ECC corrected */
#define ABNBIT	0x0100		/* Abnormal Completion */
#define MEBIT	0x0080		/* Memory Error */
#define DEBIT	0x0040		/* Data Error */
#define TTBIT	0x0020		/* Tiline Timeout */
#define IEBIT	0x0010		/* ID Error */
#define REBIT	0x0008		/* Rate Error */
#define CTBIT	0x0004		/* Command Timer */
#define SEBIT	0x0002		/* Search Error */
#define UEBIT	0x0001		/* Unit Error */

#define HARDERR	0x21FF		/* Hard error mask */

/* Disk commands */

#define STOREREG  0		/* Store registers */
#define WRITEFMT  1		/* Write format */
#define READDATA  2		/* Read data */
#define WRITEDATA 3		/* Write data */
#define READUFMT  4		/* Read unformated */
#define WRITEUFMT 5		/* Write unformated */
#define DISKSEEK  6		/* Disk Seek */
#define RESTORE   7		/* Restore */

struct device {
   unsigned int status;
   unsigned int command;
   unsigned int sector;
   unsigned int cylinder;
   unsigned int wdcount;
   char        *address;
   unsigned int drive;
   unsigned int error;
};

struct geometry {
   unsigned int  wrdstrk;
   unsigned char secstrk;
   unsigned char ohead;
   unsigned int  cylinders;
};

struct uformat {
   unsigned int  track;
   unsigned char secrec;
   unsigned char sector;
   unsigned int  wdsect;
};

struct devtab cftab;

int bkread();
int bkwrite();

int partition[MAXPARTITION+1];
int disks[MAXDISK];

extern int bkerror;
extern daddr_t rootmax;
extern unsigned int diskaddr;
extern unsigned int diskunit;

static struct device *DSKADDR;
static struct buf cfbuf;
static int units[MAXDISK] = { 0x800, 0x400, 0x200, 0x100 };

static int
diskio (unit, cmd, cyl, head, sect, buff, len)
	char *buff;
{
   DSKADDR->status = 0;
   DSKADDR->command = (cmd << 8) | head;
   DSKADDR->sector = 0x100 | sect;
   DSKADDR->cylinder = cyl;
   DSKADDR->wdcount = len;
   DSKADDR->address = (char *)buff;
   DSKADDR->drive = unit;
   DSKADDR->error = 0;

WAIT:
   while ((DSKADDR->error & IDLEBIT) == 0) ;
   if (cmd == DISKSEEK)
   {
      if ((DSKADDR->status & (unit >> 4)) == 0)
         goto WAIT;
   }

   return (DSKADDR->error & HARDERR);
}

/* For the disk strategy is very minimalistic: link the block
 * into the service queue, call cfstart which unlinks the block again
 * and performs a synchroneous read or write.
 */
 
void
cfstart()
{
	register struct buf *bp;
	int rc, slot;
	register unsigned int count;

	 /* Unlink immediately; there is always 0 or 1 request in the queue */
	if ((bp = cftab.d_actf) == 0)
		return;
	cftab.d_active++;
	count = -bp->b_wcount;

	/* Read or write the buffer to the disk. Normal I/O to a kernel disk
	 * buffer is easy, direct I/O to user space (swapping, "raw" disk
	 * access) requires juggling the page map.
	 */
	if (bp->b_flags & B_PHYS) {
		/* Perform I/O in blocks of up to one page (4KB). Map the user
		 * space block into kernel memory.
		 */
		count = min(256,count);
		slot = bp->b_slot;
	} else {
	        slot = 0;
	}
	dskslot (slot);
	if (bp->b_flags & B_READ)
		rc = bkread(bp->b_dev, bp->b_blkno, bp->b_addr, count);
	else
		rc = bkwrite(bp->b_dev, bp->b_blkno, bp->b_addr, count);
}

void
cfintr()
{
	register struct buf *bp;
	register int unit;

	if (cftab.d_active == 0)
		return;
	bp = cftab.d_actf;
	cftab.d_active = 0;
	if (bkerror) {
		if (++cftab.d_errcnt <= 10) {
			cfstart();
			return;
		}
		bp->b_flags |= B_ERROR;
	}
	cftab.d_errcnt = 0;
	if ((bp->b_wcount += 256) == 0) {
		cftab.d_actf = (struct buf *)bp->av_forw;
		bp->b_flags |= B_DONE;
		iodone(bp);
	} else {
		bp->b_addr += 512;
		bp->b_blkno += 1;
	}
	cfstart();
}

void
cfstrategy (bp)
	register struct buf *bp;
{
	daddr_t max;
	int s;

	if (bp->b_dev & 0x0F00) max = 65535;
	else max = rootmax + SWPBLKS;

	if (bp->b_blkno > max) {
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
	if (cftab.d_active == 0)
		cfstart();
	splx(s);
}

/* Physical I/O access to the disk */

void
cfread (dev)
{
	physio (cfstrategy, &cfbuf, dev, B_READ);
}

void
cfwrite (dev)
{
	physio (cfstrategy, &cfbuf, dev, B_WRITE);
}

/* Set up disk and partition tables */

int
cfpart ()
{
	long parlen;
	int i, cylinc;
	int cyls, heads;
	int cyl, sec, sectlen, head;
	struct geometry geo;
	struct uformat fmt;

	DSKADDR = (struct device *)diskaddr;
	for (i = 0; i < MAXDISK; i++) {
		if (diskio (units[i], STOREREG, 0, 0, 0, (char *)&geo, 6) != 0)
			disks[i] = 0;
		else
			disks[i] = 1;
	}
	if (diskio (diskunit, STOREREG, 0, 0, 0, (char *)&geo, 6) != 0)
		return (-1);
	if (diskio (diskunit, READUFMT, 0, 0, 0, (char *)&fmt, 6) != 0)
		return (-1);

	cyls = geo.cylinders & 0x7FF;
	heads = geo.cylinders >> 11;
	sectlen = fmt.wdsect << 1;

	parlen = 65535L; /* Maximum blocks per partition */
	if (sectlen < 512)
		parlen = parlen * 2L;
	sec = parlen % geo.secstrk;
	parlen = parlen / geo.secstrk;
	head = parlen % heads;
	cyl = parlen / heads;
	if (head || sec) cyl++;

	cylinc = cyl;
	partition[0] = 0;
	for (i = 1; i < MAXPARTITION; i++) {
		partition[i] = cyl;
		if (cyl >= cyls)
			partition[i] = 0xF000;
		cyl += cylinc;
	}
	return (0);
}
