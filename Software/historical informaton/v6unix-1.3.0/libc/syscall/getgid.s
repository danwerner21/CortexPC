// Unix V6 system call
//
	.globl	cerror
	getgid = 47

	.globl	_getgid
_getgid:
	sys	getgid
	b	@cerror

