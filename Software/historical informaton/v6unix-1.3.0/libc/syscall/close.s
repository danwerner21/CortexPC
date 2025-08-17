// Unix V6 system call
//
	.globl	cerror
	close = 6

	.globl	_close
_close:
	sys	close
	b	@cerror

