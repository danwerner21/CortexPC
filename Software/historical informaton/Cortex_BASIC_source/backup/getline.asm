       TITL 'GETLINE ROUTINE - CORTEX BASIC REV. 1.1'
       IDT  'GETLINE'
*
* 
*
       DEF  GTLN,EDER,C2000,B20,B08
*
       REF  AINC,EDTMP,CVDIZ
       REF  DCNT,JMPR0
       REF  LNUM,LSTL,NIC,SFSN
       REF  TYP0$,TYP11$,TYPB$,TYPC$,TYPE$
       REF  GETCR$,IOB
       REF  CRLF,NOERR,MODE
       REF  ERRLS2,BLSTOR
ERROR  EQU  >2F80
       DXOP OUTINT,13
       PAGE
*
*^E   *** EDIT LINE ***
*
GTCE   MOV  @MODE,R14         ;IDLE ?
       JNE  GTLLP             ;N, IGNORE CHARACTER
       MOV  @IOB,R7           ;GET BUFFER ADR
       BLWP @CVDIZ            ;CONVERT #
       NOP
       DATA ERROR+13          ;NO SUCH LINE
       BL   @SFSN             ;SEARCH FOR STATEMENT #
       MOV  @IOB,R7           ;RESET BUFFER ADR
       SETO @DCNT             ;RESET INDENT COUNTER
       BL   @LSTL             ;LIST LINE
       DATA TYPC$             ;OUT CRLF
       JMP  GTCE2
*
*GET INPUT LINE
*       R6 = MAX # OF CHARACTERS
*       R7 = I/O POINTER
*
GTLN   MOV  R11,@BLSTOR       SAVE RETURN ADDRESS
       LI   R6,NIC            ;MAXIMUM OF NIC CHARACTERS
       MOV  @IOB,R7           ;GET IOB PTR
       MOV  R7,R3
       MOV  R6,R1
*
       CLR  *R3+              ;CLEAR BUFFER
       DECT R1                ;DONE?
       JGT  $-4               ;N
*
       MOV  @AINC,R1          ;AUTO-INCREMENT
       JEQ  GTLLP$            ;N
       A    @LNUM,R1          ;Y, ADD IN LAST LINE #
       JLT  GTCR              ;-VE, TERMINATE AUTO-INC
       OUTINT R1              ;CONVERT
       MOVB @B20,*R7+         ;SPACE
*
GTCE2  DATA TYPB$             ;TYPE
       LI   R6,NIC            ;GET NEW COUNT
       A    @IOB,R6           ;ADD BUFFER ADR
       S    R7,R6             ;SUB CURRENT ADR
       PAGE
GTLLP$ CLR  R15               ;RESET INSERT FLAG
GTLLP  DATA GETCR$            ;LOOP
       CI   R0,>2000          CONTROL ?
       JHE  $+4               N, LEAVE INSERT FLAG
       CLR  R15               Y, KILL INSERT FLAG
       BL   @JMPR0            ;DO JUMP ON R0
GTJMP  BYTE GTLD-GTJMP/2,>17  ^W(ETB)DELETE
       BYTE GTCE-GTJMP/2,>05  ^E(ENQ)EDIT
       BYTE GTCF-GTJMP/2,>09  ^I(HT) CURSOR RIGHT
       BYTE GTBS-GTJMP/2,>08  ^H(BS) CURSOR LEFT
       BYTE GTLI-GTJMP/2,>16  ^V(SYN)INSERT
       BYTE GTLLP3-GTJMP/2,>0C ^L(FF) CLEAR SCREEN
       BYTE GTLF-GTJMP/2,>0A  LF     IGNORE
       BYTE GTCR-GTJMP/2,>0D  CR
       BYTE GTRB-GTJMP/2,>7F  RUBOUT
       DATA 0
       CI   R0,>2000          ;<BLK?
       JL   GTLB              ;Y, OUT BELL
       MOVB *R7,R5            ;EXPANDING BUFFER?
       JNE  GTLLP1            ;N
       DEC  R6                ;Y, ROOM?
       JLT  GTLLP2            ;N, OUT BELL
GTLLP1 MOV  R15,R15           NORMAL MODE ?
       JNE  INS               N, INSERT
       MOVB R0,*R7+           ;Y, STORE BYTE
       JMP  GTLLP3
*
GTLF   EQU  GTLLP
*
GTLLP2 INC  R6                ;RESTORE R6
GTLB   LI   R0,>0700          ;OUT BELL
GTLLP3 DATA TYP0$             ;ECHO CHARACTER
       JMP  GTLLP
*
GTCR   MOVB *R7,*R7+          AT END OF LINE?  <<<
       JEQ  GTCR1             Y, EXIT          <<<
       DATA TYP11$,>0900      N, CURSOR RIGHT  <<<
       JMP  GTCR              AND LOOP         <<<
GTCR1  MOV  @BLSTOR,R11       GET RETURN ADDRESS
       RT
       PAGE
*BACKSPACE  (^H)
*
GTBS   C    @IOB,R7           ;BEGINNING OF BUFFER?
       JEQ  GTLB              ;Y, OUT BELL
       DEC  R7                ;N, BACKUP
       JMP  GTRB1             ;OUT BACKSPACE
*
*RUBOUT
*
GTRB   CLR  R15               RESET INSERT FLAG
       C    @IOB,R7           ;BEGINNING OF BUFFER?
       JEQ  GTLB              ;Y, OUT BELL
       DEC  R7                ;N, BACKUP
       MOVB @B20,*R7          ;STORE BLANK
       DATA TYP11$,>0800      ;OUT BKSP,BLK
       DATA TYP11$,>2000
C2000  EQU  $-2
B20    EQU  C2000
*
GTRB1  DATA TYP11$            ;OUT BKSP
B08    EQU  $
       DATA >0800
       JMP  GTLLP             ;GOTO LOOP
*
*FORWARD SPACE (^I)
*
GTCF   MOVB *R7+,R0           ;GET CHARACTER
       JNE  GTLB+4            ;OK TO SEND
       DEC  R7                ;TOO FAR
       JMP  GTLB              ;OUT BELL
       PAGE
*
* EDIT ERROR RECOVERY ROUTINE
*
EDER   EQU  $
       MOV  @NOERR,R3         ERROR TRAP ENABLED?
       JEQ  EDER1             Y, HANDLE IT
       B    @CRLF             N, IGNORE TRAP
EDER1  MOV  *R11,R3           PICKUP ERROR CHARACTERS
       DEC  R7
       MOV  R7,R5             ;Y, MARK
       BL   @ERRLS2           OUTPUT ERROR MESSAGE
       MOV  @EDTMP,R6         RESTORE # 'FREE SPACES' IN IOB
*
       DATA TYPC$             ;OUT CRLF
       DATA TYPB$             ;OUT BUFFER
       DATA TYP11$,>0D00      ;OUT CR ONLY
       MOV  @IOB,R7
       CLR  R15               ;RESET INSERT FLAG
GTLI4  C    R7,R5             ;POSITIONED?
       JHE  GTLLP             ;Y, GOTO LOOP
       CLR  R0
       MOVB *R7+,R0           ;N, OUT CHARACTER
       DATA TYP0$
       JMP  GTLI4             ;DO AGAIN
       PAGE
*
* DELETE
*
GTLD   MOVB *R7,*R7           ;ANYTHING TO DELETE?
       JEQ  GTLB              ;N, OUT BELL
       MOV  R7,R1             ;Y, SAVE CURRENT POSITION
*
DEL1   MOVB @1(7),*R7+        ;OVERWRITE WITH FOLLOWING CHAR
       JNE  DEL1              ;LOOP TILL WE COPY THE NULL
       MOVB *R1,*R1           ;EMPTY LINE?
       JEQ  DEL1A             ;Y, DONT OUTPUT IT THEN
       DATA TYPE$             ;OUTPUT REST OF LINE
DEL1A  DATA TYP11$,>2000      ;AND A SPACE
       S    R1,R7             ;CALCULATE # BS TO OUTPUT
DEL2   DATA TYP11$,>0800      ;OUTPUT A BACKSPACE
       DEC  R7                ;DONE?
       JNE  DEL2              ;N, LOOP
DEL3   MOV  R1,R7             ;RESTORE POINTER
       INC  R6                ;SOMETHING TO DELETE
       JMP  GTLLP             ;AND LOOP
       PAGE
*
* INSERT
*
GTLI   SETO R15               ;FLAG AS INSERTING
       JMP  GTLLP             AND LOOP
*
* INSERT CHARACTERS
*
INS    DEC  R6                ;ROOM ?
       JLT  GTLB              ;N
       JEQ  GTLB              ;N
       MOV  R7,R1             ;Y, SAVE CURRENT POSITION
*
       MOVB *R7,*R7+          ;FOUND THE END ?
       JNE  $-2               ;N, KEEP LOOKING
*
INS1   MOVB @-1(7),*R7        ;COPY IN PREVIOUS CHARACTER
       DEC  R7                ;BACKUP
       C    R7,R1             ;ARE WE BACK WHERE WE STARTED?
       JH   INS1              ;N, CONTINUE MOVEING BUFFER
       MOVB R0,*R7+           ;Y, INSERT THE CHARACTER
       DATA TYPE$             ;OUTPUT THE UPDATED BUFFER
       INC  R1                ;BUMP POINTER
INS2   MOVB *R1+,R0           ;ANYMORE BACKSPACES?
       JEQ  GTLLP             ;N, DO MAIN LOOP
       DATA TYP11$,>0800      ;Y, OUT A BS
       JMP  INS2              ;LOOP TILL CURSOR POSITIONED
       END
