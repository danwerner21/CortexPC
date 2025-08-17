/*
 * The user structure.
 * One allocated per process.
 * Contains all per process data
 * that doesn't need to be referenced
 * while the process is swapped.
 * The user block is USIZE bytes
 * long; resides at virtual kernel
 * loc 0xf400; contains the system
 * stack per user; is cross referenced
 * with the proc structure for the
 * same process.
 */
struct user
{
	int		u_rsav[2];		/* save sp,bp when exchanging stacks */
	char		u_segflg;		/* flag for IO; user or kernel space */
	char		u_error;		/* return error code */
	char		u_uid;			/* effective user id */
	char		u_gid;			/* effective group id */
	char		u_ruid;			/* real user id */
	char		u_rgid;			/* real group id */
	struct proc*	u_procp;		/* pointer to proc structure */
	char*		u_base;			/* base address for IO */
	unsigned int	u_count;		/* bytes remaining for IO */
	long		u_offset;		/* offset in file for IO */
	struct inode*	u_cdir;			/* pointer to inode of current directory */
	char		u_dbuf[DIRSIZ];		/* current pathname component */
	char*		u_dirp;			/* current pointer to inode */
	struct	{				/* current directory entry */
		int	u_ino;
		char	u_name[DIRSIZ];
	} u_dent;
	struct inode*	u_pdir;			/* inode of parent directory of dirp */
	struct file*	u_ofile[NOFILE];	/* pointers to file structures of open files */
	int		u_arg[6];		/* arguments to current system call */
	int		u_tsize;		/* text size (*64) */
	int		u_dsize;		/* data size (*64) */
	int		u_ssize;		/* stack size (*64) */
	int		u_sep;			/* flag for I and D separation */
	int		u_qsav[2];		/* label variable for quits and interrupts */
	int		u_ssav[2];		/* label variable for swapping */
	int		u_signal[NSIG];		/* disposition of signals */
	int		u_utime;		/* this process user time */
	int		u_stime;		/* this process system time */
	long		u_cutime;		/* sum of childs' utimes */
	long		u_cstime;		/* sum of childs' stimes */
	int*		u_ar0;			/* address of users saved R0 */
	int		u_prof[4];		/* profile arguments */
	char		u_intflg;		/* catch intr from sys */

						/* kernel stack per user
						* extends from u + USIZE
						* backward not to reach here
						*/
} u;

/* u_error codes */
#define	EFAULT	106
#define	EPERM	1
#define	ENOENT	2
#define	ESRCH	3
#define	EINTR	4
#define	EIO	5
#define	ENXIO	6
#define	E2BIG	7
#define	ENOEXEC	8
#define	EBADF	9
#define	ECHILD	10
#define	EAGAIN	11
#define	ENOMEM	12
#define	EACCES	13
#define	ENOTBLK	15
#define	EBUSY	16
#define	EEXIST	17
#define	EXDEV	18
#define	ENODEV	19
#define	ENOTDIR	20
#define	EISDIR	21
#define	EINVAL	22
#define	ENFILE	23
#define	EMFILE	24
#define	ENOTTY	25
#define	ETXTBSY	26
#define	EFBIG	27
#define	ENOSPC	28
#define	ESPIPE	29
#define	EROFS	30
#define	EMLINK	31
#define	EPIPE	32

/* network */
#define ENETSTAT 35     /* user status available (not an error) */
#define ENETDOWN 36     /* net i/f not available */
#define ENETCON  37     /* too many net connections */
#define ENETBUF  38     /* no more net buffer space */
#define ENETPARM 39     /* bad mode in net open or request in net ioctl */
#define ENETRNG  40     /* bad local net addr, port, raw link or proto range */
#define ENETDED  41     /* can't open: destination dead */
#define ERAWBAD  42     /* unable to send on raw connection */
#define ENETERR  43     /* attempt to read or write on closed connection */
#define ENETREF  44     /* can't open: destination refused (reset) */
#define ENETUNR  45     /* can't open: destination unreachable */
#define ENETTIM  46     /* can't open: destination not responding (timeout) */

