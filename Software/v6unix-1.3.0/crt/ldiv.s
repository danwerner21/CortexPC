// C run time library

// Signed division for longs, args are on the stack, result in r2,r3
// Signed & unsigned are the same if the divisor is positive, so
// negate the divisor (and remainder) if the divisor is negative

	.globl	ldiv
ldiv:
	mov	(sp)+,r2
	mov	(sp)+,r3
	mov	(sp)+,r4
	mov	(sp)+,r5
	ai	sp,-8

// Perform 32 bit division: r2,r3 / r4,r5 => r2,r3 remainder in r0,r1
idiv32:
	mov	r4,r4
	jlt	neg		// negate divisor if needed
	jne	L1		// if divisor == 0 return
	mov	r5,r5
	jeq	L9

L1:	mov	r2,r0
	sra	r0,15
	mov	r0,r1
	li	r12,32

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

L9:	b	(r11)

neg:	neg	r4
	neg	r5
	joc	L1
	dec	r4
	jmp	L1
