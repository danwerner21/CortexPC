/*
 * General TTY subroutines.
 *
 */
#include "param.h"
#include "user.h"
#include "conf.h"
#include "tty.h"
#include "reg.h"
#include "systm.h"
#include "inode.h"
#include "file.h"

/* SYTEM CALLS GTTY AND STTY */

extern int ttyaddrs[1];

/*
 * Stuff common to stty and gtty.
 * Check legality and switch out to individual
 * device routine.
 * v  is 0 for stty; the parameters are taken from u.u_arg[].
 * c  is non-zero for gtty and is the place in which the device
 * routines place their information.
 */
void
sgtty(v)
	int *v;
{
	register struct file *fp;
	register struct inode *ip;
	int dev;

	if ((fp = getf(u.u_arg[0])) == NULL)
		return;
	ip = fp->f_inode;
	if ((ip->i_mode&IFMT) != IFCHR) {
		u.u_error = ENOTTY;
		return;
	}
	dev = ip->i_addr[0];
	(*cdevsw[MAJOR(dev)].d_sgtty)(dev, v);
}

/*
 * The routine implementing the gtty system call.
 * Just call lower level routine and pass back values.
 */
void
gtty()
{
	register int *up, *vp;
	int v[3];

	vp = &v[0];
	sgtty(vp);
	if (u.u_error)
		return;
	up = (int *)u.u_arg[1];
	suword(up,   *vp++);
	suword(++up, *vp++);
	suword(++up, *vp);
}

/*
 * The routine implementing the stty system call.
 * Read in values and call lower level.
 */
void
stty()
{
	register int *up;

	up = (int *)u.u_arg[1];
	u.u_arg[1] = fuword(up);
	u.u_arg[2] = fuword(++up);
	u.u_arg[3] = fuword(++up);
	sgtty(0);
}

/*
 * flush all TTY queues
 */
void
flushtty(tp)
	register struct tty *tp;
{
	register int sps;

	while (getc(&tp->t_canq) >= 0);
	while (getc(&tp->t_outq) >= 0);
	wakeup(&tp->t_rawq);
	wakeup(&tp->t_outq);
	sps = spl7();
	while (getc(&tp->t_rawq) >= 0);
	tp->t_delct = 0;
	splx(sps);
}

/*
 * Wait for output to drain, then flush input waiting.
 */
void
wflushtty(tp)
	register struct tty *tp;
{
	register int sps;

	sps = spl7();
	while (tp->t_outq.c_cc) {
		tp->t_state |= ASLEEP;
		sleep(&tp->t_outq, TTOPRI);
	}
	flushtty(tp);
	splx(sps);
}

/*
 * Common code for gtty and stty functions on typewriters.
 * If v is non-zero then gtty is being done and information is
 * passed back therein;
 * if it is zero stty is being done and the input information is in the
 * u_arg array.
 */
int
ttystty(tp, v)
	register struct tty *tp;
	register int *v;
{
	register int flags;

	if(v) {
		*v++ = tp->t_speeds;
		*v++ = (tp->t_erase << 8) | tp->t_kill;
		flags = tp->t_flags;
		if (tp->t_rawq.c_cc || tp->t_canq.c_cc)
			flags |= DAVAIL;
		*v = flags;
		return(1);
	}
	wflushtty(tp);
	v = &u.u_arg[1];
	tp->t_speeds = *v++;
	tp->t_kill   = *v & 0377;
	tp->t_erase  = *v++ >> 8;
	tp->t_flags  = *v;
	return(0);
}

/* DRIVER SUPPORT ROUTINES */

/*
 * transfer raw input list to canonical list,
 * doing erase-kill processing and handling escapes.
 * It waits until a full line has been typed in cooked mode,
 * or until any character has been typed in raw mode.
 */
int
canon(tp)
	register struct tty *tp;
{
	register char *bp;
	char *bp1;
	register int c;
	register int raw;
	int sps;

	raw = tp->t_flags & (RAW|CBREAK);

	sps = spl7();
	while ((!raw && tp->t_delct == 0) ||
	       (raw && tp->t_rawq.c_cc==0)) {
		if ((tp->t_state&CARR_ON)==0)
			return(0);
		sleep(&tp->t_rawq, TTIPRI);
	}
	splx(sps);
loop:
	bp = &canonb[2];
	while ((c = getc(&tp->t_rawq)) >= 0) {
		if (!raw) {
			if (c == 0377) {
				tp->t_delct--;
				break;
			}
			if (bp[-1] != '\\') {
				/* Turn Backspace to ^H . */
				if (c == CBACKSP)
					c = tp->t_erase;

				if (c == tp->t_erase) {
					if (bp > &canonb[2])
						bp--;
					continue;
				}
				if (c == tp->t_kill)
					goto loop;
				if (c == CEOT)
					continue;
			}
		}
		*bp++ = c;
		if (bp >= canonb+CANBSIZ)
			break;
	}
	bp1 = bp;
	bp = &canonb[2];
	while (bp < bp1) {
		putc(*bp++, &tp->t_canq);
	}
	return(1);
}

/*
 * put character on TTY output queue, adding delays,
 * expanding tabs, and handling the CR/NL bit.
 * It is called both from the top half for output, and from
 * interrupt level for echoing.
 * The arguments are the character and the tty structure.
 */
void
ttyoutput(c, tp)
	register int c;
	register struct tty *tp;
{
	register char *colp;
	register int flags;
	register int raw;

	flags = tp->t_flags;
	raw = tp->t_flags & (RAW|CBREAK);

	if (!c && !raw) return;

	/* Ignore EOT in normal mode to avoid hanging up
	 * certain terminals
	 */
	if (c == 004 && !raw)
		return;
	/* Turn tabs to spaces as required. */
	if (c == '\t' && (flags&XTABS)) {
		do {
			ttyoutput(' ');
		} while (tp->t_col & 07);
		return;
	}

	/* Turn <nl> to <cr><lf> if desired. */
	if (flags & CRMOD)
		if (c=='\n')
			ttyoutput('\r');

	/* Turn Backspace to ^H . */
	if (c == CBACKSP && !raw)
		c = CERASE;

	if (putc(c, &tp->t_outq))
		return;

	colp = &tp->t_col;
	if (!raw) switch (c) {

	/* ordinary */
	default:
		(*colp)++;
		break;

	/* backspace */
	case CERASE:
		if (*colp)
			(*colp)--;
		break;

	/* newline */
	case '\n':
		*colp = 0;
		break;

	/* carriage return */
	case '\r':
		*colp = 0;
	}
}

/*
 * Place a character on raw TTY input queue, putting in delimiters
 * and waking up top half as needed.
 * Also echo if required.
 * The arguments are the character and the appropriate
 * tty structure.
 */
void
ttyinput(ac, tp)
	register struct tty *tp;
{
	register int c;
	register int flags;
	register int raw;

	flags = tp->t_flags;

	raw = flags & (RAW|CBREAK);

	c = ac & 0377;
	if (!raw) {
		if (c == 0x7F) c = tp->t_erase;
		if ((flags & CRMOD) && c == '\r')
			c = '\n';
		if (c==CQUIT) {
			signal(tp, SIGQIT);
			goto flush;
		}
 		if ((c==CINTR || c==002)) {
			signal(tp, SIGINT);
			goto flush;
		}
 		if (tp->t_addr == ttyaddrs[0] && c==020) {
			prproc();
			return;
		}
	}

	if (tp->t_rawq.c_cc>=TTYHOG) {
	flush:
		flushtty(tp);
		return;
	}

	putc(c, &tp->t_rawq);
	if (raw || (c=='\n' || c==004)) {
		if (!raw && putc(0377, &tp->t_rawq)==0)
			tp->t_delct++;
		wakeup(&tp->t_rawq);
		if (flags & SLIP) {
			wakeup (&netmain);
		}
	}

	if (!raw && (flags & ECHO)) {
		if (c == tp->t_erase) {
			c = CERASE;
			ttyoutput(c, tp);
			ttyoutput(' ', tp);
		}
		ttyoutput(c == tp->t_kill ? '\n' : c, tp);
		(*tp->t_start)(tp);
	}
}

/*
 * Called from device's read routine after it has
 * calculated the tty-structure given as argument.
 * The pc is backed up for the duration of this call.
 * In case of a caught interrupt, an RTI will re-execute.
 */
void
ttread(tp)
	register struct tty *tp;
{
	register char *base, c;
	register int n;

	if (tp->t_canq.c_cc || canon(tp)) {
		n = u.u_count;
		base = u.u_base;
		while (n && tp->t_canq.c_cc) {
			c = getc(&tp->t_canq);
			if(u.u_segflg)
				*base++ = c;
			else
				subyte(base++, c);
			--n;
		}
		u.u_base    = base;
		u.u_offset += u.u_count - n;
		u.u_count   = n;
	}
}

/*
 * Called from the device's write routine after it has
 * calculated the tty-structure given as argument.
 */
void
ttwrite(tp)
	register struct tty *tp;
{
	register unsigned char *base;
	register int n;
	int sps;

	base = (unsigned char*) u.u_base;
	n = u.u_count;
	u.u_offset += n;
	while (n--) {
		sps = spl7();
		while (tp->t_outq.c_cc > TTHIWAT) {
			(*tp->t_start)(tp);
			sleep(&tp->t_outq, TTOPRI);
		}
		splx(sps);
		ttyoutput(u.u_segflg ? *base++ : fubyte(base++), tp);
	}
	u.u_count = 0;
	u.u_base = (char*) base;
	(*tp->t_start)(tp);
}

/* CHARACTER LIST ROUTINES */

/*
 * The actual structure of a clist block manipulated by
 * getc and putc (mch.s)
 */
struct cblock {
	struct cblock *c_next;
	char c_info[6];
};

/*
 * The character lists-- space for 6*NCLIST characters if lucky,
 * or 6 less otherwise: getc/putc needs the structs to be 8-aligned;
 * cinit() takes care of that.
 */
struct cblock cfree[NCLIST];

/* List head for unused character blocks. */
struct cblock *cfreelist;

/*
 * Initialize clist by freeing all character blocks, then count
 * number of character devices. (Once-only routine)
 */

#define CROUND 7

void
cinit()
{
	register struct cblock *cp;
	register struct cdevsw *cdp;
	register int ccp;

	cp = (struct cblock*) (((int)cfree + CROUND) & ~CROUND);
	for (; cp <= &cfree[NCLIST-1]; cp++) {
		cp->c_next = cfreelist;
		cfreelist = cp;
	}
	ccp = 0;
	for(cdp = cdevsw; cdp->d_open; cdp++)
		ccp++;
	nchrdev = ccp;
}

/* 
 * Character list get/put. Reworked from pdp-11 assembly by the BKUNIX
 * project.
 */
int
getc(p)
	register struct clist *p;
{
	register struct cblock *bp;
	register int c, s;

	s = spl7();
	if (p->c_cf == 0) {
		c = -1;
		p->c_cc = 0;
		p->c_cf = p->c_cl = 0;
	} else {
		c = *p->c_cf++ & 0377;
		if (--p->c_cc <= 0) {
			bp = (struct cblock*) ((int)(p->c_cf - 1) & ~CROUND);
			p->c_cf = p->c_cl = 0;
			goto reclaim;
		}
		if (((int)p->c_cf & CROUND) == 0) {
			bp = (struct cblock*) p->c_cf - 1;
			p->c_cf = bp->c_next->c_info;
reclaim:		bp->c_next = cfreelist;
			cfreelist = bp;
		}
	}
	splx(s);
	return c;
}

int
putc(c, p)
	int c;
	register struct clist *p;
{
	register struct cblock *bp;
	register char *cp;
	register s;

	s = spl7();
	cp = p->c_cl;
	if (cp == 0) {
		bp = cfreelist;
		if (bp == 0) {
eof:			splx(s);
			return -1;
		}
		p->c_cf = bp->c_info;
		goto alloc;
	}
	if (((int)cp & CROUND) == 0) {
		bp = (struct cblock*) cp - 1;
		bp->c_next = cfreelist;
		if (bp->c_next == 0)
			goto eof;
		bp = bp->c_next;
alloc:		cfreelist = bp->c_next;
		bp->c_next = 0;
		cp = bp->c_info;
	}
	*cp++ = c;
	p->c_cc++;
	p->c_cl = cp;
	splx(s);
	return 0;
}
