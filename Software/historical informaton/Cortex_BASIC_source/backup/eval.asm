       TITL 'EVAL - CORTEX BASIC REV. 1.1'
       IDT  'EVAL'
*
*       EVARZ           ;EVALUATE VARIABLE
*       EVERZ           ;EVALUATE EXPRESSION
*       EVSDZ           ;EVALUATE STRING
*       EVFX            ;EVALUATE AND FIX
*       CKEX            ;CHECK FOR EXPRESSION
*       EVAL            ;RECURSIZE EVALUATOR
*       EVALS2          ;2ND ENTRY IN RECURSIVE EVALUATOR
*       ADDF            ;ADD TWO VARIABLES
*       SUBF            ;SUBTRACT TWO VARIABLES
*
       DXOP LOADF,0           ;LOAD FPAC
       DXOP STORE,1           ;STORE FPAC
       DXOP FADD,2            ;ADD TO FPAC
       DXOP FSUB,3            ;SUBTRACT FROM FPAC
       DXOP FMUL,4            ;MULTIPLY FPAC
       DXOP FDIV,5            ;DIVIDE FPAC
       DXOP SCALE,6           ;SCALE FPAC
       DXOP NORMAL,7          ;NORMALIZE FPAC
       DXOP CLEAR,8           ;CLEAR FPAC
       DXOP NEGATE,9          ;NEGATE FPAC
       DXOP FLOATF,10         ;FLOAT FPAC
       DXOP EVFIX,11          ;EVALUATE AND FIX
       DXOP OUTFP,12          ;OUT FLOATING POINT #
       DXOP OUTINT,13         ;OUT INTEGER
ERROR  EQU  >2F80             ;XOP XX,14  (ERROR CALL)
ERROR2 EQU  ERROR+>20
*
       REF  ABSF              ;ABSOLUTE VALUE FUNCTION
       REF  ASCF              ;CONVERT ASCII FUNCTION
       REF  ATNF              ;ARC-TANGENT FUNCTION
       REF  COSF              ;COSINE FUNCTION
       REF  EXPF              ;EXPONENTIAL FUNCTION
       REF  FRAF              ;FRACTIONAL PART FUNCTION
       REF  INTF              ;INTEGER PART FUNCTION
       REF  LOGF              ;LOG FUNCTION
       REF  NKYF              ;KEY FUNCTION
       REF  SINF              ;SINE FUNCTION
       REF  SQRF              ;SQUARE ROOT FUNCTION
       REF  SYSF              ;SYSTEM FUNCTION
       REF  TICF              ;TIC FUNCTION
       REF  SGNF              ;SIGN FUNCTION
       REF  BITF              ;BIT FUNCTION
       REF  CRBF              ;CRB FUNCTION
       REF  CRFF              ;CRF FUNCTION
       REF  MEMF              ;MEM FUNCTION
       REF  MWDF              ;MEMORY WORD FUNCTION
       REF  LENF              ;STRING LENGTH FUNCTION
       REF  MCHF              ;STRING MATCH FUNCTION
       REF  POSF              ;STRING SEARCH FUNCTION
       REF  COLF              ;PIXEL COLOUR TEST
       REF  MODF              ;MOD FUNCTION
       REF  POWF              ;POWER FUNCTION
       REF  LORF              ;LOGICAL OR FUNCTION
       REF  LXORF             ;LOGICAL EXCLUSIVE OR FUNCTION
       REF  LANDF             ;LOGICAL AND FUNCTION
       REF  ANDF              ;AND FUNCTION
       REF  ORF               ;OR FUNCTION
       REF  NOTF              ;NOT FUNCTION
       REF  LNOTF             ;LOGICAL NOT FUNCTION
       REF  FIX               ;FIX ROUTINE
       REF  FLOAT             ;FLOAT ROUTINE
       REF  RANDZ             ;GET RANDOM NUMBER ROUTINE
       REF  ADRF              ;RETURN ADDRESS OF VARIABLE FUCTION
       REF  EVSKB,EVSKE       ;EVALUATION STACK
       REF  FPAC,FPAC2        ;FLOATING POINT ACCUMULATOR
       REF  TEMP              ;TEMP FLOATING POINT REGISTER
       REF  E$TEMP,TEMP2,TEMP4
       REF  FUZZ              ;FUZZ VALUE
       REF  UFT               ;USER-FUNCTION-TABLE INDEX
       REF  VDT               ;VARIABLE-DEFINITION-TABLE INDEX
       REF  VNT               ;NVARIABLE-NAME-TABLE INDEX
       REF  IOB               ;I/O-BUFFER INDEX
       REF  DLIM              ;DELIMITER INDEX
       REF  NVD               ;NEXT-VARIABLE-DEFINITION INDEX
       REF  NVS               ;NEXT-VARIABLE-STORAGE INDEX
       REF  WPR2              ;SECONDARY WORKSPACE
       REF  RTSTOR            ;TEMP. STORE FOR R11
*
       DEF  EVSFR             ;RETURN FOR FUNCTIONS
       DEF  EVSFR$            ;RETURN WITH R2 RELOAD
       DEF  EVOP3A            ;RETURN FOR OPERATORS
       DEF  B01,B05           ;BYTES
       DEF  C1,C4,C6          ;WORDS
       DEF  EVSDZ
       DEF  EVARZ
       DEF  CKEX
       DEF  EVERZ
       DEF  EVFX
       DEF  EVAL              ;FULL EXPRESSION EVALUATION
       DEF  EVALS2            ;PARTIAL EXPRESSION EVALUATION
       DEF  SUBF
       DEF  ADDF
       PAGE
*       EVSD EVALUATES FOR A STRING CONSTANT OR VARIABLE
*       BEGINNING AT THE PBC (R8).  MULTIPLE RETURNS ARE
*       USED ACCORDING TO WHAT IS FOUND.  IF NO STRING
*       OR VARIABLE IS FOUND, THE PBC (PROGRAM BYTE COUNTER)
*       IS LEFT UNCHANGED.
*
* CALLING SEQUENCE:
*
*       BLWP @EVSDZ
*         STRING CONSTANT RETURN ("---")
*         STRING VARIABLE RETURN ($VAR)
*       NEITHER
*
*       IN  R8 = PBC (PROGRAM BYTE COUNTER)
*       OUT R0 = DLIM (DELIMITER
*           R2 = PTR TO STRING
*           R8 = PBC UPDATED
*
* EXCEPTIONS AND CONDITIONS:
*
*       ERRORS INCLUDE EXPECTING VARIABLE, SYNTAX ERROR,
*       UNDEFINED FUNCTION, ILLEGAL DELIMITER, STORAGE
*       OVERFLOW, UNDEFINED VARIABLE, EXPRESSION TOO
*       COMPLEX, SUBSCRIPT ERROR, TOO MANY SUBSCRIPTS,
*       UNDIMENSIONED VARIABLE, AND ALL ARITHMETIC ERRORS.
       PAGE
EVSDZ  DATA WPR2,EVSD
*
EVSD   MOV  @16(13),R8        ;GET PBC
       CB   *R8,@B43          ;$?
       JEQ  EVSD2             ;Y
       JL   EVSD1             ;NOT STRING
       CB   *R8+,@B45         ;" OR '?
       JH   EVSD1             ;N
       MOV  R8,R2             ;Y
*
       MOVB *R8+,R0           ;LOOK FOR "0
       JNE  $-2
*
       CLR  R0
       MOVB *R8+,R0           ;RETURN DELIMITER
       JMP  EVER1
*
EVSD1  C    *R14+,*R14+       ;NOT STRING, RETURN 4(14)
       RTWP
*
EVSD2  INCT R14
       INC  @16(13)           ;SKIP $
*
*  THIS SHOULD FALL THROUGH INTO EVAR
       PAGE
*       EVAR CALLS THE EXPRESSION EVALUATOR FOR
*       ONE ITEM ONLY, NAMELY A VARIABLE.  WITH
*       R5 ZEROED, THE EVALUATOR WILL EVALUATE
*       UP TO THE FIRST OPERATER OR DELIMITER.
*       UPON RETURNING, EVAR WILL CHECK TO SEE
*       IF THE RESULT IS IN THE EXPRESSION STACK
*       IN WHICH CASE AN EXCEPTION WOULD OCCUR.
*
* CALLING SEQUENCE:
*
*       BLWP @EVARZ
*       NORMAL RETURN
*
*       IN  R8 = PBC
*       OUT R0 = DLIM
*           R2 = ADR
*           R8 = PBC UPDATED
*
* EXCEPTIONS AND CONDITIONS:
*
*       SAME AS EVSD
       PAGE
EVAR   CLR  R5                ;SET FOR VARIABLE ONLY
       BL   @EVALS1           ;EVALUATE
       CI   R2,EVSKB          ;# IN STACK?
       JL   EVER1             ;N, RETURN
*
ERR22  DATA ERROR+22          ;EXPECTING VARIABLE
EVARZ  DATA WPR2,EVAR
       PAGE
*       CKEX IS CALLED TO SEE IF AN EXPRESSION
*       COULD OCCUR NEXT.  THE EXCEPTION WOULD
*       OCCUR IF A DELIMITER WAS THE NEXT ITEM.
*
* CALLING SEQUENCE:
*
*       BL @CKEX
*         NO EXPRESSION
*       EXPRESSION
*
*       IN  R8 = PBC
*
CKEX   MOVB *R8,@DLIM
       JEQ  CKEX2             ;EOL
       CB   *R8,@B38          ;FUNCTION?
       JL   CKEX1             ;Y
       CB   *R8,@B4C          ;OPERATOR OR VARIABLE?
       JL   CKEX2             ;N, TAKE ERROR RETURN
*
CKEX1  INCT R11
CKEX2  RT
       PAGE
*       EVER WILL EVALUATE A FULL EXPRESSION AND
*       WILL EXIT THRU THE ERROR CALL
*       WITH ANY EXCEPTION.
*
* CALL SEQUENCE:
*
*       BLWP @EVERZ
*
*       IN  R8 = PBC
*       OUT R0 = DLIM
*           R2 = ADR
*           R8 = PBC UPDATED
*
* EXCEPTIONS AND CONDITIONS:
*
*       SAME AS EVSD
*
EVERZ  DATA WPR2,EVER
EVER   BL   @EVALS            ;EVALUATE
EVER1  MOV  R2,@4(13)         ;RETURN PARAMETERS
EVER2  MOV  R0,@DLIM          ;RETURN DELIMITER
       MOV  R0,*R13
       MOV  R8,@16(13)        ;UPDATE PBC
       RTWP
       PAGE
*       EVFX USES AN XOP CALL SINCE IT IS
*       USED SO OFTEN AND RETURNS A 1 WORD,
*       2'S COMPLEMENT INTEGER IN THE OPERAND
*       OF THE EXOP.  EVFX DOES A FULL
*       EVALUATION (R5 <> 0) AND RETURNS
*       R0, R8, AND THE RESULT.
*
*       SINCE R0 IS ALTERED, EVFX R0 WOULD
*       BE ILLEGAL.
*
* CALLING SEQUENCE:
*
*       XOP XX,11
*
*       IN  R8 = PBC
*       OUT R0 = DLIM
*           R8 = PBC UPDATED
*         *R11 = #
*
* EXCEPTIONS AND CONDITIONS:
*
*       SAME AS EVSD WITH THE ADDITION OF
*       A FIX ERROR POSSIBILITY.
*
*       R0,R8 CANNOT BE USED AS THE RETURNING
*       FIELD.
*
EVFX   MOV  R11,@RTSTOR       SAVE ADDRESS
       BL   @EVALS            EVALUATE
       BL   @FIX              FIX RESULT
       MOV  @RTSTOR,R12       RESTORE RETURN ADDRESS
       MOV  R1,*R12           RETURN #
       JMP  EVER2
       PAGE
*       EVAL IS A RECURSIVE STACK EXPRESSION EVALUATOR.
*       R6 POINTS TO THE BOTTOM OF THE STACK WHERE
*       OPERANDS AND OTHER DATA ARE STACKED
*       WHILE R7 POINTS TO THE TOP OF THE STACK WHERE
*       OPERATORS ARE STACKED.  BOTH ENDS OF THE STACK
*       ARE MARKED WITH NULLS TO CHECK FOR PROPER
*       EXPRESSION TERMINATION.  THE ROUTINE CONSISTS
*       MAINLY OF A MULTIPLEXOR AND A PRECEDENCE ORIENTED
*       PROCESSOR.
*
*       VARIABLES ARE STORED ON A STACK AS AN ADDRESS.
*       CONSTANTS GO DIRECTLY ON THE STACK FOLLOWED BY
*       A -1 TO INDICATE A CONSTANT VALUE.  IE:
*
*               * (PTR) * ----- /XXXX XXXX XXXX/
*               * 4110  *
*               * 0000  *
*               * 0000  *
*               *  -1   *
*
*       USER FUNCTIONS, SYSTEM FUNCTIONS, AND DIMENSIONED
*       VARIABLES ALL RECURSE ON THE EVALUATOR TO
*       OBTAIN THE ARGUMENTS.
*
*       THE LEFT BYTE OF R5 CONTROLS WHETHER A OPERATOR
*       IS UNARY OR NOT.  WHEN NULL AND AN OPERATOR IS
*       ENCOUNTERED, IT MUST BE UNARY!  UPON ENCOUNTERING
*       A VARIABLE, THIS LEFT BYTE IS SET INDICATING
*       THE NEXT OPERATOR WILL NOT BE UNARY.
*
*       THE RIGHT BYTE OF R5 CONTROLS HOW FAR THE
*       EVALUATION IS TO GO.  IE, ONE ONLY WANTS THE
*       FIRST VARIABLE FOR AN ASSIGNMENT STATEMENT OR
*       ANYWHERE ONE WANTS TO STORE A VALUE.  THUS,
*       WHEN THE RIGHT BYTE OF R5 IS ZERO, EVALUATION
*       WILL TERMINATE AT THE FIRST OPERATOR.
*
*       WHEN A DELIMITER IS ENCOUNTED, THE RIGHT BYTE
*       OF R5 IS SET TO ZERO THUS TERMINATING THE
*       EVALUATION OF THE EXPRESSION.
*
*       THE LEAST SIGNIFICANT BIT OF THE OPERATOR
*       ADDRESS IS USED TO INDICATE A UNARY OPERATION.
*
*       USER FUNCTIONS USE THE STACK QUITE EXTENSIVELY
*       PUSHING FUNCTION ADDRESS, DUMMY ARGUMENTS, PLC,
*       R5, AND A RETORE-STACK-POINTER (R4) FOR EACH
*       LEVEL OF FUNCTIONS.
*
* CALLING SEQUENCE:
*
*       BL @EVALS       ;START EVALUATION
*
*       IN  R5  = EXPRESSION FLAG
*
*         (FALL THRU TO EVAL)
*
*       BL @EVAL        ;RECURSIVE EVALUATOR
*
*           R5  = EXPRESSION FLAG
*           R6  = BOTTOM OF STACK
*           R7  = TOP OF STACK
*           R8  = PLC
*
* EXCEPTIONS AND CONDITIONS:
*
*       IF R6 >= R7, THEN AN EXPRESSION TOO
*          COMPLEX OCCURS.
       PAGE
*USER FUNCTION EVALUATION
*
*STACK: R5
*       FUNC ADR
*       DUMMY #1
*       DUMMY #2
*       DUMMY #3
*       . . .
*       R8
*       R4 = STACK+2
*
ERR38  DATA ERROR2,38         ;UNDEFINED FUNCTION
*
EVAFN  MOV  R5,*R6+           ;SAVE R5
       MOV  R6,R4             ;MARK
       MOV  @UFT,R3           ;GET TABLE ADR
       SRL  R0,6              ;GET INDEX
       A    R0,R3             ;INDEX
       MOV  *R3+,*R6+         ;STACK ADR
       JEQ  ERR38             ;NOT DEFINED, ERROR
       MOV  *R3,R1            ;GET COUNT
       JEQ  EVAFN5            ;NO ARGUMENTS
       MOV  @VDT,R3
       MOV  *R3+,*R6+         ;SAVE OLD DUMMIES
       MOV  *R3+,*R6+
       MOV  *R3,*R6+
       AI   R3,-4
       SETO R5                ;DO FULL EVAL IF NOT [
       CB   *R8+,@B4A         ;[?
       JEQ  EVAFN1            ;Y
       DEC  R8                ;N
       CLR  R5
       SETO R1                ;SET COUNT TO -1
*
EVAFN1 MOV  R1,*R6+           ;PUSH COUNT
       MOV  R3,*R6+
       MOV  R4,*R6+
       BL   @EVAL             ;EVALUATE NEXT PARAMETER
       DECT R6
       MOV  *R6,R4            ;POP R4
       DECT R6
       MOV  *R6,R3            ;POP SYMBOL TABLE PTR
       DECT R6
       MOV  *R6,R1            ;POP COUNT
       MOV  R6,*R3+           ;LOAD NEW DUMMY ADR
       MOV  *R2+,*R6+         ;STACK
       MOV  *R2+,*R6+
       MOV  *R2,*R6+
       INC  R1                ;LOOKING FOR VARIABLE ONLY?
       JEQ  EVAFN2            ;Y
       DECT R1                ;N, DONE?
       JEQ  EVAFN4            ;Y
       SETO R5                ;N, DO FULL
       CI   R0,>3F00          ;,?
       JEQ  EVAFN1            ;Y
EVER37 DATA ERROR2,37         ;ILLEGAL DELIMITER
*
EVAFN2 DEC  R8                ;BACKUP OVER DELIMITER
       JMP  EVAFN5
*
EVAFN4 CI   R0,>4B00          ;]?
       JNE  EVER37            ;N, ERROR
*
EVAFN5 MOV  R4,*R6+           ;PUSH R4
       MOV  R8,*R6+           ;PUSH R8
       MOV  *R4,R8            ;GET DEF ADR
       SETO R5
       BL   @EVAL             ;EVALUATE
       DECT R6
       MOV  *R6,R8            ;POP R8
       DECT R6
       MOV  *R6,R6            ;POP R6 (R4)
       MOV  R6,R4
       INCT R4                ;MOVE TO DUMMY STORAGE
       MOV  @VDT,R3
       MOV  *R4+,*R3+         ;RESTORE OLD VARIABLE ADDRESSES
       MOV  *R4+,*R3+
       MOV  *R4,*R3
       JMP  EVSFR             ;RETURN
       PAGE
*DO SYSTEM STRING OPERAND
*
EVSFO  CB   *R8,@B43          ;$?
       JEQ  EVSFO2            ;Y
       JL   EVSFO1
       CB   *R8,@B45          ;" OR '?
       JH   EVSFO1            ;N
       INC  R8                ;Y
       MOV  R8,R2             ;SAVE ADR
*
       MOVB *R8+,R0           ;MOVE TO END OF STRING
       JNE  $-2
       MOVB *R8+,R0           ;GET DELIMITER
       INCT R11
*
EVSFO1 RT
*
EVSFO2 INC  R8                ;MOVE TO VARIABLE
       INCT R11               ;SET RETURN
       CLR  R5
       B    @EVAL             ;DO EVALUATION
       PAGE
*SYSTEM FUNCTION EVALUATION
*
EVSF   MOV  R5,*R6+           ;STACK R5,R0
       MOV  R0,*R6+
       LI   R1,1              ;DEFAULT SECOND PARAMETER TO 1
       CB   *R8+,@B4A         ;[?
       JEQ  EVSF1             ;Y
       CLR  R5                ;N, DO VARIABLE ONLY
       DEC  R8
       BL   @EVAL
       DEC  R8
       JMP  EVSF2
*
EVSF1  BL   @EVSFO            ;CHECK FOR STRING
       JMP  EVSF1A            ;N
       MOV  R2,*R6+           ;STACK ADR
       CI   R0,>3F00          ;,?
       JNE  EVSF1B            ;N
       BL   @EVSFO            ;GET SECOND STRING
       JMP  EVER37
       MOV  R2,R1
       JMP  EVSF1B
*
EVSF1A SETO R5                ;DO FULL EVALUATION
       BL   @EVAL
       CI   R0,>4B00          ;]?
       JEQ  EVSF2             ;Y
       CI   R0,>3F00          ;,?
       JNE  EVER37            ;N, ERROR
*
*      2 PARM FUNCTIONS LEAVE PARM1 ON THE STACK UNPROTECTED
*      IF IT IS A CONSTANT (AND FORGETS ABOUT IT !)
*
       C    R2,R6             ;CONST ON STACK ?
       JL   E$$1              ;N, LEAVE ALONE
       LI   R11,E$TEMP        ;Y, GET ITS STOREAGE
       MOV  R2,R5             ;GET ITS PLACE ON THE STACK
       MOV  R11,R2            ;SAVE STOREAGE ADR
       MOV  *R5+,*R11+        ;COPY IT
       MOV  *R5+,*R11+
       MOV  *R5,*R11
E$$1   MOV  R2,*R6+           ;STACK ADR
       SETO R5
       BL   @EVAL             ;GET SECOND PARAMETER
       BL   @FIX
*
EVSF1B DECT R6                ;POP 1ST OPERAND
       MOV  *R6,R2
       CI   R0,>4B00          ;]?
       JNE  EVER37            ;N, ERROR
*
EVSF2  DECT R6                ;POP ID
       MOV  *R6,R3
       SRL  R3,7              ;GET INDEX
       MOV  @SFUNP->36(3),R11
       JEQ  SYSERR            ;0, SYSTEM ERROR
       CLEAR 0                ;CLEAR FPAC
       BL   *R11              ;GOTO ROUTINE
*
EVSFR  DECT R6                ;POP R5
       MOV  *R6,R5
*
EVSF4  MOV  *R2+,*R6+         ;MOVE RESULT ON STACK
       MOV  *R2+,*R6+
       MOV  *R2,*R6+
       JMP  EVAL6             ;CONTINUE
*
SYSERR DATA ERROR2,-1         ;SYSTEM ERROR
*
* FUNCTION RETURN WITH R2 RELOAD
*
EVSFR$ LI   R2,FPAC           ;RELOAD R2
       JMP  EVSFR             ;RETURN
*
*GET RANDOM NUMBER
*
EVRND  BLWP @RANDZ            ;GET RANDOM # IN FPAC
       LI   R2,FPAC
       JMP  EVSF4
       PAGE
*
EVALS  SETO R5                ;FULL EXPRESSION
*
EVALS1 MOV  @16(13),R8        ;GET PBC
*
EVALS2 LI   R6,EVSKB          ;GET BEGINNING STACK PTR
       LI   R7,EVSKE          ;GET END STACK PTR
*
*EVALUATE SIMPLE EXPRESSION
*
EVAL   MOV  R11,*R6+          ;STACK RETURN
       CLR  *R6+              ;MARK OPERAND STACK
       CLR  R0                ;MARK OPERATOR STACK
       MOVB R0,R5             ;IF OPERATOR: MUST BE UNARY
*
EVAL1  DEC  R7                ;PUSH ON OPERATOR STACK
       MOVB R0,*R7
       C    R6,R7             ;STACK OVERFLOW?
       JHE  EVER27            ;Y, ERROR
*
EVAL2  MOVB *R8+,R0           ;PARSE
       JEQ  EVADL             ;DONE
       CI   R0,>6F00          ;VARIABLE?
       JH   EVAV              ;Y
       JEQ  EVFP              ;N, FP #
       CI   R0,>6200          ;CONSTANT?
       JHE  EVAL3             ;Y
       CI   R0,>4C00          ;"( OR OPERATER?
       JH   EVOP              ;OPERATER
       JEQ  EVAL1             ;"(, PUSH
       CI   R0,>3800          ;DELIMITER?
       JHE  EVADL             ;Y
       CI   R0,>1B00          ;SYSTEM FUNCTION?
       JHE  EVSF              ;Y
       B    @EVAFN            ;FUNCTION
*
EVFP   MOVB *R8+,*R6+         ;FP CONSTANT
       MOVB *R8+,*R6+
       MOVB *R8+,*R6+
       MOVB *R8+,*R6+
       MOVB *R8+,*R6+
       MOVB *R8+,*R6+
       JMP  EVAL6             ;MARK
*
EVAL3  CI   R0,>6D00          ;1 WORD INTEGER?
       JHE  EVAL4             ;Y
       SWPB R0
       AI   R0,->63           ;N, -1 THRU 9
       JMP  EVAL5             ;STORE #
*
EVER10 DATA ERROR+10          ;STORAGE OVERFLOW
       PAGE
*UNARY MINUS
*
EVAL9  LI   R0,>6100          ;STACK -
EVAL91 JMP  EVAL1
*
*VARIABLE
*
EVAV   CI   R0,>7300          ;RND?
       JEQ  EVRND             ;Y
       MOV  @VNT,R1           ;GET VARIABLE TABLE
       SRL  R0,7              ;SWAP & X 2
       A    R0,R1             ;INDEX
       MOV  @->70*2(1),R1     ;DIMENSIONED
       JLT  EVADV             ;Y
       MOV  @VDT,R1
       A    R0,R1
       MOV  @->70*2(1),R0     ;DEFINED?
       JNE  EVAL7             ;Y
       SWPB R5                ;POSITION EVALUATION TYPE LJ IN R5
       MOVB R5,R5             ;FULL OR VARIABLE EVALUATION ?
       SWPB R5                ;RESTORE EVALUATION TYPE TO ORIGINAL POSITION
       JNE  EVER40            ;NE - FULL EVAL - DO NOT DEFINE - ERROR
       MOV  @NVS,R0           ;VARIABLE EVALUATION ONLY - DEFINE I   T
       AI   R0,-6
       C    R0,@NVD           ;OK?
       JL   EVER10            ;N, STORAGE OVERFLOW
       MOV  R0,@->70*2(1)
       MOV  R0,@NVS           ;UPDATA NVS
       JMP  EVAL7
EVER40 DATA ERROR2,40         ;UNDEFINED VARIABLE
*
*CONSTANTS
*
EVAL4  MOVB *R8+,R0           ;1 WRD CONSTANT
       SWPB R0
       MOVB *R8+,R0
       SWPB R0
*
EVAL5  CLR  *R6+              ;MOVE INTO STACK
       MOV  R0,*R6+           ;INSERT 0,#,0
       CLR  *R6+
*
EVAL6  SETO R0                ;MARK AS CONSTANT
*
EVAL7  MOV  R0,*R6+           ;MARK STACK
       CLR  R0
*
EVAL8  MOVB @BFF,R5           ;IF OPERATOR: MUST NOT BE UNARY
       C    R6,R7             ;STACK OVERFLOW?
       JL   EVAL2             ;N, OK
*
EVER27 DATA ERROR+27          ;Y, EXPRESSION TOO COMPLEX
       PAGE
*OPERATOR
*
*HANDLE UNARY OPERATORS
*
EVOPA  CI   R0,>5D00          ;"+?
       JEQ  EVAL2             ;Y, IGNOR
       CI   R0,>5C00          ;N, "-?
       JEQ  EVAL9             ;Y, STACK UNARY -
       CI   R0,>5200          ;NOT?
       JEQ  EVAL1             ;Y, STACK
       CI   R0,>5300          ;LNOT?
       JEQ  EVAL1             ;Y, STACK
EVER0  DATA ERROR+1           ;N, SYNTAX ERROR
*
*HANDLE OPENING AND CLOSING PAREN'S
*
EVOP0  CI   R0,>4D00          ;")?
       JNE  EVOP1             ;N
EVOP0A CB   *R7,@B4C          ;(?
       JNE  EVOP1             ;N
       INC  R7                ;Y, POP
       JMP  EVAL2
*
EVOP0B MOVB *R7,R1            ;NULL?
       JNE  EVOP0A            ;N
       JMP  EVADL             ;Y, TERMINATE
*
*OPERATER ENTRY
*
EVOP   MOVB R5,R1             ;NEED UNARY OPERATER?
       JEQ  EVOPA             ;Y
       CI   R0,>4D00          ;)?
       JEQ  EVOP0B            ;Y
       ANDI R5,>FF            ;N, ALLOW UNARY ONLY TO FOLLOW
       JNE  EVOP1             ;FULL EXPRESSION
*
*DELIMITER ENTRY
*
EVADL  CLR  R5                ;SET TO TERMINATE
       CLR  R0                ;DO CR
*
EVOP1  MOVB *R7,R1            ;LOOK AT TOP OPERATOR
       MOV  R0,R4
       ANDI R4,>FE00          ;LAP OFF LOW BIT
       CB   R4,R1             ;HIGHER PRECEDENCE?
       JH   EVAL91            ;Y, STACK
       MOVB *R7+,R3           ;N, POP OPERATER
       JEQ  EVOP4             ;DONE
       SRL  R3,7              ;SWAP & X 2
       MOV  @EVATB->9C(3),R11
*
       DECT R6                ;POP VARIABLE
       MOV  *R6,R1            ;EMPTY?
       JEQ  EVER0             ;Y
       INC  *R6               ;CONSTANT?
       JNE  EVOP2             ;N
       AI   R6,-6             ;Y
       MOV  R6,R1
*
EVOP2  COC  @C1,R11           ;UNARY OPERATION?
       JEQ  EVOP3             ;Y
       DECT R6                ;N, GET SECOND OPERAND
       MOV  *R6,R2            ;EMPTY?
       JEQ  EVER0             ;Y
       INC  *R6               ;CONSTANT?
       JNE  EVOP3             ;N
       AI   R6,-6             ;Y
       MOV  R6,R2
*
EVOP3  BL   *R11              ;DO OPERATION
*
EVOP3A MOV  *R2+,*R6+         ;MOVE RESULTS ON STACK
       MOV  *R2+,*R6+
       MOV  *R2,*R6+
       SETO *R6+              ;MARK AS CONSTANT
       JMP  EVOP0             ;CONTINUE
*
EVOP4  MOVB @-1(8),R0         ;GET DELIMITER
       DECT R6                ;POP VARIABLE
       MOV  *R6,R2            ;EMPTY?
       JEQ  EVER0             ;Y, ERROR
       INC  *R6               ;CONSTANT?
       JNE  EVOP5             ;N
       AI   R6,-6             ;Y
       MOV  R6,R2
*
EVOP5  AI   R6,-4             ;DONE, POP STACK
       MOV  *R6,R11
       MOV  @2(6),R1          ;STACK EMPTIED?
       JNE  EVER0             ;N, ERROR
       RT
       PAGE
EVAL8P JMP  EVAL8
*
*DIMENSIONED VARIABLE
*
EVADV  MOV  @VDT,R1
       A    R0,R1
       MOV  @->70*2(1),R4     ;DEFINED?
       JEQ  ERR39             ;N, UNDIMENSIONED VARIABLE
       MOV  R5,*R6+           ;STACK R5
       CLR  *R6+              ;ZERO INDEX & STACK
*
EVADV1 MOV  R4,*R6+           ;STACK R4
       SETO R5                ;DO FULL EXPRESSION
       BL   @EVAL             ;RECURSE
       BL   @FIX              ;FIX #
       DECT R6                ;POP BASE ADR
       MOV  *R6,R4
       DECT R6                ;READY FOR INDEX POP
       C    R1,*R4+           ;EXCEED DIMENSION?
       JH   ERR17             ;Y, SUBSCRIPT ERROR
       MOV  *R4+,R5           ;GET DEL MULTIPLIER
       CI   R5,-1             ;DONE?
       JEQ  EVADV2            ;Y
       MPY  R5,R1             ;MULTIPLY
       A    R2,*R6+           ;ADD TO INDEX & STACK
       CI   R0,>3F00          ;,?
       JEQ  EVADV1            ;Y, DO AGAIN
*
ERR18  DATA ERROR+18          ;N, TOO FEW SUBSCRIPTS
*
EVADV2 A    *R6,R1            ;ADD FINAL DIMENSION
       MPY  @C6,R1            ;MULTIPLY BY 6
       A    R4,R2             ;ADD BASE
       DECT R6                ;POP R5
       MOV  *R6,R5
       MOV  R2,*R6+           ;STACK ADDRESS
       CI   R0,>4B00          ;]?
       JEQ  EVAL8P            ;Y
       CI   R0,>4000          ;;?
       JNE  ERR19             ;N, TOO MANY SUBSCRIPTS
       MOV  R5,*R6+           ;Y, STACK R5
       SETO R5                ;DO FULL EVAL
       BL   @EVAL             ;RECURSE
       BL   @FIX              ;GET INTEGER
       CI   R0,>4B00          ;]?
       JNE  EVADVE            ;N, ERROR
       DECT R6                ;Y, POP R5
       MOV  *R6,R5
       DECT R6                ;POP ADR
       MOV  *R6,R0
       DEC  R1                ;INDEX=COUNT-1
       JLT  ERR17             ;ERROR
       A    R1,R0             ;INDEX
       C    R0,@IOB           ;EXCEED MEMORY?
       JHE  ERR17             ;Y, ERROR
       B    @EVAL7            ;N, CONTINUE
*
ERR17  DATA ERROR+17          ;SUBSCRIPT ERROR
*
ERR19  DATA ERROR+19          ;TOO MANY SUBSCRIPTS
*
ERR39  DATA ERROR2,39         ;UNDIMENSIONED VARIABLE
*
EVADVE DATA ERROR2,37         ;INVALID DELIMITER
*
EVATB  DATA ORF,LORF
       DATA ANDF,LANDF
       DATA NOTF,LNOTF
       DATA LXORF
       DATA FUZF
       DATA EQUF,GTHF
       DATA GEQF,LTHF
       DATA LEQF,NEQUF
       DATA SUBF,ADDF
       DATA DIVF,MULF
       DATA POWF,UMNF+1
*
EVPR   EQU  $-EVATB
       DATA 0
B01    EQU  $+1
C1     DATA 1
C4     DATA 4
B05    EQU  $+1
       DATA 5
       DATA 2
       DATA 3
C6     DATA 6
*
SFUNP  DATA ABSF              1B
       DATA ADRF              1C
       DATA ASCF              1D
       DATA ATNF              1E
       DATA COSF              1F
       DATA EXPF              20
       DATA FRAF              21
       DATA INTF              22
       DATA LOGF              23
       DATA NKYF              24
       DATA SINF              25
       DATA SQRF              26
       DATA SYSF              27
       DATA TICF              28
       DATA SGNF              29
       DATA BITF              2A
       DATA CRBF              2B
       DATA CRFF              2C
       DATA MEMF              2D
       DATA MWDF              2E
       DATA LENF              2F
       DATA MCHF              30
       DATA POSF              31
       DATA COLF              32
       DATA MODF              33
       DATA 0                 34
       DATA 0                 35
       DATA 0                 36
       DATA 0                 37
       PAGE
*       ARITHMETIC OPERATIONS SUCH AS ADD, SUBTRACT,
*       MULTIPLY, DIVIDE, UNARY MINUS, AND RELATIONAL
*       OPERATORS SUCH AS FUZZ, EQUAL, NOT EQUAL,
*       LESS THAN, GREATER THAN, LESS THAN OR EQUAL,
*       GREAT THAN OR EQUAL, AND NOT EQUAL USE
*       OPERANDS POINTED TO BY R1,R2 AND RETURN A
*       RESULT POINTED TO BY R2.
*
*       PREP DETERMINES IF BOTH OPERANDS ARE INTEGER
*       VALUES (IN WHICH CASE AN ATTEMPT IS MADE TO
*       DO AN INTERATION OPERATION) OR IF 1 OR BOTH
*       OF THE OPERANDS IS A FLOATING POINT NUMBER
*       (IN WHICH CASE BOTH ARE FLOATED AND A FLOATING
*       POINT OPERATION IS PERFORMED.)
*
*       RELATIONAL OPERATORS ARE GIVEN A VALUE CORRE-
*       SPONDING TO THE COMPARISON TO BE MADE.  BIT 0
*       IS THE EQUAL BIT, BIT 1 IS THE LESS-THAN BIT,
*       AND BIT 2 IS THE GREATER-THAN BIT.  HENCE:
*
*         0000  =  FUZZ
*         0001  =  EQUAL
*         0002  =  LESS-THAN
*         0003  =  LESS-THAN OR EQUAL
*         0004  =  GREATER-THAN
*         0005  =  GREATER-THAN OR EQUAL
*         0006  =  NOT EQUAL (LESS-THAN, GREATER-THAN)
*
* CALLING SEQUENCE:
*
*       BL @ADDF
*       BL @SUBF
*       BL @MULF
*       BL @DIVF
*
*       IN (R1) = OPERAND 1
*          (R2) = OPERAND 2
*
*       OUT (R2) = RESULT
*
*
*       B @FUZF         ;==
*       B @EQUF         ;=
*       B @LTHF         ;<
*       B @LEQF         ;<=
*       B @GTHF         ;>
*       B @GEQF         ;>=
*       B @NEQUF        ;<>
*
*       IN (R1) = OPERAND 1
*          (R2) = OPERAND 2
*
*       OUT (R2) = RESULT
*       EXITS TO EVOP3A
*
*       B @UMNF         ;- (UNARY MINUS)
*
*       IN (R1) = OPERAND
*
*       OUT (R2) = RESULT
*       EXISTS TO EVOP3A
       PAGE
*ARITHMETIC PREPARATION
*       BL @PREP
*         VECTOR
*       BOTH INTEGERS
*
*       IN      R1 = SOURCE 1
*               R2 = SOURCE 2
*
*       INT OUT R3 = S1
*               R1 = S2
*
PREP   MOV  *R1,@TEMP         ;LOAD TEMP
       MOV  @2(1),@TEMP2
       MOV  @4(1),@TEMP4
       LOADF *R2              ;LOAD FPAC
       MOV  *R1+,R3           ;INTEGER?
       JNE  PREP4             ;N
       MOV  *R2+,R3           ;INTEGER?
       JNE  PREP4             ;N
       MOV  *R1,R3            ;BOTH INTEGERS
       MOV  *R2,R1            ;GET INTEGERS
       B    @2(11)
       PAGE
ADDF   MOV  R11,R10           ;SAVE RETURN
       BL   @PREP             ;PREPARE OPEANDS
       DATA 2*>40+>2C20
       A    R3,R1             ;BOTH INTEGERS
       JNO  PREP5A            ;NO OVERFLOW
PREP4  BLWP @FLTMZ            ;FLOAT TEMP
       FLOATF 0               ;FLOAT FPAC
       X    *R11              ;DO FP OPERATION
       DATA TEMP
       JMP  PREP5B            ;RETURN
*
PREP5  NEG  R1                ;NEGATE RESULT
*
PREP5A MOV  R1,@FPAC2         ;SAVE RESULT
*
PREP5B LI   R2,FPAC           ;GET ADDRESS OF RESULT
       B    *R10              ;RETURN
*
FLTMZ  DATA TEMP,FLOAT
*
* SUBTRACT
*
*
SUBF   MOV  R11,R10           ;SAVE RETURN
       BL   @PREP
       DATA 3*>40+>2C20
       S    R3,R1             ;INTEGERS
       JNO  PREP5A
       JMP  PREP4             ;OVERFLOW
       PAGE
*
*MULTIPLY
*
MULF   MOV  R11,R10           ;SAVE RETURN
       BL   @PREP
       DATA 4*>40+>2C20
       MOV  R3,R3             ;R3<0?
       JLT  MULF5             ;Y
       MOV  R1,R1             ;R1<0?
       JLT  MULF6             ;Y
*
MULF2  MPY  R3,R1             ;R1,R2 = R3 X R1
       MOV  R1,R1             ;OVERFLOW?
       JNE  PREP4             ;Y
       MOV  R2,R1             ;OVERFLOW?
       JLT  PREP4             ;Y
       JMP  PREP5A            ;N
*
MULF5  NEG  R3                ;R3<0
       MOV  R1,R1             ;R1<0
       JGT  MULF7             ;N
       NEG  R1                ;Y
       JMP  MULF2             ;Y
*
MULF6  NEG  R1                ;SIGNS DIFFERENT
MULF7  MPY  R3,R1             ;R1,R2 = R3 X R1
       MOV  R1,R1             ;OVERFLOW?
       JNE  PREP4             ;Y
       MOV  R2,R1             ;OVERFLOW?
       JLT  PREP4             ;Y
       JMP  PREP5             ;RETURN
       PAGE
*
*DIVIDE
*
DIVF   MOV  R11,R10           ;SAVE RETURN
       BL   @PREP
       DATA 5*>40+>2C20
       MOV  R3,R3             ;SOURCE<0?
       JLT  DIVF5             ;Y
       JEQ  ERR28             ;DIVISION BY ZERO
       MOV  R1,R2             ;DESTINATION<0?
       JLT  DIVF6             ;Y
*
DIVF2  CLR  R1                ;POSITIVE RESULT
       DIV  R3,R1             ;DIVIDE
       MOV  R2,R2             ;REMAINDER?
       JNE  PREP4             ;Y, FLOAT
       MOV  R1,R1             ;OVERFLOW?
       JLT  PREP4             ;Y, FLOAT
       JMP  PREP5A            ;N, STORE RESULTS
*
DIVF5  NEG  R3                ;SOURCE<0
       MOV  R1,R2             ;DESTINATION<0?
       JGT  DIVF7             ;N
       NEG  R2                ;Y, POSITIVE RESULT
       JMP  DIVF2
*
DIVF6  NEG  R2                ;SIGNS DIFFERENT
DIVF7  CLR  R1
       DIV  R3,R1             ;DIVIDE
       MOV  R2,R2             ;REMAINDER?
       JNE  PREP4             ;Y, OVERFLOW
       MOV  R1,R1             ;OVERFLOW?
       JLT  PREP4             ;Y
       JMP  PREP5             ;RETURN
*
ERR28  DATA ERROR+28          ;DIVISION BY ZERO
       PAGE
*
*RELATIONAL OPERATERS
*
FUZF   EQU  $
EQUF   EQU  $
LTHF   EQU  $
LEQF   EQU  $
GTHF   EQU  $
GEQF   EQU  $
NEQUF  EQU  $
       MOV  @EVATB->AA+EVPR(3),R4
       BL   @SUBF
       LI   R3,4              ;GET MATCH CONSTANT
       MOV  R4,R4             ;FUZZ?
       JNE  EQUF1             ;N
       INC  R4                ;Y, R4=0001
       MOV  *R2,R1            ;GET EXPONENT
       JEQ  EQUF1             ;INTEGER
       ANDI R1,>7F00          ;ISOLATE
       CI   R1,FUZZ           ;LESS THAN FUZZ?
       JL   EQUF2A            ;Y, EQUAL
       JMP  EQUF3             ;N, UNEQUAL, SET R3 TO -2
*
EQUF1  MOV  *R2+,R1           ;GET SIGN OF RESULT
       JNE  EQUF2
       MOV  *R2,R1
EQUF2  JGT  EQUF3
       JNE  $+4
EQUF2A DEC  R3
       DECT R3
*
EQUF3  CLEAR 0                ;CLEAR FPAC
       COC  R3,R4             ;CORRESPONDING BITS SET?
       JNE  UMNF3
       INC  @FPAC2            ;RETURN 1
       JMP  UMNF3
       PAGE
*
*UNARY MINUS
*
UMNF   LOADF *R1              ;LOAD FPAC
       MOV  *R1,R1            ;INTEGER
       JEQ  UMNF1             ;Y
       NEGATE 0               ;N, NEGATE FPAC
       JMP  UMNF3
*
UMNF1  NEG  @FPAC2
*
UMNF3  LI   R2,FPAC           ;RETURN
UMNF4  B    @EVOP3A
B38    BYTE >38
B43    BYTE >43
B45    BYTE >45
B4A    BYTE >4A
B4C    BYTE >4C
BFF    BYTE >FF
       EVEN
       END
