	rwp    = 0x0080
	wp1    = 0xf000
	wp2    = 0xf030
	wp3    = 0xf050

	inoblk = 0x8400
	indblk = 0x8600
	datblk = 0x8800
	
// The Unix boot loader starts here. It has to fit in one 512 byte sector.
// It loads the file "unix" from the root directory of the first block device
// into low memory, i.e. starting from location 0.
// This boot loader therefore has to be run from higher in memory. The system
// loads it at location 0x8000.
// This loader is loaded by the "track 1" loader. On entry WS is the ROM WS.


// Unix Boot Loader

boot:
	mov	r1,@wp3+24	// Device address into WS R12.
	mov	r2,@wp3+18	// Device unit into WS R9.
	mov	r3,@wp3+16	// save heads into WS R8.
	mov	r4,@sectrk	// sectors per track.
	mov	r5,@mulsec	// multi-sector mode?
	jne	2f
	inct	@read+2		// no, bypass * 2 shift.

2:	lwpi	wp1
	li	r0,1		// get root dir = inode 1
	blwp	@geti
	
	// set up directory scan
	mov	r0,r10
	mov	@6(r10),r9	// get size (assume int)
	sra	r9,4		// cnt = size / 16
	clr	r8		// lbn = 0
	li	r7,datblk
	jmp	2f

	// read directory blocks one by one
1:	mov	r8,r1		// pbn, lbn2pbn(inop, lbn)
	mov	r10,r0
	blwp	@lbn2pbn
	inc	r8		// lbn++
	mov	r7,@wp3+20	// cfread(pbn, datblk)
	mov	r0,@wp3+2
	blwp	@read

	// scan block
	mov	r7,r6		// p = &datblk
5:	li	r5,unix		//   q = "unix"
	li	r4,14		//   n = 14
	mov	(r6)+,r3	//   i = p->inode
4:	movb	(r6)+,r0	//     if (!*p && !*q && i) goto found
	jne	6f
	movb	(r5),r1
	jne	6f
	mov	r3,r3
	jne	found
6:	cb	r0,(r5)+	//     if (*p++!==*q++) i = 0
	jeq	3f
	clr	r3
3:	dec	r4		//     while (--n>0)
	jgt	4b
	dec	r9		//   if (--cnt<0) goto error;
	jlt	error
	ci	r6,datblk+512	// while (p < (datblk+512)) 
	jl	5b

2:	mov	r9,r9		// while (cnt>0)
	jgt	1b

error:	
	li	r12,0x1FE0	// Front panel CRU
	sbo	11		// Fault
	idle

found:	
	mov	r3,r0		// "unix" found, fetch inode
	blwp	@geti
	mov	r0,r10
	clr	r8		// lbn = 0
2:	mov	r8,r1		// pbn = lbn2pbn(inop, lbn)
	mov	r10,r0
	blwp	@lbn2pbn
	mov	r7,@wp3+20	// cfread(pbn, datblk)
	mov	r0,@wp3+2
	blwp	@read
	mov	r8,r8		// if (lbn==0)
	jne	1f
	mov	@2(r7),r9	//   cnt = (hdr.text + hdr.data + 8 + 511) / 512
	a	@4(r7),r9
	ai	r9,8+511
	srl	r9,9
	clr	r4		//   p = 0
	mov	r7,r5		//   q = datblk + 16
	ai	r5,16
1:	mov	(r5)+,(r4)+	// do { *p++=*q++; } while (q<datblk+512)
	ci	r5,datblk+512
	jl	1b
	inc	r8		// lbn++
	mov	r7,r5		// q = datblk
	dec	r9		// while (--cnt)
	jne	2b

	lwpi	rwp
	mov	@wp3+24,r1	// Restore vals in ROM WS for kernel.
	mov	@wp3+18,r2
	mov	@wp3+16,r3
	mov	@sectrk,r4
	mov	@mulsec,r5

	blwp	@0		// start kernel through reset vector

// get a pointer to the inode in R0 of caller
// pointer left in R0 of caller

geti:
	wp2;getino

getino:
	mov	(r13),r10
	mov	r10,r2
	ai	r10,31
	sla	r10,5
	mov	r10,r9
	srl	r9,9
	mov	r10,r8
	andi	r8,511
	li	r1,inoblk
	c	r2,@lstino
	bjeq	L9
	mov	r1,@wp3+20
	mov	r9,@wp3+2
	blwp	@read
L9:
	mov	r2,@lstino
	a	r1,r8
	mov	r8,(r13)
	rtwp

// convert file logical block no. to physical block no.
// lbn = r12, inop = r11

	FLARGE = 0x1000

lbn2pbn:
	wp2;l2p

l2p:
	mov	(r13),r11
	mov	@2(r13),r12
	// check inode sanity
	ci	r12,0x7fff
	bjh	error

	mov	r11,r2		// if (inop->flags & FLARGE) goto large;
	mov	(r2),r2
	andi	r2,FLARGE
	bjne	large

small:	mov	r12,r2		// if (blkno>7) goto error;
	ci	r2,7
	bjh	error
	sla	r2,1		// return (inop->addr[2*blkno]);
	a	r11,r2
	mov	@8(r2),(r13)
	rtwp

large:	mov	r12,r2		// ind = (inop->addr[2*blkno/256]);
	srl	r2,8
	sla	r2,1
	a	r11,r2
	mov	@8(r2),r10
	bjeq	error

	// fetch indirect block as needed
	c	r10,@lstind	// if (ind!=lstind)
	bjeq	1f
	li	r0,indblk	//    read(ind,indblk)
	mov	r0,@wp3+20
	mov	r10,@wp3+2
	blwp	@read
	mov	r10,@lstind	// lstind = ind

	// fetch physical block number from indirect block
1:	mov	r12,r9			// offs = (blkno & 0xff) *2
	andi	r9,0xff
	sla	r9,1
	mov	@indblk(r9),(r13)	// R0 = indblk[offs]
	rtwp

// Driver for the 990 TILINE disks.

// defines for the TILINE disks

	CMDMASK  = 0xF8FF
	SECREC   = 0x0100

// defines for disk commands.

	READCMD  = 0x0200	// Read.
	SEEKCMD  = 0x0600	// Seek.

read:
	wp3; cfread

// read 256 words starting at block no. R1 to addr R10
// R12 = device address.
// R9 = device unit.

cfread:
	// seek
	sla	r1,1		// * 2, may bypass...
	mov	@sectrk,r3	// sectors per track
	mov	r3,r2
	sla	r2,1		// * 2
	a	r1,r2		// block += sectrk * 2
	clr	r1
	div	r3,r1
	ori	r2,SECREC
	mov	r2,r5		// sector
	clr	r0
	div	r8,r0		// div heads
	ori	r1,SEEKCMD
	mov	r1,r4		// head | SEEK
	mov	r0,r6		// cyl
	li	r7,512
	mov	r9,r2
	srl	r2,4
	bl	@issue
	// read block
	andi	r4,CMDMASK
	ori	r4,READCMD
	clr	r2
	bl	@issue
	rtwp

// issue commands to TILINE

issue:
1:	mov	@14(r12),r0	// Check for busy
	jlt	2f
	jmp	1b
2:	mov	r12,r3		// Not busy, send command.
	clr	(r3)+
	mov	r4,(r3)+
	mov	r5,(r3)+
	mov	r6,(r3)+
	mov	r7,(r3)+
	mov	r10,(r3)+
	mov	r9,(r3)+
	clr	(r3)		// Start the operation
3:	mov	(r3),r0		// wait until done.
	jlt	4f
	jmp	3b
4:	mov	r2,r2		// If Seek command
	jeq	5f
	mov	(r12),r1	// then check attention bits.
	coc	r2,r1
	jne	3b
5:	b	(r11)

unix:	"unix\0"		// name of kernel
	.even

	.data
lstino:	0
lstind:	-1
sectrk: 0
mulsec: 0
