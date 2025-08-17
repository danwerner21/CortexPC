// Unix V6 system call
//
	.globl	cerror
	sync = 36

	.globl	_sync
_sync:
	sys	sync
	b	@cerror

