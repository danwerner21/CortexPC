       TITL 'ARCTANGENT FUNCTION - CORTEX BASIC REV. 1.1'
       IDT  'ATNF'
*
*       ATNF            ;ARC-TANGENT FUNCTION
*
       REF  FUNFX       ;FIX ARGUMENT
       REF  PLYX,PLYXX  ;EVALUATE POLYMONIAL
       REF  EVSFR$      ;EXIT TO EVALUATOR
       REF  C8000
       REF  FPAC        ;FLOATING POINT ACCUMULATOR
       REF  TEMP        ;TEMP REGISTER
       REF  DS,DS1      ;DATA STORAGE
       DEF  ATNF         ;ARCTANGENT FUNCTION
*
       DXOP LOADF,0    ;LOAD FPAC
       DXOP STORE,1    ;STORE FPAC
       DXOP FADD,2     ;ADD TO FPAC
       DXOP FSUB,3     ;SUBTRACT FROM FPAC
       DXOP FMUL,4     ;MULTIPLY FPAC
       DXOP FDIV,5     ;DIVIDE FPAC
       DXOP SCALE,6    ;SCALE FPAC
       DXOP NORMAL,7   ;NORMALIZE FPAC
       DXOP CLEAR,8    ;CLEAR FPAC
       DXOP NEGATE,9   ;NEGATE FPAC
       DXOP FLOATF,10  ;FLOAT FPAC
       PAGE
*
*       COMPUTE ARC-TANGENT OF ARGUMENT (R2).
*       RESULT IS IN RADIANS.
*
* CALLING SEQUENCE:
*
*       B @ATNF
*
*       EXIT TO EVSFR$
*
*
* EXCEPTIONS AND CONDITIONS:
*
*       FLOATING POINT ERRORS
*
ATNF    BL   @FUNFX           FIX
        JMP  ATNF5            0
*
ATNF0   CLR  R1
        MOVB *R2,R1
        CLR  R4
        MOV  @FPAC,R2
        S    @ATNC6,R2        <1?
        JLT  ATNF1            Y
        STORE @TEMP           MOVE TO TEMP
        LOADF @ATNC6          LOAD 1
        FDIV @TEMP            GET INVERSE
        INC  R4               SET FLAG
*
ATNF1   STORE @TEMP           MOVE TO TEMP
        FSUB @ATNC0           F-C0
        MOV  @FPAC,R2         GET SIGN
        LOADF @TEMP           MOVE TEMP TO FPAC
        MOV R2,R2             >.268...?
        JLT ATNF2             N
        FADD @ATNC1           Y
        STORE @DS             SAVE IN DS
        LOADF @TEMP           MOVE TEMP TO FPAC
        FMUL @ATNC1
        FSUB @ATNC6           SUBTRACT 1
        FDIV @DS              F=F/DS
*
ATNF2  STORE @DS              STORE IN DS
       BL   @PLYXX            EVALUATE
       DATA ATNC2
       STORE @DS1             SAVE IN DS1
       BL   @PLYX             EVALUATE
       DATA ATNC3
       FDIV @DS1              DIVIDE BY DS1
       FMUL @DS               MULTIPLY BY DS
       MOV  R2,R2             WAS IT >.268...?
       JLT  ATNF3             N
       FADD @ATNC4            Y, ADD CONSTANT
*
ATNF3  DEC  R4
       JNE  ATNF4
       FADD @ATNC5
*
ATNF4  SRC  R3,1              GET SIGN BIT
       SZC  @C8000,@FPAC
       A    R3,@FPAC          ADD SIGN
* EXIT TO EVALUATOR & RELOAD R2 WITH FPAC
ATNF5  B   @EVSFR$
*
ATNC0  DATA >4044,>9851,>7A7B
ATNC1  DATA >411B,>B67A,>E858
ATNC2  DATA 4
ATNC6  DATA >4110,>0000,>0000
       DATA >4225,>10EB,>4200
       DATA >42CF,>B153,>9710
       DATA >4316,>CA99,>3433
       DATA >42C5,>33FE,>142D
ATNC3  DATA 3
       DATA >41C9,>8867,>F42A
       DATA >427D,>9444,>406E
       DATA >4312,>AED9,>3E72
       DATA >42C5,>33FE,>142D
ATNC4  DATA >4086,>0A91,>C16C
ATNC5  DATA >C119,>21FB,>5444
       END
