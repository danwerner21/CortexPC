// Unix V6 system call
//
	.globl	cerror
	getpid = 20

	.globl	_getpid
_getpid:
	sys	getpid
	b	@cerror

