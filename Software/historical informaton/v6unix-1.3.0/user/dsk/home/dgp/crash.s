	.globl	_main
	.text
_main:
	mov	r11,r0
	bl	@csv
	bjmp	L1
L2:	li	r0,L4
	mov	r0,(sp)
	bl	@_printf
	clr	r2	// crash on /12 
	clr	(r2)
	bjmp	L3
L3:	lst	r2	// crash on /10
	idle		// crash on all
L1:	bjmp	L2
	.globl
	.data
L4:	.byte	0x47,0x6f,0x6f,0x64,0x62,0x79,32,87,111,114,108,100,33,10,0
