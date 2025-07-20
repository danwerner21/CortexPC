// Unix V6 system call
// cerror is the common end of nearly all system calls.
// It converts from the AS error signal (carry flag set,
// error number is r0), to the C error signal (return -1,
// error number in global variable errno).

	.comm	_errno,2

	.globl	cerror
cerror:
	joc	1f
	mov	r0,r2
	b	(r11)
1:	mov	r0,@_errno
	li	r2,-1
	b	(r11)

