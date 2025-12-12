\ IDE disk driver

\ defines for the IDE registers
#cfregs = $fe00
#cfdata = cfregs + 0
#cferr  = cfregs + 1	\ rd
#cffeat = cfregs + 1	\ wr
#cfcnt  = cfregs + 2
#cflba0 = cfregs + 3
#cflba1 = cfregs + 4
#cflba2 = cfregs + 5
#cflba3 = cfregs + 6
#cfstat = cfregs + 7	\ rd
#cfcmd  = cfregs + 7	\ wr

\ defines for IDE commands in upper byte
#SETFEAT = $ef00	\ set feature
#RDSECT  = $2000	\ read sector
#WRSECT  = $3000	\ write sector
#IDENT   = $ec00	\ identify drive
#BYTEMOD = $0100	\ 8 bit access mode
	
\ defines for status bits
#ERR     = $0100	\ error condition
#DRQ     = $0800	\ data request

\ init CF IDE, set 8-bit mode

	.rel    2
	.ent    .Disc_init
	.formal 0,0
start:	movb	@cfstat,r0      \ wait IDE ready
	jlt	start
	clr     r0
	movb	r0,@cflba3
	li	r0,BYTEMOD
	movb	r0,@cffeat
	li	r0,SETFEAT
	movb	r0,@cfcmd
	rtwp
	.rel    1
.Disc_init:
	.ptr    start
	.end

\ Seek disk block

	.rel    2
	.ent    ..ideseek
..ideseek:
	movb	@cfstat,r0	\ wait IDE ready
	jlt	..ideseek
	mov	r10,r0  	\ requested sector number
	movb	r0,@cflba1	\ move the LBA to the CF IDE
	swpb	r0		\    registers and set transfer
	movb	r0,@cflba0	\    length to 1 sector
	li	r0,$e000
	movb	r0,@cflba3
	swpb	r0
	movb	r0,@cflba2
	li	r0,$0100
	movb	r0,@cfcnt
	b	[r11]
	.end

\ read 512 bytes from disk block #r10 to buffer (r9)

	.rel    2
	.ent    .Disc_read
        .ext    ..ideseek
	.formal 2,2
start:	bl	@..ideseek      \ set up sector
.1:	movb	@cfstat,r0	\ wait IDE ready
	jlt	.1
	li	r0,RDSECT	\ issue read command
	movb	r0,@cfcmd
.2:	movb	@cfstat,r0	\ wait IDE ready
	jlt	.2
	movb	r0,r1
	andi	r0,DRQ		\ and data ready
	jeq	.2
	andi	r1,ERR		\ check for error
	jne	.4

	li	r1,512/4	\ CPU transfer
        sla     r9,1
.3:	movb	@cfdata,[r9]+
	movb	@cfdata,[r9]+
	movb	@cfdata,[r9]+
	movb	@cfdata,[r9]+
	dec	r1
	jne	.3

.4:	movb	@cferr,r12	\ fetch error code
	srl	r12,8
	rtwp
	.rel    1
.Disc_read: .ptr    start
	.end

\ write 512 bytes from buffer (r9) to disk block #r10

	.rel    2
	.ent    .Disc_write
        .ext    ..ideseek
	.formal 2,2
start:	bl	@..ideseek      \ set up sector
.1:	movb	@cfstat,r0	\ wait IDE ready
	jlt	.1
	li	r0,WRSECT	\ issue write command
	movb	r0,@cfcmd
.2:	movb	@cfstat,r0	\ wait IDE ready
	jlt	.2
	andi	r0,DRQ		\ and data ready
	jeq	.2

	li	r1,512/4	\ CPU transfer
        sla     r9,1
.3:	movb	[r9]+,@cfdata
	movb	[r9]+,@cfdata
	movb	[r9]+,@cfdata
	movb	[r9]+,@cfdata
	dec	r1
	jne	.3

	movb	@cfstat,r12	\ check for error
	andi	r12,ERR
	jeq	.4
	movb	@cferr,r12	\ fetch error code
	srl	r12,8
.4:	rtwp
	.rel    1
.Disc_write: .ptr    start
	.end
