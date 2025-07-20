       TITL 'MEMORY ROUTINES - CORTEX BASIC REV. 1.1'
        IDT 'MEMF'
*
*       MEMY            ;MEM COMMAND
*       MEMF            ;MEM FUNCTION
*       ASCF            ;ASC FUNCTION
*
        REF EVERZ       ;EVALUATE EXPRESSION
        REF PFIX        ;FIX 16 BIT NUMBER
        REF GPRM2       ;GET PARAMETERS
        REF FPAC,FPAC2  ;FLOATING POINT ACCUMULATOR
        REF B4A         ;>4A
        REF NLIN        ;EXIT ENTRY TO MULTIPLEXOR
        REF EVSFR$      ;EXIT TO EVAL WITH R2 ='FPAC'
        DXOP STORE,1    ;STORE FPAC
        DXOP CLEAR,8    ;CLEAR FPAC
        DXOP EVFIX,11   ;EVALUATE AND FIX
        DXOP OUTFP,12   ;OUT FLOATING POINT #
ERROR   EQU >2F80               ;XOP XX,14  (ERROR CALL)
       PAGE
*
*       THE MEM COMMAND ALLOWS PBASIC TO ALTER
*       A BYTE OF MEMORY WHILE THE MEM FUNCTION
*       ALLOWS PBASIC TO READ A BYTE OF
*       MEMORY.
*
* CALLING SEQUENCE:
*
*       B @MEMY
*
*       EXIT TO NLIN
*
*
*       B @MEMF
*
*       IN  (R2) = ADDRESS
*       OUT (R2) = RESULT
*       EXIT TO EVSFR$
*
* EXCEPTIONS AND CONDITIONS:
*
*       EVALUATION ERRORS
*       SYNTAX ERROR
*
       PAGE
        DEF MEMY
*
MEMY    CB *R8+,@B4A    ;LEFT BRACKET?
          JNE ERR1      ;N, ERROR
        BLWP @EVERZ     ;GET ADDRESS
        BL @PFIX        ;GET INTEGER
        BL @GPRM2       ;CHECK PARAMETERS
        SWPB R3         ;LEFT JUSTIFY
        MOVB R3,*R1     ;STORE BYTE IN MEMORY
        B @NLIN
*
ERR1    DATA ERROR+1
*
*READ MEMORY
*
* ENTRY POINT:
*
        DEF MEMF
*
MEMF    BL @PFIX         ;GET PARAMETER
        MOVB *R1,R1      ;GET BYTE
MEMF1   SRL R1,8         ;RIGHT JUSTIFY
        CLEAR 0          ;CLEAR FPAC
        MOV R1,@FPAC2    ;SAVE
       B @EVSFR$
        PAGE
* ABSTRACT:
*
*       ASC RETURNS THE DECIMAL INTEGER VALUE
*       OF THE FIRST BYTE OF THE ARGUMENT.
*
* CALLING SEQUENCE:
*
*       B @ASCF
*
*       IN  (R2) = ADDRESS
*       OUT (R2) = RESULT
*       EXIT TO EVSFR$
*
* EXCEPTIONS AND CONDITIONS:  (NONE)
*
* EXTERNAL ROUTINES:
*
*       REF MEMF1       ;USE MEMF EXIT
*
* LOCAL DATA:  (NONE)
*
* ENTRY POINT:
*
        DEF ASCF
*
ASCF   EQU  $
       MOVB *R2,R1
       JMP  MEMF1
        END
