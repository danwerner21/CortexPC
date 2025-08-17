       TITL 'DEF STATEMENT - CORTEX BASIC REV. 1.1'
       IDT  'DEF'
*
*       DEFY            ;DEF COMMAND
*
       REF  LINE              ;MULTIPLEXOR
       REF  UFT               ;USER FUNCTION TABLE
       REF  MODE              ;MODE FLAG
       REF  B56
       DEF  DEFY
* ABSTRACT:
*
*       PLACE THE USER FUNCTION PROGRAM-
*       BYTE-COUNTER AND NUMBER OF ARGUMENTS
*       IN USER-FUNCTION-TABLE (UFT) THUS
*       DEFINING THE FUNCTION.
*
* CALLING SEQUENCE:
*
*       B @DEFY
       PAGE
*
*DEF COMMAND
*
*USER FUNCTION TABLE:
*
*       PBC
*       # OF ARGUMENTS
*       ...
*
DEFY   MOV  @MODE,R0          ;RUNNING?
       JEQ  DEF2              ;N, DON'T DEFINE
       CLR  R0
       MOVB *R8+,R0           ;GET TYPE
       SETO R1                ;START COUNT
*
DEF1   INC  R1                ;COUNT ARGUMENTS
       CB   *R8+,@B56         ;=?
       JNE  DEF1              ;N
       MOV  @UFT,R3           ;Y, DEFINE FUNCTION
       SRL  R0,6              ;GET INDEX
       A    R0,R3             ;INDEX
       MOV  R8,*R3+           ;SAVE ADR
       MOV  R1,*R3            ;SAVE COUNT
DEF2   B    @LINE             ;GOTO NEXT LINE
       END
