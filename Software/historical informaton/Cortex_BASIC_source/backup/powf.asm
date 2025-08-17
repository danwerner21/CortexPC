       TITL 'POWER FUNCTIONS - CORTEX BASIC REV. 1.1'
       IDT  'POWF'
*
* ROUTINE LIST:
*
*       POWF            ;POWER FUNCTION
*
       REF  LOGF              ;LOG FUNCTION
       REF  EXPF              ;EXP FUNCTION
       REF  EVOP3A            ;EXIT TO EVAL
       DXOP LOADF,0           ;LOAD FPAC
       DXOP STORE,1           ;STORE FPAC
       DXOP FMUL,4            ;MULTIPLY FPAC
       DXOP FLOATF,10         ;FLOAT FPAC
       REF  TEMP,TEMP6        ;TEMPORARY STORAGE
       DEF  POWF
       PAGE
* ABSTRACT:
*
*       RAISE (R1) TO THE (R2) POWER USING
*       LOG AND EXPONENTIAL FUNCTIONS.
*
*               R1 ^ R2 = EXP( R1 * LOG(R2))
*
* CALLING SEQUENCE:
*
*       B @POWF
*
*       IN  (R1) = ARG1
*           (R2) = ARG2
*       OUT (R2) = RESULT
*
*       EXIT TO EVOP3A
*
* EXCEPTIONS AND CONDITIONS:
*
*       PRESERVE R0
*       EXP,LOG, AND FP ERRORS
*
       DEF  POWF
*
POWF   MOV  R1,R4             ;PERSERVE R0,R1
       MOV  R0,@TEMP6
       BL   @LOGF             ;GET LOG(R2)
       STORE @TEMP            ;MOVE TO TEMP
       LOADF *R4              ;GET B
       FLOATF 0               ;FLOAT IF NECESSARY
       FMUL @TEMP             ;MULTIPLY BY TEMP
       BL   @EXPF             ;GET EXP(R1*LOG(R2))
       MOV  @TEMP6,R0         ;RESTORE R0
       B    @EVOP3A
       END
