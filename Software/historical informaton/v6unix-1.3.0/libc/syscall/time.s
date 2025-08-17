// Unix V6 system call
//
	.globl	cerror
	time = 13

	.globl	_time
_time:
	sys	time
	b	@cerror

