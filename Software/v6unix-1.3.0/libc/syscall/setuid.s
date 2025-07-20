// Unix V6 system call
//
	.globl	cerror
	setuid = 23

	.globl	_setuid
_setuid:
	sys	setuid
	b	@cerror

