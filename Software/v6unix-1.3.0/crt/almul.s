// C run time library:  *= for (unsigned) longs

	.globl	almul
almul:
	mov	(sp),r12
	mov     @2(r12),r2
        mov     @4(sp),r3
        mov     (r12),r0
        mpy     r3,r0
        mov     r1,r13
        mov     @2(sp),r0
        mpy     r2,r0
        a       r1,r13
        mpy     r3,r2
        a       r13,r2
	mov	r2,(r12)
	mov	r3,@2(r12)
        b       (r11)

