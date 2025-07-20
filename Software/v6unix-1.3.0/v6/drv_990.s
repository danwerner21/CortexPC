// Driver for the 990 TILINE disks.

	dskwp = 0xf220

// defines for the TILINE disks

	CMDMASK  = 0xF8FF
	SECREC   = 0x0100
	ERRMSK   = 0x21FF

// defines for disk commands.

	STOREREG = 0x0000	// Store registers (drive geometry).
	READCMD  = 0x0200	// Read.
	WRITECMD = 0x0300	// Write.
	SEEKCMD  = 0x0600	// Seek.
	RESTORE  = 0x0700	// Restore (Reset).

// Drive select bits.

	DSK0	= 0x0800	// DSK0 
	DSK1	= 0x0400	// DSK1
	DSK2	= 0x0200	// DSK2
	DSK3	= 0x0100	// DSK3

// argument offsets.

	dev	= 18
	blkno	= 20
	addr	= 22
	wcount	= 24

	.text

// issue command

issue:
	mov	r12,r5
1:	mov	@14(r12),r2	// Wait until idle.
	jlt	2f
	jmp	1b
2:	clr	(r5)+		// Send command.
	mov	@cmdblk+2,(r5)+
	mov	@cmdblk+4,(r5)+
	mov	@cmdblk+6,(r5)+
	mov	@cmdblk+8,(r5)+
	mov	@cmdblk+10,(r5)+
	mov	@cmdblk+12,(r5)+
	li	r2,0x1000	// Set interrupt on completion bit.
	mov	r2,(r5)		// Start operation.
	b	(r11)

// interrupt handler

	.globl	_bkintr
_bkintr:
	dskwp; bkintr
bkintr:
	li	r1,0x8000
	mov	@14(r12),r2	// Wait until done.
	jlt	4f
3:	rtwp
4:	andi	r2,ERRMSK	// Errors?
	jne	5f		// Yes, bail.
	mov	r8,r8		// No, Seek started?
	jeq	6f
	movb	r1,@14(r12)	// Yes, clear seek interrupt.
	socb	r8,@1(r12)	// Set attention mask.
	mov	r8,r9
	clr	r8
	rtwp
6:	mov	r7,r7		// Seek complete, Check attention bits?
	jeq	5f
	mov	(r12),r6	// Yes, get them.
	coc	r7,r6		// Seek complete?
	jne	3b		// No, continue waiting
	szcb	r9,@1(r12)	// Yes, clear attention mask.
	mov	@cmdblk+2,r2	// Set read/write command.
	andi	r2,CMDMASK	
	soc	r4,r2
	mov	r2,@cmdblk+2
	clr	r7		// No more attention bits.
	bl	@issue
	rtwp
5:	mov	r2,@_bkerror	// Save errors.
	movb	r1,@14(r12)	// Clear Read/Write interrupt.
	inct	r14
	rtwp

// Seek disk block

seek:
	mov	@_diskaddr,r12	// Do set up.
	mov	r12,@dskwp+24	// dskwp r12
	mov	r4,@dskwp+8	// dskwp r4
	mov	r8,@cmdblk+10
	li	r8,512
	mov	r8,@cmdblk+8	
	clr	r1
	movb	@dev(bp),r1	// Get disk unit.
	andi	r1,0xF000	// isolate unit select.
	srl	r1,11
	mov	@dsktbl(r1),r2
	mov	r2,r7		// Set up attention bits
	soc	@dskslot,r2	// Add in the disk(memory) slot
	mov	r2,@cmdblk+12
	mov	r7,@dskwp+16	// dskwp r8
	srl	r7,4
	mov	r7,@dskwp+14	// dskwp r7
	clr	r0
	mov	@blkno(bp),r1	// Requested block number
	mov	@_mulsec,r2	// Multi-sector mode?
	jeq	2f
	sla	r1,1		// Yes, block *= 2
	jnc	2f
	inc	r0
2:	mov	@_disksectrk,r3	// sectrk (skip over track 0 and 1)
	mov	r3,r2		//   * 2
	sla	r2,1
	a	r2,r1		// block += sectrk * 2
	jnc	3f
	inc	r0
3:	div	r3,r0
	ori	r1,SECREC
	mov	r1,@cmdblk+4	// sector
	mov	r0,r1
	clr	r0
	div	@_diskheads,r0
	ori	r1,SEEKCMD
	mov	r1,@cmdblk+2	// head | CMD
	movb	@dev(bp),r1	// Get disk unit.
	andi	r1,0x0F00	// isolate partition.
	srl	r1,7
	a	@_partition(r1),r0
	mov	r0,@cmdblk+6	// cyl
	mov	r11,r1
	bl	@issue
	b	(r1)

// read 'wcount' words from 'dev' starting at 'blkno' to 'addr'

	.globl	_bkread
_bkread:
	mov     r11,r0
	bl      @csv
	li	r4,READCMD
	jmp	bkcmn

// write 'wcount' words from 'addr' to 'dev' starting at 'blkno'

	.globl	_bkwrite
_bkwrite:
	mov     r11,r0
	bl      @csv
	li	r4,WRITECMD

bkcmn:
	mov	@wcount(bp),r12
	sla	r12,1
	jeq	2f
	mov	@addr(bp),r8
	bl	@seek
	jne	2f
	clr	r2
2:	b	@cret

// Store disk slot

	.globl	_dskslot
_dskslot:
	mov	(sp),@dskslot
	b	(r11)

dsktbl:	DSK0
	DSK1
	DSK2
	DSK3

	.bss
	.globl _bkerror
_bkerror: .=.+2
dskslot: .=.+2
cmdblk:	.=.+16
	cmdlen = .-cmdblk

