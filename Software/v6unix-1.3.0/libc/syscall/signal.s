// Unix V6 system call
//
//	signal(n, 0);		-- default action on signal(n)
//	signal(n, odd);		-- ignore signal(n)
//	signal(n, label);	-- goto label on signal(n)
//
//	returns old label, only one level.
//
	NSIG   = 20
	signal = 48
	indir  = 0

	.comm	__sigvec, 40
	.comm	__twp, 32

	.globl	cerror

	.globl	_signal
_signal:
	mov	(sp),r1			// 'n' in range?
	ci	r1,NSIG
	jlt	1f
	li	r0,45			// EINVAL<<1 + 1
	sra	r0,1			// r0 = EIVAL, set carry
	b	@cerror

1:	mov	r1,@sig2
	sla	r1,1
	mov	@__sigvec(r1),r2	// old vector to r2
	mov	@2(sp),r0
	mov	r0,@__sigvec(r1)	// new vector
	mov	r0,@sig3
	jeq	1f			// if 0 or odd, use it
	andi	r0,1
	jne	1f
	mov	r1,r0			// else substitute tvect entry
	sla	r1,1
	a	r1,r0
	ai	r0,tvect
	mov	r0,@sig3
1:	li	r0,sig1			// indirect syscall to 'signal'
	sys	indir
	jnc	1f
	b	@cerror

1:	coc	@one,r0			// if old vector was odd, use that value
	jeq	1f
	mov	r0,r2
1:	b	(r11)

one:	1

tvect:
	mov 	@__sigvec+ 0,r1; jmp 1f;
	mov 	@__sigvec+ 2,r1; jmp 1f;
	mov 	@__sigvec+ 4,r1; jmp 1f;
	mov 	@__sigvec+ 6,r1; jmp 1f;
	mov 	@__sigvec+ 8,r1; jmp 1f;
	mov 	@__sigvec+10,r1; jmp 1f;
	mov 	@__sigvec+12,r1; jmp 1f;
	mov 	@__sigvec+14,r1; jmp 1f;
	mov 	@__sigvec+16,r1; jmp 1f;
	mov 	@__sigvec+18,r1; jmp 1f;
	mov 	@__sigvec+20,r1; jmp 1f;
	mov 	@__sigvec+22,r1; jmp 1f;
	mov 	@__sigvec+24,r1; jmp 1f;
	mov 	@__sigvec+26,r1; jmp 1f;
	mov 	@__sigvec+28,r1; jmp 1f;
	mov 	@__sigvec+30,r1; jmp 1f;
	mov 	@__sigvec+32,r1; jmp 1f;
	mov 	@__sigvec+34,r1; jmp 1f;
	mov 	@__sigvec+36,r1; jmp 1f;
	mov 	@__sigvec+38,r1; jmp 1f;
1:
	// note: r0,r1 saved by 'signal' syscall
	bl	(r1)
	mov	(sp)+,r1
	mov	(sp)+,r0
	limi	0
	mov	(sp)+,@__twp+26
	mov	(sp)+,@__twp+28
	mov	(sp)+,@__twp+30
	lwpi	__twp
	rtwp

	.data
sig1:	signal
sig2:	0
sig3:	0
