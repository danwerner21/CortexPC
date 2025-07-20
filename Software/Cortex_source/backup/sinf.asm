       TITL 'SINE FINCTION - CORTEX BASIC REV. 1.1'
        IDT  'SINF'
*
*       COSF            ;COSINE FUNCTION
*       SINF            ;SINE FUNCTION
*
        REF FUNFX       ;FIX ARGUMENTS
        REF FUNBK       ;BREAK ARGUMENTS
        REF PLYX,PLYXX  ;EVALUATE POLYNOMIAL
        REF FPAC        ;FLOATING POINT ACCUMULATOR
        REF TEMP        ;3 WRD TEMPORARY STORAGE
        REF DS,DS1      ;3 WRD TEMPORARY STORAGE
*
        DXOP LOADF,0    ;LOAD FPAC
        DXOP STORE,1    ;STORE FPAC
        DXOP FADD,2     ;ADD TO FPAC
        DXOP FSUB,3     ;SUBTRACT FROM FPAC
        DXOP FMUL,4     ;MULTIPLY FPAC
        DXOP FDIV,5     ;DIVIDE FPAC
        DXOP NEGATE,9   ;NEGATE FPAC
       PAGE
*
*       CALCULATE SINE/COSINE FUNCTIONS OF (ARG).
*
* CALLING SEQUENCE:
*
*       BL @SINF
*       BL @COSF
*
*       IN  (R2) = ARG
*       OUT (R2) = RESULT
*
* EXCEPTIONS AND CONDITIONS:
*
*       FLOATING POINT ERRORS
* ENTRY POINT:
*
       DEF SINF,COSF
*
COSF    MOV R11,R10
        BL @FUNFX       FIX SIGN
          JMP COSF1     0, RETURN 1
        LI R3,1         START WITH 1
        JMP COSF2
*
COSF1   LOADF @COSC2    COS 0 = 1
        JMP COSF5       RETURN
*
SINF    MOV R11,R10
        BL @FUNFX       FIX EXPONENT
          JMP COSF5     ;0, RETURN 0
        SLA R3,1        IF +: 0, ELSE 2
*
COSF2   FMUL @COSC0     FPAC=FPAC*C1
        BL @FUNBK       BREAK INTO FRAC AND INT
        A R3,R1
        SRL R1,1        QUADRANT 2 OR 4?
          JNC COSF3     N
        STORE @TEMP     Y, GET 1-FRACTION
        LOADF @COSC2    LOAD FPAC WITH 1
        FSUB @TEMP      SUBTRACT FRACTION
*
COSF3   SRL R1,1        EFFECTIVE QUAD 3 OR 4?
          JNC COSF4     N
        NEGATE 0        Y, NEGATE
*
COSF4   STORE @DS       STORE IN DS
        BL @PLYXX       EVALUATE POLYNOMIAL X*X
          DATA COSC1
        STORE @DS1      SAVE IN DS1
        BL @PLYX        EVALUATE POLYNOMIAL X
          DATA COSC3
        FDIV @DS1       FPAC=FPAC/DS1
        FMUL @DS        FPAC=(FPAC/DS1)*DS
*
COSF5   LI R2,FPAC      RETURN
        B *R10
*
COSC0   DATA >40A2,>F983,>6E4E
COSC1   DATA 2
COSC2   DATA >4110,>0000,>0000
        DATA >4273,>4DCA,>815D
        DATA >4411,>7825,>55B4
COSC3   DATA 5
        DATA >BE95,>3606,>2DEE
        DATA >4041,>E3F5,>31B8
        DATA >C1C6,>5036,>51D0
        DATA >4311,>B7C5,>5A8F
        DATA >C3A9,>3BA0,>C828
        DATA >441B,>70D4,>8BB5
       END
