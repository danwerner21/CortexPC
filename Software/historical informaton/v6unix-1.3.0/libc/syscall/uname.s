// Unix V6 system call
//
	.globl	cerror
	uname = 27

	.globl	_uname
_uname:
	sys	uname
	b	@cerror

