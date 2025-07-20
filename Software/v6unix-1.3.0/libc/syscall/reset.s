// Unix V6 system call
// setexit() and reset() were forerunners of setjmp/longjmp.
// Not really system calls, but they might as well go here.
//

	.globl	_setexit
	.globl	_reset
	.globl	csv, cret

_setexit:
	mov	r11,r0
	bl	@csv
	mov	bp,@sbp
	mov	@16(bp),@spc //20
	b	@cret

_reset:
	mov	@sbp,bp
	mov	@spc,@16(bp) //20
	b	@cret

	.bss
sbp:	.=.+2
spc:	.=.+2

