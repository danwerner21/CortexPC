/*
 * A clist structure is the head
 * of a linked list queue of characters.
 * The characters are stored in 4-word
 * blocks containing a link and 6 characters.
 * The routines getc and putc (m45.s or m40.s)
 * manipulate these structures.
 */
struct clist
{
	int	c_cc;		/* character count */
	char*	c_cf;		/* pointer to first block */
	char*	c_cl;		/* pointer to last block */
};

/*
 * A tty structure is needed for
 * each UNIX character device that
 * is used for normal terminal IO.
 * The routines in tty.c handle the
 * common code associated with
 * these structures.
 * The definition and device dependent
 * code is in each driver. (kl.c dc.c dh.c)
 */
struct tty
{
	struct	clist t_rawq;	/* input chars right off device */
	struct	clist t_canq;	/* input chars after erase and kill */
	struct	clist t_outq;	/* output list to device */
	int	t_flags;	/* mode, settable by stty call */
	int	*t_addr;	/* device address */
	void	(*t_start)();	/* device start routine */
	char	t_delct;	/* number of delimiters in raw q */
	char	t_col;		/* printing column of device */
	char	t_erase;	/* erase character */
	char	t_kill;		/* kill character */
	char	t_state;	/* internal state, not visible externally */
	char	t_char;		/* character temporary */
	int	t_speeds;	/* output+input line speed */
	int	t_dev;		/* device name */
};

#define	TTIPRI	10
#define	TTOPRI	20

#define	CERASE	010		/* ^H */
#define	CBACKSP	030		/* <= */
#define	CEOT	004		/* ^D */
#define	CKILL	025		/* ^U */
#define	CQUIT	034		/* ^\ */
#define	CINTR	003		/* ^C */

/*
 * Speeds
 */
#define B0	0
#define B50	1
#define B75	2
#define B110	3
#define B134	4
#define B150	5
#define B200	6
#define B300	7
#define B600	8
#define B1200	9
#define	B1800	10
#define B2400	11
#define B4800	12
#define B9600	13
#define B19200	14

/* limits */
#define	TTHIWAT	50
#define	TTLOWAT	30
#define	TTYHOG	256

/* flags(modes) */
#define	TANDEM	 0000001
#define	CBREAK	 0000002
#define	LCASE	 0000004
#define	ECHO	 0000010
#define	CRMOD	 0000020
#define	RAW	 0000040
#define	ODDP	 0000100
#define	EVENP	 0000200
#define ANYP	 0000300
#define	NLDELAY	 0001400
#define	TBDELAY	 0001000
#define	XTABS	 0002000
#define SLIP	 0004000
#define	DAVAIL	 0010000
#define	CRDELAY	 0020000
#define	VTDELAY	 0040000
#define BSDELAY  0100000
#define ALLDELAY 0177400

/* Hardware bits */
#define	DONE	0200
#define	IENABLE	0100

/* Internal state bits */
#define	TIMEOUT	0001		/* Delay timeout in progress */
#define	WOPEN	0002		/* Waiting for open to complete */
#define	ISOPEN	0004		/* Device is open */
#define	SSTART	0010		/* Has special start routine at addr */
#define	CARR_ON	0020		/* Software copy of carrier-present */
#define	BUSY	0040		/* Output in progress */
#define	ASLEEP	0100		/* Wakeup when output done */

#ifdef KERNEL
int getc();
int putc();
#endif
