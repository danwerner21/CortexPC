// Unix V6 system call
//
	.globl	cerror
	dup2 = 40

	.globl	_dup2
_dup2:
	sys	dup2
	b	@cerror

