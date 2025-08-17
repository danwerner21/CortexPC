// Unix V6 system call
//
	.globl	cerror
	mknod = 14

	.globl	_mknod
_mknod:
	sys	mknod
	b	@cerror

