//
// netopen() system call interface
//
	.globl	cerror, _netopen
	netopen = 50

_netopen:
	sys	netopen
	b	@cerror


