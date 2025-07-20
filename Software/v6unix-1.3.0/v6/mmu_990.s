
// Stack swapping routines
// - savu
// - aretu
// - retu

lstr12:
	lstr12w-26; lstr12c

lstr12c:
	mov	@22(r13),r14
	mov	@24(r13),r15
	rtwp
	.bss
lstr12w:
	.=.+6
	.text

// savu(&buf)
// Save stack & base pointer in buffer
//
	.globl	_savu
_savu:
	stst	r12
	limi	2
	mov	(sp),r0
	mov	sp,(r0)+
	mov	bp,(r0)
	blwp	@lstr12

// retu(slot)
// Switch kernel map to use _u area in 'slot' and
// restore stack & base pointer from buffer. The
// _u area for slot zero is special.
//
	.globl	_retu
_retu:
	stst	r12
	limi	2
	li	r1,_sysmap
	mov	(sp),r0
	jeq	1f
	dec	r0
	sla	r0,11
	ai	r0,0x0100
1:	mov	r0,@6(r1)
	lmf	r1,0
	li	r1,0xe000
	mov	(r1)+,sp
	mov	(r1),bp
	blwp	@lstr12
	
// aretu(&buf)
// Restore stack & base pointer from buffer
//
	.globl	_aretu
_aretu:
	stst	r12
	limi	2
	mov	(sp),r0
	mov	(r0)+,sp
	mov	(r0),bp
	blwp	@lstr12

// usrslot(slot)
// Sets slot into the user map.

	.globl	_usrslot
_usrslot:
	mov	(sp),r1
	sla	r1,11
	mov	r1,@_usrmap+2
	mov	r1,@_usrmap+6
	li	r1,_usrmap
	lmf	r1,1
	b	(r11)

// c = gsbyte (slot, addr)

	.globl	_gsbyte
_gsbyte:
	mov	(sp),r1		// get slot
	sla	r1,11
	mov	r1,@srcmap+6
	clr	@srcmap+2
	mov	@2(sp),r1
	lds	@srcmap
	movb	(r1),r2
	srl	r2,8
	b	(r11)

// psbyte (slot, addr, c)

	.globl	_psbyte
_psbyte:
	mov	(sp),r1		// get slot
	sla	r1,11
	mov	r1,@dstmap+6
	clr	@dstmap+2
	mov	@2(sp),r1
	mov	@4(sp),r0
	swpb	r0
	ldd	@dstmap
	mov	r0,(r1)
	b	(r11)

// Memory management support routines
// c = fubyte(addr)		- fetch a byte from user space
// w = fuword(addr)		- fetch a word from user space
// subyte(addr, c)		- store a byte to user space
// suword(addr, w)		- store a word to user space
// sustr(addr, s)		- store a string to user space
// copyin(dst, src, cnt)	- copy cnt bytes from user space src
// copyout(dst, src, cnt)	- copy cnt bytes to user space dst
// clearseg(slot)		- clear memory in location 'slot'
// copyimg(dst,src)		- copy image in slot src to slot dst

	.globl	_fubyte
_fubyte:
	mov	(sp),r1
	lds	@_usrmap
	movb	(r1),r2
	sra	r2,8
	b	(r11)

	.globl	_fuword
_fuword:
	mov	(sp),r1
	lds	@_usrmap
	mov	(r1),r2
	b	(r11)

	.globl	_subyte
_subyte:
	mov	(sp),r1
	mov	@2(sp),r2
	swpb	r2
	ldd	@_usrmap
	movb	r2,(r1)
	b	(r11)

	.globl	_sustr
_sustr:
	mov	(sp),r1
	mov	@2(sp),r2
1:	movb	(r2)+,r0
	jeq	2f
	ldd	@_usrmap
	movb	r0,(r1)+
	jmp	1b
2:	ldd	@_usrmap
	movb	r0,(r1)
	b	(r11)

	.globl	_suword
_suword:
	mov	(sp),r1
	ldd	@_usrmap
	mov	@2(sp),(r1)
	b	(r11)

	.globl	_copyin
_copyin:
	mov	(sp),r0
	mov	@2(sp),r1
	mov	@4(sp),r2
1:	lds	@_usrmap
	movb	(r1)+,(r0)+
	dec	r2
	jne	1b
	b	(r11)

	.globl	_copyout
_copyout:
	mov	(sp),r0
	mov	@2(sp),r1
	mov	@4(sp),r2
1:	ldd	@_usrmap
	movb	(r1)+,(r0)+
	dec	r2
	jne	1b
	b	(r11)

	.globl	_clearimg
_clearimg:
	mov	(sp),r1
	sla	r1,11
	mov	r1,@dstmap+6
	li	r2,0x1000		// clear memory
	li	r12,errcru
1:	lds	@dstmap
	clr	(r2)+
	stcr	r1,0			// TILINE timeout?
	jlt	2f 			// Yes, non-existant memory.
	ci	r2,0xf000
	jl	1b
	clr	r2
	b	(r11)
2:	clr	r1
	ldcr	r1,0
	li	r12,mapper
	sbo	mapclr
	sbz	mapclr
	seto	r2
	b	(r11)

	.globl	_copyimg
_copyimg:
	mov	(sp),r1
	sla	r1,11
	mov	r1,@dstmap+2
	mov	r1,@dstmap+6
	clr	r2
	mov	@2(sp),r3
	sla	r3,11
	mov	r3,@srcmap+2
	mov	r3,@srcmap+6
	jeq	3f			// special case slot 0
2:
	lds	@srcmap
	mov	(r2),r0
	ldd	@dstmap
	mov	r0,(r2)+
	ci	r2,0xf000
	jl	2b
	b	(r11)

3:	ldd	@dstmap
	mov	@0xe000(r2),(r2)+
	ci	r2,0x1000
	jl	3b
4:	ldd	@dstmap
	mov	(r2),(r2)+
	ci	r2,0xe000
	jl	4b
	b	(r11)

// netcopybuf (dst, dslot, src, sslot, cnt)

	.globl	_netcopybuf
_netcopybuf:
	stst	r12
	limi	2
	mov	(sp),r1			// dst address
	mov	@2(sp),r2		// dst slot
	sla	r2,11
	mov	r2,@dstnet+2
	mov	r2,@dstnet+6
	mov	@4(sp),r2		// src address
	mov	@6(sp),r3		// src slot
	sla	r3,11
	mov	r3,@srcnet+2
	mov	r3,@srcnet+6
	mov	@8(sp),r3		// cnt
2:
	lds	@srcnet
	movb	(r2)+,r0
	ldd	@dstnet
	movb	r0,(r1)+
	dec	r3
	jgt	2b
	blwp	@lstr12

srcmap:
	0xF000				// L1 4K - _u/kernel
	0x0000				// B1 changes with slot
	0x1000				// L2 57K - user code
	0x0000				// B2 changes with slot
	0x0800				// L3 4K  - WS - TILINE
	0x0000				// B3 
dstmap:
	0xF000				// L1 4K - _u/kernel
	0x0000				// B1 changes with slot
	0x1000				// L2 57K - user code
	0x0000				// B2 changes with slot
	0x0800				// L3 4K  - WS - TILINE
	0x0000				// B3

srcnet:
	0xF000				// L1 4K - _u/kernel
	0x0000				// B1 changes with slot
	0x1000				// L2 57K - user code
	0x0000				// B2 changes with slot
	0x0800				// L3 4K  - WS - TILINE
	0x0000				// B3 
dstnet:
	0xF000				// L1 4K - _u/kernel
	0x0000				// B1 changes with slot
	0x1000				// L2 57K - user code
	0x0000				// B2 changes with slot
	0x0800				// L3 4K  - WS - TILINE
	0x0000				// B3

