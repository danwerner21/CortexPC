       TITL 'RENUMBER COMMAND - CORTEX BASIC REV. 1.1'
       IDT  'RENUM'
*
*
*
       DEF  RENUMS,NINE
*
       REF  MOVEB,MOVEL,TYPB$,CRLF,EDIT1,LSTL
       REF  MOVEBN,MOVELN
       REF  NVD,NVS,BUS,VNT,SLT,DCNT,IOB
       REF  ESCFLG,RENCMP,RENLNE,RENRET
       REF  RNGOTO,RNGSUB,RNRSTR,RNON,CKEX
       REF  RNINP,RNIF,RNELSE,RNERR,RNREM
*
       REF  RENSTA,RENINC         START & INCREMENT HOLDERS
       REF  MODEOK
*
       DXOP OUTINT,13
       DXOP EVFIX,11
*
ERROR  EQU  >2F80
*
* RENUMBER COMMAND
*   FORM :                       NEW LINES :-
*
*     RENUM                     FROM 10 IN STEPS OF 10
*     RENUM <ARG1>              FROM ARG1 IN STEPS OF 10
*     RENUM STEP <ARG1>         FROM 10 IN ARG1 STEPS
*     RENUM <ARG1> STEP <ARG2>  FROM ARG1 IN ARG2 STEPS
*
*
* REPLACES ALL REFERENCES TO STATEMENTS IN
*           GOTO
*           GOSUB
*           RESTOR
*           ON
*           INPUT
*           ERROR
* WITH THE NEW STATEMENT NUMBER
*
* METHOD:
*  1. GO THROUGH PROGRAM AND BUILD UP A TABLE OF ALL
*     STATEMENTS CONTAINING REFERENCES TO OTHER STATEMENTS.
*     TABLE CONTAINS STATEMENT NUMBER REFERENCED AND ADDRESS
*     IN PSEUDO CODE OF THIS REFERENCE.
*  2. GO THROUGH PROGRAM AGAIN AND CHANGE OLD STATEMENT
*     NUMBERS TO NEW. CHECK EACH OLD STATEMENT NUMBER
*     AGAINST ALL NUMBERS IN THE TABLE BUILT UP IN STEP 1.
*     IF A MATCH IS FOUND, CHANGE THE STATEMENT REFERENCE
*     AT THE ADDRESS GIVEN IN THE TABLE FROM OLD TO NEW
*     STATEMENT NUMBER. THERE MAY BE MORE THAN ONE
*     ENTRY IN THE TABLE FOR EACH OLD STATEMENT NUMBER.
*
* DESIGN CONSIDERATIONS:
*    THIS PROGRAM IS NOT OPTIMISED FOR SPEED, AS IT IS NOT
*    AN EXECUTION TIME ROUTINE. IT IS DESIGNED TO USE THE
*    MINIMUM DATA STORAGE CONSISTENT WITH GOOD PROGRAM
*    STRUCTURE.
*    DATA AREA FOR THE TABLE IS TAKEN FROM THE TOP OF THE
*    VARIABLE DEFINITION TABLE (@NVD+2) TO (MAXIMALLY)
*    THE BOTTOM OF THE VARIABLE STORAGE AREA (@NVS-2).
*
* ERRORS:
*    IF THE PROGRAM RUNS OUT OF MEMORY WHILE TRYING TO
*    BUILD THE TABLE (STEP 1), THE POWER BASIC SYSTEM
*    RETURNS AN ERROR 10 - STORAGE OVERFLOW
*
* NOTE:
*    BEFORE RUNNING THE RENUM ROUTINE IT IS FIRST NECESSARY
*    TO MODIFY THE WAY VALUES 1 TO 9 ARE STORED INTERNALLY
*    (>64 - >6C) - THEY NEED TO BE STORED AS >6D >0001 TO
*    >6D >0009.  WHEN RENUMBERING IS COMPLETE THE REVERSE
*    OPERATION MUST BE PERFORMED.
*    IT IS NOT NECESSARY TO DO THIS FOR VALUES -1 AND 0
*    (>62 AND >63) AS THEY ARE NOT VALID LINE NUMBERS.
*    THIS REQUIRES EACH LINE IN THE PROGRAM TO BE 'LISTED
*    TO THE IOB' AND THE EDITOR INVOKED ON THE IOB (WITH
*    THE APPROPRIATE COMPRESSION/UNCOMPRESSION FLAG SET).
*
* CALLING SEQUENCE:
*
*      B    @RENUM
*
* DATA:
*
*      R0-R2     GENERAL
*      R3        CURRENT OPCODE
*      R4        TABLE POINTER
*      R5        SLT POINTER
*      R6        END OF AVAILABLE STORAGE
*      R7        NEW STATEMENT NUMBER
*      R8        PSEUDO CODE BYTE POINTER (PBC)
*
* STORE REF'D OPCODES (DEF'D IN EDIT)
*
GOTO   EQU  $+1
       DATA RNGOTO
GOSUB  EQU  $+1
       DATA RNGSUB
RSTR   EQU  $+1
       DATA RNRSTR
ON     EQU  $+1
       DATA RNON
INP    EQU  $+1
       DATA RNINP
ERR    EQU  $+1
       DATA RNERR
REM    EQU  $+1
       DATA RNREM
IF     EQU  $+1
       DATA RNIF
ELSE   EQU  $+1
       DATA RNELSE
*
QUOTE  BYTE >45
DQUOTE BYTE >44
EXCLAM BYTE >47
INT2B  BYTE >6D
HINT2B BYTE >6E
FP6B   BYTE >6F
CONCAT BYTE >3C
HASH   BYTE >3E
COMMA  BYTE >3F
QUEST  BYTE >41
SEMCOL BYTE >40
TO     BYTE >38
STEP   BYTE >3A
THEN   BYTE >3B
       EVEN
       PAGE
*
* BEFORE EXECUTING THE RENUMBERING ROUTINE, UNCOMPRESS
* THE STORED PROGRAM FOR VALUES 1 TO 9.
*
RENUMS EQU  $
       BL   @MODEOK           ABORT IF RUNNING !
       LI   R1,10             DEFAULT INCREMENT = 10
       MOV  R1,R2             DEFAULT NEW START = 10
       CB   *R8+,@STEP        ; 'STEP' ?
       JEQ  REN$1             ;Y, GET STEP
       DEC  R8                ;N, BACKUP
       BL   @CKEX             ;EXPRESSION ?
       JMP  REN$2             ;N, TAKE DEFAULTS
       EVFIX R2               ;Y, GET START LINE #
       CB   R0,@STEP          ;'STEP' ?
B3A    EQU  $-2
       JNE  REN$2             ;N, TAKE DEFAULT STEP
REN$1  EVFIX R1               ;Y, GET INCREMENT
REN$2  MOV  R1,@RENINC        ;SAVE STEP SIZE
       S    R1,R2             ;BACKUP START
       MOV  R2,@RENSTA        ;SAVE START LINE
*
       SETO @ESCFLG           NO ESCAPE DURING RENUMBER
       BL   @UNCOMP           UNCOMPRESS PROGRAM
       DATA 0
       MOV  @NVD,R4           INITIALISE TABLE POINTER
       MOV  @NVS,R6           SET R6 TO END OF AVAILABLE MEMORY
       AI   R6,-6             - 4 (FOR OVERFLOW TEST)
       MOV  @RENSTA,R2        INITIALISE STATEMENT #
       MOV  @VNT,R5           INITIALISE SLT PTR
*
* END OF LINE ENCOUNTERED - GET PBC OF NEXT LINE
*
ENDLIN DECT R5
       DECT R5                TO NEXT LINE
       C    @SLT,R5           END OF PROGRAM?
       JL   SETUP
*
* END OF PROGRAM - UPDATE ALL PBC ENTRIES IN THE TABLE THAT
* CORRESPOND TO ACTUAL PROGRAM LINE #S; RENUMBER THE SLT
* ENTRIES; CHECK FOR NON-EXISTENT LINE #S; CLEAR DOWN THE
* TABLE AREA AND RETURN TO THE INTERPRETER
*
       CLR  *R4               Y - PUT ZERO TO MARK END OF TABLE
       MOV  @RENSTA,R7        INITIALISE NEW STATEMENT #
       MOV  @VNT,R5           INITIALISE SLT POINTER
       DECT R5                POINT TO "FIRST LINE #"
RENST  EQU  $                 RENUMBER STATEMENT
       A    @RENINC,R7        INCREMENT STATEMENT #
       AI   R5,-4             POINT TO NEXT SLT ENTRY
       C    R5,@SLT           END OF PROGRAM?
       JL   ENDRNS
       MOV  @NVD,R4           N - INITIALISE TABLE POINTER
SCHTAB EQU  $                 SEARCH TABLE FOR STMT # MATCH
       MOV  *R4,*R4           END OF TABLE?
       JEQ  ENDSTB
       C    *R5,*R4+          N - MATCH?
       JNE  SCHINC
       DECT R4                Y - BACK UP TO STATEMENT #
       NEG  *R4+              MARK AS DONE
       MOV  *R4,R0            PUT NEW STMT NO IN STMT REF
       MOVB R7,*R0+           (MAY CROSS WORD BOUNDARY)
       SWPB R7
       MOVB R7,*R0
       SWPB R7
SCHINC INCT R4                POINT TO NEXT TABLE ENTRY
       JMP  SCHTAB            LOOP
*
* ALL TABLE ENTRIES CORRESPONDING TO THAT LINE # DONE.
* NOW UPDATE THE SLT ENTRY
*
ENDSTB EQU  $
       MOV  R7,*R5            PUT NEW STMT NO IN SLT
       JMP  RENST             LOOP
*
* ALL SLT ENTRIES DONE. NOW CHECK FOR NON-EXISTENT LINE #S
* CLEAR DOWN TABLE AREA IN THE PROCESS
*
ENDRNS EQU  $
       MOV  @NVD,R4           REF TABLE
CHECK  MOV  *R4,R0            GET STATEMENT # ENTRY
       JLT  CHECKA            STATEMENT # = -VE
       JEQ  CHCKND            STATEMENT # = 0 (FINISHED)
       MOV  @2(R4),R1         GET PBC ENTRY
       MOV  @SLT,R5           GET END OF SLT
CHECKB MOV  *R5+,R2           GET STATEMENT #
       MOV  *R5+,R8           GET DISPLACEMENT INTO BUS
       A    @BUS,R8           REF START OF STMT IN BUS
       C    R1,R8             PBC ENTRY IN THIS LINE?
       JL   CHECKB            N - TRY NEXT LINE
       BL   @MOVEBN           Y - OUTPUT MSG TO IOB
       DATA >0D0A             CR LF
       TEXT -'Bad line No.('
       OUTINT R0              OUT 'OFFENDING' STMT # TO IOB
       BL   @MOVELN           OUT MSG TO IOB
       TEXT -')in new line '
       OUTINT R2              OUT LINE # TO IOB
       BL   @MOVELN           NULL TERMINATE
       DATA 0
       DATA TYPB$             OUTPUT IOB
CHECKA CLR  *R4+              CLEAR STMT # ENTRY IN TABLE
       CLR  *R4+              CLEAR PBC ENTRY IN TABLE
       JMP  CHECK             BACK FOR NEXT ENTRY
CHCKND BL   @UNCOMP           COMPRESS PROGRAM
NINE   DATA 9
       B    @CRLF             RETURN TO INTERPRETER
*
* PROGRAM NOT EXHAUSTED - CONTINUE BUILDING TABLE ENTRIES
*
SETUP  A    @RENINC,R2        INCREMENT STATEMENT #
       MOV  *R5,R8            SET UP PSEUDO CODE POINTER
       A    @BUS,R8           ADD PSEUDO CODE BASE
GOTOTS EQU  $                 WHILE NOT END OF LINE
       MOVB *R8,*R8           END OF LINE (0)?
       JEQ  ENDLIN
       MOVB *R8,R3            N - SAVE OPCODE
*
* GOTO OR GOSUB
*
       CB   *R8,@GOTO         GOTO?
       JEQ  T2COMB            Y - CHECK VALIDITY OF CODE FOLLOWING
       CB   *R8,@GOSUB        GOSUB?
       JEQ  T2COMB            Y - CHECK VALIDITY OF CODE FOLLOWING
       JMP  TEST3             JUMP ROUND 'TRAP'
TS2LPE EQU  $                 SKIP "TO"
T2COMA INC  R8
*
* TEST 2 COMMON CODE - ALSO USED BY TESTS 1, 3 & 6
* (CHECKS FOR VALID DELIMITER - 0, ::, ! )
*
T2COM  CB   *R8,@INT2B        2 BYTE INTEGER?
       JNE  T2WARN            N - PRINT WARNING MSG
T2COMB INC  R8
       INCT R8                POINT TO DELIMITER
       MOVB *R8,*R8           0?
       JEQ  TS2OK
       CB   *R8,@CONCAT       ::?
       JEQ  TS2OK
       CB   *R8,@EXCLAM       !?
       JNE  T2WARN            N - PRINT WARNING MSG
TS2OK  DECT R8                POINT TO STMT REF
       BL   @STORE            STORE IN TABLE
       JMP  TSTEND
*
* RESTOR
*
TEST3  CB   *R8,@RSTR         RESTOR?
       JNE  TEST4
       INC  R8
       MOVB *R8,*R8           RESTOR W/O LINE NO?
       JEQ  TSTEND
       CB   *R8,@CONCAT       ::?
       JEQ  TSTEND
       CB   *R8,@EXCLAM       !?
       JEQ  TSTEND
       CB   *R8,@HASH         #?
       JEQ  TSTEND
       JMP  T2COM             MUST BE STMT EXPRESSION
*
* ON AND IF
*
TEST4  CB   *R8,@ON           ON?
       JEQ  TS4LP
       CB   *R8,@IF           IF?
       JNE  TEST5
TS4LP  INC  R8                SKIP EXPRESSION
FIX2   CB   *R8,@THEN         THEN?
       JEQ  TS4LPE
       CB   *R8,@CONCAT       ':' ?
       JEQ  TS4LPE            Y, TREAT AS A 'THEN'
*    SKIP NOS IN CASE THEY CONTAIN "THEN" P-CODE
       CB   *R8,@INT2B        2-BYTE INTEGER?
       JEQ  TS4INT
       CB   *R8,@HINT2B       2-BYTE HEX INTEGER?
       JEQ  TS4INT
       CB   *R8,@FP6B         6-BYTE FP NUMBER?
       JNE  FIX
       AI   R8,6              SKIP 6 BYTE FP NUMBER
       JMP  TS4LP
TS4INT INCT R8                SKIP 2 BYTE INTEGER
       JMP  TS4LP
FIX    CB   *R8,@QUOTE        '?
       JEQ  FIX1              Y
       CB   *R8,@DQUOTE       "?
       JNE  TS4LP             N - SKIP OVER CODE
*    STRING - SKIP OVER CHARACTERS UNTIL THE NULL
* NOTE: ASCII ';' SAME AS ENCODED 'THEN'
FIX1   MOVB *R8+,R0
       JNE  FIX1              NOT NULL - CONTINUE SKIP OP
       JMP  FIX2              NULL - CHECK NEXT CHAR
TS4LPE INC  R8                SKIP "THEN"
       CB   R3,@IF            WAS THIS AN IF?
       JEQ  GOTOTS            Y - TEST STMT
TES4LP INC  R8                N (ON) - SKIP GOTO
       BL   @STORE            STORE IN TABLE
       CB   *R8,@COMMA        ,?
       JNE  TSTEND            N - END OF STMT
       JMP  TES4LP            Y - INC PAST
*
* INPUT
*
TEST5  CB   *R8,@INP          INPUT?
       JEQ  SKPINC
*
* ERROR
*
       CB   *R8,@ERR          ERROR STATEMENT?
       JEQ  T2COMA
*
* REM
*
       CB   *R8,@RNREM        REM?
       JNE  TEST8
STRING MOVB *R8+,R0           SKIP TO END OF LINE (0 BYTE)
       JNE  STRING
       JMP  SKPINC
*
* ELSE
*
TEST8  CB   *R8,@ELSE         ELSE?
       JNE  TSTEND
SKPNDA INC  R8                SKIP PAST
SKPEND JMP  GOTOTS            TEST SUBORDINATE STMT
*
*
*
SKP2   INCT R8
SKPINC INC  R8
TSTEND EQU  $
*
SKIP   EQU  $                 SKIP TO END OF CURRENT STMT
       MOVB *R8,*R8           0 (EOL)?
       JEQ  SKPEND            Y - TERMINATE SKIP
       CB   *R8,@CONCAT       :: (EO STMT) ?
       JEQ  SKPNDA            N
*
* CODE FOR INPUT'S ? - THEN CHECK FOR VALID DELIMITER
*
       CB   *R8,@QUEST        ??
       JNE  NTQST             N
       CB   R3,@INP           INPUT STMT?
       JNE  NTQST             N
       INC  R8                ? FOUND - CHECK STMT REF
       CB   *R8,@INT2B        2 BYTE INTEGER?
T2WARN JNE  WARN              N - PRINT WARNING
       INC  R8
       INCT R8                POINT TO DELIMITER
       MOVB *R8,*R8           0?
       JEQ  TS2OK
       CB   *R8,@CONCAT       ::?
       JEQ  TS2OK
       CB   *R8,@EXCLAM       !?
       JEQ  TS2OK
       CB   *R8,@COMMA        ,?
       JEQ  TS2OK
       CB   *R8,@SEMCOL       :?
       JNE  WARN
       JMP  TS2OK
*
* TEST FOR STRINGS
*
NTQST  EQU  $
       CB   *R8,@QUOTE        '?
       JEQ  STRING            Y
       CB   *R8,@DQUOTE       "?
       JEQ  STRING            Y
       CB   *R8,@EXCLAM       !? (STORED SAME AS STRING)
       JEQ  STRING
*
* TEST FOR NUMBERS
*
       CB   *R8,@INT2B        2 BYTE INTEGER?
       JEQ  SKP2              Y
       CB   *R8,@HINT2B       2 BYTE HEX INTEGER?
       JEQ  SKP2              Y
       CB   *R8,@FP6B         6 BYTE FP NUMBER?
       JNE  SKPINC            N
       AI   R8,6              Y - SKIP NUMBER
       JMP  SKIP
       PAGE
*
* LINE NUMBERS AS EXPRESSIONS - OUTPUT WARNING MESSAGE
*
WARN   BL   @MOVEBN           PUT MESSAGE IN BUFFER
       DATA >0D0A             CR LF
       TEXT -'Problem with new line '
       OUTINT R2              OUT LINE # INTO IOB
       DATA TYPB$             OUTPUT BUFFER
       JMP  TSTEND            FIND END OF STATEMENT
       PAGE
*
* STORE STATEMENT NUMBER AND PBC ADDRESS IN TABLE
*
STORE  C    R6,R4             STORAGE OVERFLOW?
       JL   STOVFL
       MOVB *R8+,*R4+         N - STORE STMT NO
       MOVB *R8,*R4+          (CAN BE ODD BOUNDARY)
       DEC  R8
       MOV  R8,*R4+           STORE CURRENT PBC
       INCT R8                POINT TO NEXT BYTE IN P-CODE
       B    *R11              RETURN
*
* OVERFLOW - CLEAR TABLE AREA THEN OUTPUT OVERFLOW MESSAGE.
* (RETURNS TO INTERPRETER)
*
STOVFL MOV  @NVD,R4           INITIALISE TABLE POINTER
       INCT R6                SET R6 TO LAST WORD TO BE CLEARED
CLRLP  CLR  *R4+
       C    R6,R4             DONE?
       JHE  CLRLP
       DATA ERROR+10          STORAGE OVERFLOW (ERROR 10)
       PAGE
*
* THIS ROUTINE PERFORMS THE NECESSARY COMPRESSION/
* UNCOMPRESSION OF THE STORED PROGRAM
*
*      BL   @UNCOMP
*      DATA [FLAG]
*
*  FLAG = 0 THEN UNCOMPRESS
*       = 9 THEN COMPRESS
*       ANYTHING ELSE WILL CAUSE PROBLEMS!    (BIG ONES !!!)
*
*  RENLNE - LAST STATEMENT LINE NUMBER THAT WAS LISTED
*  RENCMP - COMPRESSION/UNCOMPRESSION FLAG (USED IN EDIT)
*
UNCOMP MOV  *R11+,@RENCMP     GET COMPRESSION FLAG
       MOV  R11,@RENRET       SAVE RETURN ADDRESS
       CLR  @RENLNE           START AT THE FIRST LINE #
       SETO @DCNT             CLEAR INDENTATION COUNTER
UNCMP1 MOV  @VNT,R8
       DECT R8                BACK UP TO SLT TERMINATOR
UNCMP2 C    R8,@SLT           REACHED END OF SLT?
       JLE  UNCMP3            Y - RETURN TO CALLER
       AI   R8,-4             N - GET NEXT LINE #
       C    @RENLNE,*R8       RENLNE => LINE #?
       JHE  UNCMP2            Y - GOTO NEXT LINE
       MOV  *R8,@RENLNE       N - SAVE CURRENT LINE #
*
* FOUND NEXT LINE TO BE COMPRESSED/UNCOMPRESSED
*
       MOV  @IOB,R7            REF IOB
       MOV  *R8+,R1           GET LINE #; R8 REFS PBC ENTRY
       BL   @LSTL             LIST THIS LINE TO IOB
       BL   @EDIT1            EDIT LINE IN IOB
*
* SLT/VNT/ ETC MAY HAVE CHANGED - FIND THE NEXT LINE TO
* BE OPERATED ON (WHEN STATEMENT # ENTRY IN SLT > RENLNE)
*
       JMP  UNCMP1
UNCMP3 MOV  @RENRET,R11       RESTORE RETURN ADDRESS
       B    *R11              RETURN TO CALLING ROUTINE
       END
