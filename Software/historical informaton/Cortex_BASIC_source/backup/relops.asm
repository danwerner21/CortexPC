       TITL 'RELATIONAL OPERATORS - CORTEX BASIC REV. 1.1'
       IDT  'RELOPS'
*
*
*
       DEF  ANDF              ;AND FUNCTION
       DEF  ORF               ;OR FUNCTION
       DEF  NOTF              ;NOT FUNCTION
*
       DXOP CLEAR,8           ;CLEAR FPAC
*
       REF  FPAC,FPAC2        ;FLOATING POINT ACCUMULATOR
*
* ABSTRACT:
*
*       PERFORM 'AND' FUNCTION ON TWO OPERANDS
*       AND RETURN A VALUE OF 1 OR 0 ACCORDINGLY.
*
*
* CALLING SEQUENCE:
*
*       BL @ANDF
*
*       IN (R1) = OPERAND 1
*          (R2) = OPERAND 2
*
*       OUT (R2) = 1 OR 0 IN FPAC
*
*      NORMAL EXIT - RETURN
*
       PAGE
*
*AND OPERATOR
*
ANDF   CLEAR 0                ;CLEAR FPAC
       MOV  *R1+,R3           ;INTEGER?
       JNE  ANDF1             ;N
       MOV  *R1,R3            ;Y, GET INTEGER
       JEQ  ANDF3             ;0, RESULT=0
*
ANDF1  MOV  *R2+,R3           ;INTEGER?
       JNE  ANDF2             ;RESULT=1
       MOV  *R2,R3            ;Y
       JEQ  ANDF3             ;0, RESULT=0
*
ANDF2  INC  @FPAC2            ;RESULT=1
*
ANDF3  LI   R2,FPAC
       RT
       PAGE
* ABSTRACT:
*
*       PERFORM AN 'OR' OPERATION ON THE
*       2 OPERANDS AND RETURN A VALUE OF
*       1 FOR TRUE AND 0 FOR FALSE.
*
* CALLING SEQUENCE:
*
*       BL @ORF
*
*       IN (R1) = OPERAND 1
*          (R2) = OPERAND 2
*
*       OUT (R2) = 1 OR 0 IN FPAC
*
*      NORMAL EXIT - RETURN
*
*
*OR OPERATOR
*
ORF    CLEAR 0                ;CLEAR FPAC
       MOV  *R1+,R3
       JNE  ANDF2             ;RESULT=1
       MOV  *R1,R3
       JNE  ANDF2
       MOV  *R2+,R3
       JNE  ANDF2             ;RESULT=1
       MOV  *R2,R3
       JNE  ANDF2
       JMP  ANDF3             ;RESULT=0
       PAGE
* ABSTRACT:
*
*       PERFORM A 'NOT' FUNCTION ON THE OPERAND
*
* CALLING SEQUENCE:
*
*       BL @NOTF
*
*       IN (R1) = OPERAND
*
*       OUT (R2) = 1 OR 0 IN FPAC
*
*
NOTF   EQU  $+1               ;INDICATE AS UNARY OPERATION
NOT    CLEAR 0                ;CLEAR FPAC
       MOV  *R1+,R3
       JNE  ANDF3
       MOV  *R1,R3
       JEQ  ANDF2
       JMP  ANDF3
       END
