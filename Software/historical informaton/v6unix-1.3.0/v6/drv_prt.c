/*
 * Serial printer driver.
 * We treat the serial printer as an output only TTY.
 */

#include "param.h"
#include "conf.h"
#include "tty.h"
#include "user.h"
#include "proc.h"

#define NPRT 1
struct tty prt[NPRT];


/* CRU base address */
#define PRTBASE		(int*)0x0060

/* Bit offsets on an EIA board */
#define	TTYXMIT		8
#define	TTYDTR		9
#define	TTYRTS		10
#define	TTYWRQ		11
#define	TTYRRQ		12
#define	TTYNSF		13
#define	TTYEINT		14


/* Open the device: set initial flags, set RTS to on and enable receive
 * interrupts. Transmit interrupts are enabled once there is something to
 * send (see ttstart).
 */
void
prtopen (dev, flag)
{
	register int *addr;
	register struct tty *tp;
	extern void prtstart ();

	if (!flag || MINOR(dev) >= NPRT) {
		u.u_error = ENXIO;
		return;
	}

	addr = PRTBASE;
	if (stcr16 (addr) == 0xFFFF) {
		u.u_error = ENXIO;
		return;
	}

	tp = &prt[MINOR(dev)];
	tp->t_dev = dev;
	tp->t_addr = addr;
	tp->t_start = prtstart;
	if ((tp->t_state&ISOPEN) == 0) {
		tp->t_state = ISOPEN|CARR_ON;
		tp->t_flags = CRMOD;
		tp->t_speeds = (B9600 << 8) | B9600;
		tp->t_erase = 0;
		tp->t_kill  = 0;
	}
	sbo (addr, TTYDTR);
	sbo (addr, TTYRTS);
	sbo (addr, TTYEINT);
}

/* Close the device: wait for the output queue to flush and then switch
 * off RTS and disable the interrupts.
 */
void
prtclose (dev, flag)
{
	register struct tty *tp;
	int *addr;

	tp = &prt[MINOR(dev)];
	addr = tp->t_addr;
	wflushtty (tp);
	sbz (addr, TTYEINT);
	sbz (addr, TTYDTR);
	sbz (addr, TTYRTS);
	tp->t_state = 0;
}

/* (Re-)start the EIA transmitting. If it is already transmitting it is
 * no-op. If the output queue is empty, disable the interrupt until there is
 * something to send. Otherwise, place the next character in the xmit buffer
 * and enable xmit interrupts.
 */
void
prtstart (tp)
	register struct tty *tp;
{
	register int c, *addr;

	addr = tp->t_addr;
	if (tb (addr, TTYXMIT) == 1)
		return;
	if ((c=getc (&tp->t_outq)) >= 0) {
		ldcr8(addr, c);
	}
}


void
prtwrite (dev)
{
	ttwrite (&prt[MINOR(dev)]);
}

/* Handle transmitter buffer empty interrupt; see low.S
 * Start/stop sending as needed. If the process was suspended on a full queue
 * wake it up again once on the low water mark or on an empty queue.
 */
void
prtint (xea)
{
	register int minor;
	register struct tty *tp;

	minor = xea & 0x3f;
	tp = &prt[minor];
	prtstart (tp);
	if (tp->t_outq.c_cc == 0 || tp->t_outq.c_cc == TTLOWAT)
		wakeup (&tp->t_outq);
}

