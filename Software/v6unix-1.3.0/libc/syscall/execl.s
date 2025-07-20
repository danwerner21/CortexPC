// Unix V6 system call
//
	.globl	cerror
	execl = 11

	.globl	_execl
_execl:
	ai	sp,-4
	mov	@4(sp),(sp)
	mov	sp,r0
	ai	r0,6
	mov	r0,@2(sp)
	sys	execl
	joc	1f
	ai	sp,4
	jmp	2f

1:	ai	sp,4
	seto	r1
	inc	r1
2:	b	@cerror

