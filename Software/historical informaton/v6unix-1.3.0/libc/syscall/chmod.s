// Unix V6 system call
//
	.globl	cerror
	chmod = 15

	.globl	_chmod
_chmod:
	sys	chmod
	joc	1f
	li	r0,0
1:	b	@cerror

