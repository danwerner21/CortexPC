// Unix V6 system call
//
	.globl	cerror
	kill = 37

	.globl	_kill
_kill:
	sys	kill
	b	@cerror

