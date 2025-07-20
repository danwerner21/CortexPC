// Unix V6 system call
//
	.globl	cerror
	fstat = 28

	.globl	_fstat
_fstat:
	sys	fstat
	b	@cerror

