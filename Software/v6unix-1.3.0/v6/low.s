// Low memory for the kernel:
// - Interrupt and XOP vectors
// - Startup initialisation
// - Interrupt and system call interface to C code
//

	.globl _u,_main,_edata,start
	.globl _ttyinput,_trap,_clock,_rootdev
	.globl _runrun,_swtch

	_u     = 0xe000
	usize  = 0x0ff0
	topsys = _u + usize

	TRAP   = 0x0040
	ERRTRP = 0x0080
	USER   = 0x0100
	DLYTRP = 0x0200


// Define workspace areas
//
	rwp	= 0x0080
	cwp	= 0xf000
	iwp	= 0xf020
	dwp	= 0xf300

	syswp	= 0xf060  // syscall ws
	clkwp	= 0xf0a0  // clock ws
	ttywp	= 0xf0e0  // tty ws
	dskwp	= 0xf120  // disk ws
	tapwp	= 0xf160  // tape ws
	c43wp	= 0xf1a0  // CI403 ws
	prtwp	= 0xf1e0  // printer ws
	v0wp	= 0xf220  // 911 VDT 0 ws
	v1wp	= 0xf260  // 911 VDT 1 ws

	clkstk	= topsys-0x200	// clock stack
	ttystk	= topsys-0x300  // tty stack
	dskstk	= topsys-0x400  // disk stack
	tapstk	= topsys-0x500  // tape stack
	c43stk	= topsys-0x600  // CI403 stack
	prtstk	= topsys-0x700  // printer stack
	v0stk	= topsys-0x800  // 911 VDT 0 stack
	v1stk	= topsys-0x900  // 911 VDT 1 stack


// *************************
// Interrupt and XOP vectors
// *************************

intbase = .
.if CORTEX
	cwp; start	// int 0 = reset
.endif
.if TI990_12 + TI990_10A + TI990
	rwp; start	// int 0 = reset, start with ROM WS.
.endif
	iwp; pwrint	// int 1 = power failing.
	iwp; errint	// int 2 = error int, illegal op, etc.
	iwp; noop	// int 3
.if CORTEX
	ttyintval = .-intbase/4-1
	ttywp; tty0int	// int 4 = CORTEX 9902 tty0 int
.endif
.if 1-CORTEX
	iwp; noop	// int 4
.endif
	clkintval = .-intbase/4-1
	clkwp; clock	// int 5 = clock int
.if TTYEIA
	ttyintval = .-intbase/4-1
	ttywp; tty0int	// int 6 = EIA board tty0 int
.endif
.if 1-TTYEIA
	iwp; noop	// int 6
.endif
	iwp; noop	// int 7
.if TI990_10A
.if TTY9902
	ttyintval = .-intbase/4-1
	ttywp; tty0int	// int 8 = 9902 ACA tty0 int
.endif
.if 1-TTY9902
	iwp; noop	// int 8
.endif
.endif
.if 1-TI990_10A
.if VDT911
	v1intval = .-intbase/4-1
	v1wp; v1int	// int 8 = VDT 911 tty2 interrupt
.endif
.if 1-VDT911
	iwp; noop	// int 8
.endif
.endif
.if CORTEX
	iwp; noop	// int 9
.endif
.if TI990_12 + TI990_10A + TI990
	tapintval = .-intbase/4-1
	tapwp; tapint	// int 9 = tape interrupt
.endif
.if VDT911
	v0intval = .-intbase/4-1
	v0wp; v0int	// int 10 = VDT 911 tty1 interrupt
.endif
.if 1-VDT911
	iwp; noop	// int 10
.endif
.if TTY403
	c43intval = .-intbase/4-1
	c43wp; c4030int	// int 11 = CI403 tty[3-6] interrupt
.endif
.if 1-TTY403
	iwp; noop	// int 11
.endif
	iwp; noop	// int 12
.if CORTEX
	iwp; noop	// int 13
.endif
.if TI990_12 + TI990_10A + TI990
	dskintval = .-intbase/4-1
	dskwp; dskint	// int 13 = disk interrupt
.endif
.if CORTEX
	iwp; noop	// int 14
.endif
.if TI990_12 + TI990_10A + TI990
	prtintval = .-intbase/4-1
	prtwp; prtint	// int 14 = printer
.endif
	iwp; noop	// int 15

// XOP vectors
//
	iwp; bpt	// xop 0 = breakpoint
	syswp; syscal	// xop 1 = system trap
	iwp; noop	// xop 2
	iwp; noop	// xop 3
	iwp; noop	// xop 4
	iwp; noop	// xop 5
	iwp; noop	// xop 6
	iwp; noop	// xop 7
	iwp; noop	// xop 8
	iwp; noop	// xop 9
	iwp; noop	// xop 10
	iwp; noop	// xop 11
	iwp; noop	// xop 12
	iwp; noop	// xop 13
	iwp; noop	// xop 14
	iwp; noop	// xop 15

.if TI990_12 + TI990_10A + TI990
	.=.+32		// skip over ROM workspace.
.endif

	.comm	_user,2		// flag for previous mode

// ***************************
// Startup initialisation code
// ***************************

start:
	clr	@_rootdev		// Initialize for dsk0

.if TI990_12 + TI990_10A + TI990
	// Save boot disk information left behind by boot loader
	// This is later used in the TILINE disk driver (drv_990.s)
	//
	mov	r1,@_diskaddr		// Disk device address.
	mov	r2,@_diskunit		// booted unit.
	mov	r3,@_diskheads		// Disk heads.
	mov	r4,@_disksectrk		// Disk sectors/track.
	mov	r5,@_mulsec		// Multi-sector mode switch.
	mov	r2,r6
	srl	r6,9			// Set up boot unit minor number.
	movb	@units(r6),@_rootdev
.endif

	lwpi	cwp		// Switch to C workspace
	li	r0,_edata	// Clear kernel bss area and user block
1:	clr	(r0)+
	ci	r0,0xf000
	jl	1b
.if 0
	li	r0,iwp		// Clear WS areas
1:	clr	(r0)+
	ci	r0,0xf700
	jl	1b
.endif

	// Set up stack 
	li	sp,topsys-2
.if CORTEX
	// Set LOAD/NMI vector to trap handler
	li	r0,dwp
	mov	r0,@0xfffc
	li	r0,nmi
	mov	r0,@0xfffe

	li	r0,3		// use 3 for TMS-9995
	mov 	r0,@_cpuid

	// Init mapper with system map
	mov	@_sysmap+ 0,@_mmu+ 0
	mov	@_sysmap+ 2,@_mmu+ 2
	mov	@_sysmap+ 4,@_mmu+ 4
	mov	@_sysmap+ 6,@_mmu+ 6
	mov	@_sysmap+ 8,@_mmu+ 8
	mov	@_sysmap+10,@_mmu+10
	mov	@_sysmap+12,@_mmu+12
	mov	@_sysmap+14,@_mmu+14
.endif

.if TI990_12 + TI990_10A + TI990
	li	r12,_sysmap	// Set default memory maps.
	lmf	r12,0
	li	r12,_usrmap
	lmf	r12,1

	li	r12,errcru	// Get the CPU type ID.
	stcr	r0,0
	andi	r0,0xf
	mov	r0,@_cpuid

.endif
.if TTY9902
	li	r12,tty0addr	// initialize
	sbo	reset
	li	r0,acacnf	// set up 9600-8-N-1
	ldcr	r0,8
	sbz	ldir		// skip setting the timer register
	li	r0,acarat
	ldcr	r0,0
	sbo	rtson
	sbo	rienb
	sbo	xienb
.if TI990_10A
	sbo	acadtr
	sbo	acaenb
	sbo	acaien
.endif
.endif
.if TTYEIA
	li	r12,tty0addr	// initialize
	sbo	ttydtr
	sbo	ttyrts
	sbo	ttyeint
.endif
.if TI990_12 + TI990_10A + TI990
	ckon			// start 120Hz line clock.
.endif

	// Start mapper
	li	r12,mapper
	sbo	mapon

	// Call main and on return, enter user mode at 0x1000
	// (see main.c for code of trampoline)
	clr	@umode
	bl	@_main

	seto	@umode
.if CORTEX 
	li	r12,mapper
	sbz	mapon
	mov	@_mmu+12,@_sysmap+12
	movb	@_mmu+14,@_sysmap+14
	mov	@_usrmap+ 0,@_mmu+ 0
	mov	@_usrmap+ 2,@_mmu+ 2
	mov	@_usrmap+ 4,@_mmu+ 4
	mov	@_usrmap+ 6,@_mmu+ 6
	mov	@_usrmap+ 8,@_mmu+ 8
	mov	@_usrmap+10,@_mmu+10
	mov	@_usrmap+12,@_mmu+12
	mov	@_usrmap+14,@_mmu+14
	sbo	mapon
	limi	15
	b	@0x1000
.endif
.if TI990_12 + TI990_10A + TI990
	blwp	@gouser
.endif


// ***********************************
// Interrupt and system call interface
// ***********************************
	
	// layout of interrupt workspace
	pst = r15	// previous ST
	ppc = r14	// previous PC
	pwp = r13	// previous WP
	xea = r11	// XOP effective address

// Error interrupt, we need to kill off the user program
// or generally cleanup.
errint:

.if TI990_12 + TI990_10A + TI990
	li	xea,ERRTRP
	li	r12,errcru	// Get error status
	stcr	r0,0
	clr	r2		// Clear status
	ldcr	r2,0
.if 0				// set to 1 to stop on an error
	limi	0
	idle			// press run (go) to continue.
.endif
	mov	r0,r2
	andi	r2,0x0010	// Arith overflow.
	jne	aritherr
	mov	r0,r2
	andi	r2,0x9000	// Bus error (non-existant memory, parity).
	jne	buserr
	mov	r0,r2
	andi	r2,0x0B00	// Memory fault (Map error, seg vio.)
	jne	segvio
	mov	r0,r2
	andi	r2,0x6000	// Priv or Illegal inst
	jne	illop

	// Unsupported error interrupt -- die...
	li	r12,panelcru	// Front panel CRU
	ldcr	r0,8		// Display error status
	swpb	r0
	ldcr	r0,8
	swpb	r0
	sbo	11		// Fault LED
.endif

// Power failing interrupt
pwrint:
	limi	0
	idle

// ignore any unused vector calls
noop:
	rtwp

.if TI990_12 + TI990_10A + TI990
buserr:	inc	xea
segvio:	inc	xea
	li	r12,mapper	// Clear map error.
	sbo	mapclr
	sbz	mapclr
illop:	inc	xea
aritherr:
	inc	xea
	jmp	settrp
.endif

// Breakpoint
bpt:
	limi	2
	li	r4,15
	li	xea,USER+TRAP
	jmp	settrp

.if CORTEX
one:	0x0001			// nmi flag

nmi:
	limi	2
	ci	pwp,iwp		// if the nmi was inside an interrupt handler, delay
	jne	1f		// the trap until return from the handler
	inc	@28(pwp)
	rtwp

1:	mov	pst,@iwp+30	// else it is an immediate breakpoint trap
	mov	ppc,@iwp+28
	mov	pwp,@iwp+26
	lwpi	iwp
	li	xea,TRAP
	mov	@umode,r0
	jeq	settrp
	ori	xea,USER
	jmp	settrp
.endif

syscal:
	// system calls always originate from user space, so start a
	// fresh system stack.
	limi	2
	li	r4,15
	andi	xea,0x003f
	ori	xea,USER

settrp:
	li	r0,topsys-14 // -24
	li	r1,_trap

trap:

.if CORTEX
	// handle delayed nmi trap
	coc	@one,ppc	// nmi received in handler?
	jne	1f
	szc	@one,ppc	// clear flag & add delayed trap flag
	ori	xea,DLYTRP
.endif

	// Enter kernel mode
1:	mov	@umode,r2
	jeq	1f
.if CORTEX
	li	r12,mapper
	sbz	mapon
	mov	@_sysmap+ 0,@_mmu+ 0
	mov	@_sysmap+ 2,@_mmu+ 2
	mov	@_sysmap+ 4,@_mmu+ 4
	mov	@_sysmap+ 6,@_mmu+ 6
	mov	@_sysmap+ 8,@_mmu+ 8
	mov	@_sysmap+10,@_mmu+10
	mov	@_sysmap+12,@_mmu+12
	mov	@_sysmap+14,@_mmu+14
	sbo	mapon
.endif
	clr	@umode
1:	mov	r2,@_user

	// Save caller context on the system stack. Note that most
	// C workspace registers will be saved by 'csv' at the start of
	// the _trap or interrupt routine.
	mov	xea,(r0)+	// XOP Effective Address = syscall no.
	mov	pwp,(r0)+	// WP

	ci	pwp,cwp
	jhe	1f
.if CORTEX
	movb	@_usrmap+1,@_mmu+0xd	// WS >1010
	mov	@0xd024,(r0)+		// R10 = sp
	mov	ppc,(r0)+		// PC
	mov	pst,(r0)+		// ST
	mov	@0xd010,(r0)+		// R0
	mov	@0xd012,(r0)+		// R1
	movb	@_sysmap+0xd,@_mmu+0xd
.endif
.if TI990_12 + TI990_10A + TI990
	lds	@_usrmap
	mov	@20(pwp),(r0)+	// R10 = sp
	mov	ppc,(r0)+	// PC
	mov	pst,(r0)+	// ST
	lds	@_usrmap
	mov	@0(pwp),(r0)+	// R0
	lds	@_usrmap
	mov	@2(pwp),(r0)+	// R1
.endif
	jmp	2f

1:	mov	@20(pwp),(r0)+	// R10 = sp
	mov	ppc,(r0)+	// PC
	mov	pst,(r0)+	// ST
	mov	@0(pwp),(r0)+	// R0
	mov	@2(pwp),(r0)+	// R1
2:	ai	r0,-14 // -24

	stwp	r6		// setup C ws 
	ai	r6,32 
	li	r7,cpc		// go run C code
	blwp	r6

	mov	(r0)+,xea	// fetch xea
	mov	(r0)+,pwp	// WP
	ci	pwp,cwp
	jhe	3f
.if CORTEX
	movb	@_usrmap+1,@_mmu+0xd	// WS >1010
	mov	(r0)+,@0xd024		// R10
	mov	(r0)+,ppc		// PC
	mov	(r0)+,pst		// ST
	mov	(r0)+,@0xd010		// R0
	mov	(r0)+,@0xd012		// R1
	movb	@_sysmap+0xd,@_mmu+0xd
.endif
.if TI990_12 + TI990_10A + TI990
	ldd	@_usrmap
	mov	(r0)+,@20(pwp)	// R10
	mov	(r0)+,ppc	// PC
	mov	(r0)+,pst	// ST
	ldd	@_usrmap
	mov	(r0)+,@0(pwp)	// R0
	ldd	@_usrmap
	mov	(r0)+,@2(pwp)	// R1
.endif
	jmp	4f

3:	mov	(r0)+,@20(pwp)	// R10
	mov	(r0)+,ppc	// PC
	mov	(r0)+,pst	// ST
	mov	(r0)+,@0(pwp)	// R0
	mov	(r0)+,@2(pwp)	// R1
4:	mov	xea,r0

	andi	xea,USER	// if caller was user mode, reload user map
	jeq	1f
.if CORTEX
	li	r12,mapper
	sbz	mapon
	mov	@_usrmap+ 0,@_mmu+ 0
	mov	@_usrmap+ 2,@_mmu+ 2
	mov	@_usrmap+ 4,@_mmu+ 4
	mov	@_usrmap+ 6,@_mmu+ 6
	mov	@_usrmap+ 8,@_mmu+ 8
	mov	@_usrmap+10,@_mmu+10
	mov	@_usrmap+12,@_mmu+12
	mov	@_usrmap+14,@_mmu+14
	sbo	mapon
.endif
	seto	@umode

1:	andi	r0,DLYTRP	// if caller issued delayed trap, trap after 1 instruction
	jeq	1f
.if CORTEX
	lrex
.endif
.if TI990_12 + TI990_10A + TI990
.endif
1:	rtwp

.if TI990_12 + TI990_10A + TI990

// Disk interrupt handler.

dskint:
	limi	2
	li	r4,dskintval
	blwp	@_bkintr
	rtwp
	li	r1,_cfintr
	li	r12,dskstk
	b	@int

// Tape interrupt handler.

tapint:
	limi	2
	li	r4,tapintval
	li	r1,_mtintr
	li	r12,tapstk
	b	@int


// Printer interrupt handler (TTYEIA card).

prtint:
	limi	2
	li	r4,prtintval
	li	r12,printcru
	tb	ttywrq		// is an output character needed?
	jne	1f
	sbz	ttywrq		// clear xmit interrupt
	li	r1,_prtint
	jmp	2f
1:	sbz	ttynsf		// clear status change and ignore
	sbz	ttyrrq		// clear receive interrupt and ignore
	rtwp

2:	li	r12,prtstk
	jmp	int
.endif

// TTY interrupt handler.

.if TTY403
c4030int:
	limi	2
	li	r4,c43intval
	clr	xea		// tty3..tty6
c403cmn:
	li	r1,_c403intr
	li	r12,c43stk
	jmp	int2
.endif

.if VDT911
v0int:
	limi	2
	li	r4,v0intval
	li	r12,v0stk
	clr	xea
	jmp	1f
v1int:
	limi	2
	li	r4,v1intval
	li	r12,v1stk
	li	xea,1
1:
	li	r1,_v911rint
	jmp	int2
.endif

tty0int:
	limi	2
	li	r4,ttyintval
	clr	xea		// tty0
ttycmn:
	mov	@_ttyaddrs(xea),r12
	srl	xea,1
.if TTY9902
	tb	rbint		// is an input character waiting?
	jne	1f
	sbz	rienb		// switch reveive int off
	li	r1,_acarint
	jmp	2f
1:	tb	xbint		// is an output character needed?
	jne	1f
	sbz	xienb		// switch xmit int off
	li	r1,_acaxint
	jmp	2f
1:	sbz	timenb		// reset and disable all other 9902 interrupts
	sbz	dscenb
.endif
.if TTYEIA
	tb	ttyrrq		// is an input character waiting?
	jne	1f
	sbz	ttyrrq		// clear receive interrupt
	li	r1,_acarint
	jmp	2f
1:	tb	ttywrq		// is an output character needed?
	jne	1f
	sbz	ttywrq		// clear xmit interrupt
	li	r1,_acaxint
	jmp	2f
1:	sbz	ttynsf		// status change, clear and ignore
.endif
	rtwp

2:	li	r12,ttystk
	jmp	int2

// CLOCK interrupt handler.

clock:
	limi	2
	li	r4,clkintval

.if TI990_12 + TI990_10A + TI990
	// Reset the clock interrupt line
	ckof
	ckon

	// Update the panel lights with the process PC
	li	r12,panelcru
	ldcr	r14,8
	swpb	r14
	ldcr	r14,8
	swpb	r14
.endif

	// Perform profiling at 60Hz
	li	r12,clkstk
	mov	@umode,r0	// only profile user mode
	jeq	1f
	mov	@_uprof+6,r0	// scale = 0 means profiling is off
	jeq	1f
	mov	ppc,r0
	s	@_uprof+4,r0	// i = (pc - base)*(scale/65536)
	mpy	@_uprof+6,r0	
	inc	r0
	sla	r0,1
	c	r0,@_uprof+2	// i > len means ignore
	jhe	1f
	a	@_uprof,r0	// buf[i] += 1
	inc	(r0)

	// Call the clock() routine once per second
1:	li	r1,_clock
	inc	@clkcnt
	mov	@clkcnt,r0
	ci	r0,HZ-1
	jgt	3f

.if VDT911
	// vdt 911 doesn't generate write interrupts, so fake it.
	clr	xea
	li	r1,_v911xint
	andi	r0,1
	jne	1f
	mov	@_v911genint,r0
	jne	int2
1:	inc	xea
	mov	@_v911genint+2,r0
	jne	int2
.endif
	rtwp

3:	clr	@clkcnt

int:
	// Save caller context on the system stack.
	// If caller was in user mode, start a fresh system stack,
	// else continue with the present system stack
	clr	xea
int2:
.if 1-CORTEX
	ci	pwp,cwp
	jhe	1f
	lds	@_usrmap
1:
.endif
	mov	@20(pwp),r0
	mov	@umode,r2
	jeq	1f
	mov	r12,r0
	ori	xea,USER
1:	ai	r0,-14 // -24
	b	@trap

// C code execution
// call the C level routine -- e.g. trap(xea, pwp, psp, ppc, pst)

cpc:
	mov	@0(pwp),sp	// get sp
	mov	@2(pwp),r1	// get func address

.if TI990_12
	mov	@8(pwp),r2
	lim	r2		// set int level
.endif
.if TI990_10A + TI990 + CORTEX
	mov	@8(pwp),@limins+2
limins:	limi	0		// set int level
.endif


1:	bl	(r1)		// call to _trap, _clock, _acarint, etc.

	limi	2
	mov	(sp),r0		// fetch xea, if called from kernel mode
	andi	r0,USER		// just return to caller. Otherwise,
	jeq	3f		// if there is a higher priority process
2:	mov	@_runrun,r0	// (runrun>0), switch to that process now
	jeq	3f		// else return to caller
	bl	@_swtch
	jmp	2b

	// restore the caller context and return to caller
3:	mov	sp,(pwp)
	rtwp

	.globl	_sysmap, _usrmap
.if CORTEX

/* Initial state of kernel and user mode page maps. For the kernel
 * map only pages 12, 13 and 14 change. Page 0 of the user map is
 * always 0x80 (physical page 0, write protected).
 */
_sysmap:
   0x00; 0x08; 0x01; 0x09; 0x02; 0x0a; 0x03; 0x0b
   0x04; 0x0c; 0x05; 0x0d; 0x06; 0x0e; 0x07; 0x0f

_usrmap:
   0x00; 0x11; 0x12; 0x13; 0x14; 0x15; 0x16; 0x17
   0x18; 0x19; 0x1a; 0x1b; 0x1c; 0x1d; 0x1e; 0x0f

.endif
.if 1-CORTEX

_sysmap:
   0x2000	// L1 57K - kernel
   0x0000	// B1
   0x1000	// L2 4K - _u/swapper
   0x0000	// B2  changes with slot
   0x0000	// L3 4K  - WS + TILINE
   0x0000	// B3

_usrmap:
   0xF002	// L1 4K - _u
   0x0000	// B1
   0x1000	// L2 57K - user code
   0x0000	// B2  changes with slot
   0x0802	// L3 4K  - WS - NO TILINE
   0x0000	// B3

.endif

// these must be located <0x1000, so that they remain accessible
// with the map active.
//
	.globl _uprof
umode:		.=.+2			// flag for current mode
clkcnt:		.=.+2			// count out 1 second
_uprof:		.=.+8			// copy of u.u_prof for current process
_rootdev:	.=.+2			// root file system minor/major
	.globl _cpuid
_cpuid:		.=.+2

	.globl _ttyaddrs
// tty device addrs
_ttyaddrs:	tty0addr;tty1addr;tty2addr;tty3addr

.if TTY403
	.globl _c403addrs
// CI403 device addrs
_c403addrs:	c4030addr;c4031addr;c4032addr;c4033addr
.endif

.if VDT911
	.globl _v911addrs,_v911genint
// 911 VDT device addrs
_v911addrs:	v9110addr;v9111addr
_v911genint:	0;0
.endif

.if TI990_12 + TI990_10A + TI990
// disk unit/minor number translation table.
units:		0x3020;0x1000;0x0000


// These CAN NOT be in the bss area as it is cleared on start!!!
	.globl _diskaddr,_diskunit,_diskheads,_disksectrk,_mulsec
_diskaddr:	.=.+2
_diskunit:	.=.+2
_diskheads:	.=.+2
_disksectrk:	.=.+2
_mulsec:	.=.+2
.endif
