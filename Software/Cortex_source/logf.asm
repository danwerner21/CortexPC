       TITL 'LOG FUNCTION - CORTEX BASIC REV. 1.1'
        IDT 'LOGF'
*
* ROUTINE LIST:
*
*       LOGF            ;NATURAL LOG FUNCTION
*
        REF FUNFX       ;FIX ARGUMENT
        REF PLYX,PLYXX  ;EVALUATE POLYNOMIAL
        REF GETP2       ;GET POWER OF 2
       REF FPAC         ;FLOATING POINT ACCUMULATOR
       REF TEMP         ;3 WRD TEMPORARY STORAGE
       REF DS,DS1       ;3 WRD TEMPORARY STORAGE
       REF  B40
        DXOP LOADF,0    ;LOAD FPAC
        DXOP STORE,1    ;STORE FPAC
        DXOP FADD,2     ;ADD TO FPAC
        DXOP FSUB,3     ;SUBTRACT FROM FPAC
        DXOP FMUL,4     ;MULTIPLY FPAC
        DXOP FDIV,5     ;DIVIDE FPAC
        DXOP NORMAL,7   ;NORMALIZE FPAC
ERROR   EQU >2F80       ;XOP XX,14  (ERROR CALL)
       PAGE
*
*       COMPUTE THE NATURAL LOG OF (R2)
*
* CALLING SEQUENCE:
*
*       BL @LOGF
*
*       IN  (R2) = ARG
*       OUT (R2) = RESULT
*
* EXCEPTIONS AND CONDITIONS:
*
*       LOG OF NON-POSITIVE NUMBER
*
* ENTRY POINT:
*
       DEF LOGF
*
ERR26   DATA ERROR+26   ;LOG OF NON-POSITIVE NUMBER
*
LOGF    MOV R11,R10     ;SAVE RETURN
        BL @FUNFX       ;FIX
          JMP ERR26     ;LOG 0
        MOV R3,R3       ;NEGATIVE?
          JNE ERR26     ;Y, ERROR
        MOVB @B40,@FPAC ;LAOD EXPONENT
        SRL R1,6        ;SWAP & X 4
        AI R1,->100     ;UNBIAS
        MOV *R2,R2      ;GET FPAC
        LI R0,>80
        COC R0,R2       ;HIGH BIT SET?
          JEQ LOGF2     ;Y
        CLR R3          ;N, FIND POWER OF 2
*
LOGF1   INC R3          ;COUNT
        SRL R0,1
        COC R0,R2       ;BIT SET?
          JNE LOGF1     ;N, CONTINUE TO COUNT
        S R3,R1
        BL @GETP2       ;GET POWER OF 2 & MULTIPLY
*
LOGF2   MOV R1,R2
        STORE @TEMP     ;MOVE TO TEMP
        FSUB @LOGC0
        LI R0,FP1       ;GET FP1
        MOV @FPAC,R1    ;CHECK SIGN
          JGT LOGF3
          JEQ LOGF3
        LI R0,LOGC1
        DEC R2
*
LOGF3   LOADF @TEMP     ;MOVE TEMP TO FPAC
        FADD *R0
        STORE @DS       ;SAVE IN DS
        LOADF @TEMP     ;RELOAD TEMP
        FSUB *R0
        FDIV @DS        ;DIVIDE BY DS
        STORE @DS       ;SAVE IN DS
        BL @PLYXX
          DATA LOGC2
        STORE @DS1      ;STORE IN DS1
        BL @PLYX
          DATA LOGC3
        FDIV @DS1       ;DIVIDE BY DS1
        FMUL @DS        ;MULTIPLY BY DS
        STORE @TEMP     ;STORE IN TEMP
        LI R0,>8C00
        MOV R2,R2
          JLT LOGF4
        SRL R0,1
        JMP LOGF5
*
LOGF4   NEG R2
        SRA R0,1
*
LOGF5   MOV R2,R1
        CLR R2
        LOADF R0        ;LOAD FPAC
        NORMAL 0
        FMUL @LOGC4
        FADD @TEMP
*
LOGF6   LI R2,FPAC
        B *R10
*
LOGC0   DATA >40B5,>04F3,>33FA
LOGC1   DATA >4080,>0000,>0000  1/2
LOGC2   DATA 3
FP1     DATA >4110,>0000,>0000
        DATA >C214,>BBC5,>DCDB
        DATA >423D,>C2D5,>31F0
        DATA >C22D,>165C,>4BE0
LOGC3   DATA 2
        DATA >C212,>53EF,>500E
        DATA >425D,>76C2,>314A
        DATA >C25A,>2CB8,>97BF
LOGC4   DATA >40B1,>7217,>F7D2
        EVEN
        END
