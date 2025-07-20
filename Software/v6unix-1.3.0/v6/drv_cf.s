// Driver for the CF Card on the breadboard

// defines for the CF card registers
	cfregs = 0xfe00
	cfdata = cfregs + 0
	cferr  = cfregs + 1	// rd
	cffeat = cfregs + 1	// wr
	cfcnt  = cfregs + 2
	cflba0 = cfregs + 3
	cflba1 = cfregs + 4
	cflba2 = cfregs + 5
	cflba3 = cfregs + 6
	cfstat = cfregs + 7	// rd
	cfcmd  = cfregs + 7	// wr

// defines for CF card commands in upper byte
	SETFEAT = 0xef00	// set feature
	RDSECT  = 0x2000	// read sector
	WRSECT  = 0x3000	// write sector
	IDENT   = 0xec00	// identify drive
	BYTEMOD = 0x0100	// 8 bit access mode
	
// defines for status bits
	ERR     = 0x0100	// error condition
	DRQ     = 0x0800	// data request

// image vectors left behind by boot loader
	cfdsk0	= 0xff00	// base sector of DSK0 image (32 bit)
	cfdsk1  = 0xff04	// base sector of DSK1 image (32 bit)

// arguments to cf_card_rd, cf_card_wr
	dev	= 18
	blkno	= 20
	addr	= 22
	wcount	= 24

	.text

// Seek disk block
seek:
1:	movb	@cfstat,r0	// wait card ready
	jlt	1b
	li	r2,cfdsk0	// starting sector of dsk0 image
	mov	@dev(bp),r0
	jeq	1f
	li	r2,cfdsk1	// starting sector of dsk1 image
1:	mov	@blkno(bp),r1	// requested sector number
	mov	(r2)+,r0	// add start to requested to get
	a	(r2),r1		//   32 bit LBA of req. sector
	jnc     1f
	inc	r0
1:	movb	r1,@cflba1	// move the LBA to the CF Card
	swpb	r1		//    registers and set transfer
	movb	r1,@cflba0	//    length to 1 sector
	andi	r0,0x0fff
	ori	r0,0xe000
	movb	r0,@cflba3
	swpb	r0
	movb	r0,@cflba2
	li	r0,0x0100
	movb	r0,@cfcnt
	b	(r11)

// Read 512 byte block from the CF Card
rdblock:
1:	movb	@cfstat,r0	// wait card ready
	jlt	1b
	li	r0,RDSECT	// issue read command
	movb	r0,@cfcmd
1:	movb	@cfstat,r0	// wait card ready
	movb	r0,r1
	jlt	1b
	andi	r0,DRQ		// and data ready
	jeq	1b
	andi	r1,ERR		// check for error
	jne	2f
	li	r1,512/4
1:	movb	@cfdata,(r8)+
	movb	@cfdata,(r8)+
	movb	@cfdata,(r8)+
	movb	@cfdata,(r8)+
	dec	r1
	jne	1b
	b	(r11)

2:	movb	@cferr,r2	// fetch error code
	srl	r2,8
	b	(r11)

// read 'wcount' words from 'dev' starting at 'blkno' to 'addr'
	.globl	_bkread
_bkread:
	mov     r11,r0
	bl      @csv
	mov	@wcount(bp),r12
	srl	r12,8
	jeq	2f
	mov	@addr(bp),r8
1:	bl	@seek
	bl	@rdblock
	jne	2f
	inc	@blkno(bp)
	dec	r12
	jne	1b
	clr	r2
2:	b	@cret

// write 512 byte block to the CF Card
wrblock:
1:	movb	@cfstat,r0	// wait card ready
	jlt	1b
	li	r0,WRSECT	// issue read command
	movb	r0,@cfcmd
1:	movb	@cfstat,r0	// wait card ready
	jlt	1b
	andi	r0,DRQ		// and data ready
	jeq	1b
	li	r1,512/4
1:	movb	(r8)+,@cfdata
	movb	(r8)+,@cfdata
	movb	(r8)+,@cfdata
	movb	(r8)+,@cfdata
	dec	r1
	jne	1b
	movb	@cfstat,r0	// check for error
	andi	r0,ERR
	jeq	1f
	movb	@cferr,r2	// fetch error code
	srl	r2,8
1:	b	(r11)

// write 'wcount' words from 'addr' to 'dev' starting at 'blkno'
	.globl	_bkwrite
_bkwrite:
	mov     r11,r0
	bl      @csv
	mov	@wcount(bp),r12
	srl	r12,8
	jeq	2f
	mov	@addr(bp),r8
1:	bl	@seek
	bl	@wrblock
	jne	2f
	inc	@blkno(bp)
	dec	r12
	jne	1b
	clr	r2
2:	b	@cret

// init CF card, set 8-bit mode
	.globl	_bkinit
_bkinit:
1:	movb	@cfstat,r0
	jlt	1b
	movb	@zero,@cflba3
	li	r0,BYTEMOD
	movb	r0,@cffeat
	li	r0,SETFEAT
	movb	r0,@cfcmd
	b	(r11)
	
	.data
zero:	0
