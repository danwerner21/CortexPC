// Unix V6 system call
//
	.globl	cerror, _end
	break = 17

	.globl	_sbrk
_sbrk:
	a	@nd,(sp)
	sys	break
	joc	1f
	mov	@nd,r2
	mov	(sp),@nd
	b	(r11)
1:
	b	@cerror

	.globl	_brk
_brk:
	sys	break
	joc	1b
	mov	@nd,r2
	mov	(sp),@nd
	b	(r11)

	.data
nd:	_end

