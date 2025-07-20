// Unix V6 system call
//
	.globl	cerror
	sleep = 35

	.globl	_sleep
_sleep:
	sys	sleep
	b	@cerror

