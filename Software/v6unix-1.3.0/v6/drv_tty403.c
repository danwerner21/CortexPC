/*
 * CI403 Asynchronous Communications Adapter (ACA) driver
 * The CI403 is a 4 channel serial multiplexer on the TILINE.
 */

#include "param.h"
#include "conf.h"
#include "tty.h"
#include "user.h"
#include "proc.h"

#define NACA 4

#define ERRCRU   0x1FC0
#define TLTIMOUT 15

extern void spin();

/* CI403 device addresses */
extern int c403addrs[NACA];

struct device {
	unsigned int wd[4];
};

struct control {
	int init;
	int mreset;
	int xoff;
	int xmit;
	struct tty aca;
};

/*
** Command word control bits
*/

/* word 0 - status */

#define IPBIT	0x8000		/* Interrupt pending */
#define IEBIT	0x4000		/* Interrupt enable */
#define ISBIT	0x2000		/* Interrupt select */
#define IRBIT	0x1000		/* Input ready */
#define MRBIT	0x0800		/* Master Reset */
#define TEBIT	0x0400		/* Timer enable */
#define FABIT	0x0200		/* Failed channel test */
#define TSBIT	0x0100		/* Test mode select */

/* word 1 - command */

#define RDBIT	0x8000		/* Read reg */

#define GETREG(x) (((x) >> 12) & 7)
#define SETREG(x,v) ((x) = (((x) & 0x8FFF) | ((v) << 12)))

/* word 2 - output */

/* word 3 - input */

#define IVBIT	0x8000		/* Invalid data */
#define CCBIT	0x0800		/* Channel Change */

#define GETSTAT(x) (((x) >> 12) & 7)
#define SETSTAT(x,v) ((x) = (((x) & 0x8FFF) | ((v) << 12)))

/* General access */

#define GETCHAN(x) (((x) >> 8) & 7)
#define GETDATA(x) ((x) & 0xFF)

#define SETCHAN(x,v) ((x) = (((x) & 0xF8FF) | ((v) << 8)))
#define SETDATA(x,v) ((x) = (((x) & 0xFF00) | (v)))

static struct control c403dat[NACA];
static struct device *CIADDR;

/*
 * Program channel data
 */

static unsigned int progtab[] =
{
   0x4000, 0x3080, 0x1000, 0x0000, 0x3000, 0x4001, 0x4003, 0xFFFF
};

static unsigned int speeds[] =
{
   0x000, 0xC00, 0x800, 0x574, 0x476, 0x400, 0x300, 0x200,
   0x100, 0x080, 0x055, 0x040, 0x020, 0x010, 0x008
};

static unsigned int parity[] =
{
   0x03, 0x0A, 0x1A, 0x03
};

/*
 * Issue CI403 control command.
 */
static void
c403cmd (ndx, cmd, delay)
	unsigned int cmd;
{
	CIADDR->wd[ndx] = cmd;
	if (delay > 0)
		spin (delay);
}

/*
 * Program CI403 channel.
 */
static void
c403prog (chan, spd, par)
{
	int i;
	unsigned int cmd;

	progtab[2] = (progtab[2] & 0xFF00) | (speeds[spd] >> 8);
	progtab[3] = (progtab[3] & 0xFF00) | (speeds[spd] & 0xFF);
	progtab[1] = (progtab[1] & 0xFF80) | parity[par];
	progtab[4] = (progtab[4] & 0xFF80) | parity[par];

	for (i = 0; ; i++) {
		cmd = progtab[i];
		if (cmd == 0xFFFF) break;
		SETCHAN (cmd, chan);
		c403cmd (1, cmd, 256);
		c403cmd (0, i == 0 ? 0x4000 : 0x5000, 256);
	}
}

/*
 * Initialize CI403 channel - 9600-8-N-1.
 */
void
c403init (minor)
{
	int i, chan, dev;
	unsigned int cmd;

	c403dat[minor].xoff = 0;
	c403dat[minor].xmit = 0;

	if (c403dat[minor].init)
		return;

	chan = minor % 4;
	dev = minor / 4;
	CIADDR = (struct device *)c403addrs[dev];
	/* Issue Master reset on channel 0 */
	if (!c403dat[dev].mreset) {
		c403dat[dev].mreset = 1;
		c403cmd (0, MRBIT, 3*4096);
		while ((CIADDR->wd[0] & MRBIT) != 0)
			spin (4096);
	}

	c403prog (chan, B9600, 0);
	c403dat[minor].init = 1;
}

/*
 * Open the device: set initial flags, and initialize channel.
 */
void
c403open (dev, flag)
{
	register struct tty *tp;
	int minor, x, y;
	extern void c403start();

	minor = MINOR(dev);
	if (minor >= NACA) {
		u.u_error = ENXIO;
		return;
	}

	/* If we get a TILINE timout, then no device present */
	CIADDR = (struct device *)c403addrs[minor/4];
	x = splx(0);
	y = CIADDR->wd[0];
	splx(x);
	if (tb (ERRCRU, TLTIMOUT) == 1) {
		sbz (ERRCRU, TLTIMOUT);
		u.u_error = ENXIO;
		return;
	}

	tp = &c403dat[minor].aca;
	if (u.u_procp->p_pid > 1) {
		u.u_procp->p_ttyp = tp;
		tp->t_dev = dev;
	}
	tp->t_addr = (int *)minor;

	tp->t_start = c403start;
	if ((tp->t_state & ISOPEN) == 0) {
		tp->t_state = ISOPEN | CARR_ON;
		tp->t_flags = XTABS | ECHO | CRMOD;
		tp->t_speeds = (B9600 << 8) | B9600;
		tp->t_erase = CERASE;
		tp->t_kill  = CKILL;
	}
	c403init (minor);
}

/*
 * Close the device: wait for the output queue to flush.
 */
void
c403close (dev, flag)
{
	register struct tty *tp;

	tp = &c403dat[MINOR(dev)].aca;
	wflushtty (tp);
	tp->t_state = 0;
}

/*
 * (Re-)start transmitting. If it is already transmitting it is
 * no-op. If the output queue is empty, disable the interrupt until there is
 * something to send. Otherwise, place the next character(s) in the
 * xmit buffer.
 */
void
c403start (tp)
	register struct tty *tp;
{
	register int c, dev, chan, minor;
	int cnt;
	unsigned int cmd;
	register int raw;

	raw = tp->t_flags & (RAW|CBREAK);
	minor = (int)tp->t_addr;
	chan = minor % 4;
	dev = minor / 4;
	cmd = 0;
	cnt = 0;
	SETCHAN (cmd, chan);

	CIADDR = (struct device *)c403addrs[dev];
	/* send characters to the xmit FIFO */
	while ((c = getc (&tp->t_outq)) >= 0) {
		SETDATA (cmd, c);
		c403cmd (2, cmd, 256);
		cnt++;
		if (c403dat[0].xoff == 1)
			break;
	}

	/* if we sent some characters, set up xmit FIFO empty event */
	if (!raw && cnt) {
		cmd = 0x7080;
		SETCHAN (cmd, chan);
		c403cmd (1, cmd, 256);
	}
}

static void
c403xmit (minor)
{
	register struct tty *tp;

	tp = &c403dat[minor].aca;
	c403start (tp);
	if (tp->t_outq.c_cc <= TTLOWAT)
		wakeup (&tp->t_outq);
}

/*
 * Handle interrupt; see low.s
 */
void
c403intr (xea)
{
	register int c, chan, dev;
	register struct tty *tp;
	unsigned int w0, cmd;

	xea &= 0x3f;
	dev = xea * 4;
	CIADDR = (struct device *)c403addrs[xea];
	w0 = CIADDR->wd[0];
	if (w0 & IRBIT) { /* Input ready */
		cmd = CIADDR->wd[3];
		chan = GETCHAN (cmd);
		while ((cmd & IVBIT) == 0) { /* Not Invalid data */
			if (cmd & CCBIT) { /* Channel Change */
				chan = GETCHAN (cmd);
			}
			c = GETDATA (cmd);

			switch (GETSTAT (cmd)) {
			case 0: /* Input data */
				tp = &c403dat[dev+chan].aca;
				ttyinput (c, tp);
				break;

			case 1: /* Xmit FIFO empty */
				c403xmit (dev+chan);
				break;

			case 4: /* Interface control */
				if (c == 0x00) { /* Timer tick */
				   for (chan = 0; chan < NACA; chan++) {
				      if (c403dat[chan].xmit) {
					 c403xmit (chan);
				      }
				    }
				} else if (c == 0x01) { /* Stop sending */
					c403dat[0].xoff = 1;
				} else if (c == 0x02) { /* Resume sending */
					c403dat[0].xoff = 0;
				}
				break;
							
			default:
				break;
			}
			cmd = CIADDR->wd[3];
		}
	}
	c403cmd (0, w0 & 0x640F, 128);
}

/*
 * COMMON routines.
 */

void
c403read (dev)
{
	ttread (&c403dat[MINOR(dev)].aca);
}

void
c403write (dev)
{
	c403dat[MINOR(dev)].xmit = 1;
	ttwrite (&c403dat[MINOR(dev)].aca);
	c403dat[MINOR(dev)].xmit = 0;
}

void
c403sgtty (dev, v)
	int *v;
{
	int spd, par;
	int chan;
	struct tty *tp;

	tp = &c403dat[MINOR(dev)].aca;
	ttystty (tp, v);
	if (v) return;

	v = &u.u_arg[1];
	spd = v[0] & 0xFF;
	if (speeds[spd] == 0) return;

	par = (v[2] & ANYP) >> 6;
	chan = MINOR(dev) % 4;

	c403prog (chan, spd, par);
}
