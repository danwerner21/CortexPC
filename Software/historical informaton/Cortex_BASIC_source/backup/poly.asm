       TITL 'POLYNOMIAL ROUTINES - CORTEX BASIC REV. 1.1'
       IDT  'POLY'
*
* ROUTINE LIST:
*
*       FUNBK           ;BREAK FPAC INTO INTEGER & FRACTION
*       FUNFX           ;FIX FPAC INTO EXPONENT AND ABSOLUTE VALUE
*       PLYX            ;EVALUATE P(X) WITH X IN TEMP
*       PLYXX           ;EVALUATE P(X*X) WITH X IN FPAC
*
       DXOP LOADF,0           ;LOAD FPAC
       DXOP STORE,1           ;STORE FPAC
       DXOP FADD,2            ;ADD TO FPAC
       DXOP FSUB,3            ;SUBTRACT FROM FPAC
       DXOP FMUL,4            ;MULTIPLY FPAC
       DXOP FDIV,5            ;DIVIDE FPAC
       DXOP SCALE,6           ;SCALE FPAC
       DXOP NORMAL,7          ;NORMALIZE FPAC
       DXOP CLEAR,8           ;CLEAR FPAC
       DXOP NEGATE,9          ;NEGATE FPAC
       DXOP FLOATF,10         ;FLOAT FPAC
ERROR  EQU  >2F80             ;XOP XX,14  (ERROR CALL)
*
       REF  FPAC,FPAC4        ;FLOATING POINT ACCUMULATOR
       REF  TEMP              ;3 WRD TEMPORARY STORAGE
       REF  C4A00             ;>4A00
       DEF  FUNBK,FUNFX
       DEF  PLYX,PLYXX
       PAGE
* ABSTRACT:
*
*       BREAK FPAC INTO INTEGER IN R1 AND
*       LEAVE FRACTIONAL PART IN FPAC.
*
* CALLING SEQUENCE:
*
*       BL @FUNBK
*
*       IN FPAC = #
*       OUT  R1 = INTEGER PART
*          FPAC = FRACTIONAL PART
*
* EXCEPTIONS AND CONDITIONS:
*
*       ERROR 30 = FIX ERROR
*
FUNBK  MOV  @FPAC,R1          GET HIGH WORD
       A    R1,R1             REMOVE SIGN
       CI   R1,>8900          TOO BIG?
       JH   ERR30             Y, ERROR
       STORE @TEMP            ;N, MOVE TO TEMP
       SCALE @C4A00           ;SCALE
       MOV  @FPAC4,R1         LOAD LOW
       MOV  @FPAC,R2          <0?
       JGT  FUNBK1            N
       NEG  R1                Y, GET 2'S COMPLEMENT
*
FUNBK1 NORMAL 0               NORMALIZE
       NEGATE 0               NEGATE
       FADD @TEMP             ADD TEMP TO FPAC
       B    *R11              RETURN
*
ERR30  DATA ERROR+30
       PAGE
* ABSTRACT:
*
*       FIX FPAC INTO EXPONENT IN R1, SIGN BIT
*       IN R3, AND ABSOLUTE VALUE IN FPAC
*
* CALLING SEQUENCE:
*
*       BL @FUNFX
*
*       IN  R2 = #
*       OUT R1 = EXPONENT
*         (R2) = ABS(FPAC)
*           R3 = SIGN BIT
*
FUNFX  LOADF *R2              ;LOAD FPAC
       FLOATF 0               ;FLOAT IF NECESSARY
       LI   R2,FPAC           GET ADR
       MOV  *R2,R3            GET SIGN
       JEQ  FUNFX2            ZERO
       SRL  R3,15             GET SIGN BIT
       JEQ  FUNFX1            + OR 0
       NEGATE 0               -, NEGATE
*
FUNFX1 INCT R11               RETURN @2(11)
       CLR  R1
       MOVB *R2,R1            GET EXPONENT
FUNFX2 RT
       PAGE
* ABSTRACT:
*
*       EVALUATE POLYNOMIAL P(X) OR P(X*X) WITH
*       X IN TEMP OR FPAC RESPECTIVELY.  THE
*       POLYNOMIAL IS GIVEN BY:
*
*               (R0) = N,C1,C2,...,CN
*
*       WHERE   N = # OF COEFFICIENTS
*               C = COEFFICIENTS
*
* CALLING SEQUENCE:
*
*       BL @PLYXX
*       BL @PLYX
*
*       IN (R0) = N,C1,C2,...,CN
*       OUT FPAC = RESULT
*
* EXCEPTIONS AND CONDITIONS:  (NONE)
*
*EVALUATE P(X*X) WITH X IN FPAC
*
PLYXX  STORE @TEMP            MOVE X TO TEMP
       FMUL @TEMP             FPAC=FPAC*FPAC
       STORE @TEMP            MOVE X*X INTO TEMP
*
*EVALUATE P(X) WITH X IN TEMP
*
PLYX   MOV  *R11+,R0          GET CONSTANTS ADR
       MOV  *R0+,R1           GET COUNT
       LOADF *R0              LOAD 1ST CONSTANT
*
PLYXA  FMUL @TEMP             FPAC=FPAC*TEMP
       AI   R0,6              MOVE TO NEXT CONSTANT
       FADD *R0               ADD
       DEC  R1                DONE?
       JNE  PLYXA             N
       B    *R11              Y
       END
