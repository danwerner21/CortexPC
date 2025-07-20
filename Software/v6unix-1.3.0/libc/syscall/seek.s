// Unix V6 system call
//
	.globl	cerror
	lseek = 19

	// C call is (l)seek(fd, offs, whence)
	// convert seek(int, int, int) into
	// lseek(int, long, int)
	// offs is unsigned for whence==0, signed otherwise
	//
	.globl	_seek
_seek:
	dect	sp		// move fd one word down
	mov	@2(sp),(sp)
	mov	@6(sp),r0	// fetch whence
	ci	r0,3
	jlt	3f

	// handle whence == 3..5
	ai	r0,-3		// whence >=3, convert whence
	mov	r0,@6(sp)
	jeq	1f
	mov	@4(sp),r0	// whence 4 or 5, convert offs signed
.if EIS
	mpys	@L512
.endif
.if 1-EIS
	mpy	@L512,r0
.endif
	jmp	2f
1:	mov	@4(sp),r0	// whence == 3, convert offs unsigned
	mpy	@L512,r0
2:	mov	r0,@4(sp)
	mov	r1,@2(sp)
	jmp	_lseek

	// handle whence == 0..2
3:	ci	r0,0
	jeq	1f
	mov	@4(sp),@2(sp)	// whence 1 or 2, signed extension
	jgt	2f
	jeq	2f
	seto	@4(sp)
	jmp	_lseek
1:	mov	@4(sp),@2(sp)	// whence == 0, unsigned extension
2:	clr	@4(sp)
	// fall through to lseek

	.globl	_lseek
_lseek:
	sys	lseek
	joc	1f
	mov	r0,r2
	mov	r1,r3
	b	(r11)
1:	seto	r2
	seto	r3
	b	(r11)

L512:	512
	
	
