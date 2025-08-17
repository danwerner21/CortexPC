// Unix V6 system call
//
	.globl	cerror
	chown = 16

	.globl	_chown
_chown:
	sys	chown
	jnc	1f
	b	@cerror
1:
	clr	r2
	b	(r11)

