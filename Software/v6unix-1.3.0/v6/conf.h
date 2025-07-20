/*
 * Used to dissect integer device code
 * into major (driver designation) and
 * minor (driver parameter) parts.
 */
struct dev
{
	char	d_minor;
	char	d_major;
};

#define MINOR(d) (((d) >> 8) & 0xFF)
#define MAJOR(d) ((d) & 0xFF)

#define SETMINOR(d,v) ((d) = ((d) & 0x00FF) | (((v) & 0xFF) << 8))
#define SETMAJOR(d,v) ((d) = ((d) & 0xFF00) | ((v) & 0xFF))

#ifdef TI_CTX
#define NCHRDEV 4
#else
#define NCHRDEV 9
#endif

/*
 * Declaration of block device
 * switch. Each entry (row) is
 * the only link between the
 * main unix code and the driver.
 * The initialization of the
 * device switches is in the
 * file conf.c.
 */
struct bdevsw
{
	int		(*d_open)();
	int		(*d_close)();
	int		(*d_strategy)();
	struct devtab	*d_tab;
} bdevsw[2];

/*
 * Nblkdev is the number of entries
 * (rows) in the block switch. It is
 * set in binit/bio.c by making
 * a pass over the switch.
 * Used in bounds checking on major
 * device numbers.
 */
int	nblkdev;

/*
 * Character device switch.
 */
struct cdevsw
{
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_read)();
	int	(*d_write)();
	int	(*d_sgtty)();
} cdevsw[NCHRDEV];

/*
 * Number of character switch entries.
 * Set by cinit/tty.c
 */
int	nchrdev;
