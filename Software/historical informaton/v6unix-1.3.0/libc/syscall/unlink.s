// Unix V6 system call
//
	.globl	cerror
	unlink = 10

	.globl	_unlink
_unlink:
	sys	unlink
	joc	1f
	li	r0,0
1:	b	@cerror

