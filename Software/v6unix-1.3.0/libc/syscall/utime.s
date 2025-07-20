// Unix V6 system call
//
	.globl	cerror
	utime = 30

	.globl	_utime
_utime:
	sys	utime
	joc	1f
	li	r0,0
1:	b	@cerror

