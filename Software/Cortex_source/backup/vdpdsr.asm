       TITL '9928 DSR - CORTEX BASIC REV. 1.1'
       IDT  'VDPDSR'
       DEF  VDPDSR,D40
       REF  UNPACK,SENDAD,JMPR0,B20
       REF  FBCOL,STXT,B7F,XLOC,YLOC,VMODE
       REF  BITMAP,WP9928,CCSAVE,TMPBUF,BELCNT
       REF  CURFLG,CCNT,GCLEAR
*
*  SCREEN EQUATES
*
TRHC   EQU  39                TOP RIGHT HAND CORNER
TLHC   EQU  0                 TOP LEFT HAND CORNER
BLHC   EQU  920               BOTTOM LEFT HAND CORNER
BRHC   EQU  959               BOTTOM RIGHT HAND CORNER
SCRSIZ EQU  BRHC-TLHC+1       SCREEN SIZE
*
ERROR  EQU  >2F80
ERROR2 EQU  ERROR+>20
*
*        INTERNAL 9995 FLAGS USED
*
*                                   WHEN BIT IS SET
*
F$SHOW EQU  5                 SHOW ALL CONTROL CHARACTERS
F$ROLL EQU  6                 INHIBIT SCROLL
*
       COPY 'IODEFS.INC'
       PAGE
************************************************************
*                                                          *
*                   GRAPHICS MODE                          *
*                                                          *
************************************************************
GMODE  MOVB @YLOC,R8          PICK UP Y LOCATION
       SRL  R8,8              PUT IN LS BYTE
       AI   R8,>0007          ALIGN WITH CHARACTER CELL
       SRL  R8,3              FORM Y CELL#
*
       MOVB @XLOC,R7          PICK UP X LOCATION
       SRL  R7,8              PUT IN LS BYTE
       AI   R7,>0007          ALIGN WITH CHARACTER CELL
       SRL  R7,3              FORM X CELL#
       CI   R7,31             OFF RIGHT OF SCREEN ?
       JLE  GM1               N, LEAVE IT
       CLR  R7                Y, RESET X CELL#
       INC  R8                   NEXT Y CELL
*
GM1    CI   R8,23             OFF BOTTOM OF SCREEN ?
       JLE  GM2               N, LEAVE IT
       CLR  R8                Y, RESET Y CELL#
*
GM2    SLA  R8,5              R8=32*Y CELL#
       A    R7,R8             R8=X CELL# + (32*Y CELL#)
       SLA  R8,3              R8=8*[X CELL# + (32*Y CELL#)]
GM3    AI   R8,PGTBA1+>4000   ADD IN PGT BASE ADDRESS
       PAGE
************************************************************
*                                                          *
*               CHECK FOR END OF TEXT                      *
*                                                          *
************************************************************
*
*       R8 CONTENTS :-
*
* BIT ==>     0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
*
* USE ==>     0  W  0  R  R  R  R  R  C  C  C  C  C  0  0 0
*
*
*    W : VRAM WRITE BIT
*    R : ROW ADDRESS          0..23
*    C : COLUMN ADDRESS       0..31
*
GDONE1 BL   *R4               SEND ADDRESS TO VDP
*
GDONE  C    R9,R10            REACHED THE END OF MSG ?
       JH   GREXIT            Y, EXIT
       MOVB *R9+,R0           N, GET NEXT CHARACTER
       CB   R0,@B20           CONTROL ?
       JL   GCNTRL            Y, HANDLE IT.
*
       BLWP @UNPACK           UNPACK THE BIT PATTERN
       LI   R1,BITMAP         POINT TO 'BITMAP'
       LI   R0,8              DO 8 BYTES
       MOV  R0,R2             SAVE IT
       A    R0,R8             READY FOR NEXT CHARACTER
*
CWRITE MOVB *R1+,@VRAM        COPY OVER BYTE
       DEC  R0                DONE ?
       JNE  CWRITE            N, LOOP
* NOW UPDATE THE COLOUR TABLE
       AI   R8,(CTBA1-PGTBA1)-8  REF COLOUR TABLE
       BL   *R4               SEND ADDRESS TO VDP
UPCTBL MOVB @FBCOL,@VRAM      WRITE THE COLOUR INFORMATION
       DEC  R2                DONE?
       JNE  UPCTBL            N, LOOP
       AI   R8,-((CTBA1-PGTBA1)-8) Y, RESTORE CURSOR ADDRESS
       BL   *R4               RELOAD VDP ADDRESS
*
CHKBOT CI   R8,PGTBA1+(32*24*8)+>4000   OFF END OF TABLE ?
       JL   GDONE             N, CONTINUE
       AI   R8,-(32*24*8)     Y, BACKUP A SCREEN
       JMP  GDONE1            AND RESEND ADDRESS
*
*      CURSOR ON/OFF CONTROL
*
GFS    CLR  @CURFLG           ENABLE CURSOR
       JMP  GDONE             AND CONTINUE
*
GGS    SETO @CURFLG           DISABLE CURSOR
       JMP  GDONE             AND CONTINUE
  
************************************************************
*                                                          *
*           EXIT FROM GRAPHICS MODE                        *
*                                                          *
************************************************************
GREXIT ANDI R8,>1FF8          KILL PIXEL & WRITE BITS
       SWPB R8                POSITION X
       MOVB R8,@XLOC          UPDATE Y
       SLA  R8,11             POSITION Y
       MOVB R8,@YLOC          UPDATE Y
       RTWP
       PAGE
************************************************************
*                                                          *
*               VDP OUTPUT ROUTINES                        *
*                                                          *
************************************************************
VDPDSR DATA WP9928,$+2        9928DSR ENTRY VECTOR
       LI   R12,>1EE0         REF INTERNAL FLAGS
       LI   R4,SENDAD         REF 'SENDAD'
       MOV  @VMODE,R0         WHAT MODE ?
       JNE  GMODE             GRAPH MODE
       JMP  TMODE             TEXT MODE
************************************************************
*                                                          *
*              HANDLE GRAPHIC CONTROL CHARACTERS           *
*                                                          *
************************************************************
GCNTRL BL   @JMPR0            ;DO JUMP ON R0 (USES R1,R2)
GJUMP  BYTE GHT-GJUMP/2,>09   HT - CURSOR RIGHT
       BYTE GBS-GJUMP/2,>08   BS - CURSOR LEFT
       BYTE GLF-GJUMP/2,>0A   LF - CURSOR DOWN
       BYTE GVT-GJUMP/2,>0B   VT - CURSOR UP
       BYTE GFF-GJUMP/2,>0C   FF - CLEAR SCREEN & HOME
       BYTE GCR-GJUMP/2,>0D   CR - CURSOR BEGINING OF LINE
       BYTE GFS-GJUMP/2,>1C   FS - CURSOR ON
       BYTE GGS-GJUMP/2,>1D   GS - CURSOR OFF
       BYTE GRS-GJUMP/2,>1E   RS - CURSOR HOME
       DATA 0
       SETO @BELCNT           ILLEGAL, SET BELL COUNTER
       JMP  GDONE             AND LOOP
************************************************************
*                                                          *
*              CURSOR RIGHT (HT)                           *
*                                                          *
************************************************************
GHT    AI   R8,8              NEXT CHARACTER
GHT1   BL   *R4               POINT VDP TO IT
       JMP  CHKBOT            AND CHECK IT
************************************************************
*                                                          *
*              CURSOR LEFT  (BS)                           *
*                                                          *
************************************************************
GBS    AI   R8,-8             BACKUP TO PRIOR CHARACTER
GBS1   BL   *R4               SEND IT TO VDP
CHKTOP CI   R8,PGTBA1+>4000   OFF TOP ?
       JHE  GDONE             N, CONTINUE
       AI   R8,(32*24*8)      Y,ADD IN A SCREEN
       JMP  GDONE1            & RESEND IT
************************************************************
*                                                          *
*              CURSOR DOWN  (LF)                           *
*                                                          *
************************************************************
GLF    AI   R8,8*32           DOWN A LINE
       JMP  GHT1              GO UPDATE & CHECK
************************************************************
*                                                          *
*              CURSOR UP    (VT)                           *
*                                                          *
************************************************************
GVT    AI   R8,-(8*32)        UP A LINE
       JMP  GBS1              GO UPDATE & CHECK
************************************************************
*                                                          *
*              CLEAR SCREEN (FF)                           *
*               (LEAVES SPRITES !)                         *
************************************************************
FGMODE DATA TMPBUF,GCLEAR     VECTOR INTO MID
*
GFF    BLWP @FGMODE           CLEAR GRAPHICS SCREEN
*   DROP THROUGH TO CURSOR HOME
************************************************************
*                                                          *
*              CURSOR HOME  (RS)                           *
*                                                          *
************************************************************
GRS    LI   R8,PGTBA1+>4000   POINT TO START OF SCREEN
       JMP  GDONE1            & RELOAD IT
************************************************************
*                                                          *
*             CARRIDGE RETURN (CR)                         *
*                                                          *
************************************************************
GCR    ANDI R8,>FF00          THROW THE COLUMN BITS
       CLR  @CCNT             RESET BASIC'S COLUMN COUNTER
       JMP  GDONE1            RESEND IT TO VDP & CONTINUE
       PAGE
************************************************************
*                                                          *
*                   TEXT MODE                              *
*                                                          *
************************************************************
FTMODE DATA TMPBUF,STXT       VECTOR INTO MID
TMODE  EQU  $
       MOVB @YLOC,R7          GET Y POSITION
       SRL  R7,8              POSITION IT
       MPY  @D40,R7           R7=0,R8=40*YLOC
       MOVB @XLOC,R7          GET X POSITION
       SRL  R7,8              POSITION IT
       A    R7,R8             R8=LINEAR CURSOR ADDRESS
       CI   R8,40*24          VALID ?
       JHE  FTMODE            N, GO FORCE TEXT MODE
************************************************************
*                                                          *
*               REMOVE CURSOR                              *
*                                                          *
************************************************************
REMCUR AI   R8,NTBA+>4000     ADD IN TABLE START & W. BIT
       MOV  @CURFLG,R0        CURSOR ENABLED?
       JNE  RVCA              N, LEAVE SCREEN ALONE
       BL   *R4               SEND ADDRESS TO VDP
       MOVB @CCSAVE,@VRAM     WRITE BACK CHARACTER THAT
*                             WAS UNDER THE CURSOR
RVCA   BL   *R4               RESTORE VDP CURSOR ADDRESS
************************************************************
*                                                          *
*               CHECK FOR END OF MSG                       *
*                                                          *
************************************************************
DONYET C    R9,R10            REACHED THE END OF MSG ?
       JH   PUTCUR            Y, EXIT
       MOVB *R9+,R0           N, GET NEXT CHARACTER
       TB   F$SHOW            SHOW ALL CONTROL CHARACTERS ?
       JEQ  CSHOW             Y, TREAT AS NORMAL
       CB   R0,@B20           N, CONTROL ?
       JL   CNTRL             Y, HANDLE IT.
*
CSHOW  MOVB R0,@VRAM          N, WRITE CHARACTER
       INC  R8                UPDATE CURSOR
*                             CHECK FOR OFF BOTTOM OF SCREEN
*
OFFBOT CI   R8,NTBA+BRHC+>4000  OFF BOTTOM ?
       JLE  DONYET            N, CONTINUE
       TB   F$ROLL            Y, SCROLL DISABLED ?
       JNE  DOROLL            N, GO DO IT
       AI   R8,-SCRSIZ        Y, BACKUP R8
       JMP  RVCA              AND CONTINUE
       PAGE
*
*   SCROLL ROUTINE
*
DOROLL MOV  R8,R7             SAVE CURSOR
       AI   R7,-40            RESTORE CURSOR POSITION
*
       LI   R8,NTBA+40        REF. START OF LINE 1
       BL   *R4               SEND IT TO VDP
       CLR  R1                GET START OF BUFFER
*
RDSCR  MOVB @VRAM,@TMPBUF(R1) READ CHARACTER FROM VDP
       INC  R1                COUNT IT
       CI   R1,920            DONE ?
       JL   RDSCR             N, CONTINUE READING SCREEN
*
       LI   R8,NTBA+>4000     SET FOR WRITE TO TOP OF SCREEN
       BL   *R4               SEND IT TO VDP
       CLR  R1                GET START OF BUFFER
*
WRSCR  MOVB @TMPBUF(R1),@VRAM  WRITE CHARACTER TO VDP
       INC  R1                COUNT IT
       CI   R1,920            DONE ?
       JL   WRSCR             N, LOOP
*
       LI   R1,40             NOW FILL BOTTOM LINE WITH ' '
WRSP   MOVB @B20,@VRAM        WRITE SPACE
       DEC  R1                COUNT IT
       JNE  WRSP              N, LOOP
*
*   NOW RESTORE THE CURSOR
*
       MOV  R7,R8
       BL   *R4               GIVE IT TO VDP
       JMP  DONYET            & CONTINUE OUTPUT
OFFTOP CI   R8,NTBA+>4000     OFF TOP OF SCREEN
       JHE  DONYET            N, CONTINUE
*
       AI   R8,SCRSIZ         Y, ADJUST CURSOR
       BL   *R4               SEND IT TO VDP
FD$JMP JMP  DONYET            & CONTINUE
       PAGE
*
*      PUT CURSOR AND EXIT ROUTINE
*
PUTCUR AI   R8,->4000         PUT VDP IN READ MODE
       BL   *R4               SEND TO VDP
       CLR  R3
       MOVB @VRAM,R3          GET CHARACTER UNDER CURSOR
       AI   R8,>4000          PUT BACK IN WRITE MODE
       BL   *R4               SEND IT
       MOV  @CURFLG,R1        CURSOR ENABLED
       JNE  PUT$1             N, LEAVE SCREEN
       MOVB @B7F,@VRAM        WRITE CURSOR CHARACTER
*  NOW WORK OUT THE X,Y CURSOR
PUT$1  AI   R8,-(NTBA+>4000)  REMOVE TABLE BASE & WR. BIT
       CLR  R7                READY FOR DIVIDE
       DIV  @D40,R7           DO DIVIDE
* R7 = Y , R8 = X
       STWP R1                GET WP
       MOVB @1+(2*R7)(R1),@YLOC      SET YLOC
       MOVB @1+(2*R8)(R1),@XLOC      SET XLOC
       MOVB R3,@CCSAVE        SAVE CHARACTER
       SRL  R3,5              R3= 8 * CHAR
       MOV  R3,R8             SET FOR ADDRESS
* READ THE PATTERN FOR SAVED CHARACTER
       AI   R8,PGBA           R8 = ADDRESS OF CHAR PAT.
       BL   *R4               SEND ADDRESS TO VDP
       LI   R8,BITMAP         POINT TO 'BITMAP'
       MOV  R8,R1             SAVE IT
       LI   R7,8              DO 8 BYTES
       MOV  R7,R2             SAVE IT
RDPAT  MOVB @VRAM,*R8+        READ BYTE FROM VDP
       DEC  R7                COUNT IT
       JNE  RDPAT             LOOP TILL DONE
*  WRITE INVERSE AS CHARACTER >7F BIT PATTERN
       LI   R8,PGBA+>4000+(8*>7F)  REF >7F PAT. ENTRY
       BL   *R4               SEND IT TO VDP
*
WRPAT  MOVB *R1+,R0           GET PATTERN
       INV  R0                INVERT IT
       MOVB R0,@VRAM          SEND IT TO VDP
       DEC  R2                DONE ?
       JNE  WRPAT             N, LOOP
*
DEXIT  RTWP                   Y, EXIT
*
*      CURSOR ON/OFF CONTROL
*
FS     CLR  @CURFLG           ENABLE CURSOR
       JMP  FD$JMP            AND CONTINUE
*
GS     SETO @CURFLG           DISABLE CURSOR
       JMP  FD$JMP            AND CONTINUE
       PAGE
*
*       HANDLE TEXT CONTROL CHARACTERS
*
CNTRL  BL   @JMPR0            ;DO JUMP ON R0 (USES R1,R2)
CJUMP  BYTE HT-CJUMP/2,>09     HT - CURSOR RIGHT
       BYTE BS-CJUMP/2,>08     BS - CURSOR LEFT
       BYTE LF-CJUMP/2,>0A     LF - CURSOR DOWN
       BYTE VT-CJUMP/2,>0B     VT - CURSOR UP
       BYTE FF-CJUMP/2,>0C     FF - CLEAR SCREEN & HOME
       BYTE CR-CJUMP/2,>0D     CR - CURSOR BEGINING OF LINE
       BYTE FS-CJUMP/2,>1C     FS - CURSOR ON
       BYTE GS-CJUMP/2,>1D     GS - CURSOR OFF
       BYTE RS-CJUMP/2,>1E     RS - CURSOR HOME
       DATA 0
       SETO @BELCNT           ILLEGAL, SET BELL COUNTER
FDONE  JMP  FD$JMP            & TEST FOR COMPLETION
*
*     CURSOR RIGHT (HT)
*
HT     INC  R8                ADJUST CURSOR
UDATED BL   *R4               SEND IT TO VDP
       B    @OFFBOT           CHECK CURSOR ON SCREEN
*
*     CURSOR LEFT   (BS)
*
BS     DEC  R8                BACKUP CURSOR
UDATEU BL   *R4               SEND IT TO VDP
       JMP  OFFTOP            CHECK CURSOR ON SCREEN
*
*     CURSOR DOWN  (LF)
*
LF     AI   R8,40             DOWN A LINE
D40    EQU  $-2
       JMP  UDATED            UPDATE & CHECK
*
*     CURSOR UP    (VT)
*
VT     AI   R8,-40            UP A LINE
       JMP  UDATEU            UPDATE & CHECK
*
*     CLEAR SCREEN (FF)
*
FF     LI   R8,NTBA+>4000     REF TOP OF SCREEN
       BL   *R4               SEND IT TO VDP
       LI   R1,960            DO 960 CHARACTERS
CLRSCN MOVB @B20,@VRAM        WRITE ' ' TO VDP
       DEC  R1                COUNT IT
       JNE  CLRSCN            LOOP TILL DONE
*  FALL THROUGH TO CURSOR HOME
*
*     CURSOR HOME   (RS)
*
RS     LI   R8,NTBA+>4000     REF TOP OF SCREEN
RS1    BL   *R4               SEND IT TO VDP
       JMP  FDONE
*
*     CARRIAGE RETURN  (CR)
*
CR     AI   R8,-(NTBA+>4000)  STRIP OFF TABLE BASE & WR. BIT
       CLR  R7                READY FOR DIV.
       DIV  @D40,R7           SPLIT INTO X & Y
       MPY  @D40,R7           CALC 40 * Y
       AI   R8,NTBA+>4000     ADD BACK BASE & WRITE BIT
       CLR  @CCNT             RESET BASIC'S COLUMN COUNTER
       JMP  RS1               UPDATE  CURSOR LOCATION
       END

