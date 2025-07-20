       TITL 'EXP FUNCTION - CORTEX BASIC REV. 1.1'
       IDT  'EXPF'
*
*       EXPF            ;EXPONENTIAL FUNCTION
*
       REF  FUNFX             ;FIXES ARGUMENT
       REF  PLYX,PLYXX        ;EVALUATE POLYNOMIALS
       REF  FUNBK             ;BREAK FP #
       REF  GETP2             ;GET POWER OF 2
       REF  FPAC              ;FLOATING POINT ACCUMULATOR
       REF  TEMP              ;3 WRD TEMPORARY STORAGE
       REF  DS1               ;3 WRD TEMPORARY STORAGE
       REF  DS                ;3 WRD TEMPORARY STORAGE
       DEF  EXPF
*
       DXOP LOADF,0           ;LOAD FPAC
       DXOP STORE,1           ;STORE FPAC
       DXOP FADD,2            ;ADD TO FPAC
       DXOP FSUB,3            ;SUBTRACT FROM FPAC
       DXOP FMUL,4            ;MULTIPLY FPAC
       DXOP FDIV,5            ;DIVIDE FPAC
ERROR  EQU  >2F80             ;XOP XX,14  (ERROR CALL)
ERROR2 EQU  ERROR+>20
       PAGE
*
*       CALCULATE THE EXPONENTIAL VALUE OF
*       E RAISED TO (ARG)
*
* CALLING SEQUENCE:
*
*       B @EXPF
*
*       IN  (R2) = ARG
*       OUT (R2) = E ^ ARG
*
* EXCEPTIONS AND CONDITIONS:
*
*       ERROR 33 = EXP ARGUMENT TOO LARGE
*       FLOATING POINT ERRORS
       PAGE
ERR33  DATA ERROR2,33         ;EXP ARG TOO LARGE
*
EXPF   MOV  R11,R10
       BL   @FUNFX            ;FIX SIGN & FPAC
       JEQ  EXPF2             ;EXP(0)=1
       FMUL @EXPC0            ;F=F*LN 2
       BL   @FUNBK            ;BREAK TO INTEGER AND FRACTION
       CI   R1,>7D            ;ARGUMENT TOO LARGE?
       JGT  ERR33             ;Y, ERROR
       MOV  R1,R2             ;N, SAVE
       FSUB @EXPC1            ;F=(F*LN 2)-C1
       STORE @DS              ;STORE IN DS
       BL   @PLYXX            ;EVALUATE
       DATA EXPC2
       FMUL @DS               ;* DS
       STORE @DS              ;STORE IN DS AGAIN
       BL   @PLYX             ;EVALUATE
       DATA EXPC3
       STORE @TEMP            ;MOVE TO TEMP
       FSUB @DS               ;-DS
       STORE @DS1             ;SAVE IN DS1
       LOADF @TEMP            ;MOVE TEMP TO FPAC
       FADD @DS               ;ADD DS
       FDIV @DS1              ;/ DS1
       FMUL @EXPC4            ;* C4
       MOV  R2,R1             ;FIX EXPONENT
       SRL  R1,2
       SWPB R1
       A    R1,@FPAC          ;ADD TO EXPONENT
       MOV  R3,R1
       ANDI R2,>3             ;NEED POWER OF 2?
       JEQ  EXPF1             ;N
       MOV  R2,R3
       BL   @GETP2            ;MULTIPY BY POWER OF 2
*
EXPF1  MOV  R1,R1             ;NEGATIVE?
       JEQ  EXPF3             ;N, RETURN
       STORE @TEMP            ;Y, MOVE TO TEMP
       LOADF @FP1             ;GET INVERSE, LOAD FPAC
       FDIV @TEMP
       JMP  EXPF3             ;DONE
*
EXPF2  LOADF @FP1             ;EXP(0)=1
*
EXPF3  LI   R2,FPAC
       B    *R10
*
EXPC0  DATA >4117,>1547,>652C 1.442695  LOG 2 (E)
EXPC1  DATA >4080,>0000,>0000 1/2
EXPC2  DATA 2
       DATA >423C,>9D67,>06A2 60.614853
       DATA >4476,>4EF8,>C12A
       DATA >461F,>BE80,>58C1
EXPC3  DATA 3
FP1    DATA >4110,>0000,>0000
       DATA >436D,>549A,>5FE1
       DATA >4550,>02D2,>6DCF
       DATA >465B,>9820,>5C39
EXPC4  DATA >4116,>A09E,>667F 1.4142135
       END
