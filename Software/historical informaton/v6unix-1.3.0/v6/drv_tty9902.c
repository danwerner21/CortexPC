/*
 * TMS9902 Asynchronous Communications Adapter (ACA) driver
 */

#include "param.h"
#include "conf.h"
#include "tty.h"
#include "user.h"
#include "proc.h"

#define NACA 1

/* tty CRU addresses */
extern int ttyaddrs[NACA];

/* Bit offsets on a 9902 */
#define LDIR	13
#define RTSON	16
#define RIENB	18
#define XIENB	19
#define XBRE	22
#define RESET	31

#define DTRON	32
#define INTENB	38
#define ACAENB	39

static struct tty aca[NACA];

#ifdef TI990_10A
static unsigned int speeds[] =
{
   0x000, 0x000, 0x6B6, 0x5D9, 0x583, 0x55B, 0x504, 0x4AE,
   0x2B6, 0x15B, 0x0E7, 0x0AE, 0x056, 0x02B, 0x000
};

static unsigned int parity[] =
{
   0x83, 0xB2, 0xA2, 0x83
};
#endif


/* Open the device: set initial flags, set RTS to on and enable receive
 * interrupts. Transmit interrupts are enabled once there is something to
 * send (see ttstart).
 */
void
acaopen(dev, flag)
{
	register int *addr;
	register struct tty *tp;
	extern void acastart();

	if(MINOR(dev) >= NACA) {
		u.u_error = ENXIO;
		return;
	}
	tp = &aca[MINOR(dev)];
	if (u.u_procp->p_pid > 1) {
		u.u_procp->p_ttyp = tp;
		tp->t_dev = dev;
	}

	addr = (int *)ttyaddrs[MINOR(dev)];
	tp->t_addr = addr;
	tp->t_start = acastart;
	if ((tp->t_state&ISOPEN) == 0) {
		tp->t_state = ISOPEN|CARR_ON;
		tp->t_flags = XTABS|ECHO|CRMOD;
		tp->t_speeds = (B9600 << 8) | B9600;
		tp->t_erase = CERASE;
		tp->t_kill  = CKILL;
	}
	sbo(addr, RIENB);
	sbo(addr, RTSON);
#ifdef TI990_10A
	sbo(addr, ACAENB);
	sbo(addr, DTRON);
	sbo(addr, INTENB);
#endif
}

/* Close the device: wait for the output queue to flush and then switch
 * off RTS and disable the interrupts.
 */
void
acaclose(dev, flag)
{
	register struct tty *tp;
	int *addr;

	tp = &aca[MINOR(dev)];
	addr = tp->t_addr;
	wflushtty(tp);
	tp->t_state = 0;
}

/* (Re-)start the 9902 transmitting. If it is already transmitting it is
 * no-op. If the output queue is empty, disable the interrupt until there is
 * something to send. Otherwise, place the next character in the xmit buffer
 * and enable xmit interrupts.
 */
void
acastart(tp)
	register struct tty *tp;
{
	register int c, *addr;

	addr = tp->t_addr;
	if (tb(addr, XBRE)==0)
		return;
	if ((c=getc(&tp->t_outq)) >= 0) {
		ldcr8(addr, c);
		sbo(addr, XIENB);
	}
	else
		sbz(addr, XIENB);
}

/* Handle 9902 receiver buffer full interrupt; see low.S
 * Read the character from the device and reset and reenable the interrupt
 * signal. Place the character on the input queue (if the queue is full, the
 * character gets dropped).
 */
void
acarint(xea)
{
	register int c, minor, *addr;
	register struct tty *tp;

	minor = xea & 0x3f;
	tp = &aca[minor];
	addr = tp->t_addr;
	c = stcr8(addr);
	sbo(addr, RIENB);
	ttyinput(c, tp);
}

/* Handle transmitter buffer empty interrupt; see low.S
 * Start/stop sending as needed. If the process was suspended on a full queue
 * wake it up again once on the low water mark or on an empty queue.
 */
void
acaxint(xea)
{
	register int minor;
	register struct tty *tp;

	minor = xea & 0x3f;
	tp = &aca[minor];
	acastart(tp);
	if (tp->t_outq.c_cc == 0 || tp->t_outq.c_cc == TTLOWAT)
		wakeup(&tp->t_outq);
}


/*
 * COMMON routines.
 */

void
acaread(dev)
{
	ttread(&aca[MINOR(dev)]);
}

void
acawrite(dev)
{
	ttwrite(&aca[MINOR(dev)]);
}

void
acasgtty(dev, v)
	int *v;
{
	int i;
	unsigned int spd;
	int *addr;
	struct tty *tp;

	tp = &aca[MINOR(dev)];
	ttystty(tp, v);

#ifdef TI990_10A
	if (v) return;

	addr = tp->t_addr;
	v = &u.u_arg[1];

	i = v[0] & 0xFF;
	spd = speeds[i];
	if (spd == 0) return;

	i = (v[2] & ANYP) >> 6;
	sbo (addr, RESET);
	ldcr8 (addr, parity[i]);

	sbz (addr, LDIR);
	ldcr (addr, 0, spd);

	sbo (addr, RTSON);
	sbo (addr, RIENB);
	sbo (addr, XIENB);
	sbo (addr, DTRON);
	sbo (addr, INTENB);
	sbo (addr, ACAENB);
#endif
}
