       TITL 'GET CONSTANT ROUTINE - CORTEX BASIC REV. 1.1'
       IDT  'CVGC'
*
*       CVGCN           ;GET CONSTANT
*
       DEF  CVC10             ;FLOATING POINT CONSTANT 10.
       DEF  CVGCN,CVGCN1
*
*
*       CVGCN WILL GET THE POWER OF TEN CLOSEST
*       TO THE POWER OF 16 IN R8.  IE:
*
*               16^1  >  10^1
*               16^2  >  10^2
*               16^3  >  10^3
*               16^4  >  10^4
*               16^5  >  10^6
*               16^6  >  10^7
*               ....     ....
*
* CALLING SEQUENCE:
*
*       BL @CVGCN
*
*       IN    R8 = POWER OF 16
*       OUT (R0) = FP CONSTANT
       PAGE
CVGCN  CI   R8,5
       JL   CVGCN2
       INC  R8                EXP CHANGE >=5
CVGCN1 CI   R8,10
       JL   CVGCN2
       LI   R8,9              EXP CHANGE >9, USE 9
*
CVGCN2 X    *R11+             ADJUST DECIMAL COUNTER
       LI   R0,CVTB1          GET 10'S TABLE ADR
       SLA  R8,1              MAKE WORD INDEX
       A    R8,R0             INDEX
       SLA  R8,1              R8 X 2
       A    R8,R0             R3=R3+3*R8
       B    *R11              RETURN
*
CVTB1  DATA >4110,>0000,>0000 10^0
CVC10  DATA >41A0,>0000,>0000 10^1
       DATA >4264,>0000,>0000 10^2
       DATA >433E,>8000,>0000 10^3
       DATA >4427,>1000,>0000 10^4
       DATA >4518,>6A00,>0000 10^5
       DATA >45F4,>2400,>0000 10^6
       DATA >4698,>9680,>0000 10^7
       DATA >475F,>5E10,>0000 10^8
       DATA >483B,>9ACA,>0000 10^9
       END
