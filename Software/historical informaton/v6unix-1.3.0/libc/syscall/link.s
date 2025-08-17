// Unix V6 system call
//
	.globl	cerror
	link = 9

	.globl	_link
_link:
	sys	link
	joc	1f
	li	r0,0
1:	b	@cerror

