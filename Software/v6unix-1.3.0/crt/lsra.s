// C run time library

	.globl	lsra
lsra:	andi	r0,31
	jeq	L3
L2:	srl	r3,1
	sra	r2,1
	jnc	L1
	ori	r3,0x8000
L1:	dec	r0
	jne	L2
L3:	b	(r11)

