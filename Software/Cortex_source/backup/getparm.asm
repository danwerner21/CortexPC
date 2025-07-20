       TITL 'GET PARAMETER ROUTINE - CORTEX BASIC REV. 1.1'
       IDT  'GETPARM'
*
*       GPRM,GPRM1,GPRM2        ;GET PARAMETER
*
       DXOP EVFIX,11          ;EVALUATE AND FIX
ERROR  EQU  >2F80             ;XOP XX,14  (ERROR CALL)
ERROR2 EQU  ERROR+>20
*
       REF  B4A,B56
       DEF  GPRM,GPRM1,GPRM2
*
*       FIX PARAMETERS SUCH THAT:
*
*         XXX[ (R2) , R1 ] = R3
*
*       DIFFERENT ENTRY POINTS WILL PICK UP
*       THE EVALUATION AND CHECKING AT
*       DIFFERENT POINTS.
*
* CALLING SEQUENCE:
*
*       BL @GPRM
*       BL @GPRM1
*       BL @GPRM2
*
*       IN (R2) = VARIABLE
*       OUT R1  = INDEX
*           R3  = ASSIGNMENT FIXED
*
* EXCEPTIONS AND CONDITIONS:
*
*       EVALUATION ERRORS, FIX ERRORS, AND
*       EXPECTING OPERATOR.
*
*
GPRM   CB   *R8+,@B4A         ;RIGHT BRACKET?
       JNE  ERR1              ;N, ERROR
*
GPRM1  EVFIX R1               ;GET INDEX
*
GPRM2  CI   R0,>4B00          ;]?
       JNE  ERR1              ;N
       CB   @B56,*R8+         ;=?
       JNE  ERR36             ;N
       EVFIX R3               ;GET RESULT
       RT
*
ERR1   DATA ERROR+1
ERR36  DATA ERROR2,36
       END
