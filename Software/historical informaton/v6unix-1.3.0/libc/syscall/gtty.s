// Unix V6 system call
//
	.globl	cerror
	gtty = 32

	.globl	_gtty
_gtty:
	sys	gtty
	b	@cerror

