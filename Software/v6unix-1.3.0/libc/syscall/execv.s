// Unix V6 system call
//
// execv(file, argv);
//
// where argv is a vector argv[0] ... argv[x], 0
// last vector element must be 0
//
// The _exectrap flags is used by the debugger and causes
// a trace trap on the first instruction of the executed program
// to give a chance to set breakpoints.
//
// The 'lrex' instruction causes a trap after the 'sys' instruction,
// which then causes the kernel to issue another 'lrex' just before
// returning to the executable.

	.globl	_execv, cerror
	.comm	__exectrap,2
	execv = 11

	.globl	_execv
_execv:
	mov	@__exectrap,r0
	jeq	1f
	lrex
1:	sys	execv
	b	@cerror

