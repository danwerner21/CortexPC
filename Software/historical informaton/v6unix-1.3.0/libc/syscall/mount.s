// Unix V6 system call
//
	.globl	cerror
	mount  = 21
	umount = 22

	.globl	_mount
_mount:
	sys	mount
	b	@cerror

	.globl	_umount
_umount:
	sys	umount
	b	@cerror
