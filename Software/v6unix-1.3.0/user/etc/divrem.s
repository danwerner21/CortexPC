.globl	_ldiv
.globl	_lrem

_ldiv:
	mov	(sp),r0
	mov	@2(sp),r1
	div	@4(sp),r0
	mov	r0,r2
	b	(r11)

_lrem:
	mov	(sp),r0
	mov	@2(sp),r1
	div	@4(sp),r0
	mov	r1,r2
	b	(r11)
