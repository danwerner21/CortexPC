// int = ludiv(long,int)
// Divide a long by int, yielding int
// Division is unsigned, result must fit
//
	.globl _ludiv
_ludiv:
	mov	(sp),r0
	mov	@2(sp),r1
	div	@4(sp),r0
	mov	r0,r2
	b	(r11)

