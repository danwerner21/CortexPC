// Unix V6 system call
//

	.globl	cerror
	wait = 7

	.globl	_wait
_wait:
	sys	wait
	jnc	1f
	b	@cerror
1:
	mov	(sp),r2
	jeq	1f
	mov	r1,(r2)
1:	mov	r0,r2
	b	(r11)

