// C run time library

.if 0 //CORTEX
	cwp = 0xf000
.endif
	iwp = 0xf020

resvec:	cwp
	start
i1vec:	0
	0
i2vec:	0
	0
i3vec:	iwp
	ignore
	
	.globl _main, _exit, sysdie
	.text

.if 1 //-CORTEX
cwp:	.=.+32
.endif

start:	
	bl	@_main
	inct	sp
	mov	r2,(sp)
	bl	@_exit
	// does not return but for safety:
sysdie:
	limi	0
	idle

ignore:	rtwp

