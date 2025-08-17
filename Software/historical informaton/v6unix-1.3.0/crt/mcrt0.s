// C runtime startoff including monitoring

	cbufs	= 150
	write	= 4
	indir	= 0

	.globl	_monitor
	.globl	_sbrk
	.globl	_main
	.globl	_etext
	.globl	__profexit
	.comm	countbase,2

load:
	0;0;0;0;0;0;0;0		// relic header

start:
	dect	sp
	
	// calculate buffer size for poling & counting
	li	r4,_etext	// poling buf is 1/8th of text segment size
	li	r5,stext
	s	r5,r4
	ai	r4,7
	srl	r4,3
	ai	r4,3*cbufs+3	// add space for cbufs+1 function counters

	// call sbrk to reserve monitor buffer space
	mov	r4,r0		// convert word size to byte size
	sla	r0,1
	mov	r0,(sp)
	bl	@_sbrk
	ci	r2,-1		// if no space, exit with error message
	jeq	out

	// call monitor(stext, _etext, buf, bufsiz, cntsize)
	li	r0,cbufs	// push cntsiz
	mov	r0,(sp)
	dect	sp
	mov	r4,(sp)		// push bufsiz (in words)
	dect	sp
	mov	r2,(sp)		// push buf
	ai	r2,6		// reserve first buffer (3 words)
	mov	r2,@countbase
	li	r0,_etext	// push _etext
	dect	sp
	mov	r0,(sp)
	li	r0,stext	// push stext
	dect	sp
	mov	r0,(sp)
	bl	@_monitor
	ai	sp,10

	li	r0,monexit	// register exit func
	mov	r0,@__profexit

	bl	@_main		// regular calls to main & exit
	inct	sp
	mov	r2,(sp)
	bl	@_exit
	// no return

out:
	// write error message to stderr and exit
	li	r0,errwrt
	sys	indir
	clr	(sp)
	bl	@_exit
	// no return

monexit:
	// call monitor(0) -- write "mon.out" file
	dect	sp
	mov	r11,(sp)
	dect	sp
	clr	(sp)
	bl	@_monitor
	inct	sp
	mov	(sp)+,r0
	b	(r0)

errwrt:
	write; 2; 1f ; 29
1:	"No space for monitor buffer\n"
	.even

one:	1

	.globl	mcount
mcount:
	mov	(r0),r1
	coc	@one,r1
	jne	1f
	mov	r1,r3
	andi	r3,0xfffe
	mov	@countbase,r1
	jeq	2f
	li	r2,6
	a	r2,@countbase
	mov	r3,(r1)+
	mov	r1,(r0)
1:
	inc	@2(r1)
	jne	2f
	inc	(r1)
2:
	b	(r11)

stext:
