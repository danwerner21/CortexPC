//
// crypt algorithm, based on the M-209 rotor cipher
//
	.globl	_crypt
_crypt:
	mov	r11,r0
	bl	@csv
	mov	@18(bp),r1
	mov	r1,r0
	li	r1,key
	movb	@V004,(r1)+
	movb	@V034,(r1)+
1:
	ci	r1,key+64
	jhe	1f
	movb	(r0)+,(r1)+
	jne	1b
1:
	dec	r1
	
	.data
V004:	.byte 0x04
V034:	.byte 0x1c
	.text
//
//	fill out key space with clever junk
//
	li	r0,key
1:
	movb	@-1(r1),r2
	movb	(r0)+,r3
	xor	r3,r2
	movb	r2,(r1)+
	ci	r1,key+128
	jl	1b
//
//	establish wheel codes and cage codes
//
	li	r4,wheelcode
	li	r5,cagecode
	li	r6,256
2:
	clr	r2
	clr	(r4)
	li	r3,wheeldiv
3:
	clr	r0
	mov	r6,r1
	div	(r3)+,r0
	a	r1,r2
	andi	r2,0xffdf
	soc	@shift(r2),(r4)
	ci	r3,wheeldiv+6
	jhe	4f
	soc	@shift+4(r2),(r5)
4:
	ci	r3,wheeldiv+10
	jl	3b
	dect	r6
	inct	r4
	inct	r5
	ci	r4,wheelcode+256
	jl	2b
//
	.data
shift:
	0x0001; 0x0002; 0x0004; 0x0008;
	0x0010; 0x0020; 0x0040; 0x0080;
	0x0100; 0x0200; 0x0400; 0x0800;
	0x1000; 0x2000; 0x4000; 0x8000;
	0x0001; 0x0002

wheeldiv:
	32; 18; 10; 6; 4

	.bss
cagecode:	.=.+256
wheelcode:	.=.+256

	.text
//
//	make the internal settings of the machine
//	both the lugs on the 128 cage bars and the lugs
//	on the 16 wheels are set from the expanded key
//
	li	r0,key
	li	r2,cage
	li	r3,wheel
1:
	movb	(r0)+,r1
	andi	r1,0x7f00
	sra	r1,7
	mov	@cagecode(r1),(r2)+
	mov	@wheelcode(r1),(r3)+
	ci	r0,key+128
	jl	1b
//
//	now spin the cage against the wheel to produce output.
//
	li	r6,word
	li	r3,wheel+128
3:
	dect	r3
	mov	(r3),r2
	li	r0,cage
	clr	r5
1:
	mov	(r0)+,r8
	inv	r8
	mov	r2,r7
	szc	r8,r7
	jeq	2f
	inc	r5
2:
	ci	r0,cage+256
	jl	1b
//
//	we have a piece of output from current wheel
//	it needs to be folded to remove lingering hopes of
//	inverting the function
//
	clr	r4
	li	r7,26+26+10
	div	r7,r4
	ai	r5,'0'
	ci	r5,'9'
	jle	1f
	ai	r5,'A'-'9'-1
	ci	r5,'Z'
	jle	1f
	ai	r5,'a'-'Z'-1
1:
	swpb	r5
	movb	r5,(r6)+
	ci	r6,word+8
	jl	3b
	li	r2,word
	b	@cret

	.bss
key:	.=.+128
word:	.=.+32
cage:	.=.+256
wheel:	.=.+256
