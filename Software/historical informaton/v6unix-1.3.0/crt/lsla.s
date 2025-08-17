// C run time library

	.globl	lsla
lsla:	andi	r0,31
	jeq	3f
1:	sla	r2,1
	sla	r3,1
	jnc	2f
	inc	r2
2:	dec	r0
	jne	1b
3:	b	(r11)

