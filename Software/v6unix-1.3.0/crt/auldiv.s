// C run time library
// Unsigned division for longs, args are on the stack, result in r2,r3

	.globl	auldiv
auldiv:
	mov	(sp)+,r13
	mov	(r13),r2
	mov	@2(r13),r3
	mov	(sp)+,r4
	mov	(sp)+,r5
	ai	sp,-6

// Perform unsigned 32 bit division: r2,r3 / r4,r5 => r2,r3 remainder in r0,r1
udiv32:
	mov	r4,r4		// if divisor == 0 return
	jne	L1
	mov	r5,r5
	jeq	L9

L1:	clr	r0		// extend r2,r3 into r0,r1
	clr	r1
	li	r12,32		// loop count in r12

L2:	sla	r0,1		// shift 64 bits r0,r1,r2,r3 left one
	sla	r1,1
	jnc	L3
	inc	r0
L3:	sla	r2,1
	jnc	L4
	inc	r1
L4:	sla	r3,1
	jnc	L5
	inc	r2

L5:	c	r0,r4		// is divisor >= shifted dividend
	jh	L6
	jl	L8
	c	r1,r5
	jl	L8

L6:	s	r5,r1		// if so: subtract divisor, shift 1 into result
	joc	L7
	dec	r0
L7:	s	r4,r0
	inc	r3

L8:	dec	r12		// loop over all 32 bits
	jgt	L2

L9:	mov	r2,(r13)	// assign result
	mov	r3,@2(r13)
	b	(r11)
