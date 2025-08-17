       TITL 'SYSTEM FUNCTIONS - CORTEX BASIC REV. 1.1'
       IDT  'FUNC'
       COPY 'IODEFS.INC'
       DEF  MODF,SYSF,ABSF,NKYF
       DEF  INTF,FRAF
       REF  FPAC,FPAC2,TEMP
       REF  FIX,EVSFR
       REF  SYSFLG,GETC$,SYSLMT
       REF  C4A00,VDPST
ERROR  EQU  >2F80
ERROR2 EQU  ERROR+>20
*
*  DEFINE FLOATING POINT XOPS FOR THIS MODULE
*
       DXOP LOADF,0           ;LOAD FPAC
       DXOP STORE,1           ;STORE FPAC
       DXOP FADD,2            ;ADD TO FPAC
       DXOP FSUB,3            ;SUBTRACT FROM FPAC
       DXOP SCALE,6           ;SCALE PFAC
       DXOP NORMAL,7          ;NORMALIZE FPAC
       DXOP CLEAR,8           ;CLEAR FPAC
       DXOP NEGATE,9          ;NEGATE FPAC
       DXOP FLOATF,10         ;FLOAT FPAC
       PAGE
*
*      SYS FUNCTION
*
SYSF   BL   @FIX              GET PARAMETER
       CI   R1,VDPST          VDP STATUS CHECK ?
       JNE  SYSFA1            NO , NORMAL SYSTEM FUNCTION
       MOVB @VDPREG,R1        YES , FETCH VDP STATUS
       SRL  R1,8              ALIGN
       JMP  INPF1             RETURN VALUE TO CALLER
*
SYSFA1 CI   R1,SYSLMT         VALID ?
       JLE  SYSFA2            YES , RETURN SYSTEM CONSTANT
       DATA ERROR2,35         NO , PARAMETER ERROR
*
SYSFA2 A    R1,R1             GET WORD INDEX
       MOV  @SYSFLG(1),R1     GET VALUE
       JMP  INPF1             RETURN TO CALLER
       PAGE
*
* INTEGER PART FUNCTION
*
INTF   MOV  *R2,R1            INTEGER ALREADY?
       JEQ  INPF3             Y
       LOADF *R2              N - LOAD FPAC
*
* IF NUMBER => SCALING FACTOR  GET FLOATING PT OVERFLOW
*
       ANDI R1,>7F00          MASK OUT ALL BUT EXPONENT
       C    R1,@C4A00         => SCALING FACTOR?
       JHE  INPF2
       SCALE @C4A00           N - SCALE OFF FRACTION
       NORMAL 0
       JMP  INPF2
*
INPF1  CLEAR 0                SAVE #
       MOV  R1,@FPAC2
INPF2  LI   R2,FPAC           RETURN ADDRESS
INPF3  B    @EVSFR            RETURN
*
*ABSOLUTE FUNCTION
*
ABSF   LOADF *R2              ;LOAD FPAC
       MOV  *R2,R1            ;CHECK SIGN
       JEQ  ABSF1             ;INTEGER
       JGT  INPF3             ;+, RT
       NEGATE 0               ;-, NEGATE
       JMP  INPF2             ;RETURN
*
ABSF1  ABS  @FPAC2            ;INTEGER, TAKE ABS
       JNO  INPF2             ;NO OVERFLOW
       LI   R0,>4480          ;OVERFLOW, LOAD 32768 (FP)
       CLR  R2
       LOADF R0               ;LOAD R0,R1,R2
       JMP  INPF2
*
*NKY FUNCTION
*
NKYF   DATA GETC$             ;CHARACTER?
       JMP  INPF2             ;N - RETURN 0
       NOP                    ;Y - NORMAL CHARACTER
       SWPB R0                ;Y - ESCAPE CHARACTER
       BL   @FIX              ;FIX ARGUMENT
       MOV  R1,R1             ;ARG=0?
       JEQ  NKYF1             ;Y, RETURN LEC
       C    R0,R1             ;N, ARG=LEC?
       JNE  INPF2             ;N, RETURN 0
*
NKYF1  MOV  R0,R1
       JMP  INPF1
*
*  FRACTIONAL PART FUNCTION
*
*   EXPONENT SIZE:
*     ALL FRA  40 < INT+FRA < 4A  ALL INT
*
FRAF   MOV  *R2,R0            ;INTEGER?
       JNE  FRAF2             ;N
FRAF1  LI   R2,FPAC           ;Y, RETURN 0
       RT
*
FRAF2  ANDI R0,>7F00          ;GET EXPONENT
       CI   R0,>4900          ;ALL INTEGER ?
       JGT  FRAF1             ;Y
       LOADF *R2              ;N, LOAD FPAC
       CI   R0,>4100          ;ALL FRACTION ?
       JLT  FRAF1             ;Y
       STORE @TEMP            ;N, MOVE TO TEMP
       SCALE @C4A00           ;GET INTEGER
       NORMAL 0               ;NORMALIZE
       NEGATE 0               ;NEGATE
       FADD  @TEMP            ;ADD NEGATED INTEGER IN FPAC
       JMP  FRAF1
       PAGE
*
*              MOD FUNCTION
*             ==============
*
*      FORMAT :-
*           A = MOD [ ARG 1 , ARG 2 ]
*
*      THIS FUNCTION PROVIDES AN INTEGER MOD OF ARG 1
*      THE MOD # IS GIVEN BY ARG 2.
*         EG. A=MOD(9,7)   RETURNS A VALUE FOR A OF 2
*
*     *R1 CONTAINS ARG 2, *R2 CONTAINS ARG 1
*     R3,R10 ARE SPARE
*     EXIT IS VIA RT.
*
MODF   MOV  R1,R3             SAVE ARG 2
       JGT  MODF1             +VE, OK
ERR30  DATA ERROR+30          RANGE ERROR
*
MODF1  BL   @FIX              FIX PARAMETER
       MOV  R1,R0             COPY INTO R0
       SRA  R0,15             FILL WITH SIGN BIT
* R0,R1 NOW CONTAINS ARG 1
       DIVS R3                DO DIVIDE
       MOV  R1,R1             TEST SIGN BIT
       JGT  INPF1             +VE, OK
       JEQ  INPF1             0,   OK
       A    R3,R1             -VE, ADJUST !
       JMP  INPF1
       END
