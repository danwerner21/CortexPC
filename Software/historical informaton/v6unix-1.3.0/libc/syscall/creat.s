// Unix V6 system call
//
	.globl	cerror
	creat = 8

	.globl	_creat
_creat:
	sys	creat
	b	@cerror

