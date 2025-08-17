       TITL 'FLOATING POINT ROUTINES - CORTEX BASIC REV. 1.1'
       IDT  'FP'
*
*       FAD             ;ADD TO FPAC
*       FSD             ;SUBTRACT FROM FPAC
*       FMD             ;MULTIPLY FPAC
*       FDD             ;DIVIDE FPAC
*       FLDD            ;LOAD FPAC
*       FSRD            ;STORE FPAC
*       FNEG            ;NEGATE FPAC
*       FCLR            ;CLEAR FPAC
*       FNRM            ;NORMALIZE FPAC
*       FSCL            ;SCALE FPAC
*       FLOAT           ;FLOAT FPAC
*       FADDI           ;3 WRD ADDITION
*       FSUBI           ;3 WRD SUBTRACTION
*
*       THE FLOATING POINT ACCUMULATOR IS THE FIRST 3
*       WORDS OF THE FLOATING POINT REGISTERS  (R0,R1,R2).
*
*       ALL FLOATING POINT OPERATIONS ASSUME NORMALIZED
*       NUMBERS AS INPUTS AND ALL RESULTS ARE NORMALIZED.
*
*       THE FORM OF A FLOATING POINT NUMBER IS AS FOLLOWS:
*
*       1ST WORD        SCCC CCCC MMMM MMMM
*       2ND WORD        MMMM MMMM MMMM MMMM
*       3RD WORD        MMMM MMMM MMMM MMMM
*
*       WHERE  S = SIGN BIT
*              C = 7 BIT, EXCESS >40 CHARACTERISTIC
*              M = 40 BIT UNSIGNED MAGNITUDE MANTISSA
*                  0 <= M < 1
*
*       A NUMBER IS NORMALIZED WHEN THE 1ST HEX DIGIT
*       OF THE MANTISSA IS NON-ZERO.
*
*       A TRUE ZERO (ALL ZERO'S) IS USED FOR ZERO.
*
ERROR  EQU  >2F80             ;XOP XX,14  (ERROR CALL)
ERROR2 EQU  ERROR+>20
*
       DEF  FSRD
       DEF  FLDD
       DEF  FSD               ;SUBTRACT FROM FPAC
       DEF  FAD
       DEF  FNEG
       DEF  FLOAT
       DEF  FNRM
*
       REF  C7F,CF0,CFF80
       PAGE
*
*       LOAD INTO FPAC A 3 WRD FLOATING POINT #.
*
* CALLING SEQUENCE:
*
*       XOP XX,1
*
FSRD   MOV  R0,*R11+
       MOV  R1,*R11+
       MOV  R2,*R11
       RTWP
       PAGE
*
*       LOAD FLOATING POINT ACCUMULATOR WITH 3RD FLOATING
*       POINT NUMBER.
*
* CALL SEQUENCE:
*
*       XOP XX,0
*
FLDD   MOV  *R11+,R0          ;LOAD FPAC
FLDD1  MOV  *R11+,R1
       MOV  *R11,R2
FLDD2  RTWP
       PAGE
*
*       ADD OR SUBTRACT FROM FPAC A 3 WORD FLOATING
*       POINT NUMBER.
*
* CALL SEQUENCE:
*
*       XOP XX,2        ;ADD TO FPAC
*
*       XOP XX,3        ;SUBTRACT FROM FPAC
*
* EXCEPTIONS AND CONDITIONS:
*
*       ERROR 29 CAN RESULT FROM OVERFLOW
*
* EXTERNAL ROUTINE LIST:
*
*       FLDD            ;LOAD FPAC
*       FARS            ;SHIFT FPAC RIGHT 1 HEX DIGIT
*       FADDI           ;ADD R4,R5,R6 TO R0,R1,R2
*
*
*       SUBTRACTION IS MADE SIMPLY BY TOGGLING SIGN BIT
*
FSD    MOV  *R11+,R4          ;GET FIRST WORD, ZERO?
       JEQ  FLDD2             ;Y, ZERO, RETURN
       AI   R4,>8000          ;N, TOGGLE SIGN BIT
       JMP  FAD0
*
*
FAD    MOV  *R11+,R4          ;GET FIRST WORD, ZERO?
       JEQ  FLDD2             ;Y, RETURN
FAD0   MOV  R0,R0             ;N, FPAC ZERO?
       JNE  FAD0A             ;N
       MOV  R4,R0             ;Y, MOVE TEMP TO FPAC
       JMP  FLDD1
*
FAD0A  MOV  *R11+,R5          ;LOAD R4,R5,R6
       MOV  *R11,R6
       MOVB R0,R3             GET EXPONENTS
       MOVB R4,R7
       SB   R0,R0             ISOLATE
       SB   R4,R4
       SLA  R3,1              REMOVE SIGN
       JNC  FAD1              NEGATIVE?
       BL   @FADN             Y, NEGATE
*
FAD1   SRL  R3,9
       SLA  R7,1              REMOVE SIGN
       JNC  FAD2              NEGATIVE?
       NEG  R6                Y, NEGATE
       JNE  FAD1A
       NEG  R5
       JNE  FAD1B
       NEG  R4
       JMP  FAD2
FAD1A  INV  R5
FAD1B  INV  R4
*
FAD2   LI   R10,10            10 SHIFTS GIVE ZERO RESULT
       SRL  R7,9              POSITION EXPONENT
*
FAD2A  C    R3,R7             COMPARE EXPONENTS
       JEQ  FAD4              SAME, DO ADD
       JGT  FAD3              SHIFT TEMP (IS R3>R7?)
       BL   @FARS             N, R3<R7, SHIFT FPAC
       DEC  R10               COUNT SHIFTS
       JNE  FAD2A             OK
       MOV  R4,R0             FPAC=0, MOVE TEMP TO FPAC
       MOV  R5,R1
       MOV  R6,R2
       MOV  R7,R3             MOVE EXPONENT
       JMP  FAD3A
*
FAD3   SRL  R6,4              SHIFT TEMP 1 HEX DIGIT RIGHT
       MOV  R5,R9
       SLA  R9,12
       A    R9,R6             MOVE HEX DIGIT ACROSS
       SRL  R5,4
       MOV  R4,R9
       SLA  R9,12
       A    R9,R5             MOVE HEX DIGIT ACROSS
       SRA  R4,4
       INC  R7                DONE, INCREMENT EXPONENT
       DEC  R10               COUNT SHIFTS
       JNE  FAD2A             OK
*
FAD3A  MOV  R0,R0             TEMP=0, NEED NEGATION?
       JGT  FNRM3             N, FPAC OK
       BL   @FADN             Y, NEGATE FPAC
       AI   R3,>80            ADD SIGN TO EXPONENT
       JMP  FNRM3
*
FAD4   BL   @FADDI            ADD
       JLT  FAD5              NEGATIVE
       JGT  FAD6              POSITIVE
       MOV  R1,R1
       JNE  FAD6              NON-ZERO
       MOV  R2,R2
       JNE  FAD6              NON-ZERO
       JMP  FNRM4             ZERO, RTWP
*
FAD5   BL   @FADN             NEGATE FPAC
       ORI  R3,>80            ADD SIGN BIT
*
FAD6   MOVB R0,R0             CHECK ADDITION OVERFLOW
       JEQ  FNRM2             NUMBER OK
       BL   @FARS             SHIFT NUMBER
       JMP  FNRM2             NORMALIZE
*
FADN   NEG  R2                NEGATE FPAC
       JNE  FADN1
       NEG  R1
       JNE  FADN2
       NEG  R0
       RT
*
FADN1  INV  R1
FADN2  INV  R0
       B    *R11              RETURN
       PAGE
*
*       GET ABSOLUTE VALUE OF FPAC.
*       NEGATE FPAC.
*
* CALL SEQUENCE:
*
*       XOP XX,9        ;NEGATE FPAC
*
*       DEF FABS
*
*FABS   ANDI R0,>7FFF   CLEAR SIGN BIT
*       RTWP            RETURN
FNEG   MOV  R0,R0             ZERO?
       JEQ  FNEG1             Y, LEAVE TRUE ZERO
       AI   R0,>8000          N, COMPLEMENT SIGN BIT
FNEG1  RTWP RETURN
       PAGE
*
*       IF FPAC IS NOT NORMALIZED, AN ATTEMPT WILL BE
*       MADE TO NORMALIZE.  IN GENERAL, FLOAT WORKS ON
*       R0,R1,R2 AND HENCE IT CAN BE USED TO FLOAT
*       OTHER SETS OF NUMBERS THAN FPAC.
*
* CALLING SEQUENCE:
*
*       XOP XX,10       WHERE XX IS NOT USED
*
* EXCEPTIONS AND CONDITIONS:
*
*       IF R0 IS NON-ZERO, THE ROUTINE WILL ABORT.
*
* EXTERNAL ROUTING LIST:
*
*       FNRM            ;NORMALIZE
*       FCLR            ;CLEAR FPAC
*
FLOAT  MOV  R0,R0             ALREADY FLOATING?
       JNE  FNRM4             Y, RTWP
       LI   R3,>44            N, GET EXPONENT
       CLR  R2                CLEAR LOW
       MOV  R1,R1             CHECK SIGN
       JGT  FLOAT1            POSITIVE
       JEQ  FCLRP             ZERO
       NEG  R1                NEGATIVE
       LI   R3,>C4            NEGATE EXPONENT
*
FLOAT1 MOVB R1,R0             SHIFT 1 BYTE LEFT
       JEQ  FLOAT2            SHIFT 2 BYTES
       SWPB R0
       SLA  R1,8
       JMP  FNRM2
*
FLOAT2 MOV  R1,R0             SHIFT 2 BYTES
       CLR  R1
       DECT R3                ADJUST EXPONENT
       JMP  FNRM2
       PAGE
*
*       FPAC IS NORMALIZED SUCH THAT THERE EXISTS
*       A NON-ZERO HEX DIGIT IN BITS 8-11 OF THE 1ST
*       WORD OF FPAC.
*
* CALL SEQUENCE:
*
*       XOP XX,7        WHERE XX IS NOT USED
*
* EXTERNAL ROUTINE LIST:
*
*       FCLR            ;CLEAR FPAC
FNRM   CLR  R3                CLEAR R3
       MOVB R0,R3             GET EXPONENT & SIGN
       S    R3,R0             REMOVE FROM NUMBER
       JNE  FNRM1             LOOK FOR ZERO
       MOV  R1,R1
       JNE  FNRM1
       MOV  R2,R2
       JEQ  FNRM4             ZERO, RETURN
*
FNRM1  SWPB R3                READY FOR DECREMENTING
*
FNRM2  CZC  @CF0,R0           NORMALIZED?
       JNE  FNRM3             Y
       CZC  @C7F,R3           N, EXPONENT=ZERO?
       JEQ  FCLRP             Y, CANNOT NORMALIZE
       DEC  R3                N, OK TO DECREMENT
       SLA  R0,4              SHIFT R0,R1,R2 4 BITS LEFT
       MOV  R1,R9
       SRL  R9,12
       A    R9,R0             MOVE 1ST HEX DIGIT ACROSS
       SLA  R1,4
       MOV  R2,R9
       SRL  R9,12
       A    R9,R1             MOVE 2ND HEX DIGIT ACROSS
       SLA  R2,4
       JMP  FNRM2
*
FNRM3  SWPB R3                READY EXPONENT
       MOVB R3,R0             MOVE INTO FPAC
FNRM4  RTWP                   RETURN
       PAGE
*
*       MULTIPLY FPAC BY FLOATING POINT NUMBER
*       POINTED TO BY R11.
*
* CALL SEQUENCE:
*
*       XOP XX,4
*
* EXCEPTIONS AND CONDITIONS:
*
*       ERROR 29 CAN RESULT FROM OVERFLOW
*
* EXTERNAL ROUTINE LIST:
*
*       FCLR            ;CLEAR FPAC
*       FPFX            ;FIX EXPONENTS
*       FAD6            ;ADD AND NORMALIZE
*
* ENTRY POINT:
*
       DEF  FMD
*
FMD    MOV  R0,R0             ;FPAC=0?
       JEQ  FCLR              ;Y
       MOV  *R11+,R4          ;FPAC X 0?
FCLRP  JEQ  FCLR              ;Y
       MOV  *R11+,R5          ;N, LOAD R4,R5,R6
       MOV  *R11,R6
       BL   @FPFX             FIX EXPONENTS
       A    R7,R3
       DATA >FFC0
*
*DO MULTIPLICATION
*
*                       R4,R5,R6
*                       R0,R1,R2
*                       ========
*                          XX XX        A       R2 X R6
*                       XX XX           B       R2 X R5
*                    XX XX              C       R2 X R4
*                       XX XX           D       R1 X R6
*                    XX XX              E       R1 X R5
*                 XX XX                 F       R1 X R4
*                    XX XX              G       R0 X R6
*                 XX XX                 H       R0 X R5
*              XX XX                    I       R0 X R4
*              -----------------
*              R7,R8,R9,R10
*
       MOV  R6,R10
       MPY  R2,R10            R10,R11 (A)
       MOV  R6,R8
       MPY  R1,R8             R8,R9 (D)
       MPY  R0,R6             R6,R7 (G)
*
       A    R9,R10            SUM PARTIAL PRODUCTS (A,D,G)
       JNC  $+4
       INC  R8
       A    R7,R8
       JNC  $+4
       INC  R6
       MOV  R8,R9
       MOV  R6,R8             R8,R9,R10 = A+D+G
*
       MOV  R5,R6
       MPY  R2,R6             R6,R7 (B)
       A    R7,R10            SUM
       JNC  $+4
       INC  R6
       A    R6,R9
       JNC  $+4
       INC  R8                (NO CARRY OUT)
       MOV  R9,R10
       MOV  R8,R9
       MOV  R5,R7
       MPY  R1,R7             R7,R8 (E)
       MPY  R0,R5             R5,R6 (H)
*
       A    R8,R10            FINISH PARTIAL SUMS B,E,H
       CLR  R8
       JNC  $+4
       INC  R7
       A    R7,R9
       JNC  $+4
       INC  R8
       A    R6,R9
       JNC  $+4
       INC  R8
       CLR  R7
       A    R5,R8
       JNC  $+4
       INC  R7                R7,R8,R9,R10 = A+B+D+E+G+H
       MOV  R4,R5
       MPY  R2,R5             R5,R6 (C)
       A    R6,R10            SUM
       JNC  FMD4
       INC  R9
       JNC  FMD4
       INC  R8
       JNC  FMD4
       INC  R7
FMD4   A    R5,R9
       JNC  FMD5
       INC  R8
       JNC  FMD5
       INC  R7
*
FMD5   MOV  R4,R5
       MPY  R1,R5             R5,R6 (F)
       A    R6,R9             SUM
       JNC  FMD6
       INC  R8
       JNC  FMD6
       INC  R7
FMD6   A    R5,R8
       JNC  $+4
       INC  R7
       MPY  R0,R4             R4,R5 (I)
       A    R5,R8
       JNC  $+4
       INC  R7
       A    R4,R7             PRODUCT COMPLETE
       MOVB R8,R7             SHIFT 1 BYTE LEFT
       SWPB R7
       MOVB R9,R8
       SWPB R8
       MOVB R10,R9
       SWPB R9
       SWPB R10
       PAGE
*MULTIPLY AND DIVIDE ENTRY
*
*     R10 HAS GUARD BITS FOR NORMALIZING & ROUNDING
*
FMD7   MOV  R7,R0             MOVE INTO FPAC
       MOV  R8,R1
       MOV  R9,R2
*
       CZC  @CF0,R0           NORMALIZED?
       JNE  FMD9              Y, ROUND
       CZC  @C7F,R3           N, EXP=0?
       JEQ  FCLR              Y, CANNOT NORMALIZE
       DEC  R3                N, OK TO DECREMENT
       SLA  R0,4              SHIFT R0,R1,R2,R10 LEFT 4 BITS
       MOV  R1,R9
       SRL  R9,12
       A    R9,R0             MOVE 1ST HEX DIGIT ACROSS
       SLA  R1,4
       MOV  R2,R9
       SRL  R9,12
       A    R9,R1             MOVE 2ND HEX DIGIT ACROSS
       SLA  R2,4
       MOV  R10,R9
       SRL  R9,12
       A    R9,R2             MOVE 3RD HEX DIGIT ACROSS
       SLA  R10,4
*
*DONE, ROUND USING R10
*
FMD9   SLA  R10,1             GET GUARD BIT
       JNC  FNRM3             IF ZERO, DONE
       INC  R2                ELSE ROUND
       JNC  FNRM3
       INC  R1
       JNC  FNRM3
       INC  R0                IF THIS FAR, POSSIBILITY
       B    @FAD6             OF NORMALIZATION
       PAGE
*
*       SET FPAC TO TRUE ZERO.
*
* CALLING SEQUENCE:
*
*       XOP XX,8        WHERE XX IS NOT USED
*
* ENTRY POINT:
*
       DEF  FCLR
*
FCLR   CLR  R0                CLEAR FPAC
       CLR  R1
       CLR  R2
       RTWP
       PAGE
*
*       DIVIDE FPAC BY THE FLOATING POINT NUMBER POINTED
*       TO BY R11.
*
*       FPFX REMOVES EXPONENTS FROM R0,R1,R2 AND
*       R4,R5,R6, UNBIASES THEM, PERFORMS MULTIPLICATION
*       OR DIVISION (ADD OR SUBTRACT) AND THEN
*       RE-BIASES THEM.
*
* CALL SEQUENCE:
*
*       XOP XX,5
*       BL @FPFX
*
* EXCEPTIONS AND CONDITIONS:
*
*       ERROR 28 WILL RESULT FROM DIVISION BY ZERO
*       ERROR 29 WILL RESULT FROM OVERFLOW
*
* EXTERNAL ROUTINE LIST:
*
*       FCLR            ;CLEAR
*       FMD7            ;FINISH BY NORMALIZING
       PAGE
*FIX EXPONENTS
*
*       BL @FPFX
*         BIAS INSTRUCTION
*         REBIAS #
*
FPFX   MOVB R0,R3             GET EXPONENTS
       MOVB R4,R7
       SB   R0,R0             ;REMOVE FROM NUMBERS
       SB   R4,R4
       MOV  R7,R8             GET SIGN
       XOR  R3,R8             +-,-+ = NEG  ++,-- = POS
       SWPB R3                REMOV SIGN BITS
       ANDI R3,>7F
       SWPB R7
       ANDI R7,>7F
       X    *R11+             DO ADD FOR MUL, SUB FOR DIV
       A    *R11+,R3          BIAS
       CZC  @CFF80,R3         CHECK FOR OVERFLOW
       JNE  ERR29             PROBLEM
       SLA  R8,1              ADD SIGN
       JNC  $+6
       AI   R3,>80            NEGATIVE
       RT                     RETURN
*
*FLOATING POINT DIVIDE
*
* ENTRY POINT:
*
       DEF  FDD
*
FDD    MOV  R0,R0             ;FPAC=ZERO?
       JEQ  FCLR              ;Y
       MOV  *R11+,R4          ;FPAC / 0?
       JEQ  FDDER             ;Y, DIVISION BY ZERO
       MOV  *R11+,R5          ;N, LOAD R4,R5,R6
       MOV  *R11,R6
       BL   @FPFX             FIX EXPONENTS
       S    R7,R3             SUBTRACT EXPONENTS
       DATA >40               ADD >40 TO BIAS
       C    R0,R4             CHECK FOR PROPER FRACTION
       JLT  FDD2              OK
       INC  R3                IMPROPER, INCREMENT EXPONENT
       SLA  R4,4              SHIFT R4,R5,R6 LEFT 4 BITS
       MOV  R5,R9
       SRL  R9,12
       A    R9,R4             MOVE 1ST HEX DIGIT ACROSS
       SLA  R5,4
       MOV  R6,R9
       SRL  R9,12
       A    R9,R5             MOVE 2ND HEX DIGIT ACROSS
       SLA  R6,4
*
FDD2   CLR  R7                Y, CLEAR QUOTIENT
       CLR  R8
       CLR  R9
       LI   R10,40            DO 40 TIMES
*
FDD3   SLA  R0,1              SHIFT LEFT R0,R1,R2,R7,R8,R9
       SLA  R1,1
       JNC  $+4
       INC  R0
       SLA  R2,1
       JNC  $+4
       INC  R1
       SLA  R7,1
       SLA  R8,1
       JNC  $+4
       INC  R7
       SLA  R9,1
       JNC  $+4
       INC  R8
       C    R0,R4             IS R0,R1,R2<=R4,R5,R6?
       JL   FDD5              Y
       JH   FDD4              N
       C    R1,R5             MAYBE
       JL   FDD5              Y
       JH   FDD4              N
       C    R2,R6             MAYBE
       JL   FDD5              Y
FDD4   BL   @FSUBI            N, R0,R1,R2=R0,R1,R2-R4,R5,R6
       INC  R9                ENTER BIT
*
FDD5   DEC  R10               DONE?
       JNE  FDD3              N, LOOP AGAIN
       SLA  R0,1              Y, CHECK FOR ROUNDING
       C    R0,R4             2*REMAINDER < DIVISOR?
       JL   FMD7              Y, NO NEED TO ROUND
       LI   R10,>8000         ROUND
       JMP  FMD7              DONE
*
FDDER  DATA ERROR+28          ;DIVISION BY ZERO
*
ERR29  DATA ERROR+29          ;FP ERROR
       PAGE
*
*       SHIFT R0,R1,R2 RIGHT 1 HEX DIGIT WHILE
*       UPDATEING EXPONENT IN R3
*
* CALLING SEQUENCE:
*
*       BL @FARS
*
* EXCEPTIONS AND CONDITIONS:
*
*       ERROR 29 WILL RESULT ON OVERFLOW
*
* EXTERNAL ROUTINE LIST:
*
*       ERR29           ;ERROR 29
*
FARS   SRL  R2,4              SHIFT R0,R1,R2 4 BITS RIGHT
       MOV  R1,R9
       SLA  R9,12
       A    R9,R2             MOVE 1ST HEX DIGIT ACROSS
       SRL  R1,4
       MOV  R0,R9
       SLA  R9,12
       A    R9,R1             MOVE 2ND HEX DIGIT ACROSS
       SRA  R0,4
       MOV  R3,R9
       INC  R3                INCREMENT EXPONENT
       XOR  R3,R9             WATCH FOR SIGN CHANGE
       ANDI R9,>80            SIGN CHANGE?
       JNE  ERR29             Y, OVERFLOW
       RT
       PAGE
*
*       ADD OR SUBTRACT R4,R5,R6 FROM R0,R1,R2.
*
* CALL SEQUENCE:
*
*       BL @FADDI
*       BL @FSUBI
*
*   ENTRY POINT :
*
       DEF  FADDI
*
FADDI  A    R6,R2             R0,R1,R2=R0,R1,R2+R4,R5,R6
       JNC  $+8
       INC  R1
       JNC  $+4
       INC  R0
       A    R5,R1
       JNC  $+4
       INC  R0
       A    R4,R0             ADDITION COMPLETE
       RT
*
* ENTRY POINT:
*
       DEF  FSUBI
*
FSUBI  S    R6,R2             R0,R1,R2=R0,R1,R2-R4,R5,R6
       JOC  $+8
       DEC  R1
       JOC  $+4
       DEC  R0
       S    R5,R1
       JOC  $+4
       DEC  R0
       S    R4,R0             SUBTRACTION COMPLETE
       RT
       PAGE
*
*       FPAC WILL BE SHIFTED LEFT OR RIGHT IN
*       ORDER TO MAKE THE EXPONENT AGREE WITH THE
*       SCALE FACTOR POINTED TO BY R11.  THIS
*       OPERATION IS ESSENTIALLY THE OPPOSITE
*       OF NORMALIZATION.
*       SCALING TO >4600 PLACES THE DECIMAL POINT
*       BETWEEN THE SECOND AND THIRD WORD OF THE FLOATING
*       POINT NUMBER.  SCALING TO >4A00 PLACES THE DECIMAL
*       POINT AFTER THE THIRD WORD.
*       HENCE, TO INTEGERIZE A FLOATING POINT NUMBER IN FPAC,
*       ONE SCALES TO >4A00 AND THEN NORMALIZES.
*
* CALLING SEQUENCE:
*
*       XOP XX,6
*
* EXCEPTIONS AND CONDITIONS:
*
*       ERROR 29 WILL RESULT FROM A LEFT SHIFT
*
* EXTERNAL ROUTINE LIST:
*
*       ERR29           ;ERROR 29
*
* ENTRY POINT:
*
       DEF  FSCL
*
FSCL   MOV  *R11,R4           GET SCALE FACTOR
       CLR  R3
       MOV  R0,R8             SAVE SIGN
       MOVB R8,R7             GET EXPONENT
       ANDI R7,>7F00          GET EXPONENT
       S    R4,R7             GET DIFFERENCE
       JEQ  FSCL2             ALREADY SCALED
       MOVB R3,R0             ZERO EXPONENT
       SRA  R7,8              RIGHT JUSTIFY
       JGT  ERR29             SHIFT LEFT, FP ERROR?
*
FSCL1  BL   @FARS             SHIFT RIGHT
       INC  R7                DONE?
       JNE  FSCL1             N
       MOVB R8,R0             RESTORE SIGN BIT
       ANDI R0,>80FF          ;MASK EXPONENT
       AB   R4,R0
FSCL2  RTWP
*
       END
