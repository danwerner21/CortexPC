
struct netrequest {
	struct netrequest *n_next;
	struct file *n_file;
	struct ucb *n_ucb;
	struct tcb *n_tcb;
	int	n_proc;
	int	n_pid;
	int	n_request;
	int	n_slot;
	int	n_error;
	int	n_retval;
	int	n_usrrequest;
	int	n_usrfd;
	char	*n_buffer;
	int	n_len;
	int     n_count;
	int     n_eol;
	int     n_datawait;
};

#define SYSOPEN		1
#define SYSCLOSE	2
#define SYSREAD		3
#define SYSWRITE	4
#define SYSIOCTL	5
#define SYSRECVFROM	6
#define SYSSENDTO	7

#define SYSINIT		1
#define SYSSLEEP	2
#define SYSWAKEUP	3
#define SYSREQUEST	4
#define SYSCOPYIN	5
#define SYSCOPYOUT	6
#define SYSREPLY	7
