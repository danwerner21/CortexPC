// Unix V6 system call
// C interface: pid = fork()
// AS interface:
//	for the parent, child pid in r0, one word skipped after sys call
//	for the child, parent pid in r0, returns normally
//
	.globl	cerror
	fork = 2

	.globl	_fork, _par_uid
_fork:
	sys	fork
	jmp	1f		// child
	jnc	2f		// parent
	b	@cerror
1:
	mov	r0,@_par_uid
	clr	r0
2:
	mov	r0,r2
	b	(r11)

	.bss
_par_uid:	.=.+2

