// Unix V6 system call
//
	.globl	cerror
	stime = 25

	.globl	_stime
_stime:
	sys	stime
	joc	1f
	li	r0,0
1:	b	@cerror

