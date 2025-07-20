       TITL 'BASIC LINE EDITOR - CORTEX BASIC REV 1.1'
       IDT  'EDIT'
*
*  COMMANDS :-
*
*   RUN
*   SIZ(E)
*   CON(TINUE)
*   MON(ITOR)
*   <SPARE>
*   <SPARE>
*   <SPARE>
*   <SPARE>
*   <SPARE>
*   <SPARE>
*   <SPARE>
*
*
*  FUNCTIONS  DELIMITERS  OPERATORS  VARIABLES
* *01-1A FNx *38 TO      *4E OR     *62 -1
*            *39 TAB     *4F LOR    *63 0
*  1B ABS    *3A STEP    *50 AND    *64 1
*  1C ADR    *3B THEN    *51 LAND   *65 2
*  1D ASC    *3C :       *52 NOT    *66 3
*  1E ATN    *3D @       *53 LNOT   *67 4
*  1F COS    *3E #       *54 LXOR   *68 5
*  20 EXP    *3F ,       *55 ==     *69 6
*  21 FRA    *40 ;       *56 =      *6A 7
*  22 INT    *41 ?       *57 >      *6B 8
*  23 LOG    *42 %       *58 >=     *6C 9
*  24 KEY    *43 $       *59 <      *6D 2B INT.
*  25 SIN    *44 "       *5A <=     *6E 2B HEX
*  26 SQR    *45 '       *5B <>     *6F 6B FP.
*  27 SYS    *46 \ (NU)  *5C -      *70 DUMMY
*  28 TIC    *47 !       *5D +      *71 DUMMY
*  29 SGN    *48 &       *5E /      *72 DUMMY
*  2A BIT    *49 _ (NU)  *5F *      *73 RND
*  2B CRB    *4A [       *60 ^       74-FF USER
*  2C CRF    *4B ]       *61 UN.-
*  2D MEM    *4C (
*  2E MWD    *4D )
*  2F LEN
*  30 MCH
*  31 POS
*  32 COL
*  33 MOD
*  34
*  35
*  36
*  37
*
*
*   ( ) = SPECIAL      * = TABLE POSITION FIXED
       PAGE
*                       STATEMENTS
*                     ==============      
*
*
*  00                   20 TEXT           40 ON
*  01 GOTO*             21 WAIT           41 IF
*  02 GOSUB*            22 CHAR           42 DEF
*  03 ELSE*             23 NUMBER         43 NEW
*  04 REM*              24 LIST           44 END
*  05 FOR*              25 RENUM          45 (?) (PRINT)
*  06 (LET)*            26 SPRITE         46 (*) (EXTEND)
*  07 DATA              27 SHAPE          47 BIT
*  08 NEXT              28 SPUT           48 CRB
*  09 ERROR             29 SGET           49 CRF
*  0A PRINT             2A BOOT           4A MEM
*  0B CALL              2B SWAP           4B MWD
*  0C LOAD              2C                4C
*  0D INPUT             2D MOTOR          4D
*  0E READ              2E                4E
*  0F RESTOR            2F                4F
*  10 RETURN            30    
*  11 STOP              31    
*  12 UNIT              32    
*  13 TIME              33    
*  14 SAVE              34            
*  15 BASE              35   
*  16 ESCAPE            36   
*  17 NOESC             37    
*  18 RANDOM            38    
*  19 BAUD              39 MAG
*  1A ENTER             3A TOF        
*  1B PLOT              3B TON
*  1C UNPLOT            3C POP
*  1D COLOUR            3D DIM
*  1E PURGE             3E LET
*  1F GRAPH             3F (;) (PRINT)
*
*   ( ) = SPECIAL      * = TABLE POSITION FIXED
       PAGE
*
* LAST UPDATED 15:22 21/12/81
*
       DEF  EDIT,LST,LSTL,MODEOK,EMV
       DEF  LCN1,LCN2,LCN3,LCN4
       DEF  LNXT,LDEF,EDIT1
       DEF  RNGOTO,RNGSUB,RNRSTR
       DEF  RNON,RNINP,RNERR,RNREM,RNELSE,RNIF
*
       REF  EDTMP
       REF  AINC,ESCFLG
       REF  DEBUG$,FTM$
       REF  B2A,B20,B01
       REF  BUS,DDM,DLC,EBP
       REF  CKEX,CVDB20,C1
       REF  CVDIFZ,CVDIZ,EDER
       REF  FNS,GSC,GSS,LNUM
       REF  FOR2,JMPR0,R8STOR
       REF  FPAC,SLT,SLN,IOB
       REF  LINE2
       REF  MODE,NVD,NVS,EFLG
       REF  RUNP,SIZE
       REF  SSP,UFT,VDT,VNT
       REF  TYPC$,TYPBE$
       REF  HALTO$,DCNT,GOS2
       REF  CRLF,NINE,RENCMP
       REF  HOUT
       DXOP EVFIX,11
       DXOP OUTFP,12
       DXOP OUTINT,13
*
ERROR  EQU  >2F80
ERROR2 EQU  ERROR+>20
SYN    EQU  >1600             TAPE SYNC CHARACTER
STX    EQU  >0200             TAPE START OF TEXT CHARACTER
ETX    EQU  >0300             TAPE END OF TEXT CHARACTER
       COPY 'IODEFS.INC'
       PAGE
*
* CONTINUE COMMAND
*
CONT   SETO @MODE             SET TO RUN
       DATA TYPC$             OUT CRLF
       MOV  @SLN,R1           GET LINE #
       B    @GOS2
*
*   CHECK TO SEE THAT WE ARE IN KEYBOARD MODE
*
MODEOK MOV  @MODE,@MODE       ;ARE WE RUNNING
       JNE  ERR48             ;Y, NOT ALLOWED
       RT
*
ERR48  DATA ERROR2,48         ;WRONG MODE
       PAGE
* EDIT INPUT STRING
*
*       R6  = SUBSCRIPT STACK PTR
*       R7  = INPUT BYTE PTR
*       R8  = OUTPUT BYTE PTR
*       R10 = RETURN ADR
*       R12 = BEGINNING OF COMMAND PTR
*       R14 = SAVE FOR LINE COUNTER
*
EDIT   MOV  @NINE,@RENCMP     ;COMPRESSED FLAG=9 (FULL)
EDIT1  MOV  R11,R10           ;SAVE RETURN
       MOV  R6,@EDTMP         ;SAVE LINE COUNT FOR EDER ROUTINE
       CLR  @EFLG             ;KILL ERROR FLAG
       MOV  @IOB,R7           ;GET I/O PTR
       LI   R6,SSP            ;GET SUBSCRIPT STACK PTR
       SETO *R6               ;MARK SUBSCRIPT STACK
       LI   R8,EBP            ;GET EDIT BUFFER PTR
       MOV  @VNT,R5           ;SET UP RESERVED WORDS
       CLR  *R5+              ;CLEAR DUMMY VARIABLES
       CLR  *R5+
       CLR  *R5+
       LI   R0,>11D2          ;'RND'
       MOV  R0,*R5
*
*       GET LINE NUMBER
*
       BLWP @CVDIZ            ;CONVERT LINE NUMBER
       JMP  EDER3             ;OVERFLOW
       CLR  R1                ;NO NUMBER
       MOV  R1,@LNUM          ;STORE LINE NUMBER
       JLT  EDER3
       JEQ  $+4               ;LINE #
       DEC  R7                ;Y, BACKUP OVER DELIMITER
       MOV  R0,R0             ;DONE?
       JNE  ED3               ;N
*
* IF AN EMPTY LINE IS ENTERED IN AUTO-INC MODE THEN
* THE AUTO-INCREMENT MODE IS TERMINATED
*
       MOV  @AINC,R11         ;AUTO-INC MODE ?
       JNE  NOAINC            ;Y, KILL IT
       B    @EMV0             ;Y, DELETE LINE
NOAINC CLR  @AINC             ;KILL AUTO-INC FLAG
       B    *R10              ;EXIT
*
*DECODE COMMAND WORD
*
ED3    BL   @EDGLS            ;GET FIRST LETTER
       JMP  EDSCI             ;LET OR ";
       MOV  R7,R3             ;MARK
       MOV  R1,R0
       BL   @EDGL             ;GET SECOND LETTER
       JMP  EDLET
       A    R1,R0
*
*CHECK FOR TWO LETTER COMMANDS HERE
*
       CI   R0,>1920          ;IF?
       JEQ  EDIF              ;Y
       CI   R0,>39E0          ;ON?
       JEQ  EDON              ;Y
       BL   @EDGL             ;N, GET 3RD LETTER
       JMP  EDLET             ;LET
       A    R1,R0
       PAGE
*SEARCH COMMAND LIST
*
       LI   R4,EDSCL          ;Y, GET COMMAND LIST PTR
*
ED5    MOV  *R4+,R1           ;GET FIRST COMMAND
       SRL  R1,1              ;MOVE 2ND WORD INDICATOR INTO CARRY
       C    R0,R1             ;FOUND?
       JEQ  ED6               ;MAYBE
       CI   R4,EDCLE          ;N, MORE COMMANDS?
       JL   ED5               ;Y
*
EDLET  MOV  R3,R7             ;RESTORE R7
*
EDLETA MOVB @LETB,*R8+        ;INSERT >6 (IMPLIED LET)
       JMP  ED13
*
ED6    MOV  R0,R2             ;SAVE IN CASE NOT FOUND
       MOV  R7,R15
       MOV  @EDCS(4),R0       ;GET SECOND WORD
       JNC  ED9               ;FOUND
*
ED7    MOV  R0,R5
       ANDI R5,>3E            ;MASK CHARACTER
       JEQ  ED9               ;FOUND
       BL   @EDGL             ;GET LETTER
       JMP  ED8               ;PROBLEM
       SRL  R1,9              ;POSITION
       C    R5,R1             ;LETTER SAME?
       JEQ  ED7               ;Y
*
ED8    MOV  R2,R0             ;PROBLEM, CONTINUE TO LOOK
       MOV  R15,R7
       JMP  ED5
*
EDER3  BL   @EDER             ;INVALID LINE NUMBER
       TEXT '03'
*
EDIF   LI   R4,IFB            ;IF ENTRY
       JMP  ED10
*
EDON   LI   R4,ONB            ;ON ENTRY
       JMP  ED10
*
EDSCI  CB   *R7,@B2A          ;'*' ?
       JNE  EDSCI0            N, TRY OTHER SPECIALS
       INC  R7                BUMP PTR
       LI   R4,PASBX*128      LOAD EXTENDED COMMAND BYTE
       MOVB R4,*R8+           STORE IT
ECH$0  MOVB *R7+,R0           GET CHARACTER
       JEQ  ECH$1             NULL, EXIT LOOP
       CB   R0,@B20           ' ' ?
       JEQ  ECH$1             Y, END OF COMMAND
       MOVB R0,*R8+           N, COPY OVER BYTE
       JMP  ECH$0             AND LOOP
ECH$1  SB   *R8,*R8+          END, PUT A NULL
       JMP  ED14              AND CONTINUE
*
EDSCI0 CB   *R7+,@B3F         ;'?' ?
       JNE  EDSCI1            N, TRY ';'
       LI   R4,PQMBX          Y, LOAD '?' ENTRY
       JMP  ED10
EDSCI1 DEC  R7                ;BACKUP POINTER
       CB   *R7+,@B3B         ;";?
       JNE  EDLETA            ;N
       LI   R4,PSCBX          ;"; ENTRY
       JMP  ED10
       PAGE
*COMMAND TYPE DONE
*CHECK FOR GOTO OR GOSUB
*
ED9    LI   R1,EDCL           AI   R4,-EDCL  (NOT ALLOWED)
       S    R1,R4             SYSTEM COMMAND?
       JGT  ED10              N, CONTINUE TRANSLATION
       MOV  R0,R0             Y, COMMAND INSTALLED ?
       JEQ  SYSERR            N, SYSTEM ERROR
       B    *R0               Y, EXECUTE COMMAND
*
SYSERR DATA ERROR,-1          SYSTEM ERROR
*
ED10   SOC  @C1,@ESCFLG       ;NO ESCAPE IN EDIT !!!
       SLA  R4,7              ;POSITION BYTE
       MOVB R4,*R8+           ;STORE IN STREAM
       CI   R4,>300           ;CHECK SPECIAL TYPES
       JLT  ED12              ;GOTO,GOSUB
       JEQ  ED3               ;ELSE
       CI   R4,>400           ;REM?
       JNE  ED14              ;N
       B    @EDREM            ;Y
*
ED11   MOVB @B3F,*R8+         ;STORE ,
*
ED12   BLWP @CVDIZ            ;GET INTEGER
       NOP
       JMP  EDER3
       MOVB R1,*R8+           ;STORE INTEGER
       SWPB R1
       MOVB R1,*R8+
       CI   R0,>2C00          ;",?
       JEQ  ED11              ;Y
*
ED13   DEC  R7                ;FALL THRU TO ED20
*
ED14   MOV  R8,R12            ;MARK
       PAGE
*PROCESS REST OF LIST
*
ED20   BLWP @CVDIFZ           ;LOOK FOR NUMBER
       JMP  ED23              ;OVERFLOW, FP
       JMP  ED30              ;NO NUMBER
       JMP  ED20H             ;HEX
       CI   R1,-1             ;<-1?
       JLT  ED21              ;Y
       C    R1,@RENCMP        FULL COMPRESSION?
       JGT  ED21              ;N
       AI   R1,>63            ;GET CODE (62-6C)
       JMP  ED22              ;INSERT
*
ED20H  MOVB @B6E,*R8+         ;MARK AS HEX
       JMP  ED21A
*
ED21   MOVB @B6D,*R8+         ;2 BYTE INTEGER, INSERT >6D
ED21A  MOVB R1,*R8+
ED22   SWPB R1
       MOVB R1,*R8+
       JMP  ED24              ;LOOK FOR OPERATOR
*
ED23   MOVB @B6F,*R8+         ;FP #, INSERT >6F
       LI   R3,6
       LI   R4,FPAC           ;GET FPAC ADR
*
       MOVB *R4+,*R8+         ;MOVE IN NUMBER
       DEC  R3                ;DONE?
       JNE  $-4               ;N
*
ED24   DEC  R7                ;Y, BACKUP OVER DELIMITER
*
*VARIABLE OR KEY WORD
*GET 3 CHARACTERS - EXIT TO EDOP IF NON-LETTER
*
ED30   BL   @EDGWO            ;CHECK FOR WORD OPERATER
       BL   @EDGLS            ;PROCESS COMMAND LIST
       JMP  EDOP              ;NOT LETTER, OPERATOR
       MOV  R1,R0
       BL   @EDGL             ;GET NEXT LETTER
       JMP  EDV1              ;NOT LETTER, 1 CHARACTER VARIABLE
       A    R1,R0             ;ADD NEW LETTER
       CI   R0,>38C0          ;'FN'?
       JNE  ED31              ;N
       B    @EDFN             ;Y, FUNCTION
*
ED31   BL   @EDGL             ;N, GET 3RD LETTER
       JMP  EDV2              ;NOT LETTER, 2 LETTER VARIABLE
       A    R1,R0
       PAGE
*LOOK FOR 3 LETTER KEY WORDS
*
       LI   R4,EDIL           ;GET LIST PTR
*
ED32   MOV  *R4+,R1           ;GET CHARACTERS
       SRL  R1,1              ;REMOVE TERMINATOR
       C    R0,R1             ;SAME?
       JNE  ED34              ;N
       LI   R11,EDIL
       AI   R4,>1A*2
       S    R11,R4
       SRL  R4,1              ;SYSTEM FUNCTION
*
ED33   SWPB R4                ;POSITION
       MOVB R4,*R8+           ;INSERT
       JMP  ED20
*
ED34   CI   R4,EDILE          ;N, LIST DONE?
       JL   ED32              ;N
       JMP  ED35
*
*1 CHARACTER VARIABLE
*
EDV1   MOV  R0,R3             ;LOOK FOR #
       BL   @CVDB20           ;CHECK FOR DIGIT
       JMP  EDV1A             ;N
       DEC  R7                ;Y, BACKUP
       BLWP @CVDIZ            ;CONVERT
       JMP  EDER4             ;FP #, ILLEGAL VARIABLE NAME
       JMP  EDV1B             ;NO NUMBER
       CZC  @CFF80,R1         ;TOO LARGE?
       JNE  EDER4             ;Y, ILLEGAL NAME
       A    R1,R3             ;COMBINE
       ORI  R3,>0380          ;INDICATE AS NUMBER
C380   EQU  $-2
*
EDV1A  DEC  R7                ;BACKUP OVER DELIMITER
*
EDV1B  MOV  R3,R0             ;RESTORE R0
       PAGE
*2 OR 3 CHARACTER VARIABLE
*
EDV2   EQU  $
ED35   BL   @EDGP             ;LOOK FOR DIMENSION
       JMP  ED35A             ;Y, DIMENSIONED
       DEC  R7                ;BACKUP
       JMP  ED36
*
ED35A  NEG  R0                ;SET DIMENSION
       INCT R6                ;STACK -1
       SETO *R6
*
*SAVE VARIABLE NAME
*
ED36   LI   R4,>FF00+>70      ;-# OF VARIABLES
       MOV  @VNT,R5           ;GET VARIABLE TABLE ADR
*
ED37   C    R5,@VDT           ;DONE?
       JHE  ED38              ;Y, MAKE NEW VARIABLE
       C    R0,*R5+           ;VARIABLE SAME?
       JEQ  ED39              ;Y, FOUND
       INC  R4                ;STILL ROOM?
       JNE  ED37              ;Y
*
ERR5   DATA ERROR+5           ;N, TOO MANY VARIABLES
*
EDER4  BL   @EDER             ;ILLEGAL VARIABLE NAME
       TEXT '04'
*
ERR10  DATA ERROR+10          ;STORAGE OVERFLOW
       PAGE
*DEFINE NEW VARIABLE
*
ED38   MOV  @NVD,R2           ;GET END OF TABLE
       C    *R2+,*R2+         ;ADD 4
       C    R2,@NVS           ;ROOM FOR NEW VARIABLE?
       JHE  ERR10             ;N
       MOV  R2,@NVD           ;Y, UPDATE NVD
*
ED38A  AI   R2,-4             ;MOVE POINTERS UP 2 BYTES
       MOV  *R2+,*R2
       C    R2,R5             ;DONE?
       JH   ED38A             ;N
       MOV  R0,*R5            ;Y, STORE NEW NAME
       INCT @VDT              ;UPDATE VDT
*
ED39   MOV  R0,R0             ;DIMENSIONED?
       JLT  ED33              ;Y
       SWPB R4                ;N, INSERT VARIABLE CODE
       MOVB R4,*R8+
*
*       TRANSLATE CHARACTER INTO CODE
*
EDOP   CLR  R0                ;GET OPERATER
       MOVB *R7+,R0           ;GET BYTE
       JEQ  ED90              ;DONE
       MOV  R0,R1
       SWPB R1
       CI   R0,>4100          ;<"A?
       JL   EDOP1             ;Y
       CI   R0,>5B00          ;>"Z?
       JL   ED49              ;N, LETTER, TRY WORD OPERATER
       AI   R1,-26            ;SKIP ALPHABET
       CI   R0,>5E00          ;>"^?
       JH   EDER6             ;Y, INVALID CHARACTER
*
EDOP1  MOVB @EDSL->21(1),R1   ;GET CODE
       JEQ  EDER6             ;INVALID CHARACTER
       MOVB R1,*R8            ;STORE BYTE
       BL   @JMPR0            ;SWITCHBOARD ON SPECIAL CHARACTERS
EDOPTB BYTE EDOP-EDOPTB/2,>20 SP
       BYTE ED50-EDOPTB/2,>28 "(
       BYTE ED50-EDOPTB/2,>5B "[
       BYTE ED51-EDOPTB/2,>29 ")
       BYTE ED51-EDOPTB/2,>5D "]
       BYTE ED42-EDOPTB/2,>22 ""
       BYTE ED42-EDOPTB/2,>27 "'
       BYTE ED47-EDOPTB/2,>3A ":
       BYTE ED54-EDOPTB/2,>21 "!
       BYTE ED43-EDOPTB/2,>3D "=
       BYTE ED45-EDOPTB/2,>3E ">
       DATA 0
ED40   INC  R8                ;MOVE OVER CODE
ED41   B    @ED20             ;CONTINUE PARSE
*
ED42   INC  R8                ;PROCESS " OR '
       MOVB *R7+,*R8          ;GET NEXT BYTE
       JEQ  EDER2             ;PROBLEM
       CB   R0,*R8            ;CLOSE?
       JNE  ED42              ;N
       SB   *R8,*R8+          ;Y, TERMINATE WITH NULL
       JMP  ED41
*
ED43   DEC  R8                ;PROCESS "=
       CB   *R8,@B57          ;>=?
       JEQ  ED44              ;Y
       CB   *R8,@B59          ;<=?
       JEQ  ED44              ;Y
       CB   *R8+,@B56         ;==?
       JNE  ED40
       SB   @B01,@-1(8)       ;Y, MAKE ==
       JMP  ED41
*
ED44   AB   @B01,*R8+         ;MODIFY CODE
       JMP  ED41              ;GOTO ED20
*
ED45   CB   @-1(8),@B59       ;PROCESS ">
       JNE  ED40
       AB   @B02,@-1(8)       ;MAKE <>
ED41P  JMP  ED41
*
ED46   INC  R7                ;PROCESS :,THEN
ED47   CB   *R7,@B20          ;SPACE
       JEQ  ED46              ;Y, IGNOR
       CB   *R7,@B3A          ;N, IS IT ANOTHER COLON ?
       JEQ  ED46              ;Y, IGNORE IT THEN
       INC  R8                ;N, PROCESS NEW COMMAND
       B    @ED3
*
EDER6  BL   @EDER             ;ILLEGAL CHARACTER
       TEXT '06'
*
*LOOK FOR WORD OPERATER
*
ED49   DEC  R7
       BL   @EDGWO
       BL   @EDER             ;EXPECTING OPERATER
       TEXT '07'
       PAGE
*( OR [ ENTRY
*
ED50   C    R8,R12            ;COMMAND BYTE?
       JH   ED50A             ;N
       CB   @-1(8),@BCPB      ;MEM,CRB,CRF,BIT?
       JL   ED50B             ;N, LEAVE (
* FOLLOWING LINE NEEDED AS CMD BYTES BIT - MWD CODE >B38
       JMP  ED50A1            ;Y, FORCE '['
ED50A  MOVB @-1(R8),R1        IS LAST CHAR STORED
*                             STRING TERMINATOR (0)?
       JEQ  ED50B             Y
       CB   R1,@B38           N - FUNCTION ARGUMENTS?
       JHE  ED50B             ;N, LEAVE (
ED50A1 INCT R6                ;Y, MAKE [
       CLR  *R6               ;INSERT -1 INTO STACK
       SB   @B02,*R8          ;*R8=>4A
*
ED50B  DEC  *R6               ;INCREMENT TOP ITEM ON STACK
       JMP  ED40              ;LEAVE
*
*) OR ] ENTRY
*
ED51   INC  *R6               ;NEED ]?
       JLT  ED52              ;N, LEAVE >4D
       DECT R6                ;Y, DECREMENT STACK
       CI   R6,SSP            ;UNMATCHED?
       JL   EDER2             ;Y
       SB   @B02,*R8          ;N, INSERT ] CODE (>4B)
*
ED52   INC  R8                ;LEAVE CODE
ED52A  JMP  EDOP              ;LOOK FOR OPERATER
       PAGE
*PROCESS REMARKS
*
ED54   INC  R8                ;PROCESS TAIL REMARK
*
EDREM  MOVB *R7+,*R8+         ;MOVE REMARK
       JNE  EDREM
*
*END PARSE, CHECK FINAL ERRORS
*
ED90   CI   R6,SSP            ;SUBSCRIPT ERROR?
       JNE  EDER2             ;Y
       INC  *R6               ;PAREN ERROR?
       JNE  EDER2             ;Y
       SB   *R8,*R8+          ;N, MARK OUTPUT
       SB   *R8,*R8           ;DOUBLE NULL
       SETO R0                ;SET TO INSERT
       MOV  @LNUM,R1          ;GET LINE NUMBER
       JNE  EMV0              ;INSERT OR CHANGE
       MOV  @IOB,R7           ;NO LINE NUMBER
       AI   R7,30             ;MOVE INTO IOB
       MOV  R7,R0             ;MARK
       LI   R3,EBP            ;**NOTE** ALL PTRS ON WRD BOUNDARIES
*
       MOV  *R3+,*R7+         ;MOVE
       C    R3,R8             ;DONE?
       JL   $-4               ;N
*
       CLR  @ESCFLG           ;Y, ENABLE ESCAPE
       MOV  R0,R8             ;SET R8
       CB   *R8,@B01          ;CHECK TYPE, GOTO?
       JH   ED91              ;N
       SETO @MODE             ;Y, SET TO RUN MODE
       DATA TYPC$             ;OUT CRLF
*
ED91   B    @LINE2
*
EDER2  BL   @EDER             ;UNMATCHED PARENTHESIS
       TEXT '02'
       PAGE
*PROCESS FN-
*
EDFN   BL   @EDGLS            ;GET LETTER
       JMP  EDER8             ;NO LETTER
       SRL  R1,2              ;POSITION
       MOVB R1,*R8+           ;INSERT INLINE
       MOV  R8,R3             ;CHECK PREVIOUS BYTE
       DECT R3                ;LOOK FOR OPERATOR
       CI   R3,EBP            ;BEGINNING OF BUFFER?
       JNE  ED52A             ;N, DISREGUARD
       CB   *R3,@DEFXB        ;'DEF'?
       JNE  ED52A             ;N, CONTINUE
       BL   @EDGP             ;Y, LOOK FOR ( OR [
       JMP  EDFN0             ;"( OR "[, OK
       DEC  R7                ;NEITHER, LOOK FOR "=
       JMP  EDFN3
*
EDFN0  LI   R3,3              ;Y, ALLOW 3 DUMMY VARIABLES
       MOV  @VNT,R4           ;GET STORAGE ADR
*
EDFN1  BL   @EDGLS            ;GET DUMMY VARIABLE
       JMP  EDER9             ;PROBLEM
       MOVB R1,*R8+           ;INSERT IN CODE
       MOV  R1,*R4+           ;STORE CODE
       BL   @EDGP             ;GET NEXT BYTE
       JMP  EDER9
       DEC  R3                ;ROOM FOR MORE?
       JEQ  EDFN2             ;N, SEE IF ")
       CI   R1,>2C00          ;Y, DELIMITER ",?
       JEQ  EDFN1             ;Y, LOOP
*
EDFN2  CI   R1,>2900          ;N, CLOSING PAREN?
       JEQ  EDFN3             ;Y, LOOK FOR "=
       CI   R1,>5D00          ;N, "]?
       JNE  EDER2             ;N, PROBLEM
EDFN3  CB   @B3D,*R7          ;Y, "=?
       JEQ  ED52A             ;Y
       CB   @B20,*R7+         ;SPACE?
       JEQ  EDFN3             ;Y, TRY AGAIN
       BL   @EDER             ;MISSING ASSIGNMENT
       TEXT '36'
*
EDER8  BL   @EDER             ;ILLEGAL FUNCTION NAME
       TEXT '08'
*
EDER9  BL   @EDER             ;ILLEGAL FUNCTION ARGUMENT
       TEXT '09'
       PAGE
*FINISH EDIT PROCESS
*       BL @EMV
*
*       IN  R0 = 0 FOR DELETE, <>0 FOR CHANGE OR INSERT
*           R1 = LINE NUMBER
*
EMV    MOV  R11,R10           ;SAVE RETURN
EMV0   MOV  R1,R1             ;DELETE 0?
       JEQ  EMVR              ;Y, ACTION COMPLETE
       LI   R2,EBP
       S    R2,R8             ;GET # OF BYTES
       INC  R8                ;GET # OF WORDS (NEXT HIGHEST)
       SRA  R8,1
       MOV  R8,R2             ;GET # OF BYTES
       A    R2,R2
       MOV  @SLT,R6           ;GET START OF STATEMENT TABLE
       MOV  R6,R7
       S    @BUS,R7           ;R7=POINT OF QUESTION IN PSEUDO SOURCE
*
*UPON EXIT OF EMV1, R7 DISPLACES INTO PSEUDO SOURCE
*  AND R6 POINTS INTO SLT
*
EMV1   MOV  *R6,R4            ;DONE?
       JEQ  EMV2              ;Y
       C    R1,*R6            ;N, FOUND?
       JEQ  EMV5              ;Y, CHANGE OR DELETE
       JGT  EMV2              ;Y, NEW LINE
       INCT R6
       MOV  *R6+,R7           ;GET NEW POINT OF QUESTION
       JMP  EMV1
*
ERR13  DATA ERROR+13          ;NO SUCH LINE NUMBER
*
EMV2   MOV  R0,R0             ;NEW #, DELETE?
       JEQ  ERR13             ;Y, PROBLEM
*
*INSERT NEW LINE ENTRY
*
       MOV  R6,R3             ;GET SOURCE
       BL   @EMVA             ;ADJUST
       DATA 4                 ;INSERT 4 BYTES IN SLT
       MOV  R1,*R6+           ;INSERT NEW LINE #
       MOV  R7,*R6            ;INSERT DISPLACEMENT
*
EMV3   LI   R4,EBP            ;MOVE IN SOURCE LINE
       MOV  *R6,R6            ;GET ADR
       A    @BUS,R6           ;MAKE DISPLACEMENT, POINTER
       MOV  R6,R7             ;MOVE IN STRING
*
       MOV  *R4+,*R7+         ;MOVE
       DEC  R8                ;DONE?
       JNE  $-4               ;N
       PAGE
*ADJUST GOSUB
*
*       IN  R2 = PBC ADJUSTMENT
*           R3 = PLC ADJUSTMENT
*           R6 = START
*           R7 = END
*
EMV4   MOV  @GSC,R4           ;GET GOSUB STACK PTR
*
EMV4A  C    R4,@GSS           ;DONE?
       JLE  EMV4E             ;Y
       AI   R4,-4             ;N, BACKUP
       C    R6,*R4            ;LESS THAN INSERTED LINE?
       JH   EMV4C1            ;Y
       C    R7,*R4            ;GREATER THAN?
       JLE  EMV4C             ;Y
       MOV  R4,R5             ;N, DELETE ENTRY
*
EMV4B  MOV  @4(5),*R5+        ;DELETE ENTRY
       C    R5,@GSC           ;DONE?
       JL   EMV4B
       A    @CM4,@GSC         ;BACKUP PTR
       JMP  EMV4A
*
EMV4C  A    R2,*R4            ;Y, ADJUST PBC
       JMP  EMV4C2
EMV4C1 A    R3,@2(4)          ;PLC=PLC+PLC ADJUSTOR
EMV4C2 A    R2,@2(4)          ;PLC=PLC+PBC ADJUSTOR
       JMP  EMV4A
*
*ADJUST FOR/NEXT STACK
*
EMV4E  MOV  @FNS,R4           ;DO FOR/NEXT STACK
*
EMV4F  MOV  *R4,R0            ;DONE?
       JEQ  EMV4J             ;Y
       MOV  R4,R5             ;N
       AI   R5,14
       C    R6,*R5            ;LESS THAN INSERTED LINE?
       JH   EMV4I             ;Y
       C    R7,*R5            ;>=?
       JLE  EMV4H             ;Y
       BL   @FOR2             ;DELETE ENTRY
       JMP  EMV4F             ;LOOK AGAIN
*
EMV4H  A    R2,*R5            ;Y, ADJUST PBC
       JMP  EMV4I1
EMV4I  A    R3,@2(R5)         ;PLC=PLC+PLC ADJUSTOR
EMV4I1 A    R2,@2(R5)         ;PLC=PLC+PBC ADJUSTOR
       AI   R4,18             ;MOVE TO NEXT
       JMP  EMV4F
       PAGE
*ADJUST DATA POINTERS
*
EMV4J  LI   R4,DDM            ;GET DELIMITER PTR
       C    R6,*R4            ;CHECK DATA PTRS
       JH   EMV4L             ;OK
       C    R7,*R4            ;DELETED OR CHANGED?
       JLE  EMV4K             ;N, INSERTED
       CLR  *R4               ;Y, SET TO LOOK FURTHER
       JMP  EMV4KA
*
EMV4K  MOV  *R4,R0            ;DEFINED?
       JEQ  EMV4L             ;N, DON'T WORRY ABOUT IT
       A    R2,*R4            ;Y, ADJUST PBC
*
EMV4KA A    R3,@DLC           ;ADJUST PLC
*
*ADJUST FUNCTION DEFINITION STACK
*
EMV4L  MOV  @UFT,R4           ;GET POINTER
*
EMV4M  MOV  *R4,R0            ;DEFINED?
       JEQ  EMV4O             ;N
       C    R6,*R4            ;LESS THAN CHANGED LINE?
       JH   EMV4O             ;Y
       C    R7,*R4
       JLE  EMV4N             ;N
       CLR  *R4               ;UNDEFINE
       JMP  EMV4O
*
EMV4N  A    R2,*R4            ;ADJUST PBC
EMV4O  C    *R4+,*R4+         ;MOVE TO NEXT
       C    R4,@GSS           ;DONE?
       JL   EMV4M             ;N
*
EMVR   B    *R10              ;RETURN
       PAGE
*DELETE OR CHANGE LINE
*
EMV5   MOV  @2(6),R11         ;GET BOL
       S    R7,R11            ;GET -LINE LENGTH
       MOV  R0,R0             ;DELETE LINE?
       JNE  EMV6              ;N
*
*DELETE LINE
*
       MOV  R11,R2            ;Y
       MOV  R6,R3
*      AI   R3,4              ;GET SLT SOURCE
       C    *R3+,*R3+         LESS CODE THAN AI R3,4
       BL   @EMVA             ;ADJUST
CM4    DATA -4                ;DELETE 4 BYTES FROM SLT
       JMP  EMV4
*
*CHANGE LINE
*
EMV6   A    R11,R2            ;GET DELTA CHANGE
       MOV  @NVD,R0           ;CHECK STORAGE
       A    R2,R0
       C    R0,@NVS           ;ROOM?
       JHE  EMVE10            ;N
       LI   R13,EMV6A         ;GET RETURN ADR
       JMP  EMVA1
*
EMV6A  DATA 0                 ;LEAVE PLC ALONE
       INCT R6                ;MOVE TO LINE ADR
       JMP  EMV3
*
EMVE10 DATA ERROR+10          ;STORAGE OVERFLOW
       PAGE
*ALTER SOURCE CODE
*       BL @EMVA
*         DATA (SLT ADJUSTMENT)
*         R3=SLT SOURCE PTR
*
EMVA   MOV  R11,R13           ;SAVE RETURN
       MOV  @NVD,R4           ;GET THRU POINTER (END OF VARIABLE D   EFS)
       MOV  R3,R5             ;GET DESTINATION
       A    *R13,R5           ; (4=INSERT,-4=DELETE,0=CHANGE)
       MOV  R4,R0             ;CHECK FOR SIZE
       A    *R13,R0
       A    R2,R0             ;ADD LINE
       C    R0,@NVS
       JHE  EMVE10            ;OVERFLOW
       A    *R13,@VNT         ;ADJUST POINTERS
       A    *R13,@VDT
       A    *R13,@NVD
       BL   @MOVE             ;DO FIRST MOVE
*
EMVA1  MOV  R7,R3             ;MAKE HOLE IN PSEUDO SOURCE
       A    @BUS,R3           ;MAKE DISPLACEMENT INTO POINTER
       MOV  @NVD,R4           ;THRU
       MOV  R3,R5
       A    R2,R5             ;DESTINATION
       A    R2,R6
       A    R2,@SLT           ;ADJUST POINTERS
       A    R2,@VNT
       A    R2,@VDT
       A    R2,@NVD
       A    R2,@R8STOR        ;UPDATE R8STOR FOR ENTER.
       BL   @MOVE             ;DO SECOND MOVE
*
*ADJUST SLT
*
       MOV  R6,R3             ;GET POINTER
*
EMVA2  C    R3,@SLT           ;DONE?
       JLE  EMVA3             ;Y
       DECT R3                ;N
       A    R2,*R3            ;ADJUST PTR
       DECT R3
       JMP  EMVA2
*
EMVA3  MOV  *R13+,R3          ;GET PLC ADJUSTMENT
       B    *R13              ;RETURN
       PAGE
*MOVE
*       BL @MOVE
*
*       IN  R3 = SOURCE
*           R4 = SOURCE END
*           R5 = DESTINATION
*
MOVE   C    R3,R5             ;THERE?
       JEQ  MOVE4             ;Y
       JL   MOVE2             ;N, S<D
*
MOVE1  C    R3,R4             ;S>D, DONE?
       JH   MOVE4             ;Y
       MOV  *R3+,*R5+         ;MOVE DATA
       JMP  MOVE1
*
MOVE2  MOV  R4,R0             ;S<D
       S    R3,R0             ;GET # OF WORDS
       A    R0,R5
*
MOVE3  MOV  *R4,*R5           ;MOVE DATA
       C    R3,R4             ;DONE?
       JEQ  MOVE4             ;Y
       DECT R4                ;N, BACKUP
       DECT R5
       JMP  MOVE3
       PAGE
*GET LETTER
*       BL @EDGL
*         NO LETTER, R7 UNCHANGED
*       LETTER, R7 UPDATED
*
EDGLS  CB   @B20,*R7+         ;GET LETTER, SPACE?
       JEQ  EDGLS             ;Y
       DEC  R7                ;N, BACKUP
*
EDGL   CB   *R7,@B40          ;<"A?
       JLE  EDGL1             ;Y
       CB   *R7,@B5B          ;>"Z?
       JHE  EDGL1             ;Y
       CLR  R1                ;LETTER
       MOVB *R7+,R1
       SLA  R1,2              ;REMOVE UPPER BITS
       SRL  R0,5              ;ADJUST R0
       INCT R11
EDGL1  RT
*
MOVE4  EQU  EDGL1
*
*GET WORD OPERATER
*
EDGWO  MOV  R11,R5
       BL   @EDGLS            ;GET LETTER
       B    *R5               ;NO LETTER, RETURN
       DEC  R7                ;OK, PROCESS
       LI   R4,EDGWOL         ;GET WORD OPERATER LIST
*
EDGWO1 MOV  R7,R3             ;MARK
*
EDGWO2 MOVB *R4+,R1           ;GET CHARACTER, FOUND?
       JEQ  EDGWO4            ;Y
       CB   *R3+,R1           ;N, SAVE LETTER?
       JEQ  EDGWO2            ;Y
       MOVB *R4+,R1           ;N, MOVE TO NEXT
       JNE  $-2
       INC  R4                ;MOVE OVER CODE
       MOVB *R4,R1            ;DONE?
       JNE  EDGWO1            ;N, KEEP TRYING
       B    *R5               ;Y, RETURN
*
EDGWO4 MOVB *R4,*R8           ;RESERVED WORD, GET CODE
       MOV  R3,R7             ;UPDATE R7
       CB   *R8,@B3B          ;THEN?
       JEQ  EDGWO5            ;Y, PROCESS :
       B    @ED40             ;N, INSERT OPERATER
*
EDGWO5 B    @ED47
       PAGE
*GET CHARACTER
*
*       BL @EDGP
*         "( OR "[
*       OTHER
*
EDGP   CLR  R1
       MOVB *R7+,R1           ;GET CHARACTER
       CI   R1,>2000          ;SP?
       JEQ  EDGP              ;Y
       CI   R1,>2800          ;"(?
       JEQ  EDGP1             ;Y
       CI   R1,>5B00          ;"[?
       JEQ  EDGP1             ;Y
       INCT R11               ;N, RETURN 2(11)
EDGP1  RT
       PAGE
*
* COMMAND LIST
*
* SYMBOLS STORED AS:
*  3333 3222 2211 111S
*  WHERE S=0  3 LETTERS
*        S=1  4-6 LETTERS
*
*          0 1 2 3 4 5 6 7
*       0  @ A B C D E F G
*       1  H I J K L M N O
*       2  P Q R S T U V W
*       3  X Y Z
*
EDSCL  DATA >7564             RUN              A
       DATA >D266             SIZ(E)           B
       DATA >73C6             CON(TINUE)       C
       DATA >73DA             MON(ITOR)        D
       DATA >0000                              E
       DATA >0000                              F
       DATA >0000                              G
       DATA >0000                              H
       DATA >0000                              I
       DATA >0000                              J
       DATA >0000                              K
*
EDCL   DATA >A3CF             GOTO*            01
RNGOTO EQU  ($-EDCL)/2        RENUMBER OPCODE
       DATA >9BCF             GOSUB*           02
RNGSUB EQU  ($-EDCL)/2        RENUMBER OPCODE
       DATA >9B0B             ELSE*            03
RNELSE EQU  ($-EDCL)/2        RENUMBER OPCODE
       DATA >6964             REM*             04
RNREM  EQU  ($-EDCL)/2        RENUMBER OPCODE
       DATA >93CC             FOR*             05
       DATA 0                 (LET*)           06
       DATA >A049             DATA             07
NXTX   DATA >C15D             NEXT             08
       DATA >948B             ERROR            09
RNERR  EQU  ($-EDCL)/2        RENUMBER OPCODE
PRTX   DATA >4CA1             PRINT            0A
       DATA >6047             CALL             0B
       DATA >0BD9             LOAD             0C
       DATA >8393             INPUT            0D
RNINP  EQU  ($-EDCL)/2        RENUMBER OPCODE
       DATA >0965             READ             0E
       DATA >9965             RESTOR           0F
RNRSTR EQU  ($-EDCL)/2        RENUMBER OPCODE
       DATA >A165             RETURN           10
       DATA >7D27             STOP             11
       DATA >4BAB             UNIT             12
       DATA >6A69             TIME             13
       DATA >B067             SAVE             14
       DATA >9845             BASE             15
       DATA >1CCB             ESCAPE           16
       DATA >2BDD             NOESC            17
       DATA >7065             RANDOM           18
       DATA >A845             BAUD             19
       DATA >A38B             ENTER            1A
       DATA >7B21             PLOT             1B
       DATA >83AB             UNPLOT           1C
       DATA >63C7             COLOUR           1D
       DATA >9561             PURGE            1E
       DATA >0C8F             GRAPH            1F
       DATA >C169             TEXT             20
       DATA >486F             WAIT             21
       DATA >0A07             CHAR             22
       DATA >6D5D             NUMBER           23
       DATA >9A59             LIST             24
       DATA >7165             RENUM            25
       DATA >9427             SPRITE           26
       DATA >0A27             SHAPE            27
       DATA >AC27             SPUT             28
       DATA >29E7             SGET             29
       DATA >7BC5             BOOT             2A
       DATA >0DE7             SWAP             2B
       DATA >0000                              2C
       DATA >A3DB             MOTOR            2D
       DATA >0000                              2E
       DATA >0000                              2F
       DATA >0000                              30
       DATA >0000                              31
       DATA >0000                              32
       DATA >0000                              33
       DATA >0000                              34
       DATA >0000                              35 
       DATA >0000                              36
       DATA >0000                              37
       DATA >0000                              38
       DATA >385A             MAG              39
       DATA >33E8             TOF              3A
       DATA >73E8             TON              3B
       DATA >83E0             POP              3C
       DATA >6A48             DIM              3D
       DATA >A158             LET              3E
PSCBX  EQU  $-EDCL+2
       DATA 0                 ';' (PRINT)      3F
ONX    DATA >73C0             ON               40
RNON   EQU  ($-EDCL)/2        RENUMBER OPCODE
IFX    DATA >3240             IF               41
RNIF   EQU  ($-EDCL)/2        RENUMBER OPCODE
DEFX   DATA >3148             DEF              42
       DATA >B95C             NEW              43
       DATA >238A             END              44
PQMBX  EQU  $-EDCL+2
       DATA 0                 (?)  (PRINT)     45
PASBX  EQU  $-EDCL+2
       DATA 0                 (*)  (EXTEND)    46
BCP    EQU  $-EDCL/2+1        ****** INSERTS BEFORE HERE
       DATA >A244             BIT  (SEE ED50)  47
       DATA >1486             CRB              48
       DATA >3486             CRF              49
       DATA >695A             MEM              4A
       DATA >25DA             MWD              4B
EDCLE  EQU  $
*
IFB    EQU  IFX-EDCL+2
ONB    EQU  ONX-EDCL+2
PTRB   EQU  PRTX-EDCL+2
       PAGE
*
* SECOND HALF OF PRIMITIVE TABLE
*
       DATA RUNP        RUN                    A
       DATA SIZE        SIZ(E)                 B
       DATA CONT        CON(TINUE)             C
       DATA DEBUG$      MON(ITOR)              D
       DATA >0000       <SPARE>                E
       DATA >0000       <SPARE>                F
       DATA >0000       <SPARE>                G
       DATA >0000       <SPARE>                H
       DATA >0000       <SPARE>                I
       DATA >0000       <SPARE>                J
       DATA >0000       <SPARE>                K
*
EDCS   EQU  $-EDCL-2
       DATA >001E             GOTO             01
       DATA >00AA             GOSUB            02
B0A    EQU  $+1
       DATA >000A             ELSE*            03
BCPB   BYTE BCP,0             REM*             04
CFF80  DATA >FF80             FOR*             05
       DATA >0000             (LET)*           06
B02    EQU  $+1
       DATA >0002             DATA             07
       DATA >0028             NEXT             08
       DATA >049E             ERROR            09
       DATA >051C             PRINT            0A
       DATA >0018             CALL             0B
       DATA >0008             LOAD             0C
       DATA >052A             INPUT            0D
       DATA >0008             READ             0E
       DATA >93E8             RESTOR           0F
       DATA >74AA             RETURN           10
       DATA >0020             STOP             11
       DATA >0028             UNIT             12
       DATA >000A             TIME             13
       DATA >000A             SAVE             14
       DATA >000A             BASE             15
       DATA >2C02             ESCAPE           16
       DATA >00E6             NOESC            17
       DATA >6BC8             RANDOM           18
       DATA >0008             BAUD             19
       DATA >048A             ENTER            1A
       DATA >0028             PLOT             1B
       DATA >A3D8             UNPLOT           1C
       DATA >955E             COLOUR           1D
       DATA >014E             PURGE            1E
       DATA >0220             GRAPH            1F
       DATA >0028             TEXT             20
       DATA >0028             WAIT             21
       DATA >0024             CHAR             22
       DATA >9144             NUMBER           23
       DATA >0028             LIST             24
       DATA >036A             RENUM            25
       DATA >2D12             SPRITE           26
       DATA >0160             SHAPE            27
       DATA >0028             SPUT             28
       DATA >0028             SGET             29
       DATA >0028             BOOT             2A
       DATA >0020             SWAP             2B
       DATA >0000                              2C
       DATA >049E             MOTOR            2D
       DATA >0000                              2E
       DATA >0000                              2F
       DATA >0000                              30
       DATA >0000                              31
       DATA >0000                              32
       DATA >0000                              33
       DATA >0000                              34
       DATA >0000                              35
       DATA >0000                              36
       DATA >0000                              37
       DATA >0000                              38
*
       PAGE
*
* SYSTEM FUNCTION TABLE
*
EDIL   DATA >9882             ABS      1B
       DATA >9102             ADR      1C
       DATA >1CC2             ASC      1D
       DATA >7502             ATN      1E
       DATA >9BC6             COS      1F
       DATA >860A             EXP      20
       DATA >0C8C             FRA      21
       DATA >A392             INT      22
       DATA >3BD8             LOG      23
       DATA >C956             KEY      24
       DATA >7266             SIN      25
       DATA >9466             SQR      26
       DATA >9E66             SYS      27
       DATA >1A68             TIC      28
       DATA >71E6             SGN      29
*
*       ASSIGNABLE FUNCTIONS
*
       DATA >A244             BIT      2A
       DATA >1486             CRB      2B
       DATA >3486             CRF      2C
       DATA >695A             MEM      2D
       DATA >25DA             MWD      2E
*
*       CHARACTER FUNCTIONS
*
       DATA >7158             LEN      2F
       DATA >40DA             MCH      30
       DATA >9BE0             POS      31
       DATA >63C6             COL      32
       DATA >23DA             MOD      33
       DATA 0                          34
       DATA 0                          35
       DATA 0                          36
       DATA 0                          37
EDILE  EQU  $
*
* TRANSLATION TABLE INDEXED BY ASCII CODE.
*       NULLS ARE ILLEGAL.
*
       DATA >FFFF             !! FIX FOR ' ' CHARACTER !!
EDSL   DATA >4744,>3E43       ! " # $
       DATA >4248,>454C       % & ' (
       DATA >4D5F,>5D3F       ) * + ,
       DATA >5C00,>5E00       - . / 0
       DATA >0000,>0000       1 2 3 4
       DATA >0000,>0000       5 6 7 8
       DATA >003C,>4059       9 : ; <
B56    DATA >5657,>413D       = > ? @
       DATA >4C46,>4D60       [ \ ] ^
*
B59    EQU  EDSL+27
B57    EQU  EDSL+29
B46    EQU  EDSL+33
       PAGE
EDGWOL EQU  $                 WORD OPERATORS
LSTO   TEXT 'TO'
       BYTE 0,>38
LSTB   TEXT 'TAB'
       BYTE 0,>39
LSST   TEXT 'STEP'
       BYTE 0,>3A
LSTH   TEXT 'THEN'
       BYTE 0,>3B
LSOR   TEXT 'OR'
       BYTE 0,>4E
LSLOR  TEXT 'LOR'
       BYTE 0,>4F
LSAN   TEXT 'AND'
       BYTE 0,>50
LSLAN  TEXT 'LAND'
       BYTE 0,>51
LSNT   TEXT 'NOT'
       BYTE 0,>52
LSLNT  TEXT 'LNOT'
       BYTE 0,>53
LSLXO  TEXT 'LXOR'
       BYTE 0,>54,0
       PAGE
*
* LIST TRANSLATION TABLE INDEXED BY PSEUDO CODE.
*       "_ ARE UNDEFINED.
*
EDLC   EQU  $
B3A    BYTE >3A               :
B40    BYTE >40               @
       BYTE >23               #
B2C    BYTE >2C               ,
B3B    BYTE >3B               ;
B3F    BYTE >3F               ?
       BYTE >25               %
       BYTE >24               $
       BYTE >22               "
       BYTE >27               '
       BYTE >5C               \
       BYTE >21               !
       BYTE >26               &
B03    BYTE >03               _    (UNUSED)
B5B    BYTE >5B               [
B5D    BYTE >5D               ]
       BYTE >28               (
       BYTE >29               )
B1B    BYTE >1B               OR   (UNUSED)
B6D    BYTE >6D               LOR  (UNUSED)
B4E    BYTE >4E               AND  (UNUSED)
B6F    BYTE >6F               LAND (UNUSED)
B38    BYTE >38               NOT  (UNUSED)
DEFXB  BYTE DEFX-EDCL/2+1     LNOT (UNUSED)
NXTXB  BYTE NXTX-EDCL/2+1     LXOR (UNUSED)
       BYTE >3D               ==
B3D    BYTE >3D               =
       BYTE >3E               >
       BYTE >3E               >=
       BYTE >3C               <
       BYTE >3C               <=
       BYTE >3C               <>
       BYTE >2D               -
       BYTE >2B               +
       BYTE >2F               /
       BYTE >2A               *
       BYTE >5E               ^
*
       EVEN
LCN1   EQU  EDLC->3C
LCN2   EQU  EDCL-2
LCN3   EQU  EDIL->36
LCN4   EQU  EDCS+2
LNXT   EQU  NXTX-EDCL+2
LDEF   EQU  DEFX-EDCL+2
       PAGE
*
* LIST COMMAND
*   FORM :                         LISTS LINES :-
*
*     LIST                       FIRST LINE ==> LAST LINE
*     LIST <ARG 1>                    ARG 1 ==> ARG 1
*     LIST TO <ARG 1>            FIRST LINE ==> ARG 1
*     LIST <ARG 1> TO <ARG 2>         ARG 1 ==> ARG 2
*
LST    EQU  $
       BL   @MODEOK           ;ABORT IF RUNNING
       DATA TYPC$             ;OUT 'CRLF'
       CLR  R1                ;DEFAULT START=1ST LINE
       SETO R6                ;DEFAULT STOP = LAST LINE
       CB   *R8+,@B38         ; 'TO' ?
       JEQ  GSTOP             ;Y, GET STOP
       DEC  R8                ;N, BACKUP
       BL   @CKEX             ;EXPRESSION ?
       JMP  LST0B             ;N, LIST WHOLE PROGRAM
       EVFIX R1               ;Y, GET START ADDRESS
       MOV  R1,R6             ;MAKE IT THE DEFAULT STOP TOO
       CI   R0,>3800          ;'TO' ?
       JNE  LST0B             ;N, TAKE DEFAULT STOP
GSTOP  EVFIX R6               ;Y, GET STOP LINE
*
*     LIST PROGRAM LINES
*        R1= START LINE NUMBER
*        R6= STOP  LINE NUMBER
LST0B  MOV  @VNT,R8           ;GET TABLE ADR
       DECT R8                ;BACKUP
LST0   C    R8,@SLT           ;DONE?
       JLE  LST2              ;Y
       AI   R8,-4             ;MOVE TO NEXT ENTRY
       C    R1,*R8            ;SAME?
       JGT  LST0
       SETO @DCNT             ;RESET INDENT COUNTER
LST1   C    R8,@SLT           ;DONE?
       JL   LST2              ;Y
       MOV  *R8+,R1           ;N, GET LINE NUMBER
       C    R1,R6             ;PAST LAST LINE ?
       JH   LST2              ;Y, END LIST
*      MOV  @IOB,R7           ;GET BUFFER ADR !DONE IN LSTL
       BL   @LSTL             ;LIST LINE
*
*      TEST FOR HALT OUTPUT
*
       BLWP @HALTO$
       AI   R8,-6             ;PREPARE FOR NEXT LINE
       DATA TYPBE$            ;TERMINATE & OUTPUT LINE
       DATA TYPC$             ;OUT 'CRLF'
       JMP  LST1              ;LOOP TILL DONE
LST2   B    @CRLF             ;EXIT TO CRLF
       PAGE
*LIST GOTO'S AND GOSUB'S
*
LSTG1  MOVB @B2C,*R7+         ;OUT ",
       INC  R3                ;SKIP ','
*
LSTG   MOVB *R3+,R1           ;GET LINE NUMBER
       SWPB R1
       MOVB *R3+,R1
       SWPB R1
       OUTINT R1              ;CONVERT
       CB   *R3,@B3F          ;CHECK NEXT BYTE FOR ",
       JEQ  LSTG1             ;Y, ANOTHER GOTO, OR GOSUB
       JMP  LSTLX             ;N, PROCESS
       PAGE
*LIST LINE
*       BL @LSTL
*
*       IN  R1 = LINE #
*          *R8 = PBC
*       R15 = SP
*
*       OUT R7 = IOB
*           PRESERVE R6,R8
*
LSTL   MOV  R11,R10           ;SAVE RETURN
       DATA FTM$              ;FORCE TO TEXT MODE
       MOV  @IOB,R7           ;GET IO BUFFER POINTER
       MOV  *R8,R3            ;GET PBC
       A    @BUS,R3           ;MAKE DISPLACEMENT INTO POINTER
       LI   R15,>2000         ;GET SPACE
       MOVB R15,*R7+          ;OUT SPACE
       OUTINT R1              ;CONVERT LINE NUMBER
       MOV  @DCNT,R2          ;GET INDENT COUNT
       CB   *R3,@B03          ;ELSE?
       JNE  LSTL1             ;N
       DECT R2                ;Y, INDENT 2 MORE SPACES
*
LSTL1  MOVB R15,*R7+          ;OUT SPACE
       INC  R2                ;MORE?
       JLT  LSTL1             ;Y
       CB   *R3,@NXTXB        ;NEXT?
       JNE  LSTL2             ;N
       DEC  R7                ;Y, INDENT 1 LESS
*
LSTL2  MOVB R15,*R7+          ;OUT SPACE
*
LSTL3  CLR  R0
       MOVB *R3+,R0           ;GET TYPE
       CI   R0,>0600          ;IMPLIED LET?
LETB   EQU  $-2
       JEQ  LSTL4             ;Y
*
       CI   R0,PASBX*128      ;(*) EXTENDED COMMAND ?
       JNE  LETP$1            ;N, TRY PRINT
       MOVB @B2A,*R7+         ;Y, OUT A '*'
OUTEXT MOVB *R3+,*R7+         COPY OVER CHARACTER
       JNE  OUTEXT            LOOP TILL NULL FOUND
       DEC  R7                BACKUP
       JMP  LETB$2            OUT ' ' AND CONTINUE
LETP$1 CI   R0,PQMBX*128      ;(?) COMPRESSED PRINT ?
       JNE  LETB$1            ;N , TRY (;)
       MOVB @B3F,*R7+         ;Y, OUT A '?'
       JMP  LETB$2            ; OUT SPACE & CONTINUE
LETB$1 CI   R0,PSCBX*128      ;(;) COMPRESSED PRINT ?
       JNE  LSTL3A            ;N, NORMAL CODE
       MOVB @B3B,*R7+         ;Y, OUT ';'
LETB$2 SRA  R0,7              ;ADJUST R0
       JMP  LSTL3B            ;OUT SPACE & CONTINUE
LSTL3A BL   @LWRD             ;LIST COMMAND TYPE
       DATA LCN2              ;EDCL-2
LSTL3B MOVB R15,*R7+          ;OUT SPACE
       CI   R0,BCP*2          ;BIT,CRB,CRF,MEM?
       JL   LSTL4             ;N
       DEC  R7                ;Y, ELIMINATE SPACE
*
LSTL4  MOV  R0,R14            ;SAVE TYPE
       CI   R0,>3*2           ;CHECK TYPE
       JLT  LSTG              ;GOTO OR GOSUB
       JEQ  LSTL3             ;ELSE
       CI   R0,>5*2
       JLT  LSTRM             ;LT - REMARK STATEMENT
       JGT  LSTL5             ;GT - CONTINUE LOOKING
       DEC  @DCNT             ;FOR, DECREMENT COUNTER
*
LSTL5  CI   R0,LNXT           ;NEXT?
       JNE  LSTL6             ;N
       INC  @DCNT             ;Y, INCREMENT COUNTER
       JLT  LSTL6             ;COUNTER OK
       SETO @DCNT             ;RESET COUNTER
LSTL6  MOV  R0,R14            ;SAVE TYPE
       PAGE
LSTLX  MOVB *R3+,R0           ;GET CODE
       JEQ  LSTLE             ;DONE
       CB   R0,@B1B           ;USER FUNCTION?
       JL   LSTFNP            ;Y
       CB   R0,@B38           ;SYSTEM FUNCTION?
       JL   LSTSF             ;Y
       SRL  R0,8              ;READY R0,R4
       MOV  R0,R4
       CI   R0,>3C            ;TO, TAB, STEP, THEN?
       JL   LSTTT             ;Y
       CI   R0,>4E            ;< OR?
       JL   LSTLX0            ;Y
       CI   R0,>54            ;> LXOR?
       JLE  LSTTI             ;N
*
LSTLX0 CI   R0,>62            ;CHARACTER?
       JL   LSTCR             ;Y
       CI   R0,>6F            ;CONSTANT?
       JL   LSTCN             ;Y, INTEGERS
       JEQ  LSTCNF            ;Y, FLOATING POINT
*
       MOV  @VNT,R5           ;N, VARIABLE
       A    R0,R0             ;X 2
       A    R0,R5             ;INDEX & GET VARIABLE
       CLR  R2
       MOV  @-2*>70(5),R5
       JGT  LSTVN             ;DIMENSIONED?
       NEG  R5                ;Y
B4A    EQU  $+3
       LI   R2,>4A            ;OUT [
*
LSTVN  COC  @C380,R5          ;LETTER + NUMBER?
       JNE  LSTVN1            ;N
       BL   @LWRDO            ;Y, OUT CHARACTER
       SRL  R1,2
       MOV  R5,R1             ;OUT #
       ANDI R1,>7F            ;MASK
       OUTINT R1              ;CONVERT
       JMP  LSTVN2
*
LSTVN1 SLA  R5,1              ;REGULAR VARIABLE
       LI   R13,LSTVN2
       B    @LWRD2            ;LIST
*
LSTVN2 MOV  R2,R4             ;"[ NEEDED?
       JNE  LSTCR1            ;Y
       JMP  LSTLX
*
LSTLE  SB   *R7,*R7           ;PUT NULL ON END
       B    *R10
       PAGE
LSTCR  MOVB @LCN1(4),*R7+     ;EDLC->3C
       SWPB R0
       BL   @JMPR0            ;LOOK FOR CODES
LSTCRT BYTE LSTCR0-LSTCRT/2,>44 "
       BYTE LSTCR0-LSTCRT/2,>45 '
       BYTE LSTTR-LSTCRT/2,>47 !
       BYTE LSTL2P-LSTCRT/2,>3C :
       DATA 0
       BL   @LSTCR2           ;LOOK FOR DOUBLES  (>AAII)
*             ==    >=    <=    <>
       DATA >3D55,>3D58,>3D5A,>3E5B,>0000
*
*TAIL REMARK
*
LSTTR  DEC  R7                ;BACKUP OVER !
       MOVB R15,*R7+          ;OUT SPACE
       MOVB R15,*R7+
       MOVB @LCN1(4),*R7+
       JMP  LSTRM1
*
*REMARK ENTRY
*
LSTRM  DEC  R7                ;REM, MOVE BACK 1 CHAR
*
LSTRM1 MOVB *R3+,*R7+         ;MOVE INTO LINE
       JNE  $-2
       DEC  R7                ;BACKUP
       B    *R10              ;RETURN
       PAGE
*
LSTCR2 MOVB *R11+,*R7         ;MOVE CHARACTER INTO LINE
       JEQ  LSTLX             ;NOT FOUND
       CB   R0,*R11+          ;FOUND?
       JNE  LSTCR2            ;N, KEEP LOOKING
       INC  R7                ;Y, SKIP OVER CHARACTER
       JMP  LSTLX             ;CONTINUE
LSTL2P B    @LSTL2
*
LSTFNP JMP  LSTFN
LSTCR0 MOVB *R3+,*R7+         ;MOVE IN CHARACTER STRING
       JNE  $-2               ;LOOP UNTIL NULL
       DEC  R7                ;BACKUP OVER NULL
*
LSTCR1 MOVB @LCN1(4),*R7+     ;EDLC->3C
       JMP  LSTLX
*
LSTSF  BL   @LWRD             ;LIST SYSTEM FUNCTION
       DATA LCN3              ;EDIL->36
LSTSF1 CB   *R3,@B4A          ;[?
       JEQ  LSTLX             ;Y, NO SPACE
LSTSP  MOVB R15,*R7+          ;OUT SPACE
LSTLXP JMP  LSTLX
*
LSTTI  AI   R4,->12           ;MOVE BACK BY THEN
*
LSTTT  MOVB R15,*R7+          ;OUT SPACE
       A    R4,R4             ;DOUBLE INDEX
       MOV  @LSTTL->70(4),R5  38*2
*
LSTT1  MOVB *R5+,*R7+         ;MOVE LETTER
       MOVB *R5,R0            ;CHECK END
       JNE  LSTT1             ;LOOP
       CI   R4,>3B*2          ;THEN?
       JEQ  LSTL2P            ;Y
       JMP  LSTSP             ;OUT SPACE
       PAGE
LSTCN  CI   R0,>6D            ;-1 THRU 9?
       JHE  LSTCN1            ;N
       AI   R0,->63           ;Y
       MOV  R0,R1
       JMP  LSTCN2            ;CONVERT
*
LSTCN1 MOVB *R3+,R1           ;INTEGER
       SWPB R1
       MOVB *R3+,R1
       SWPB R1
B6E    EQU  $+3
       CI   R0,>6E            ;HEX?
       JEQ  LSTCN3            ;Y
*
LSTCN2 OUTINT R1              ;N, CONVERT
       JMP  LSTLXP
*
LSTCN3 BL   @HOUT             ;OUT HEX
       JMP  LSTLXP
*
LSTCNF OUTFP *R3              ;OUTPUT FP #
       AI   R3,6              ;INCREMENT OVER NUMBER
       JMP  LSTLXP
*
*USER FUNCTION ENTRY
*
LSTFN  MOVB @B46,*R7+         ;OUT "F
       MOVB @B4E,*R7+         ;OUT "N
       AI   R0,>4000
       MOVB R0,*R7+           ;OUT LETTER
       CI   R14,LDEF          ;DEF?
       JNE  LSTSF1            ;N, CHECK FOR (
       MOV  @VNT,R14          GET TABLE ADDR & CLEAR 'LDEF'
       CB   *R3,@B56          ;Y, ARGUMENTS?
       JEQ  LSTLXP            ;N
       MOVB @B5B,*R7+         ;Y, OUT "[
*
LSTFD1 MOVB *R3+,*R14         ;SAVE DUMMY NAME
       MOV  *R14+,R1
       SRA  R1,2
       BL   @LWRDO1           ;MAKE LETTER & STORE
       CB   *R3,@B56          ;=?
       JEQ  LSTFD2            ;Y
       MOVB @B2C,*R7+         ;N, OUT ','
       JMP  LSTFD1            ;LOOP AGAIN
*
LSTFD2 MOVB @B5D,*R7+         ;OUT ']'
       JMP  LSTLXP
*
LSTTL  DATA LSTO,LSTB
       DATA LSST,LSTH
       DATA LSOR,LSLOR
       DATA LSAN,LSLAN
       DATA LSNT,LSLNT
       DATA LSLXO
       PAGE
*LIST WORD
*      BL   @LWRD
*           ADDR
*
LWRD   MOV  *R11+,R4          ;GET ADR OF WORD
       MOV  R11,R13           ;SAVE RETURN
       SRA  R0,7              ;SWAP AND X 2
       A    R0,R4             ;INDEX
*
LWRD1  MOV  *R4,R5            ;GET WORD
LWRD2  BL   @LWRDO            ;OUT FIRST CHARACTER
       SLA  R1,7
       BL   @LWRDO            ;OUT 2ND OR 5TH CHARACTER
       SLA  R1,2
       BL   @LWRDO            ;OUT 3RD OR 6TH CHARACTER
       SRL  R1,3
       AI   R4,LCN4           ;MOVE TO NEXT HALF (EDCS+2)
       SRL  R5,1              ;ANOTHER HALF?
       JOC  LWRD1             ;Y
       B    *R13              ;N, RETURN
*
LWRDO  MOV  R5,R1             ;LOAD TEMP
       X    *R11+             ;EXECUTE SHIFT
       ANDI R1,>1F00          ;MASK
       JEQ  LWRDO2            ;RETURN
*
LWRDO1 AI   R1,>4000          ;ADD LETTER BITS
       MOVB R1,*R7+           ;MOVE OUT
LWRDO2 B    *R11              ;RETURN
       END
