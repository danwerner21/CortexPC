// C run time library:  convert unsigned long to float

// This has been only partially ported, pending a choice on the FP
// instruction set to use. As neither the 9995 or the 99105 have
// hardware FP, multiple routes are conceivable.

	twogig = 050000

	.globl	ultof
ultof:
	setl
	mov	@2(sp),r0
	bjt	L1
	movif	@2(sp),fr0
	seti
	b	(r11)

L1:	andi	r0, 0x7fff
	mov	r0,@2(sp)
	movif	@2(sp),fr0
	addf	$twogig,fr0
	seti
	b	(r11)
