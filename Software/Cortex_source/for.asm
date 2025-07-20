       TITL 'FOR/NEXT STATEMENTS - CORTEX BASIC REV. 1.1'
       IDT  'FOR'
*
*       FORY            ;FOR COMMAND
*       NXTY            ;NEXT COMMAND
*       FOR2            ;DELETE FROM FOR/NEXT STACK
*       GSIM            ;GET SIMPLE VERIABLE
*
ERROR  EQU  >2F80             ;XOP XX,14  (ERROR CALL)
ERROR2 EQU  ERROR+>20
*
       REF  ADDF              ;ADD
       REF  SUBF              ;SUBTRACT
       REF  EVERZ             ;EVALUATE EXPRESSION
       REF  NLIN,NLIN0        ;EXIT TO MULTIPLEXOR
       REF  BUS               ;BEGINNING USER STORAGE
       REF  EUS               ;END USER STORAGE
       REF  FNS               ;FOR/NEXT STACK
       REF  NVD               ;NEXT VARIABLE DEFINITION
       REF  NVS               ;NEXT VARIABLE STORAGE
       REF  PLC               ;PROGRAM LINE COUNTER
       REF  SLT               ;STATEMENT LINE TABLE
       REF  VDT               ;VARIABLE DEFINITION TABLE
       REF  VNT               ;VARIABLE NAME TABLE
       REF  B56
       REF  NXTXB             ;NEXT COMMAND BYTE
       DEF  FORY,FOR2
       DEF  NXTY
       DEF  GSIM
*
NRV    EQU  4                 ;NUMBER OF RESERVED WORDS
       PAGE
*       EXECUTE FOR COMMAND.  WHEN A FOR
*       STATEMENT IS EXECUTED, THE FOR/NEXT
*       STACK (FNS) IS SEARCHED FOR A ZERO
*       ENTRY.  DURING THE SEARCH, IF
*       AN IDENTICAL ENTRY IS FOUND, IT IS
*       DELETED AND THE STACK IS ROLLED DOWN.
*
*       A PRE-TEST IS MADE TO SEE IF THE
*       CONDITION IS MET IN WHICH CASE, THE
*       FOR COMMAND WILL SEARCH FOR A MATCHING
*       NEXT.
*
*       THE FOR/NEXT STACK FORMAT IS:
*
*       !    '    '    '    '    '    '    '    '    !
*         VAR -----STEP----- ------TO------ PBC  PLC
*
*       VAR = 0 INDICATES EMPTY SLOT
*
* CALLING SEQUENCE:
*
*       B @FORY
*
*       EXIT TO NLIN
* EXCEPTIONS AND CONDITIONS:
*
*       FOR W/O NEXT, STACK OVERFLOW
*       EVALUATION ERRORS
*       STORAGE OVERFLOW, ILLEGAL DELIMITER
*       EXPECTING SIMPLE VARIABLE
       PAGE
FORY   BL   @GSIM             ;GET SIMPLE VARIABLE
       MOV  *R4,R3            ;DEFINED?
       JNE  FOR0              ;Y
       MOV  @NVS,R3           ;N, DEFINE
       AI   R3,-6
       C    R3,@NVD           ;OK?
       JL   FORE10            ;N
       MOV  R3,*R4            ;Y, DEFINE
       MOV  R3,@NVS           ;UPDATE NVS
*
FOR0   MOV  @FNS,R4           ;GET F/N STACK ADR
       CB   *R8+,@B56         ;=?
       JNE  FORE36            ;N, PROBLEM
*
FOR1   MOV  *R4,R0            ;DONE?
       JEQ  FOR4              ;Y
       C    R1,R0             ;N, SAME VARIABLE?
       JEQ  FOR1A             ;Y
       AI   R4,18             ;N, MOVE TO NEXT
       C    R4,@EUS           ;ANYMORE?
       JL   FOR1              ;Y
       DATA ERROR+11          ;N, STACK OVERFLOW
*
FOR1A  BL   @FOR2             ;DELETE
       JMP  FOR1
*
FOR2   MOV  R4,R5             ;DELETE FROM STACK
       MOV  R4,R0
       AI   R0,18
*
FOR3   MOV  *R0+,*R5+         ;MOVE UP
       C    R0,@EUS           ;DONE?
       JLE  FOR3              ;N
       B    *R11              ;Y
       PAGE
FOR4   MOV  R1,*R4+           ;INSERT VAR NAME
       BLWP @EVERZ            ;GET INITIAL VALUE
       CI   R0,>3800          ;TO?
       JNE  FORE37            ;N, ERROR
       MOV  R3,R6             ;SAVE FOR PRE-TEST
       MOV  *R2+,*R3+         ;MOVE IN INITIAL VALUE
       MOV  *R2+,*R3+
       MOV  *R2,*R3
*
       CLR  *R4+              ;SET DEFAULT STEP TO 1
       CLR  *R4
       INC  *R4+
       CLR  *R4+
*
       BLWP @EVERZ            ;GET 'TO' VALUE
       MOV  R4,R7             ;SAVE FOR PRE-TEST
       MOV  *R2+,*R4+         ;MOVE IN TERMINATING VALUE
       MOV  *R2+,*R4+
       MOV  *R2,*R4+
       CLR  R5                ;SET DEFAULT SIGN
       CI   R0,>3A00          ;'STEP'?
       JNE  FOR6              ;N
       BLWP @EVERZ            ;Y, GET STEP
       MOV  R4,R1
       AI   R1,-12            ;MOVE BACK TO STEP
       MOV  *R2,R5            ;GET DIRECTION
       JNE  $+6               ;FP
       MOV  @2(2),R5          ;INTEGER
*
       MOV  *R2+,*R1+         ;MOVE INTO STEP
       MOV  *R2+,*R1+
       MOV  *R2,*R1
*
FOR6   MOV  R8,*R4            ;MOVE IN PBC,PLC
       DEC  *R4+              ;BACKUP OVER DLIM
       MOV  @PLC,*R4
*
       MOV  R6,R2             ;DO PRE-TEST
       MOV  R7,R1
       BL   @SUBF             ;R2=R2-R1  (VAR-TERM)
       MOV  *R2+,R1           ;LOOK AT RESULT, INTEGER?
       JNE  $+6               ;N
       MOV  *R2,R1            ;Y
       JEQ  FOR9              ;= IMPLIES NOT DONE
*
       XOR  R5,R1             ;'EXCLUSIVE OR' SIGNS
       JLT  FOR9              ;- INPLIES LOOP NOT COMPLETE
       PAGE
FOR7   MOV  @-16(4),R5        ;LOOK FOR NEXT
       CLR  @-16(4)           ;CLEAR FROM STACK
       MOV  @PLC,R6           ;GET PLC
*
FOR8   AI   R6,-4
       C    R6,@SLT           ;ANY MORE STATEMENTS?
       JL   ERR31             ;N
       MOV  *R6,R8            ;GET PBC
       A    @BUS,R8           ;GET ABSOLUTE ADDRESS
       CB   *R8+,@NXTXB       ;NEXT?
       JNE  FOR8              ;N
       BL   @GSIM             ;Y, GET SIMPLE VARIABLE
       C    R1,R5             ;SAME?
       JNE  FOR8              ;N, CONTINUE
       MOV  R6,@PLC           ;Y, UPDATE PLC
       B    @NLIN0
*
*FOR9    EQU DIM8
FOR9   B    @NLIN
*
FORE36 DATA ERROR2,36         ;MISSING "=
*
FORE37 DATA ERROR2,37         ;ILLEGAL DELIMITER
*
FORE10 DATA ERROR+10          ;STORAGE OVERFLOW
*
ERR31  DATA ERROR+31          ;FOR W/O NEXT
*
ERR20  DATA ERROR+20          ;EXPECTING SIMPLE VARIABLE
       PAGE
*       PROCESS THE FOOT OF A FOR/NEXT LOOP.
*
*       NEXT SEARCHS THE FOR/NEXT STACK (FNS)
*       FOR A MATCHING SIMPLE VARIABLE.  THE
*       STEP IS ADDED TO THE VARIABLE AND
*       A COMPLETION CHECK IS MADE.  IF LESS
*       THAN OR EQUAL, IT LOOPS BACK.  OTHERWISE,
*       THE STACK VARIBLE IS SET TO ZERO,
*       AND PROGRAM EXECUTION CONTINUES AFTER
*       THE NEXT STATEMENT.
*
* CALLING SEQUENCE:
*
*       B @NXTY
*
*       EXIT TO NLIN
*
* EXCEPTIONS AND CONDITIONS:
*
*       EVALUATION ERRORS
*       NEXT W/O FOR
*
       PAGE
NXTY   BL   @GSIM             ;GET SIMPLE VARIABLE
       MOV  *R4,R6            ;GET ADR
       JEQ  ERR32             ;NOT DEFINED
       MOV  @FNS,R4           ;GET F/N STACK ADR
*
NXT1   C    R1,*R4+           ;SAME?
       JEQ  NXT2              ;Y
       AI   R4,16             ;N, MOVE TO NEXT
       C    R4,@EUS           ;MORE?
       JL   NXT1              ;Y
*
ERR32  DATA ERROR+>20         ;NEXT W/O FOR
       DATA 32
*
*   (R4) = STEP 6 BYTE STORAGE AREA
*   (R6) = VARIABLE STORAGE 6 BYTE
*  6(R4) = 'TO' VALUE 6 BYTE STORAGE AREA
*
NXT2   MOV  R4,R2             SAVE PTR TO STEP
       MOV  *R2+,R5           STEP INTEGER ?
       JNE  NXT2C             N, DO FP
       MOV  *R2,R5            Y, GET SIGN
       MOV  R6,R11            SAVE PTR TO VARIABLE
       MOV  *R11+,R1          INTEGER VARIABLE?
       JNE  NXT2C             N, DO FP
       MOV  @6(R4),R1         Y, LIMIT INTEGER?
       JNE  NXT2C             N, DO FP
*
* EVERYTHING INTEGER
*
       MOV  *R11,R1           GET VARIABLE
       A    *R2,R1            R1= VAR + STEP
       JNO  SPLAT1            OK
       JMP  NXT2C             OVERFLOW, DO FP
SPLAT1 MOV  R1,*R11           UPDATE VARIABLE
       S    @6+2(R4),R1       R1= VAR - TO
       JMP  NXT2A             MAYBE
*
*     FP  FOR/NEXT LOOP
*
NXT2C  MOV  R6,R1             ;READY R2,R1
       MOV  R4,R2
       BL   @ADDF             ;ADD STEP
       MOV  R2,R1             ;MOVE IN NEW VARIABLE
       MOV  *R1+,*R6+
       MOV  *R1+,*R6+
       MOV  *R1,*R6
       MOV  R4,R1
       AI   R1,6              ;MOVE TO 'TO'
       BL   @SUBF             ;R2=R2-R1  (VAR-TO)
       MOV  *R2+,R1           ;LOOK AT RESULT, INTEGER?
       JNE  $+6               ;N
       MOV  *R2,R1            ;Y
NXT2A  JEQ  NXT3              ;0, NOT DONE
*
       XOR  R5,R1             ;EXCLUSIVE OR WITH STEP
       JLT  NXT3              ;NOT DONE
       CLR  @-2(4)            ;DONE, SET TOP OF STACK
       JMP  NXT3A
*
NXT3   MOV  @12(R4),R8        ;LOOP NOT COMPLETE
       MOV  @14(R4),@PLC      ;RESTORE PBC & PLC
NXT3A  B    @NLIN0
       PAGE
*       GSIM WILL GET NEXT ITEM AND CHECK TO
*       SEE IF IT IS A SIMPLE VARIABLE.
*
* CALLING SEQUENCE:
*
*       BL @GSIM
*
*       OUT (R4) = VARIABLE DEFINITION
*
* EXCEPTIONS AND CONDITIONS:
*
*       EXPECTING SIMPLE VARIABLE
*
GSIM   CLR  R0                ;GET VARIABLE
       MOVB *R8+,R0
       CI   R0,NRV*>100+>7000
       JL   ERR20             ;N, EXPECTING SIMPLE VARIABLE
       MOV  @VNT,R4           ;GET SYMBOL TABLE ADR
       SRL  R0,7              ;GET INDEX
       A    R0,R4
       MOV  @->70*2(4),R1
       JLT  ERR20             ;DIMENSIONED, ERROR
       MOV  @VDT,R4
       A    R0,R4
       AI   R4,->70*2
       RT
       END
