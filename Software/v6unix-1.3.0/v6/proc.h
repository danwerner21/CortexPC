/*
 * One structure allocated per active
 * process. It contains all data needed
 * about the process while the
 * process may be swapped out.
 * Other per process data (user.h)
 * is swapped with the process.
 */
struct	proc
{
	char		 p_stat;
	char		 p_flag;
	char		 p_pri;		/* priority, negative is high */
	char		 p_sig;		/* signal number sent to this process */
	char		 p_uid;		/* user id, used to direct signals */
	char		 p_time;	/* resident time for scheduling */
	char		 p_cpu;		/* cpu usage for scheduling */
	char		 p_nice;	/* nice for scheduling */
	struct tty	*p_ttyp;	/* controlling tty */
	int		 p_pid;		/* unique process id */
	int		 p_ppid;	/* process id of parent */
	unsigned int	 p_slot;	/* slot of swapable image */
/*	int		 p_size;	/* size of swapable image (*64 bytes) */
	int		 p_wchan;	/* event process is awaiting */
	int		 p_nretval;	/* NET retval */
	int		 p_nerror;	/* NET error */
	long		 p_tout;	/* Sleep time for process */
/*	struct text	*p_textp;	/* pointer to text structure */
} proc[NPROC];

/* stat codes */
#define	SSLEEP	1		/* sleeping on high priority */
#define	SWAIT	2		/* sleeping on low priority */
#define	SRUN	3		/* running */
#define	SIDL	4		/* intermediate state in process creation */
#define	SZOMB	5		/* intermediate state in process termination */
#define	SSTOP	6		/* process being traced */

/* flag codes */
#define	SLOAD	01		/* in core */
#define	SSYS	02		/* scheduling process */
#define	SLOCK	04		/* process cannot be swapped */
#define	SSWAP	010		/* process is being swapped out */
#define	STRC	020		/* process is being traced */
#define	SWTED	040		/* traced proces has been waited on */
#define SNET	0100		/* Net call */

#ifdef KERNEL
void signal();
void psignal();
void psig();
void setrun();
int  pexit();
#endif
