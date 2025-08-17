// Unix V6 system call
//
	.globl	cerror
	nice = 34

	.globl	_nice
_nice:
	sys	nice
	b	@cerror

