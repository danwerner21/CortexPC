       TITL 'LOGICAL OPERATORS - CORTEX BASIC REV. 1.1'
        IDT  'LOGOPS'
*
*       LNOTF           ;LOGICAL NOT
*       LORF            ;LOGICAL OR
*       LXORF           ;LOGICAL EXCLUSIVE OR
*       LANDF           ;LOGICAL AND
*
        REF FIX         ;FIX ARGUMENT
        REF EVOP3A      ;EXIT ENTRY TO EVALUATOR
        REF FPAC,FPAC2  ;FLOATING POINT ACCUMULATOR
        DXOP CLEAR,8    ;CLEAR FPAC
       PAGE
*
*       PERFORM LOGICAL OPERATIONS OF
*
*       LOR     = LOGICAL 'OR'
*       LNOT    = LOGICAL 'NOT'
*       LXOR    = LOGICAL EXCLUSIVE 'OR'
*       LAND    = LOGICAL 'AND'
*
* CALLING SEQUENCE:
*
*       B @LNOTF
*       B @LORF
*       B @LXORF
*       B @LANDF
*
*       IN  (R1) = ARG1
*           (R2) = ARG2
*       OUT (R2) = RESULT (1 OR 0)
*       EXIT TO EVOP3A
*
       PAGE
*
*LOGICAL OPERATERS
*
*       R1 = XXI R4,..
*       R2 = DATA
*       R3 = B *R10
*
LFIX    MOV R11,R10
        CLEAR 0
        MOV R1,R4
        BL @FIX
        MOV R4,R2
        MOV R1,R4       ;STORE OBJECT
        BL @FIX
        MOV R1,R2
        MOV *R10+,R1
        LI R3,>045A     ;B *R10
        B R1
*
* ENTRY POINT:
*
        DEF LORF
*
LORF    BL @LFIX
          DATA >0264    ;ORI R4,..
        JMP LANDF1
*
* ENTRY POINT:
*
        DEF LXORF
*
LXORF   BL @LFIX
          DATA >045A    ;B *R10
        XOR R2,R4
        JMP LANDF1
*
* ENTRY POINT:
*
        DEF LANDF
*
LANDF   BL @LFIX
          DATA >0244    ;ANDI R4,..
*
LANDF1  MOV R4,@FPAC2
*
LANDF2  LI R2,FPAC
        B @EVOP3A
*
* ENTRY POINT:
*
        DEF LNOTF
*
*LOGICAL NOT
*
LNOT    CLEAR 0         ;CLEAR FPAC
        MOV R1,R2
        BL @FIX
        INV R1
        MOV R1,@FPAC2
        JMP LANDF2
*
LNOTF   EQU LNOT+1
       END
