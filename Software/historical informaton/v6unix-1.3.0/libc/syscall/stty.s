// Unix V6 system call
//
	.globl	cerror
	stty = 31

	.globl	_stty
_stty:
	sys	stty
	b	@cerror

