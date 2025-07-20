       TITL 'SIGN FUNCTION - CORTEX BASIC REV 1.1'
       IDT  'SGNF'
*
       DEF  SGNF
*
       REF  EVSFR$,FPAC2
*
*  SIGN FUNCTION
*
SGNF   MOV  *R2+,R0           ;GET SIGN
       JNE  SGNF1
       MOV  *R2,R0            ;INTEGER, GET SIGN
       JEQ  SGNF3             ;0, RETURN 0
SGNF1  JGT  SGNF2             ;+
       DEC  @FPAC2            ;-, RETURN -1
       JMP  SGNF3             ;GET ADR & RETURN
*
SGNF2  INC  @FPAC2            ;+, RETURN 1
SGNF3  B    @EVSFR$           ;RETURN
       END
