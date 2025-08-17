/*
 * TI-990 Mag tape driver
 */

#include "param.h"
#include "buf.h"
#include "proc.h"
#include "conf.h"
#include "file.h"
#include "user.h"

#define MTADDR ((struct device *)0xF880)

#define ERRCRU   0x1FC0
#define TLTIMOUT 15

#define MAXTAPE  4

/*
** Command word control bits
*/

/* Status word 0 - mtstatus */

#define OLBIT	0x8000		/* Off Line */
#define BOTBIT	0x4000		/* Beginning Of Tape */
#define EORBIT	0x2000		/* End Of Record */
#define EOFBIT	0x1000		/* End Of File */
#define EOTBIT	0x0800		/* End Of Tape */
#define WPBIT	0x0400		/* Write Protect */
#define RWBIT	0x0200		/* Tape Rewinding */
#define TOBIT	0x0100		/* Command Time Out */

/* Status word 7  - mterror */

#define IDLEBIT	0x8000		/* Idle */
#define COMPBIT	0x4000		/* Complete */
#define ERRBIT	0x2000		/* Error */
#define INTBIT	0x1000		/* Interrupt Enable */
#define sp0BIT	0x0800		/* spare */
#define sp1BIT	0x0400		/* spare */
#define PEFBIT	0x0200		/* PE Format */
#define ABNBIT	0x0100		/* Abnormal Completion */
#define IPEBIT	0x0080		/* Interface Parity error */
#define ECEBIT	0x0040		/* Error Correction enabled */
#define HWEBIT	0x0020		/* Hard Error */
#define TMRBIT	0x0010		/* Tiline memory read error */
#define TTEBIT	0x0008		/* Tiline Timing error */
#define TTOBIT	0x0004		/* Tiline Timout error */
#define FMEBIT	0x0002		/* Format error */
#define TPEBIT	0x0001		/* Tape error check status word 0 */

#define HARDERR	0x00BE		/* IPE|HWE|TMR|TTE|TTO|FMT */

#define WEOF	2		/* Write EOF */
#define BSR	3		/* Backspace record */
#define TREAD	4		/* Tape read */
#define FSR	5		/* Forward skip record */
#define TWRITE	6		/* Tape write */
#define READST	8		/* Read status */
#define REW	10		/* Rewind */

#define	SSEEK	1
#define	SIO	2
#define	SSTAT	3
#define	SCOM	4

#define T_WRITTEN 1

struct device {
   unsigned int mtstatus;
   unsigned int mtwd1;
   unsigned int mtwd2;
   unsigned int mtwd3;
   unsigned int mtcount;
   char        *mtaddress;
   unsigned int mtcommand;
   unsigned int mterror;
};

static struct buf mtbuf;
static struct buf cmtbuf;
static struct buf rmtbuf;

static struct {
   char    t_flags;
   char    t_openf;
   daddr_t t_blkno;
   daddr_t t_nxrec;
} mtcontrol[MAXTAPE];

static int tunit[MAXTAPE] = { 8, 4, 2, 1 };

mtopen (dev, flag)
{
	register unit, ds;

	unit = MINOR(dev) & 0177;
	if (unit >= MAXTAPE || mtcontrol[unit].t_openf) {
		u.u_error = EIO;
		return;
	}
	mtcontrol[unit].t_blkno = 0;
	mtcontrol[unit].t_nxrec = 65535;
	mtcontrol[unit].t_flags = 0;

	mtbuf.b_flags |= B_TAPE;
	ds = mtcommand (dev, READST);
	if (ds & OLBIT) {
		u.u_error = EIO;
	}
	if (flag && (ds & WPBIT)) {
		u.u_error = EROFS;
	}
	if (u.u_error == 0)
		mtcontrol[unit].t_openf++;
}

mtclose (dev, flag)
{
	if (flag && (mtcontrol[MINOR(dev)&07].t_flags & T_WRITTEN) &&
	    (MTADDR->mtstatus & WPBIT == 0)) {
		mtcommand (dev, WEOF);
		mtcommand (dev, WEOF);
		mtcommand (dev, BSR);
	}
	if ((MINOR(dev) & 0200) == 0)
		mtcommand (dev, REW);
	mtcontrol[MINOR(dev) & 07].t_openf = 0;
}

mtstart ()
{
	register struct buf *bp;
	register unsigned int com;
	int unit;
	int x;
	register daddr_t *blkno;

    loop:
	if ((bp = mtbuf.b_actf) == NULL)
		return;

	unit = MINOR(bp->b_dev) & 07;
	if (mtcontrol[unit].t_openf < 0) {
		bp->b_flags |= B_ERROR;
		goto next;
	}

	blkno = &mtcontrol[unit].t_blkno;
	com = tunit[unit] << 12;

	/* If we get a TILINE timout, then no device present */
	x = splx(0);
	MTADDR->mtwd1 = 0;
	splx(x);
	if (tb (ERRCRU, TLTIMOUT) == 1) {
		sbz (ERRCRU, TLTIMOUT);
		bp->b_flags |= B_ERROR;
		bp->b_resid = OLBIT;
		goto next;
	}
	MTADDR->mtwd2 = 0;
	MTADDR->mtwd3 = 0;

	if (bp == &cmtbuf) {
		MTADDR->mtstatus = 0;
		MTADDR->mtcount = 1;
		MTADDR->mtaddress = 0;
		MTADDR->mtcommand =  com | (bp->b_resid << 8);
		if (bp->b_resid == READST)
			mtbuf.b_active = SSTAT;
		else
			mtbuf.b_active = SCOM;
		MTADDR->mterror = INTBIT;
		return;
	}

	MTADDR->mtstatus = 0;
	if (*blkno != bp->b_blkno) {
		mtbuf.b_active = SSEEK;
		if (*blkno < bp->b_blkno) {
			com |= FSR << 8;
			MTADDR->mtcount = *blkno - bp->b_blkno;
		} else {
			if (bp->b_blkno == 0) {
				com |= REW << 8;
				MTADDR->mtcount = 0;
			} else {
				com |= BSR << 8;
				MTADDR->mtcount = bp->b_blkno - *blkno;
			}
		}
		MTADDR->mtaddress = 0;
		MTADDR->mtcommand = com;
		MTADDR->mterror = INTBIT;
		return;
	}

	com |= bp->b_slot | ((bp->b_flags & B_READ)? TREAD: TWRITE) << 8;
	mtbuf.b_active = SIO;
	MTADDR->mtcount = -bp->b_wcount << 1;
	MTADDR->mtaddress = bp->b_addr;
	MTADDR->mtcommand = com;
	MTADDR->mterror = INTBIT;
	return;

next:
	mtbuf.b_actf = bp->av_forw;
	iodone (bp);
	goto loop;
}

mtstrategy (bp)
register struct buf *bp;
{
	register daddr_t *p;
	register int s;

	if (bp != &cmtbuf) {
		p = &mtcontrol[MINOR(bp->b_dev) & 07].t_nxrec;
		if (*p <= bp->b_blkno) {
			if (*p < bp->b_blkno) {
				bp->b_flags |= B_ERROR;
				iodone (bp);
				return;
			}
			if (bp->b_flags & B_READ) {
				bp->b_resid = 0;
				iodone (bp);
				return;
			}
		}
		if ((bp->b_flags & B_READ) == 0) {
			mtcontrol[MINOR(bp->b_dev) & 07].t_flags |= T_WRITTEN;
			*p = bp->b_blkno + 1;
		}
	}

	bp->av_forw = 0;
	s = spl7();
	if (mtbuf.b_actf == NULL)
		mtbuf.b_actf = bp;
	else
		mtbuf.b_actl->av_forw = bp;
	mtbuf.b_actl = bp;
	if (mtbuf.b_active == NULL)
		mtstart ();
	splx(s);
}

mtcommand (dev, com)
{
	register struct buf *bp;
	register int s;

	bp = &cmtbuf;
	cmtbuf.b_flags |= B_TAPE;
	s = spl7();
	while (bp->b_flags & B_BUSY) {
		bp->b_flags |= B_WANTED;
		sleep (bp, PRIBIO);
	}
	bp->b_flags = B_BUSY | B_READ;
	splx(s);
	bp->b_dev = dev;
	bp->b_resid = com;
	bp->b_blkno = 0;
	mtstrategy (bp);
	iowait (bp);
	if (bp->b_flags & B_WANTED)
		wakeup (bp);
	bp->b_flags = 0;
	return (bp->b_resid);
}

mtintr ()
{
	register struct buf *bp;
	register int unit;
	int state;
	unsigned int lerror;

	lerror = MTADDR->mterror;
	MTADDR->mterror = 0x8000;

	if ((bp = mtbuf.b_actf) == NULL)
		return;

	unit = MINOR(bp->b_dev) & 07;
	state = mtbuf.b_active;
	mtbuf.b_active = 0;
	if (lerror & ERRBIT) {		/* error bit */
		if (state == SSTAT)
			goto out;
		if ((MTADDR->mtstatus & EORBIT) && ((lerror & HARDERR) == 0))
			goto out;
		if ((MTADDR->mtstatus & (EOFBIT | EOTBIT)) != 0) {
			mtcontrol[unit].t_nxrec = bp->b_blkno;
			state = SCOM;
			MTADDR->mtcount = -bp->b_wcount << 1;
			goto out;
		}
		if ((lerror & HARDERR) == 0 && state == SIO) {
			if (++mtbuf.b_errcnt < 2) {
				mtcontrol[unit].t_blkno++;
				mtbuf.b_active = 0;
				mtstart ();
				return;
			}
		} else if (mtcontrol[unit].t_openf > 0 && bp != &rmtbuf &&
				(MTADDR->mtstatus & EOFBIT)==0 ) {
			mtcontrol[unit].t_openf = -1;
			/*deverror (bp, MTADDR->mtstatus, lerror);*/
		}
		bp->b_flags |= B_ERROR;
		state = SIO;
	}
out:
	switch (state) {
	case SSTAT:
		bp->b_resid = MTADDR->mtstatus;
		goto cmnret;
	case SIO:
		mtcontrol[unit].t_blkno += (bp->b_wcount >> BSHIFT);
		rmtbuf.b_resid = MTADDR->mtcount;
	case SCOM:
		bp->b_resid = -MTADDR->mtcount;
	cmnret:
		mtbuf.b_errcnt = 0;
		mtbuf.b_actf = bp->av_forw;
		iodone (bp);
		break;
	case SSEEK:
		mtcontrol[unit].t_blkno = bp->b_blkno;
		break;
	default:
		return;
	}
	mtstart ();
}

mtphys (dev)
{
	register unit;
	daddr_t a;

	unit = MINOR(dev) & 07;
	if (unit < MAXTAPE) {
		a = u.u_offset >> 9;
		mtcontrol[unit].t_blkno = a;
		mtcontrol[unit].t_nxrec = a+1;
	}
}

mtread (dev)
{
	rmtbuf.b_flags |= B_TAPE;
	mtphys (dev);
	physio (mtstrategy, &rmtbuf, dev, B_READ);
	u.u_count = -rmtbuf.b_resid;
}

mtwrite (dev)
{
	rmtbuf.b_flags |= B_TAPE;
	mtphys (dev);
	physio (mtstrategy, &rmtbuf, dev, B_WRITE);
	u.u_count = -rmtbuf.b_resid;
}
