// Unix V6 system call
//
	.globl	cerror
	getuid = 24

	.globl	_getuid
_getuid:
	sys	getuid
	b	@cerror

