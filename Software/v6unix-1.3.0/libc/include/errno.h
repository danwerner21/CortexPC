/*
 * Error codes
 */

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
#define	EFAULT	14
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
#define ENETSTAT	35	/* user status available (not an error) */
#define ENETDOWN	36	/* net i/f not available */
#define ENETCON		37	/* too many net connections */
#define ENETBUF		38	/* no more net buffer space */
#define ENETPARM	39	/* bad mode in net open or request in ioctl */
#define ENETRNG		40	/* bad addr, port, raw link or proto range */
#define ENETDED		41	/* can't open: destination dead */
#define ERAWBAD		42	/* unable to send on raw connection */
#define ENETERR		43	/* attempt I/O on closed connection */
#define ENETREF		44	/* destination refused (reset) */
#define ENETUNR		45	/* destination unreachable */
#define ENETTIM		46	/* destination not responding (timeout) */

#define sys_nerr 47

extern int errno;

