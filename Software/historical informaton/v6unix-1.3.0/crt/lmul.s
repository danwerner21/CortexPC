// C run time library

	.globl	lmul
lmul:	mov     @2(sp),r2
        mov     @6(sp),r3
        mov     (sp),r0
        mpy     r3,r0
        mov     r1,r13
        mov     @4(sp),r0
        mpy     r2,r0
        a       r1,r13
        mpy     r3,r2
        a       r13,r2
        b       (r11)

