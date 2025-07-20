// Unix V6 system call
//

	.globl	cerror
	pipe = 42

	.globl	_pipe
_pipe:
	sys	pipe
	jnc	1f
	b	@cerror
1:
	mov	(sp), r2
	mov	r0,(r2)+
	mov	r1,(r2)
	clr	r2
	b	(r11)

