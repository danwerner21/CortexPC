       TITL 'SQUARE ROOT FUNCTION - CORTEX BASIC REV. 1.1'
       IDT  'SQRF'
*
*
*
       REF  FUNFX             ;FIXES ARGUMENT
       DXOP LOADF,0           ;LOAD FPAC
       DXOP STORE,1           ;STORE FPAC
       DXOP FADD,2            ;ADD TO FPAC
       DXOP FMUL,4            ;MULTIPLY FPAC
       DXOP FDIV,5            ;DIVIDE FPAC
ERROR  EQU  >2F80             ;XOP XX,14 (ERROR CALL)
       REF  FPAC              ;FLOATING POINT ACCUMULATOR
       REF  TEMP              ;3 WRD TEMPORARY STORAGE
       REF  DS                ;3 WRD TEMPORARY STORAGE
       REF  EVSFR             ;EXIT ADDRESS
       REF  C8000
*
SQRI   EQU  4                 ;# OF NEWTON ITERATIONS
* ABSTRACT:
*
*      COMPUTE THE SQUARE ROOT OF N (R2) USING A
*      NEWTON ITERATION DEFINED AS FOLLOWS:
*
*           X(I+1) = [X(I) + N / X(I)] / 2
*
*      ITERATE FOR SQRI TIMES.  3 ITERATIONS =
*      6 DIGITS, 4 ITERATION = 11 DIGITS.
*
* CALLING SEQUENCE:
*
*      B @SQRF
*
*      IN  (R0) = 3 WRD FLOATING POINT #
*      OUT (R2) = 3 WRD FLOATING POINT ROOT OF #
*
*      NORMAL EXIT IS TO @EVSFR
*      ERROR EXIT IS THRU XOP XX.14
*
* EXCEPTIONS AND CONDITIONS:
*
*      USES R0-R4,R11
*
*      ERROR 25 = SQUARE ROOT OF NEGATIVE NUMBER
*
SQRF   DEF  SQRF              ;ENTRY POINT
       BL   @FUNFX            ;MOVE INTO FPAC
       JMP  SQRF3             ;ZERO, DONE
       MOV  R3,R3             ;NEGATIVE?
       JNE  ERR25             ;Y, ERROR
*******
* NEWTON'S ITERATION HAS QUADRADIC CONVERGENCE GIVEN
* A GOOD FIRST GUESS.  PUT FIRST GUESS IN FPAC.
*******
       STORE @DS              ;MOVE INTO DS
       LI   R0,>4000          ;GET 16^0 EXPONENT
       S    R0,R1             ;UNBIAS OLD EXPONENT
       MOVB R0,@FPAC          ;LOAD EXPONENT
       FMUL @SQRC1            ;FPAC=N*C1
       FADD @SQRC2
       SRL  R1,9              ;ODD EXPONENT?
       JNC  SQRF1             ;N
       FMUL @FP4              ;Y, MULTIPLY BY 4
*
SQRF1  EQU  $
       SWPB R1                ;POSITION EXPONENT
       AB   R1,@FPAC          ;ADJUST EXPONENT
       LI   R1,SQRI           ;DO SQRI ITERATIONS
*
*******
* ITERATION LOOP FOLLOWS.
* DO X(I+1) = [X(I) + N / X(I)] / 2
*******
SQRF2  EQU  $
       STORE @TEMP            ;MOVE FPAC TO TEMP
       LOADF @DS
       FDIV @TEMP             ;DIVIDE..N/X(I)
       FADD @TEMP             ;ADD TEMP..X(I)+N/X(I)
       FMUL @SQRC3            ;X 0.5
       DEC  R1                ;SQRI TIMES?
       JNE  SQRF2             ;N
*******
* GET ADDRESS OF RESULT AND RETURN TO EVAL
*******
SQRF3  EQU  $
       LI   R2,FPAC           ;Y, RETURN
       SZC  @C8000,*R2        SQR(0.0625) GIVES -VE VALUES
       B    @EVSFR
*
ERR25  DATA ERROR+25          ;SQUARE ROOT OF NEGATIVE NUMBER
*
SQRC1  DATA >40E4,>F92A,>F9A8 ;0.894 427
SQRC2  DATA >4039,>3E4E,>F028 ;0.223 607
SQRC3  DATA >4080,>0000,>0000 ;0.5
FP4    DATA >4140,>0000,>0000 ;4.0
       END
