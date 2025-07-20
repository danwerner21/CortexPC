       TITL 'FIX ROUTINE - CORTEX BASIC REV. 1.1'
       IDT  'FIX'
*
*       FIX     ;FIX 2'S COMPLEMENT INTEGER
*       PFIX    ;FIX 16-BIT POSITIVE INTEGER
*
       DXOP LOADF,0           ;LOAD FPAC
       DXOP SCALE,6           ;SCALE FPAC
       DXOP CLEAR,8           ;CLEAR FPAC
ERROR  EQU  >2F80             ;XOP XX,14  (ERROR CALL)
*
       REF  FPAC2             ;FLOATING POINT ACCUMULATOR
       REF  C4600
       DEF  FIX
       DEF  PFIX
       PAGE
*       FIX RETURNS THE 2'S COMPLEMENT 16-BIT INTEGER VALUE OF
*       THE 3 WORD NUMBER POINTED TO BY R2.  IF THE NUMBER IS ALREADY
*       AN INTEGER (1ST WORD = 0 ) THEN IT SIMPLY RETURNS THE SECOND WORD.
*       IF THE VALUE IS A FLOATING POINT NUMBER (1ST WORD <> 0) THEN THE
*       NUMBER IS LOADED INTO THE FLOATING POINT ACCUMULATOR, SCALED SUCH THAT
*       THE DECIMAL POINT IS AFTER THE SECOND WORD, AD THEN THE RESULT OR
*       2ND WORD OF FPAC IS RETURNED.  CARE IS TAKE TO ZERO FPAC AND
*       TO NEGATE THE RESULT DEPENDING UPON THE SIGN BIT.
*
*       PFIX RETURNS A 16-BIT RESULT RATHER THAN THE 2'S
*       COMPLEMENT VALUE.
*
* CALLING SEQUENCE:
*
*       BL @FIX
*         OR
*       BL @PFIX
*
*       IN - (R2) = 3 WORD PBASIC NUMBER.
*       OUT - R1 = RESULT.
*
*      NORMAL EXIT - RETURN
*
* EXCEPTIONS AND CONDITIONS:
*
*       ERROR 30 RESULTS IF THE ABSOLUTE VALUE OF THE NUMBER IS
*       GREATER THAN 2^15
*
*       R0 IS PRESERVED
       PAGE
FIX    MOV  *R2+,R1           ;INTEGER?
       JNE  FIX1              ;N, DO INTF
       MOV  *R2,R1            ;Y, RETURN #
       RT
*
FIX1   LOADF @-2(2)           ;LOAD FPAC
*
FIX2   MOV  R1,R2             ;SAVE
       ANDI R1,>7F80          ;MASK TO EXPONENT + 1 BIT
       CI   R1,>4400          ;TOO LARGE?
       JGT  ERR30             ;Y
       SCALE @C4600           ;N, GET INTEGER
       MOV  @FPAC2,R1
       CLEAR 0                ;LEAVE FPAC ZERO
       SLA  R2,1              ;NEGATIVE?
       JNC  FIX3              ;N
       NEG  R1                ;Y
FIX3   RT
*
ERR30  DATA ERROR+30          ;FIX ERROR
*
*
*
PFIX   LOADF *R2              ;LOAD FPAC
       MOV  *R2,R1            ;INTEGER?
       JEQ  PFIX1             ;Y
       SCALE @C4600           ;N, SCALE
*
PFIX1  MOV  @FPAC2,R1         ;GET RESULT
       RT
       END
