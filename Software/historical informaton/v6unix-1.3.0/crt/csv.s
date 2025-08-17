// C run time library

	.globl	csv, _end
csv:
	ai	sp,-18 // -22
	mov	sp,r1
	ci	sp,_end
	jl	overflow
	mov	r8,(r1)+
	mov	r7,(r1)+
	mov	r6,(r1)+
	mov	r5,(r1)+
	mov	r4,(r1)+
	mov	r3,(r1)+
	mov	r2,(r1)+
	mov	bp,(r1)+
	mov	r0,(r1)+
	mov	sp,bp
	dect	sp
	b	(r11)

overflow:
	b	@sysdie

	.globl	cret
cret:
	mov	bp,sp
	mov	(sp)+,r8
	mov	(sp)+,r7
	mov	(sp)+,r6
	mov	(sp)+,r5
	mov	(sp)+,r4
	mov	(sp)+,r11
	mov	(sp)+,r11
	mov	(sp)+,bp
	mov	(sp)+,r11
	b	(r11)

	.globl	sext
sext:
	seto	r0
	jlt	1f
	clr	r0
1:	b	(r11)

