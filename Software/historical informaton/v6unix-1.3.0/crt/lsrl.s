// C run time library

	.globl	lsrl
lsrl:	andi	r0,31
	jeq	L6
L5:	srl	r3,1
	srl	r2,1
	jnc	L4
	ori	r3,0x8000
L4:	dec	r0
	jne	L5
L6:	b	(r11)

