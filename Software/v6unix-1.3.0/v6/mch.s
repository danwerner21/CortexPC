// Assembly language assistance routines, dealing with
// machine dependent hardware details.
//


// Emulate 'lst r0' instruction on machines without EIS
//

.if 1-EIS
	.globl	lstr0
lstr0:
	lstr0w-26; lstr0c

lstr0c:
	mov	(r13),r15
	rtwp

	.bss
lstr0w:
	.=.+6
	.text

lstr12:
	lstr12w-26; lstr12c

lstr12c:
	mov	@24(r13),r15
	rtwp
	.bss
lstr12w:
	.=.+6
	.text
.endif

// Setup the user mode of operation.
//

.if 1-CORTEX
	.globl	gouser
gouser:
	gousrw-26; gousrc
gousrc:
	li	r14,0x1000
	stst	r15
	ori	r15,0x01CF	// Enable user map and non-priv.
	rtwp
	.bss
gousrw:
	.=.+6
	.text
.endif

// val = stcr16(base)
// Read 16 bit CRU value starting at 'base'
//
	.globl _stcr16
_stcr16:
	mov	(sp),r12
	stcr	r2,0
	b	(r11)

// val = stcr8(base)
// Read 8 bit CRU value starting at 'base'
//
	.globl _stcr8
_stcr8:
	mov	(sp),r12
	clr	r2
	stcr	r2,8
	swpb	r2
	b	(r11)

// ldcr(base,wid,val)
// Write 'wid' bit value 'val' to CRU starting at 'base'
//
	.globl _ldcr
_ldcr:
	mov	@4(sp),r2
	mov	(sp),r12
	mov	@2(sp),r3
	andi	r3,0xf
	sla	r3,6
	ori	r3,0x3002
	x	r3
	b	(r11)

// ldcr8(base,val)
// Write 8 bit value 'val' to CRU starting at 'base'
//
	.globl _ldcr8
_ldcr8:
	mov	@2(sp),r2
	mov	(sp),r12
	swpb	r2
	ldcr	r2,8
	b	(r11)

// bit = tb(base,offset)
// Test CRU bit at 'base'+'offset'
//
	.globl _tb
_tb:
	mov	@2(sp),r3
	mov	(sp),r12
	clr	r2
	andi	r3,0x00ff
	ori	r3,0x1f00	// ori'tb'
	x	r3
	jne	1f
	inc	r2		// return 1 if bit set, 0 if not set
1:	b	(r11)

// sbo(base, offset) and sbz(base,offset)
// Set or reset the CRU bit at 'base'+'offset'
//
	.globl _sbz, _sbo
_sbo:
	mov	@2(sp),r3
	andi	r3,0x00ff
	ori	r3,0x1d00	// ori'sbo'
	jmp	1f
_sbz:
	mov	@2(sp),r3
	andi	r3,0x00ff
	ori	r3,0x1e00	// ori 'sbz'
1:	mov	(sp),r12
	x	r3
	b	(r11)

// idle()
// Release bus and wait for next interrupt
//
	.globl	_idle
_idle:
	stst	r0
	limi	15	
	idle
	limi	2
.if 1-EIS
	blwp	@lstr0
.endif
.if EIS
	lst	r0
.endif
	b	(r11)

// kputc(c)
// Send a character to the console tty, using poll-wait
// Interrupts are switched off.
//
	.globl _kputc
_kputc:
	mov	(sp),r2
	mov	r2,r2		// \0 is a no-op
	jeq	2f
	stst	r12
	andi	r12,0xF
	mov	r12,@kpu010+2
	limi	2
	li	r12,tty0addr	// load tty0 device address
.if TTY9902
1:	tb	xbre		// wait for empty buffer
	jne	1b
.endif
.if TTYEIA
1:	tb	ttyxmit		// wait for empty buffer
	jeq	1b
	sbz	ttywrq
.endif
	swpb	r2
	ldcr	r2,8		// send chararacter
	swpb	r2
kpu010:	limi	2
2:	b	(r11)

.if TTY403
	.globl _spin
_spin:
	mov	(sp),r1
1:	dec	r1
	jgt	1b
	b	(r11)
.endif

// old = spl0()
// Enable all interupts, return old priority
//
	.globl	_spl0
_spl0:
	stst	r2
	andi	r2,0xf
	limi	15
	b	(r11)

// old = spl7()
// Disable all interrupts, return old priority
//
	.globl	_spl7
_spl7:
	stst	r2
	limi	2
	andi	r2,0xf
	b	(r11)

// old = splx(new)
// Set interrupt priority level to 'new', return old priority
//
	.globl _splx
_splx:
	stst	r0
	limi	2
	mov	(sp),r1
	andi	r1,0xf
	mov	r0,r2
	andi	r2,0xf
	andi	r0,0xfff0
	soc	r1,r0
.if 1-EIS
	blwp	@lstr0
.endif
.if EIS
	lst	r0
.endif
	b	(r11)

// C language runtime support
// Included here because the kernel is not linked with the standard
// C runtime support library.

	.globl	csv
csv:
	stst	r12
	limi	2
	ai	sp,-18
	mov	sp,r1
	mov	r8,(r1)+
	mov	r7,(r1)+
	mov	r6,(r1)+
	mov	r5,(r1)+
	mov	r4,(r1)+
	mov	r3,(r1)+
	mov	r2,(r1)+
	mov	bp,(r1)+
	mov	r0,(r1)+
	jeq	1f
	mov	sp,bp
	dect	sp
.if 1-EIS
	blwp	@lstr12
.endif
.if EIS
	lst	r12
.endif
	b	(r11)
1:	idle

	.globl	cret
cret:
	stst	r12
	limi	2
	ci	bp,0xf000
	jhe	1b
	mov	bp,sp
	mov	(sp)+,r8
	mov	(sp)+,r7
	mov	(sp)+,r6
	mov	(sp)+,r5
	mov	(sp)+,r4
	mov	(sp)+,r11
	mov	(sp)+,r11
	mov	(sp)+,bp
	mov	(sp)+,r11
.if 1-EIS
	blwp	@lstr12
.endif
.if EIS
	lst	r12
.endif
	b	(r11)

	.globl	sext
sext:
	seto	r0
	jlt	1f
	clr	r0
1:	b	(r11)

	.globl	lsra
lsra:
	andi	r0,31
	jeq	3f
1:	srl	r3,1
	sra	r2,1
	jnc	2f
	ori	r3,0x8000
2:	dec	r0
	jne	1b
3:	b	(r11)

	.globl	lsrl
lsrl:
	andi	r0,31
	jeq	3f
1:	srl	r3,1
	srl	r2,1
	jnc	2f
	ori	r3,0x8000
2:	dec	r0
	jne	1b
3:	b	(r11)

	.globl	lsla
lsla:
	andi	r0,31
	jeq	3f
1:	sla	r2,1
	sla	r3,1
	jnc	2f
	inc	r2
2:	dec	r0
	jne	1b
3:	b	(r11)

	.globl	lmul
lmul:	mov     @2(sp),r2
        mov     @6(sp),r3
        mov     (sp),r0
        mpy     r3,r0
        mov     r1,r12
        mov     @4(sp),r0
        mpy     r2,r0
        a       r1,r12
        mpy     r3,r2
        a       r12,r2
        b       (r11)

	.globl	lrem
lrem:
	mov	(sp)+,r2
	mov	(sp)+,r3
	mov	(sp)+,r4
	mov	(sp)+,r5
	ai	sp,-8

// Perform 32 bit division: r2,r3 / r4,r5 => r2,r3 remainder in r0,r1
irem32:
	clr	r12		// clear sign flag
	mov	r4,r4
	jlt	rneg		// negate divisor if needed
	jne	L1		// if divisor == 0 return
	mov	r5,r5
	jeq	M9

M1:	mov	r2,r0
	sra	r0,15
	mov	r0,r1
	li	r12,32

M2:	sla	r0,1		// shift 64 bits r0,r1,r2,r3 left one
	sla	r1,1
	jnc	M3
	inc	r0
M3:	sla	r2,1
	jnc	M4
	inc	r1
M4:	sla	r3,1
	jnc	M5
	inc	r2

M5:	c	r0,r4		// is divisor >= shifted dividend
	jh	M6
	jl	M8
	c	r1,r5
	jl	M8

M6:	s	r5,r1		// if so: subtract divisor, shift 1 into result
	joc	M7
	dec	r0
M7:	s	r4,r0
	inc	r3

M8:	dec	r12		// loop over all 32 bits
	jgt	M2

M9:	mov	r12,r12
	jeq	LX
	neg	r0
	neg	r1
	joc	LX
	dec	r0
LX:	mov	r0,r2
	mov	r1,r3
	b	(r11)

rneg:	seto	r12
	neg	r4
	neg	r5
	joc	M1
	dec	r4
	jmp	M1


	.globl	ldiv
ldiv:
	mov	(sp)+,r2
	mov	(sp)+,r3
	mov	(sp)+,r4
	mov	(sp)+,r5
	ai	sp,-8

// Perform 32 bit division: r2,r3 / r4,r5 => r2,r3 remainder in r0,r1
idiv32:
	mov	r4,r4
	jlt	neg		// negate divisor if needed
	jne	L1		// if divisor == 0 return
	mov	r5,r5
	jeq	L9

L1:	mov	r2,r0
	sra	r0,15
	mov	r0,r1
	li	r12,32

L2:	sla	r0,1		// shift 64 bits r0,r1,r2,r3 left one
	sla	r1,1
	jnc	L3
	inc	r0
L3:	sla	r2,1
	jnc	L4
	inc	r1
L4:	sla	r3,1
	jnc	L5
	inc	r2

L5:	c	r0,r4		// is divisor >= shifted dividend
	jh	L6
	jl	L8
	c	r1,r5
	jl	L8

L6:	s	r5,r1		// if so: subtract divisor, shift 1 into result
	joc	L7
	dec	r0
L7:	s	r4,r0
	inc	r3

L8:	dec	r12		// loop over all 32 bits
	jgt	L2

L9:	b	(r11)

neg:	neg	r4
	neg	r5
	joc	L1
	dec	r4
	jmp	L1
