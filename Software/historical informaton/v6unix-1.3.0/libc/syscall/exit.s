// Unix V6 system call
// C interface: exit(code)
// AS interface: exit code in (sp), does not return
//
	.globl	cerror
	exit = 1

	.globl	__exit
__exit:
	sys	exit
	// does not return

	.bss
	.globl	__exitfunc, __profexit
__exitfunc:
	.=.+2
__profexit:
	.=.+2

	.text
	.globl	_exit
	.text
_exit:
	mov	r11,r0
	bl	@csv
	mov	@__exitfunc,r0
	jeq	1f
	bl	(r0)
1:	mov	@__profexit,r0
	jeq	1f
	bl	(r0)
1:
	mov	@18(bp),(sp)
	sys	exit
	// does not return

