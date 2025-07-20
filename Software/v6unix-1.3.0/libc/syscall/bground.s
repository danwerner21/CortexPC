// LSX special system call
//
	.globl	cerror
	bground = 62

	.globl	_bground
_bground:
	sys	bground
	b	@cerror

