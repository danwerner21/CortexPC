       TITL 'START MODULE - CORTEX BASIC REV. 1.1'
       IDT  'START'
*
*      CORTEX BASIC REVISION 1.1
*
       COPY 'IODEFS.INC'
       PAGE
       DEF  BEGN1,NRV,LNSZ,NIC,BUFEND,RBUF,RBUFE,OUTBUF
       DEF  EXTWP,CONFIG,PRAM,MEMSIZ,SRATE,SETVEC
       DEF  B43,NEWP1,FUZZ,FNSZ,GSSZ,MID0,OVINT,ILLMID
       DEF  WP10L,SQRI,RANDS,CALLWP,CALL12,SYSFLG
       DEF  DMYHDR,DH$ARF,DH$PGN,DH$SLT,DH$VNT,DH$SIZ
       DEF  NOERR,DTEND,DH$END,DH$PLL,DH$HCS,HDRSIZ
       DEF  E$TEMP,BELCNT,CLKWS,CLKADR,CLKT01,CLKT02
       DEF  NEWP,DS,DS1,DS2,DS3,TRAFLG,CNTDWN
       DEF  SWPTBL,WPR1,WPR103,WPR104,WPR108
       DEF  BEGN,TY0,IOB,EUS,FNS,GSS,EVSKE,EDTMP
       DEF  CCSAVE,UFT,BUS,EORBUS,SLT,EVSKB,EBP,SSP
       DEF  RENSTA,RENINC,VNT,VDT,NVD,NVS
       DEF  CURFLG,UNIT,DCNT,MODE,PLC,DLIM,DLC,DDM
       DEF  LNUM,SLN,CCNT,EFLG,ENUM,ELNM,ELSF
       DEF  FFLG,HFLG,IFLG,PLF,BCRU,BAUD,R8STOR,SYSLMT
       DEF  GSC,AINC,RENCMP,FPAC3,FPAC4,FPWP
       DEF  TEMP,TEMP2,TEMP4,TEMP6,FPAC,FPAC2
       DEF  F$WHO,CVCH,CVHD,CVHD01,CVHD12,CVHD15
       DEF  WPR2,WPR1,PTRS,NEWY,PRDY,DSRCNT,RTSTOR
       DEF  CRLF,CRLF1,OUTDSR,ERROR2,ERROR
       DEF  RECPTR,INPTR,RENLNE,RENRET,STACNT,IOSTOR
       DEF  DEVTBL,VDPWP0,XLOC,YLOC,NXLOC,NYLOC,FBCOL,VMODE
       DEF  BITMAP,VDPWP1,V1R5L,V1R6L,ESCFLG,VDPST,FDCDON
       DEF  V1R3L,V1R4L,LOADPT,WP9928,ASMPTR
       DEF  WORKS,WORK5,WORK11
       DEF  INITFG,BPT0V,BPTSET,BPTADD
       DEF  BPTDTA,ASMOPC,MREGS,MREG1,MREG2,MREG3,MREG6
       DEF  MREG13,MREG14,EREGS,IREGS,PC,IREG6,IREG12
       DEF  P$EUS,P$FNS,P$GSS,P$UFT,P$IOB,XREGS
       DEF  HALTO$,BLSTOR
*
       REF  CMOF$,VDPDSR,SETTXT
       REF  CENTRO
       REF  GETC$,TIMC,MIDPC,INT4PC,C1
       REF  FLDD,FSRD,FAD,FSD,FMD,FDD,FSCL,FNRM
       REF  FCLR,FNEG,FLOAT,EVFX,CVBD,CVBI,ERRS
       REF  CKEX,CLRV,EDIT,GTLN,NLIN
       REF  TYPE$,D$LDWP,D$LDPC,MONTOP
       REF  MBEGN,MPRDY,MCRLF,B62
       REF  LOOK$,B47,FP5E5,FIX,EVERZ,START
       PAGE
*
*
* ALLOCATION ADDRESSES
*
ERAM   EQU  >F000             END OF RAM
BROM   EQU  >0000             BEGINNING OF ROM
PRAM   EQU  >ECF6             BEGINING OF PERMANENT RAM
*
* SYSTEM EQUATES
*
NRV    EQU  4                 NUMBER OF RESERVED VARIABLES
LNSZ   EQU  132               I/O BUFFER SIZE
NIC    EQU  LNSZ-32           NUMBER ON INPUT CHARACTERS
FUZZ   EQU  >3B00             APPROXIMATELY 1E-7
FNSZ   EQU  10                NEXTED FOR/NEXT STACK SIZE
GSSZ   EQU  20                NEXTED GOSUB STACK SIZE
SQRI   EQU  4                 # OF SQR NEWTON ITERATIONS
*
P$EUS  EQU  PRAM-2
P$FNS  EQU  P$EUS-(FNSZ*18)
P$GSS  EQU  P$FNS-(GSSZ*4)
P$UFT  EQU  P$GSS-(26*4)-4
P$IOB  EQU  P$UFT-LNSZ+4
*
* DEFINE FLOATING POINT XOPS
*
       DXOP LOADF,0           LOAD FPAC
       DXOP STORE,1           STORE FPAC
       DXOP FADD,2            ADD TO FPAC
       DXOP FSUB,3            SUBTRACT FROM FPAC
       DXOP FMUL,4            MULTIPLY FPAC
       DXOP FDIV,5            DIVIDE FPAC
       DXOP SCALE,6           SCALE FPAC
       DXOP NORMAL,7          NORMALIZE FPAC
       DXOP CLEAR,8           CLEAR FPAC
       DXOP NEGATE,9          NEGATE FPAC
       DXOP FLOATF,10         FLOAT FPAC
       DXOP EVFIX,11          EVALUATE AND FIX
       DXOP OUTFP,12          OUT FLOATING POINT #
       DXOP OUTINT,13         OUT INTEGER
ERROR  EQU  >2F80             XOP XX,14  (ERROR CALL)
ERROR2 EQU  ERROR+>20
       PAGE
*
*       RAM
*
        DORG PRAM
*
* RANDON NUMBER GENERATOR SEED
*
RANDS  DATA 0                 RANDOM SEED
**** ABOVE RAM IS CLEARED ON RUN ******
EDTMP  DATA 0
*
* SYSTEM POINTERS
*
PTRS   EQU  $                 SYSTEM POINTERS
IOB    DATA 0                 I/O BUFFER
EUS    DATA 0                 END-USER-STORAGE
FNS    DATA 0                 FOR/NEXT STACK
GSS    DATA 0                 GOSUB STACK
UFT    DATA 0                 USER FUNCTION TABLE
BUS    DATA 0                 BEGINNING OF USER STORAGE
EORBUS DATA 0                 END OF RAM - BEGINNING OF USER
STACNT DATA 0                 STATEMENT INDENT COUNTER
RENCMP DATA 0                 COMPRESSION LIMIT
RENRET DATA 0                 RENUMBER TMP. STORAGE
RENLNE DATA 0                 RENUMBER CURRENT LINE #
RENSTA DATA 0                 RENUMBER START LINE #
RENINC DATA 0                 RENUMBER INCREMENT
*
* CLOCK INTERRUPT REGISTERS
*
CLKWS  DATA 0                 FINE COUNTER
CLKADR DATA 0                 HOURS
       DATA 0                 MINUTES
       DATA 0                 SECONDS
CLKT01 DATA 0                 TIC COUNTER
CLKT02 DATA 0                 TIC COUNTER
DS     DATA 0,0,0             TEMPORARY DATA STORAGE
DS1    DATA 0,0,0
       DATA 0,0,0,0           R12,R13,R14,R15
R8STOR DATA 0                 STORE FOR R8 DURING ENTER
RTSTOR DATA 0                 TEMP. STORE FOR R11
INT1WP EQU  $-20
CNTDWN DATA 0                 R11 - DELAY COUNTER
TRAP1  DATA 0                 R12 - BLWP VECTOR POINTER
       DATA 0,0,0             R13-15 CONTEXT
BELCNT DATA 0                 BELL COUNTER
TRAFLG DATA 0                 TRACE FLAG
CALLWP BSS  16*2              CALL WORKSPACE
CALL12 EQU  CALLWP+24         CALL WORKSPACE R12
SLN    DATA 0                 SAVED LINE NUMBER
CCNT   DATA 0                 COLUMN COUNTER
CURFLG DATA 0                 CURSOR FLAG 0=ON,-1=OFF
       PAGE
*
ELSF   DATA 0                 ELSE FLAG
FFLG   DATA 0                 FORMATTING FLAG
IFLG   DATA 0                 INPUT FLAG
PLF    DATA 0                 PROGRAM LOAD FLAG
MIDWP  BSS  16*2              MID HANDLER WORKSPACE
OVINT  DATA 0                 A.O. INTERRUPT HANDLER
MID0   DATA 0                 MID OPCODE 0 HANDLER
ILLMID DATA 0                 ILLEGAL MID HANDLER
OUTDSR DATA 0                 OUTPUT PRE-PROCESSOR
INT4WP EQU  $                 LEVEL 4 INTERRUPT WORKSPACE
       DATA 0,0,0,0           R0-R3
LOADPT DATA 0                 R4
       DATA 0                 R5
INPTR  DATA 0                 R6
DSRCNT DATA 0                 R7
RECPTR DATA 0                 R8
TRAP4  DATA 0                 R9
       DATA 0,0,0,0,0,0       R10-15
RBUF   BSS  40                RECEIVE BUFFER
RBUFE  EQU  $
OUTBUF BSS  80                OUTPUT BUFFER
BUFEND EQU  $-2
*
*  WORKSPACES FOR VDP ROUTINES
*
       EVEN
VDPWP0 DATA 0                 R0
XLOC   BYTE 0                 R1 MSB
YLOC   BYTE 0                 R1 LSB
       DATA 0,0               R2-R3
BITMAP BYTE 0,0,0,0,0,0,0,0   R4-R7
       DATA 0,0,0,0,0,0,0,0   R8-R15
********
       EVEN
VDPWP1 BSS  16*2              R0-R15
V1R3L  EQU  VDPWP1+7          LSB OF R3
V1R4L  EQU  VDPWP1+9          LSB OF R4
V1R5L  EQU  VDPWP1+11         LSB OF R5
V1R6L  EQU  VDPWP1+13         LSB OF R6
********
       EVEN
WP9928 BSS  16*2              9928 DSR WORKSPACE
       PAGE
* I/O LOCAL STORAGE AREA
       EVEN
IOSTOR EQU  $
CCSAVE BYTE 0                 DEVICE 1    CURSOR CHR. SAVE
FBCOL  BYTE 0                 DEVICE 1    FGND/BGND COLOUR
NXLOC  BYTE 0                 DEVICE 1    NEW X ORDINATE
NYLOC  BYTE 0                 DEVICE 1    NEW Y ORDINATE
       DATA 0,0               DEVICE 2    UNLD PTR, CR COUNT
       DATA 0,0               DEVICE 3    UNLD PTR, CR COUNT
       DATA 0,0               DEVICE 4    CENTRONICS PRINTER
       DATA 0,0               DEVICE 5
       DATA 0,0               DEVICE 6
       DATA 0,0               DEVICE 7
       DATA 0,0               DEVICE 8
       DATA 0,0               DEVICE 9
       DATA 0,0               DEVICE 10
       DATA 0,0               DEVICE 11
       DATA 0,0               DEVICE 12
       DATA 0,0               DEVICE 13
       DATA 0,0               DEVICE 14
       DATA 0,0               DEVICE 15
       DATA 0,0               DEVICE 16
       PAGE
*
*      EXTENDED COMMAND WORKSPACE
*
EXTWP  BSS  16*2
*
* LOBUG WORKSPACE DEFINITION
*
       EVEN
WORKS  EQU  $                 ASMBLR WORKSPACE
WORK5  EQU  WORKS+10
WORK11 EQU  WORKS+22
BLSTOR EQU  WORKS+32          GETLINE RETURN STORAGE
ASMPTR EQU  BLSTOR+2          ASSEMBLER INPUT BUFFER PTR
INITFG EQU  ASMPTR+2          INITIALIZED FLAG
BPT0V  EQU  INITFG+2          USER VECTORS FOR BPT ZERO
BPTSET EQU  BPT0V+2           BREAKPOINTS ARE SET FLAG
BPTADD EQU  BPTSET+2          BREAKPOINT ADDRESS STORE
BPTDTA EQU  BPTADD+32         BREAKPOINT DATA STORE
ASMOPC EQU  BPTDTA+32         4 BYTES
MREGS  EQU  ASMOPC+4          MAIN REGISTERS
MREG1  EQU  MREGS+2
MREG2  EQU  MREGS+4
MREG3  EQU  MREGS+6
MREG6  EQU  MREGS+12
MREG13 EQU  MREGS+26
MREG14 EQU  MREGS+28
EREGS  EQU  MREGS+10          ECHO REGISTERS (R11-R15 ONLY)
IREGS  EQU  EREGS+12          R/W REGISTERS (R10-R15 ONLY)
PC     EQU  EREGS+16          ASMBLR PC STORAGE
IREG6  EQU  IREGS+12
IREG12 EQU  IREGS+24
XREGS  EQU  IREGS+14          HEX I/O, MESSAGE
       DORG XREGS+32
       PAGE
************************************************************
*
*          TAPE/EPROM HEADER BLOCKS
*
*          BASIC               MACHINE CODE
*   ********************   ******************** RUN  =>A5A5
*   *  AUTO RUN FLAG   *   *  AUTO RUN FLAG   * NORUN=>5A5A
*   ********************   ********************
*   *   8 BYTE NAME    *   *   8 BYTE NAME    * NULL FILLED
*   ********************   ********************
*   *   DISP TO SLT    *   *       >0000      *
*   ********************   ********************
*   *   DISP TO VNT    *   *   LOAD ADDRESS   *
*   ********************   ********************
*   *     NVD - VDT    *   *   ENTRY POINT    * (OFFSET)
*   ********************   ********************
*   *   LOAD LENGTH    *   *   LOAD LENGTH    * (BYTES)
*   ********************   ********************
*   *     CHECKSUM     *   *     CHECKSUM     * HEADER ONLY
*   ********************   ********************
*
DMYHDR EQU  $
DH$ARF BSS  2            RUN/NORUN
DH$PGN BSS  8            PROGRAM NAME
DH$SLT BSS  2            DISP TO SLT
DH$VNT BSS  2            DISP TO VNT
DH$SIZ BSS  2            NVD-VDT
DH$PLL BSS  2            LENGTH
DH$HCS BSS  2            HEADER CHECKSUM
HDRSIZ EQU  $-DMYHDR     HEADER BLOCK SIZE
DH$END EQU  $            END OF DUMMY HEADER
****************
SLT    DATA 0                 STATEMENT LOCATION TABLE
VNT    DATA 0                 VARIABLE DEFINITION TABLE
VDT    DATA 0                 NEXT VARIABLE DEFINITION
NVD    DATA 0                 NEXT VARIABLE POINTER
NVS    DATA 0                 NEXT VARIABLE STORAGE
GSC    DATA 0                 GOSUB STACK COUNTER
TY0    DATA 0                 OUTPUT CHARACTER BUFFER
AINC   DATA 0                 AUTO-INCREMENT
DCNT   DATA 0                 INDENT COUNTER
MODE   DATA 0                 MODE SWITCH
PLC    DATA 0                 PROGRAM LINE COUNTER
DLIM   DATA 0                 DELIMITER
DLC    DATA 0                 DATA LINE COUNTER
DDM    DATA 0                 DATA DELIMITER PTR
LNUM   DATA 0                 LAST ENTERED LINE #
F$WHO  DATA 0                 0=BASIC, -1= MONITOR
*
DS2    DATA 0,0,0             TEMP FP STORAGE
DS3    DATA 0,0,0             TEMP FP STORAGE
E$TEMP DATA 0,0,0             EVALUATOR TEMP STORAGE
*
* STACKS FOR EDIT,LIST, AND EVAL
*
CVCH   DATA 0,0,0
CVHD   EQU  $
CVHD01 EQU  CVHD+1
CVHD12 EQU  CVHD+12
CVHD15 EQU  CVHD+15
EVSKB  EQU  $+14              BEGINNING OF STACK
EBP    BSS  132               EDIT BUFFER
SSP    BSS  30                SUBSCRIPT STACK
EVSKE  EQU  $                 END STACK
*
* FLOATING POINT WORKSPACE AND TEMPORARY STORAGE
*
TEMP   DATA 0
TEMP2  DATA 0
TEMP4  DATA 0
TEMP6  DATA 0
FPAC   DATA 0                 FP ACCUMULATER
FPAC3  EQU  $+1
FPAC2  DATA 0
FPAC4  DATA 0
       BSS  13*2
FPWP   EQU  FPAC              FLOATING POINT WORKSPACE
*
* 2ND LEVEL REGISTERS
*
WPR2   BSS  16*2              EVAL, TRIG, CONV REGISTERS
*
* 1ST LEVEL REGISTERS - RESET WORKSPACE ALSO
*
WP10L  EQU  $+1               R0 LSB REFERENCE
WPR1   DATA 0,0,0             R0 - R2
WPR103 DATA 0                 R3
WPR104 DATA 0                 R4
       DATA 0,0,0             R5 - R7
WPR108 DATA 0                 R8
WPR109 DATA 0                 R9 (POINTER REGISTER)
       DATA 0,0,0,0,0,0       R10 - R15
ENDRAM EQU  $
*
* CHECK THAT ENDRAM IS OK
*
       ASMIF (>F0FC-ENDRAM)&>8000
  SPLAT !!! - RUN OUT OF INTERNAL RAM
       ASMEND
       PAGE
       RORG BROM
*
* INTERRUPT VECTOR TABLE
*
IV0    DATA WPR1,START
IV1    DATA INT1WP,INT1PC
IV2    DATA MIDWP,MIDPC
IV3    DATA CLKWS,CLKI
IV4    DATA INT4WP,INT4PC
*
*      USER READABLE SYSTEM FLAGS
*
SYSFLG EQU  $          *** SYSTEM FLAG AREA ***
HFLG   DATA 0          HELP FLAG            SYS(0)
ENUM   DATA 0          LAST ERROR NUMBER    SYS(1)
ELNM   DATA 0          LAST ERROR LINE #    SYS(2)
BCRU   DATA 0          CRU BASE ADDRESS     SYS(3)
EFLG   DATA 0          ERROR FLAG           SYS(4)
UNIT   DATA 0          UNIT FLAG            SYS(5)
ESCFLG DATA 0          ESCAPE DISABLE FLAG  SYS(6)
MEMSIZ DATA 0          AUTO-SIZE LOW LIMIT  SYS(7)
NOERR  DATA 0          EDIT ERR ENABLE FLG  SYS(8)
VMODE  DATA 0          MODE 0=TEXT,-1=GRAPH SYS(9)
FDCDON DATA 0          FDC INTERRUPT FLAG   SYS(10)
       DATA -1         DUMMY
VDPST  EQU  11         VDP STATUS FLAG      SYS(11)
       DATA BEGN       BASIC ENTRY POINT    SYS(12)
       DATA TRAP1      INT1   BLWP VECTOR   SYS(13)
       DATA TRAP4      INT4   BLWP VECTOR   SYS(14)
       DATA MID0       MID 0  BLWP VECTOR   SYS(15)
       DATA OVINT      AO     BLWP VECTOR   SYS(16)
       DATA ILLMID     ILLMID BLWP VECTOR   SYS(17)
       DATA OUTDSR     OUTPUT BLWP VECTOR   SYS(18)
       DATA BPT0V      BP0    BL   VECTOR   SYS(19)
SYSLMT EQU  19         MAX SYSTEM LIMIT
*
       DATA -1,-1
       PAGE
*
* XOP VECTOR TABLE
*
       DATA FPWP,FLDD         XOP XX,0  LOAD FPAC
       DATA FPWP,FSRD         XOP XX,1  STORE FPAC
       DATA FPWP,FAD          XOP XX,2  ADD TO FPAC
       DATA FPWP,FSD          XOP XX,3  SUBTRACT FROM FPAC
       DATA FPWP,FMD          XOP XX,4  MULTIPLY FPAC
       DATA FPWP,FDD          XOP XX,5  DIVIDE FPAC
       DATA FPWP,FSCL         XOP XX,6  SCALE FPAC
       DATA FPWP,FNRM         XOP XX,7  NORMALIZE
       DATA FPWP,FCLR         XOP XX,8  CLEAR
       DATA FPWP,FNEG         XOP XX,9  NEGATE
       DATA FPWP,FLOAT        XOP XX,10 FLOAT FPAC
       DATA WPR2,EVFX         XOP XX,11 EVALUATE AND FIX
       DATA WPR2,CVBD         XOP XX,11 OUT FP #
       DATA WPR2,CVBI         XOP XX,13 OUT INTEGER #
D$WPR1 DATA WPR1,ERRS         USER ERROR
       DATA -1,-1
MONITR B    @MONTOP           ENTRY TO MONITOR
       PAGE
*
*      DEVICE TABLE:-
*
*  1,  IF THE ENTRY IS 0 THEN THE DEVICE IS NOT PRESENT.
*  2,  IF THE LS BIT OF THE ENTRY IS A '0' THEN
*           THE ENTRY IS THE ADDRESS OF THE TRANSFER
*           VECTOR FOR THE SERVICE ROUTINE.
*  3,  IF THE LS BIT OF THE ENTRY IS A '1' THEN
*           THE DEVICE IS A 9902, BITS 1-14 FORM THE
*           R12 CONTENTS FOR THE SERVICE ROUTINE AND
*           A CR DELAY WILL BE ADDED IF THE MSB IS A '1'
*
DEVTBL DATA VDPDSR            1-VDP
       DATA (2*EIA02)+>0001   2-EIA 9902, NO CR DELAY
       DATA (2*CASS02)+>0001  3-CASS. 9902, NO CR DELAY
       DATA CENTRO            4-CENTRONICS PRINTER
       DATA 0                 5-UNUSED DEVICE
       DATA 0                 6-UNUSED DEVICE
       DATA 0                 7-UNUSED DEVICE
       DATA 0                 8-UNUSED DEVICE
       DATA 0                 9-UNUSED DEVICE
       DATA 0                 10-UNUSED DEVICE
       DATA 0                 11-UNUSED DEVICE
       DATA 0                 12-UNUSED DEVICE
       DATA 0                 13-UNUSED DEVICE
       DATA 0                 14-UNUSED DEVICE
       DATA 0                 15-UNUSED DEVICE
       DATA 0                 16-UNUSED DEVICE
DTEND  EQU  $
*
*      COLOUR SWAP TABLE
*
SWPTBL DATA >0001
       DATA >0203
       DATA >0405
       DATA >0607
       DATA >0809
       DATA >0A0B
       DATA >0C0D
       DATA >0E0F
*
*               SYSTEM CONFIGURATION FLAGS
*              ============================
*
*      X X X X   X X X X   X X X X   X X X X
*                                    ^ ^ ^ ^
*                                      ! ! !- MAPPER FITTED
*                                      ! !--- PROG.  FITTED
*                                      !----- FLOPPY FITTED
*
CONFIG DATA >0000             DEFAULT TO MINIMUM SYSTEM
       PAGE
* CLOCK INTERRUPT ROUTINE
*
*      R0=FINE COUNTER
*      R1=HOURS
*      R2=MINUTES
*      R3=SECONDS
*      R4-R5=TOTAL TIC COUNTS
*
CLKI   EQU  $
       LI   R12,>1EE0         REF ONCHIP DECREMENTER
       SBO  1                 RE-ENABLE CLOCK INTERRUPT
       DEC  @CNTDWN           COUNTDOWN
       MOV  @BELCNT,R12       GET BELL COUNTER
       SRL  R12,1             COUNT
       MOV  R12,@BELCNT       RESTORE IT
       ASMIF CLKLED
       LI   R12,2*CLKLED      POINT TO CLOCK LED
       ASMELS
       CLR  R12
       ASMEND
*  TEST SHIFT FROM A COUPLE OF LINES AGO
       JNC  NOBELL            NOT IN USE, LEAVE BELL ALONE
       SBO  BELLON-CLKLED     IN USE, TURN BELL ON
       JMP  DOTIC
NOBELL SBZ  BELLON-CLKLED     BELL OFF
DOTIC  SBZ  CLKLED-CLKLED     LED ON
       CI   R0,100/2          TIME FOR LED OFF?
       JL   DOTIC1            N, LEAVE IT ON
       SBO  CLKLED-CLKLED     Y, TURN IT OFF
DOTIC1 INC  R5                COUNT TICS
*
* IF R5 = 0 THEN LEAST SIG WORD OF TIC COUNTER HAS
* 'OVERFLOWED'  - UPDATE R4
*
       JNE  $+4
       INC  R4
       CI   R0,99             100 TICS?
       JHE  CLKI1             Y, 1 SECOND
       INC  R0                N, COUNT
       JMP  CLKI2             EXIT
*
* 100 TICKS HAVE OCCURED - UPDATE REAL TIME CLOCK
*
CLKI1  CLR  R0                CLEAR FINE COUNTER
       INC  R3                COUNT SECONDS
       CI   R3,60             <60?
       JLT  CLKI2             Y, RETURN
       CLR  R3                N
       INC  R2                INCREMENT MINUTE
       CI   R2,60             <60?
       JLT  CLKI2             Y, RETURN
       CLR  R2                N,
       INC  R1                INCREMENT HOUR
       CI   R1,24             <24 HOURS?
       JLT  CLKI2             Y, RETURN
       CLR  R1                N
CLKI2  EQU  $
       RTWP
       NOP
       RTWP
       PAGE
************************************************************
*                                                          *
*           LEVEL 1 INTERRUPT SERVICE ROUTINE              *
*                                                          *
************************************************************
INT1PC MOV  R12,R12           TRAP SET UP?
       JEQ  BUSTIM            N, ERROR
       BLWP *R12              Y, TAKE TRAP
       RTWP
*
BUSTIM TB   BUSINT-0          WAS IT A BUS TIMEOUT ?
       JEQ  ERR42             N, ERROR THEN
       SBZ  BTENBL-0          Y, RESET IT
       SBO  BTENBL-0
       RTWP                   AND EXIT
ERR42  DATA ERROR2,42         INTERRUPT W/O TRAP
************************************************************
*                                                          *
* SET LOAD VECTORS AND CRITICAL SYSTEM POINTERS            *
*                                                          *
************************************************************
*
*    THIS ROUTINE MUST :-
*      1.   RESET THE RECEIVE BUFFER POINTERS
*      2.   SET THE UNIT FLAG TO >0001
*      3.   SETUP AND START THE DECREMENTER
*      4.   SETUP THE NMI VECTORS FOR WARMSTART
*      5.   RESET THE ON-CHIP FLAG REGISTER
*      6.   SET THE VDP UP FOR TEXT MODE
*      7.   ENABLE ALL INTERRUPTS
*
SETVEC EQU  $
       DATA CMOF$             TURN CASSETTE OFF & FLUSH BUF.
       LI   R1,>FFFA          ;POINT TO DECREMENTER
       MOV  @C1,@UNIT         ;SET UNIT 1
       LI   R12,>1EE0         ;POINT TO FLAGS
       MOV  @TIMC,*R1+        ;LOAD DECREMENTER
       MOV  @D$LDWP,*R1+      ;SET UP LOAD WP
       MOV  @D$LDPC,*R1+      ;SET UP LOAD PC
       LDCR R1,0              ;CONFIG DECREMENTER &CLR FLAGS
       SBO  1                 ;ENABLE DECREMENTER
       MOVB @B47,@FBCOL       ;FCOL=BLUE,BCOL=CYAN
       DATA SETTXT            ;SET TEXT MODE
       CLR  @CURFLG           ;CURSOR ON
       RT                     ;EXIT
       PAGE
************************************************************
*                                                          *
*                 BAUD COMMAND                             *
*                                                          *
************************************************************
ERR43  DATA ERROR2,43         INVALID BAUD RATE
ERR37  DATA ERROR2,37         INVALID DELIMITER
ERR46  DATA ERROR2,46         INVALID DEVICE #
*
BAUD   EVFIX R1               GET DEVICE #
       DEC  R1                DEVICE #  1..16
       CI   R1,15             VALID?
       JH   ERR46             N, ERROR IT
       A    R1,R1             Y, MAKE TABLE INDEX
       MOV  @DEVTBL(1),R12    GET DEVICE ENTRY
       COC  @C1,R12           IS IT A 9902?
       JNE  ERR46             N, ERROR IT
       ANDI R12,>7FFE         ISOLATE CRUBASE
       CI   R0,>3F00          ',' FOLLOWING ?
       JNE  ERR37             N, ERROR IT
       BLWP @EVERZ            Y, GET BAUD RATE
       LOADF *R2              EVAL STACK ==> FPAC
       FLOATF 0               FORCE FLOATING POINT IF REQ.
       STORE @DS1             SAVE IT
       LOADF @FP5E5           FPAC = 5E+5
       FDIV @DS1              CALC (5E+5/RATE)
       LI   R2,FPAC           POINT TO FPAC
       BL   @FIX              TRY TO FIX IT
       CLR  R2                SET FOR NO PRE-DIVIDE
       LI   R3,1024
       C    R1,R3             IN RANGE ?
       JL   BAUD1             Y, LEAVE IT
       MOV  R3,R2             N, SET PRE-DIVIDE
       SRL  R1,3              ADJUST REGISTER VALUE
       C    R1,R3             NOW VALID?
       JHE  ERR43             N, ERROR IT
BAUD1  A    R1,R2             ADD IN PRE-DIVIDE
       LI   R11,NLIN          SET RETURN ADDRESS TO NLIN
*
SRATE  SBO  31                RESET 9902
       MOV  *R11,*R11         DELAY
       LDCR @B62,8            2 STOP,EV PAR,7 BITS,/3 CLOCK
       SBZ  13                NO TIMER
       LDCR R2,12             LOAD XMT/REC BAUD RATE
       SBO  18                ENABLE REC. INTERRUPTS
       RT                     EXIT
B43    DATA >4300
       PAGE
************************************************************
*                                                          *
*                   NEW STATEMENT                          *
*                                                          *
************************************************************
ERR49  DATA ERROR2,49         ;ILLEGAL MEMORY ADDRESS
NEWY   MOV  @EORBUS,R1        ;GET DEFAULT MEMORY SIZE
       BL   @CKEX             ;EXPRESSION ?
       JMP  NEWY1             ;N, TAKE DEFAULT
       EVFIX R2               ;Y, GET NEW LIMIT
       AI   R2,HDRSIZ         ;ALLOW ROOM FOR HEADER BLOCK
       C    R2,@IOB           ;IN SYSTEM RAM ?
       JHE  ERR49             ;Y, ILLEGAL MEMORY ADDRESS
       C    R2,@MEMSIZ        ;BELOW MEMSIZ ???
       JL   ERR49             ;Y, ILLEGAL MEMORY ADDRESS
       MOV  R2,R1             ;N, SET R1
NEWY1  MOV  R1,@EORBUS        ;SET NEW EORBUS
       MOV  R1,@BUS           ;SAVE NEW BUS
*
* NEW
*
NEWP   LI   R11,BEGN1         ;LOAD CONTINUATION ADDRESS
*                                ( FOR BL @CLRV )
NEWP1  EQU  $                 ;AUTO-RUN ENTRY POINT
       MOV  @BUS,R1           ;GET BEGINNING USER STORAGE
       MOV  R1,@SLT           ;SET STATEMENT LOCATION TABLE
       CLR  *R1+              ;LEAVE NULL
       MOV  R1,@VNT           ;SET VARIABLE DEFINITION TABLE
       MOV  R1,R3
       AI   R3,NRV*2          ;SKIP RESERVED WORDS
       MOV  R3,@VDT           ;SET NEXT VARIABLE DEFINITION
       AI   R3,NRV*2
       MOV  R3,@NVD           ;SET NEXT VARIABLE POINTER
       B    @CLRV             ;CLEAR *R1 TO EUS
       PAGE
************************************************************
*                                                          *
*               KEYBOARD MODE ROUTINE                      *
*                                                          *
************************************************************
PRDY   LI   R1,MPRDY          ;GET MESSAGE
*
PRDY1  CLR  @AINC             ;CLEAR AUTO-INC FLAG
       CLR  @TRAFLG           ;TRACE OFF
       JMP  CRLF0
*
CRLF   LI   R1,MCRLF          ;OUT CRLF
*
CRLF0  CLR  @MODE             ;SET MODE TO IDLE
       CLR  @ESCFLG           ;ENABLE ESCAPE
       CLR  @F$WHO            ;FLAG IN BASIC
       DATA TYPE$             ;OUT STRING R1
       CLR  @PLF              ;CLEAR PROGRAM LOAD
       BL   @GTLN             ;GET INPUT LINE
CRLF1  BL   @EDIT             ;EDIT STATEMENT
       JMP  CRLF
************************************************************
*                                                          *
*                SYSTEM WARM START                         *
*               ===================                        *
*                                                          *
*      SET CRITICAL SYSTEM POINTERS AND RE-ENTER THE       *
*      INTERPRETER.                                        *
*                                                          *
************************************************************
BEGN   LWPI WPR1              ;GET RIGHT WORKSPACE
       BL   @SETVEC           ;RE-INITIALIZE
BEGN1  LWPI WPR1              ;MAKE SURE WORKSPACE OK
       LI   R1,MBEGN          ;GET MESSAGE
       JMP  PRDY1
       PAGE
************************************************************
*                                                          *
*                HALT OUTPUT CHECK ROUTINE                 *
*               ===========================                *
*                                                          *
*      USE THE SPACE BAR TO HALT AND THEN SINGLE STEP      *
*      THE OUTPUT.                                         *
*                                                          *
************************************************************
HALTO$ DATA WPR2,HALT1
*
HALT1  DATA GETC$             CHARACTER?
       RTWP                   N, EXIT
       NOP                    Y, NORMAL CHARACTER
       CI   R0,>2000          SPACE?
       JNE  EXIT              N, EXIT
WAITK  DATA LOOK$             Y, ANOTHER KEY?
       JMP  WAITK             LOOP
       CI   R0,>2000          Y, SPACE?
       JNE  HALT1             N, READ IT THEN
EXIT   RTWP                   Y, LEAVE IT FOR NEXT TIME
       END
