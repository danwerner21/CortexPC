// Unix V6 system call
//
	.globl	cerror
	setgid = 46

	.globl	_setgid
_setgid:
	sys	setgid
	b	@cerror

