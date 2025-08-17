// Unix V6 system call
//
	.globl	cerror
	chdir = 12

	.globl	_chdir
_chdir:
	sys	chdir
	b	@cerror

