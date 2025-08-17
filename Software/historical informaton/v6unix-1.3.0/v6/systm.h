/*
 * Build control
 */

/*
 * Random set of variables
 * used by more than one
 * routine.
 */
char		canonb[CANBSIZ];	/* buffer for erase and kill (#@) */
struct inode*	rootdir;		/* pointer to inode of root directory */
int		execnt;			/* number of processes in exec */
unsigned long	time;			/* time in sec from 1970 */

/*
 * Mount structure.
 * One allocated on every mount.
 * Used to find the super block.
 */
struct	mount
{
	int		 m_dev;		/* device mounted */
	struct buf	*m_bufp;	/* pointer to superblock */
	struct inode	*m_inodp;	/* pointer to mounted on inode */
} mount[NMOUNT];

int	mpid;			/* generic for unique process id's */
char	runin;			/* scheduling flag: a process needs swapping in */
char	runout;			/* scheduling flag: a process needs swapping out */
char	runrun;			/* scheduling flag: schedule a higher prio process */
char	curpri;			/* more scheduling */
int	maxmem;			/* actual max memory per process */
int	rootdev;		/* dev of root see conf.c */
int	swapdev;		/* dev of swap see conf.c */
daddr_t	swplo;			/* block number of swap space */
int	nswap;			/* size of swap space */
int	updlock;		/* lock for sync */
int	rablock;		/* block to be read ahead */
/*char	regloc[];		/* locs. of saved user registers (trap.c) */

/*
 * structure of the system entry table (sysent.c)
 */
struct sysent {
	int	count;		/* argument count */
	void	(*call)();	/* name of handler */
};
struct sysent sysent[64];

/*
 * uts name structure
 */
struct utsname {
	char sysname[33];
	char nodename[33];
	char release[33];
	char version[33];
	char machine[33];
};

#ifdef KERNEL
extern int netmain;	/* wchan# for network */

/* system call entry prototypes */
void nullsys();	
void rexit();	
void fork();	
void read();	
void write();	
void open();	
void close();	
void wait();	
void creat();	
void link();	
void unlink();	
void exec();	
void chdir();	
void gtime();	
void mknod();	
void chmod();	
void chown();	
void sbreak();	
void stat();	
void lseek();	
void getpid();	
void smount();	
void sumount();	
void setuid();	
void getuid();	
void stime();	
void ptrace();	
void nosys();	
void fstat();	
void nosys();	
void nullsys();	
void stty();	
void gtty();	
void nice();	
void sslep();	
void tsleep();	
void sync();	
void kill();	
void getswit();	
void dup();	
void dup2();	
void pipe();	
void times();	
void profil();	
void nosys();	
void setgid();	
void getgid();	
void ssig();	
void utsinfo();	
void reboot();	
void utime();	
void sysnet();	
void sioctl();	
void netopen();	
#endif
