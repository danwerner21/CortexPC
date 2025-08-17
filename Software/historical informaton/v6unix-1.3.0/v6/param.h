/*
 * tunable variables
 */

#ifdef TI_CTX
#define	CMAPSIZ	8		/* size of core allocation area */
#else
#define	CMAPSIZ	31		/* size of core allocation area */
#endif
#define	SMAPSIZ	8		/* size of swap allocation area */

#define	NBUF	15		/* size of buffer cache */
#define	NINODE	100		/* number of in core inodes */
#define	NFILE	100		/* number of in core file structures */
#define	NMOUNT	5		/* number of mountable file systems */
#define	NEXEC	3		/* number of simultaneous exec's */
#define	MAXMEM	(56)		/* max core per process in KB */
#define	SSIZE	20		/* initial stack size (*64 bytes) */
#define	SINCR	20		/* increment of stack (*64 bytes) */
#define	NOFILE	15		/* max open files per process */
#define	CANBSIZ	256		/* max size of typewriter line */
#define	NCALL	20		/* max simultaneous time callouts */
#define	NPROC	CMAPSIZ+SMAPSIZ-1 /* max number of processes */
#define	NTEXT	40		/* max number of pure texts */
#define	NCLIST	100		/* max total clist size */
#define	HZ	60		/* Ticks/second of the clock */

#define NNETREQ	10		/* Number of outstanding network requests */

#define SWPBLKS	1024		/* number of swap area disk blocks */

#define BOTUSR 0x1000
#define TOPUSR 0xf000
#define UCORE  0xe000

#define BSHIFT 9
#define BSIZE  512

/*
 * priorities
 * probably should not be
 * altered too much
 */

#define	PSWP	-100
#define	PINOD	-90
#define	PRIBIO	-50
#define PRINET	-40
#define	PPIPE	1
#define	PWAIT	40
#define	PSLEP	90
#define	PUSER	100

#define PZERO	25

/*
 * signals
 * dont change
 */

#define	NSIG	20
#define		SIGHUP	1	/* hangup */
#define		SIGINT	2	/* interrupt (^C) */
#define		SIGQIT	3	/* quit (^\) */
#define		SIGINS	4	/* illegal instruction */
#define		SIGTRC	5	/* trace or breakpoint */
#define		SIGIOT	6	/* iot */
#define		SIGEMT	7	/* emt */
#define		SIGFPT	8	/* floating exception */
#define		SIGKIL	9	/* kill */
#define		SIGBUS	10	/* bus error */
#define		SIGSEG	11	/* segmentation violation */
#define		SIGSYS	12	/* sys */
#define		SIGPIPE	13	/* end of pipe */

/*
 * fundamental constants
 * cannot be changed
 */

#define	USIZE	1024		/* size of user block in bytes */
#ifndef NULL
#define	NULL	0
#endif
#define	NODEV	(-1)
#define	ROOTINO	1		/* i number of all roots */
#define	DIRSIZ	14		/* max characters per directory */

#define daddr_t unsigned int
#define time_t  unsigned long

