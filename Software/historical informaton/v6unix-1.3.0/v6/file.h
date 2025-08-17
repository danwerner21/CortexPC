/*
 * One file structure is allocated
 * for each open/creat/pipe call.
 * Main use is to hold the read/write
 * pointer associated with each open
 * file.
 */
struct	file
{
	char		f_flag;
	char		f_count;	/* reference count */
	char		f_type;
	char		f_proto;
	struct inode*	f_inode;	/* pointer to inode structure */
	unsigned long	f_offset;	/* read/write character pointer */
} file[NFILE];

/* flags */
#define	FREAD	1
#define	FWRITE	2
#define	FPIPE	4
#define	FNET	8

/* file function prototypes */
#ifdef KERNEL
struct inode* owner();
int suser();
struct file *getf();
struct file *falloc();
int ufalloc();
void closef();
#endif
