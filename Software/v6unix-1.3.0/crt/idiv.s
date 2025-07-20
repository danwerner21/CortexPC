// C Runtime -- perform signed int divide and modulo
// The compiler places the lhs in r2 and the rhs in r3
// and expects the result in r2

	.globl	idiv
idiv:
	mov	r2,r1
	mov	r2,r0
	sra	r0,15
.if EIS
	divs	r3
.endif
.if 1-EIS
	div	r3,r0
.endif
	mov	r0,r2
	b	(r11)

	.globl	irem
irem:
	mov	r2,r1
	mov	r2,r0
	sra	r0,15
.if EIS
	divs	r3
.endif
.if 1-EIS
	div	r3,r0
.endif
	mov	r1,r2
	b	(r11)

