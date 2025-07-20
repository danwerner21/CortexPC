// Unix V6 system call
//
	.globl	cerror
	ptrace = 26

	.globl	_ptrace,_errno
_ptrace:
	clr	@_errno
	sys	ptrace
	b	@cerror

