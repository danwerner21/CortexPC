	IDT	'UBOOT'
	TITL	'UNIX Track 1 loader'
*
* UBOOT - Track 1 loader.
*	This loader loads the Unix loader.
*
R0	EQU	0
R1	EQU	1
R2	EQU	2
R3	EQU	3
R4	EQU	4
R5	EQU	5
R6	EQU	6
R7	EQU	7
R8	EQU	8
R9	EQU	9
R10	EQU	10
R11	EQU	11
R12	EQU	12
R13	EQU	13
R14	EQU	14
R15	EQU	15
*
* Disk commands and codes
*
DEVADR	EQU	>F800		Disk TILINE address
DEVUNT	EQU	>0800		Disk Unit 0 = >0800
*					  1 = >0400
*					  2 = >0200
*					  3 = >0100
*
STRRCM	EQU	>0000		Store registers
READCM	EQU	>0200		Read
WRITCM	EQU	>0300		Write
REDUCM	EQU	>0400		Read unformatted
SEEKCM	EQU	>0600
RESTCM	EQU	>0700		Restore
*
ERRMSK	EQU	>21FF
*
PNLCRU	EQU	>1FE0		Front panel CRU
	PAGE
*
START	MOV	R1,@DSKADR	Save ROM Address and Unit.
	MOV	R2,@DSKUNT
*
	LI	R1,RESTCM	Restore disk
	MOV	R1,@DSKCMD
	LI	R1,6
	MOV	R1,@DSKLEN
	CLR	@DSKCYL
	CLR	@DSKSEC
	CLR	@DSKHED
	BLWP	@DISKH
	DATA	DSKPRM,0,OOPS0
*
	LI	R1,STRRCM	Store registers
	MOV	R1,@DSKCMD
	BLWP	@DISKH
	DATA	DSKPRM,DSKREG,OOPS1
*
	LI	R1,REDUCM	Read unformatted
	MOV	R1,@DSKCMD
	BLWP	@DISKH
	DATA	DSKPRM,DSKFMT,OOPS1
*
	CLR	R1		Calc Block 0 offset
	MOV	@DSKREG+2,R3	Get sectors/track
	SRL	R3,8
	MOV	R3,R2		sectrk * 2
	SLA	R2,1
	DIV	R3,R1
	ORI	R2,>0100
	MOV	R2,@DSKSEC	Sector
	CLR	R0
	MOV	@DSKREG+4,R4	Get tracks/cyl
	SRL	R4,11
	DIV	R4,R0
	MOV	R1,@DSKHED	Head
	MOV	R0,@DSKCYL	Cylinder
	LI	R1,SEEKCM	Seek
	MOV	R1,@DSKCMD
	CLR	@DSKLEN
	BLWP	@DISKH
	DATA	DSKPRM,0,OOPS2
*
	MOV	@DSKCMD,R4
	ANDI	R4,>F8FF
	ORI	R4,READCM	Read
	MOV	R4,@DSKCMD
	LI	R4,512
	MOV	R4,@DSKLEN
	BLWP	@DISKH
	DATA	DSKPRM,>8000,OOPS3
*
	MOV	@DSKADR,R1	Device address.
	MOV	@DSKUNT,R2	Unit.
	MOV	@DSKREG+4,R3	Heads
	SRL	R3,11
	MOV	@DSKREG+2,R4	Get sectors/track
	SRL	R4,8
	MOV	@DSKFMT+4,R6	Get sector size
	JNE	GOTSSZ
	CLR	R5
	JMP	BOOT
GOTSSZ	SLA	R6,1
	CI	R6,512		If size == 512
	JLT	SMLSEC
	CLR	R5		 then clear 
	JMP	BOOT
SMLSEC	SETO	R5		 else set
BOOT	B	@>8000
*
OOPS3	EQU	$		Read sector error.
	INC	R15
OOPS2	EQU	$		Seek error.
	INC	R15
OOPS1	EQU	$		Store registers error.
	INC	R15
OOPS0	EQU	$		Restore error.
	INC	R15
	LI	R0,>DEAD
	MOV	R15,R1
	LI	R12,PNLCRU	Put status on front panel
	LDCR	R15,8
	SWPB	R15
	LDCR	R15,8
	SBO	11		Fault
	IDLE			DIE
*
DSKPRM	EQU	$
DSKCMD	DATA	$-$		Command
DSKHED	DATA	$-$		Head
DSKSEC	DATA	$-$		Sector
DSKCYL	DATA	$-$		Cylinder
DSKLEN	DATA	$-$		Length
DSKUNT	DATA	$-$		Unit
DSKREG	BSS	6		Disk registers
DSKFMT	BSS	6		Disk format
	PAGE
*
* Disk Handler
*
DISKH	DATA	DISKWS,DISKEP
DISKWS	DATA	0,0,0,0,0,0,0,0
	DATA	0,0,0,0
DSKADR	DATA	DEVADR,0,0,0
*
DISKEP	EQU	$
	MOV	R12,R7
	AI	R7,14
	MOV	R14,R10		Ignore error now
	BL	@BZYCHK
	MOV	*R14+,R10	Get parm addr
	MOV	*R10+,R0	Get disk command
	MOV	*R10+,R1	Get head
	SOC	R1,R0
	MOV	*R10+,R1	Get Sector
	MOV	*R10+,R2	Get Track
	MOV	*R10+,R3	Get Length
	MOV	*R14+,R4	Get buffer addr
	MOV	*R10+,R5	Get Unit
	MOV	*R14+,R10	Get error vector
	MOV	R0,R9		Check if seek or restore
	ANDI	R9,>0F00
	CI	R9,SEEKCM
	JLT	DSK010
	MOV	R5,R9
	SRL	R9,4
	JMP	DSK020
DSK010	CLR	R9
DSK020	BL	@ISSUE
	RTWP
*
ISSUE	MOV	R12,R7		Issue TILINE command
	CLR	*R7+
	MOV	R0,*R7+
	MOV	R1,*R7+
	MOV	R2,*R7+
	MOV	R3,*R7+
	MOV	R4,*R7+
	MOV	R5,*R7+
	CLR	*R7		Start transfer
BZYCHK	MOV	*R7,R8		Get status
	JLT	ISU020		Done?
	JMP	BZYCHK		No, wait.
ISU020	MOV	R9,R9		Seek or Restore?
	JEQ	ISU025		No, go on.
	MOV	*R12,R6		Yes, check attention bits.
	COC	R9,R6
	JNE	BZYCHK
ISU025	ANDI	R8,ERRMSK	Yes, Errors?
	JEQ	ISU030		No
	MOV	R10,R14		Yes, set error vector.
	MOV	*R12,@4(R13)
	MOV	*R7,@6(R13)	Set error status.
ISU030	RT
*
	END
