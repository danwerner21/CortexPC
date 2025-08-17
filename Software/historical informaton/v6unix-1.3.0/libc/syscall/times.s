// Unix V6 system call
//
	.globl	cerror
	times = 43

	.globl	_times
_times:
	sys	times
	b	@cerror

