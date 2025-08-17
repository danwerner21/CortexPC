
#include "param.h"
#include "user.h"
#include "systm.h"
#include "proc.h"
#include "inode.h"
#include "mmu.h"

#ifdef TI_CTX
#define SYSBASE	 0x1ee0
#define ACABASE	 0x0000
#define RIENB	 18
#define DECENBL	 1

int *decrreg = (int*)0xfffa;
#endif

int maxslot = 0;
int maxproc = 0;

/*
 * icode is the hex bootstrap program executed in user mode
 * to bring up the system. The equivalent assembly is:
 *		li	r0,initp
 *		mov	r0,(sp)
 *		dect	sp
 *		li	r0,init
 *		mov	r0,(sp)
 *		sys	exec
 *	1:	jmp	1b
 *	initp:	init; 0
 *	init:	"/etc/init"
 */
int	icode[] = {
	0x0200,	BOTUSR+0x14,
	0xc680,
	0x064a,
	0x0200, BOTUSR+0x18,
	0xc680,
	0x2c60, 11,
	0x10ff,
	BOTUSR+0x18, 0x0000,
	0x2f65, 0x7463, 0x2f69, 0x6e69, 0x7400
};

/* copy of u.u_proc[], used by low-level clock routine */
extern int uprof[];

/* Set the user mode mapping registers to select the
 * current user process.
 */
void
sureg()
{
#ifdef TI_CTX
	unsigned int phy, i;
	phy = u.u_procp->p_slot<<4;
	sysmap[14] = phy++;
	for(i=1; i<15; i++)
		usrmap[i] = phy++;
#else
        usrslot (u.u_procp->p_slot);
#endif

	uprof[0] = u.u_prof[0];
	uprof[1] = u.u_prof[1];
	uprof[2] = u.u_prof[2];
	uprof[3] = u.u_prof[3];
}

/*
 * Initialization code, called from mch.S as soon as a stack
 * has been established.
 * Functions:
 *	hand craft 0th process
 *	call all initialization routines
 *	fork - process 0 to schedule
 *
 * infinite loop at loc BOTUSR+0x12 means that
 *	"/etc/init" could not be executed.
 */

int
main()
{
	register int i;
	int *p;

	/*
	 * zero and free all of core
	 */
	printk("\nMem");
	for (i = 1; i < CMAPSIZ; i++) {
		printk(".");
		if (clearimg(i) < 0)
		   break;
	}
	maxmem = 64*i;
	maxslot = i;
	maxproc = i + SMAPSIZ - 1;
	printk("%dKB\n", maxmem);
	maxmem = min(maxmem, MAXMEM);

	p = (int *)((unsigned int)&u + sizeof (struct user));
	*p = 0xa5a5;

#ifdef TI_CTX
	/* start clock and console TTY */
	*decrreg = 12500;
	sbo(SYSBASE, DECENBL);
	sbo(ACABASE, RIENB);
#else
	/* Set up parition table */
	cfpart();
#endif

	/* set up system process */
	proc[0].p_slot  = 0;
	proc[0].p_stat  = SRUN;
	proc[0].p_flag |= SLOAD|SSYS;
	u.u_procp = &proc[0];

	/* set up 'known' i-nodes */
	cinit();
	binit();
	iinit();
	rootdir = iget(rootdev, ROOTINO);
	rootdir->i_flag &= ~ILOCK;
	u.u_cdir = iget(rootdev, ROOTINO);
	u.u_cdir->i_flag &= ~ILOCK;

	/* setup network queues */
	netinit();

	/*
	 * make init process
	 * enter scheduling loop
	 * with system process
	 */
	if(newproc()) {
		/* process 1, load 'init' */
		sureg();
		copyout(BOTUSR, icode, sizeof icode);
		/*
		 * Return from main jumps to trampoline
		 * code just copied out.
		 */
		return 0;
	}
	/* process 0, sched() never returns */
	sched();
}
