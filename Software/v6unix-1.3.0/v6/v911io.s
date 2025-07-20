//
// v911io.s - 911 VDT io speed up routines.
//

//
// Bit offsets on a 911 vdt board 
//

// SETB word 0
V911HILO =	7
V911WSTB =	8
V911TEST =	9
V911CMOV =	10
V911BINK =	11
V911KENB =	12
V911DUAL =	13
V911DENB = 	14
V911WSEL =	15
// SETB word 1
V911DCSR =	12
V911KACK =	13
V911BEEP =	14

// TB word 0
V911KRDY =	15
// TB word 1
V911CMSB =	10
V911KMSB =	11
V911DTR  =	12
V911PST  =	13
V911KPAR =	14

V911SIZE =	1920
//
// v911clear (addr)
//
	.globl	_v911clear
_v911clear:
	mov	(sp),r12
	sbo	V911WSEL
	sbz	V911DCSR
	li	r2,0x2000
	ldcr	r2,11
	li	r3,V911SIZE
	sbz	V911WSEL
	sbz	V911BINK
	sbz	V911DENB
CLR010:
	ldcr	r2,8
	sbo	V911WSTB
	sbz	V911CMOV
	dec	r3
	jgt	CLR010
	sbo	V911DENB
	sbo	V911BINK
	sbo	V911WSEL
	sbz	V911DCSR
	b	(r11)
//
// v911scroll (addr)
//
	.globl	_v911scroll
_v911scroll:
	mov	(sp),r12
	sbo	V911WSEL
	sbz	V911DCSR
	sbz	V911WSEL
	sbz	V911BINK
	li	r2,V911SIZE-1
	mov	r2,r4
SCL010:
	li	r3,0x2020
SCL020:
	sbo	V911WSEL
	ldcr	r2,11
	sbz	V911WSEL
	ori	r3,0x8000
	ldcr	r3,8
	stcr	r3,8
	sbo	V911WSTB
	ai	r2,-80
	jgt	SCL020
	jeq	SCL020
	dec	r4
	mov	r4,r2
	ci	r2,V911SIZE-81
	jeq	SCL030
	ci	r2,V911SIZE-1041
	jne	SCL010
	jmp	SCL020
SCL030:
	li	r2,V911SIZE-80
	sbo	V911WSEL
	ldcr	r2,11
	sbo	V911DCSR
	sbz	V911WSEL
	sbo	V911BINK
	b	(r11)
