
	.globl	_mmu
	_mmu = 0xfe40		// address of the mapper

// Stack swapping routines
// - savu
// - aretu
// - retu

// savu(&buf)
// Save stack & base pointer in buffer
//
	.globl	_savu
_savu:
	stst	r12
	limi	0
	mov	(sp),r0
	mov	sp,(r0)+
	mov	bp,(r0)
	lst	r12
	b	(r11)


// retu(slot)
// Switch kernel map to use _u area in 'slot' and
// restore stack & base pointer from buffer. The
// _u area for slot zero is special.
//
	.globl	_retu
_retu:
	stst	r12
	limi	0
	mov	(sp),r0
	sla	r0,12
	jne	1f
	li	r0,0x0700
1:	movb	r0,@_mmu+14
	li	r0,0xe000
	jmp	1f
	
// aretu(&buf)
// Restore stack & base pointer from buffer
//
	.globl	_aretu
_aretu:
	stst	r12
	limi	0
	mov	(sp),r0
1:	mov	(r0)+,sp
	mov	(r0),bp
	lst	r12
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
	stst	r0
	limi	0
	movb	@_mmu+0xd,r12
	mov	(sp),r1
	srl	r1,12
	movb	@_usrmap(r1),@_mmu+0xd
	mov	(sp),r1
	andi	r1,0x0fff
	ori	r1,0xd000
	movb	(r1),r2
	sra	r2,8
	movb	r12,@_mmu+0xd
	lst	r0
	b	(r11)

	.globl	_fuword
_fuword:
	stst	r0
	limi	0
	movb	@_mmu+0xd,r12
	mov	(sp),r1
	srl	r1,12
	movb	@_usrmap(r1),@_mmu+0xd
	mov	(sp),r1
	andi	r1,0x0fff
	ori	r1,0xd000
	mov	(r1),r2
	movb	r12,@_mmu+0xd
	lst	r0
	b	(r11)

	.globl	_subyte
_subyte:
	stst	r0
	limi	0
	movb	@_mmu+0xd,r12
	mov	(sp),r1
	srl	r1,12
	movb	@_usrmap(r1),@_mmu+0xd
	mov	(sp),r1
	andi	r1,0x0fff
	ori	r1,0xd000
	mov	@2(sp),r2
	swpb	r2
	movb	r2,(r1)
	movb	r12,@_mmu+0xd
	lst	r0
	b	(r11)

	.globl	_sustr
_sustr:
	stst	r0
	limi	0
	movb	@_mmu+0xd,r12
	mov	(sp),r1
	srl	r1,12
	movb	@_usrmap(r1),@_mmu+0xd
	mov	(sp),r1
	andi	r1,0x0fff
	ori	r1,0xd000
	mov	@2(sp),r2
1:	movb	(r2)+,r0
	jeq	2f
	movb	r0,(r1)+
	jmp	1b
2:	movb	r0,(r1)
	movb	r12,@_mmu+0xd
	lst	r0
	b	(r11)

	.globl	_suword
_suword:
	stst	r0
	limi	0
	movb	@_mmu+0xd,r12
	mov	(sp),r1
	srl	r1,12
	movb	@_usrmap(r1),@_mmu+0xd
	mov	(sp),r1
	andi	r1,0x0fff
	ori	r1,0xd000
	mov	@2(sp),(r1)
	movb	r12,@_mmu+0xd
	lst	r0
	b	(r11)

	.globl	_copyin
_copyin:
	stst	r3
	limi	0
	mov	@_mmu+0xc,r12
	mov	@2(sp),r2
	srl	r2,12
	movb	@_usrmap(r2),@_mmu+0xc
	mov	@2(sp),r2
	a	@4(sp),r2
	srl	r2,12
	movb	@_usrmap(r2),@_mmu+0xd
	mov	(sp),r0
	mov	@2(sp),r1
	andi	r1,0x0fff
	ori	r1,0xc000
	mov	@4(sp),r2
1:	movb	(r1)+,(r0)+
	dec	r2
	jne	1b
	mov	r12,@_mmu+0xc
	lst	r3
	b	(r11)

	.globl	_copyout
_copyout:
	stst	r3
	limi	0
	mov	@_mmu+0xc,r12
	mov	(sp),r2
	srl	r2,12
	movb	@_usrmap(r2),@_mmu+0xc
	mov	(sp),r2
	a	@4(sp),r2
	srl	r2,12
	movb	@_usrmap(r2),@_mmu+0xd
	mov	(sp),r0
	andi	r0,0x0fff
	ori	r0,0xc000
	mov	@2(sp),r1
	mov	@4(sp),r2
1:	movb	(r1)+,(r0)+
	dec	r2
	jne	1b
	mov	r12,@_mmu+0xc
	lst	r3
	b	(r11)

	.globl	_clearimg
_clearimg:
	stst	r0
	movb	@_mmu+0xd,r12		// save kernel MMU[13]
	li	r3,14
	mov	(sp),r1
	sla	r1,12
	ai	r1,0x0100
2:	limi	0
	movb	r1,@_mmu+0xd		// remap MMU[13] to user page
	li	r2,0xd000		// clear user page
1:	clr	(r2)+
	clr	(r2)+
	clr	(r2)+
	clr	(r2)+
	ci	r2,0xe000
	jne	1b
	ai	r1,0x0100		// next page, but allow interrupts in between
	movb	r12,@_mmu+0xd
	lst	r0
	dec	r3			// repeat for all 16 pages
	jne	2b
	clr	r2
	b	(r11)

	.globl	_copyimg
_copyimg:
	stst	r0
	mov	@_mmu+0xc,r12		// save kernel MMU[12] and MMU[13]
	mov	@2(sp),r3		// prepare new values in R1
	jeq	3f			// special case slot 0
	sla	r3,4
	mov	(sp),r1
	sla	r1,12
	soc	r3,r1
	li	r3,15
2:	limi	0
	mov	r1,@_mmu+0xc		// remap MMU[12] and MMU[13] to dst and src page
	li	r2,0xc000		// copy one page
1:	mov	@0x1000(r2),(r2)+
	mov	@0x1000(r2),(r2)+
	mov	@0x1000(r2),(r2)+
	mov	@0x1000(r2),(r2)+
	ci	r2,0xd000
	jne	1b
	ai	r1,0x0101		// next page but allow for interrupts in between
	mov	r12,@_mmu+0xc
	lst	r0
	dec	r3			// repeat for 16 pages
	jne	2b
	b	(r11)

	// Due to the wiring error on the Cortex PCB, and due to the
	// special location of the _u area for process 0, the fork copy
	// for process 0 into process 1 has to be special cased.
mmutab:
	0x1007; 0x1108;	0x1201; 0x1309
	0x1402; 0x150a;	0x1603; 0x170b
	0x1804; 0x190c;	0x1a05; 0x1b0d
	0x1c06; 0x1d0e;	0x1e00; 0x1f0f

3:	li	r1,mmutab
	seto	r3
2:	limi	0
	mov	(r1)+,@_mmu+0xc		// remap MMU[12] and MMU[13] to dst and src page
	li	r2,0xc000		// copy one page
1:	mov	@0x1000(r2),(r2)+
	mov	@0x1000(r2),(r2)+
	mov	@0x1000(r2),(r2)+
	mov	@0x1000(r2),(r2)+
	ci	r2,0xd000
	jne	1b
	mov	r12,@_mmu+0xc
	lst	r0
	srl	r3,1			// repeat for 16 pages
	jne	2b
	b	(r11)
