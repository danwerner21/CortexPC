// Unix V6 system call
//
	.globl	cerror
	open = 5

	.globl	_open
_open:
	sys	open
	b	@cerror

