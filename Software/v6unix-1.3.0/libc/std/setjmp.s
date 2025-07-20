
.globl	_setjmp
.globl	_longjmp

_setjmp:
	mov	(sp),r2
	mov	bp,(r2)+
	mov	sp,(r2)+
	mov	r11,(r2)
	clr	r2
	b	(r11)

_longjmp:
	mov	(sp),r3
	mov	@2(r5),r2
	jne	1f
	li	r2,1
1:	mov	(r3)+,bp
	mov	(r3)+,sp
	mov	(r3),r11
	b	(r11)

