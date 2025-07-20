/*
 * EIA Asynchronous Communications Adapter (ACA) driver
 */

#include "param.h"
#include "conf.h"
#include "tty.h"
#include "user.h"
#include "proc.h"

#define NACA 1

/* Bit offsets on an EIA board */
#define	TTYXMIT		8
#define	TTYDTR		9
#define	TTYRTS		10
#define	TTYWRQ		11
#define	TTYRRQ		12
#define	TTYNSF		13
#define	TTYEINT		14

/* tty CRU addresses */
extern int ttyaddrs[NACA];

static struct tty eiaaca[NACA];

/* Open the device: set initial flags, set RTS to on and enable receive
 * interrupts. Transmit interrupts are enabled once there is something to
 * send (see ttstart).
 */
void
acaopen (dev, flag)
{
	register int minor, *addr;
	register struct tty *tp;
	extern void acastart();

	minor = MINOR(dev);
	if (minor >= NACA) {
		u.u_error = ENXIO;
		return;
	}

	addr = (int *)ttyaddrs[minor];
	if (addr == 0xFFFF || stcr16 (addr) == 0xFFFF) {
		u.u_error = ENXIO;
		return;
	}

	tp = &eiaaca[minor];
	if (u.u_procp->p_pid > 1) {
		u.u_procp->p_ttyp = tp;
		tp->t_dev = dev;
	}

	tp->t_addr = addr;
	tp->t_start = acastart;
	if ((tp->t_state & ISOPEN) == 0) {
		tp->t_state = ISOPEN | CARR_ON;
		tp->t_flags = XTABS | ECHO | CRMOD;
		tp->t_speeds = (B9600 << 8) | B9600;
		tp->t_erase = CERASE;
		tp->t_kill  = CKILL;
	}
	sbo (addr, TTYDTR);
	sbo (addr, TTYRTS);
	sbo (addr, TTYEINT);
}

/* Close the device: wait for the output queue to flush and then switch
 * off RTS and disable the interrupts.
 */
void
acaclose (dev, flag)
{
	register struct tty *tp;
	int *addr;

	tp = &eiaaca[MINOR(dev)];
	addr = tp->t_addr;
	wflushtty (tp);
	tp->t_state = 0;
}

/* (Re-)start the EIA transmitting. If it is already transmitting it is
 * no-op. If the output queue is empty, disable the interrupt until there is
 * something to send. Otherwise, place the next character in the xmit buffer
 * and enable xmit interrupts.
 */
void
acastart (tp)
	register struct tty *tp;
{
	register int c, *addr;

	addr = tp->t_addr;
	if (tb (addr, TTYXMIT)==1)
		return;
	if ((c = getc (&tp->t_outq)) >= 0) {
		ldcr8 (addr, c);
	}
}


/* Handle EIA receiver buffer full interrupt; see low.S
 * Read the character from the device and reset and reenable the interrupt
 * signal. Place the character on the input queue (if the queue is full, the
 * character gets dropped).
 */
void
acarint (xea)
{
	register int c, minor, *addr;
	register struct tty *tp;

	minor = xea & 0x3f;
	tp = &eiaaca[minor];
	addr = tp->t_addr;
	c = stcr8 (addr);
	ttyinput (c, tp);
}

/* Handle transmitter buffer empty interrupt; see low.S
 * Start/stop sending as needed. If the process was suspended on a full queue
 * wake it up again once on the low water mark or on an empty queue.
 */
void
acaxint (xea)
{
	register int minor;
	register struct tty *tp;

	minor = xea & 0x3f;
	tp = &eiaaca[minor];
	acastart (tp);
	if (tp->t_outq.c_cc == 0 || tp->t_outq.c_cc == TTLOWAT)
		wakeup (&tp->t_outq);
}

/*
 * COMMON routines.
 */

void
acaread (dev)
{
	ttread (&eiaaca[MINOR(dev)]);
}

void
acawrite (dev)
{
	ttwrite (&eiaaca[MINOR(dev)]);
}

void
acasgtty (dev, v)
	int *v;
{
	register struct tty *tp;

	tp = &eiaaca[MINOR(dev)];
	ttystty (tp, v);
}

