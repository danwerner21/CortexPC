       TITL 'DIM STATEMENT - CORTEX BASIC REV. 1.1'
       IDT  'DIM'
*
*       DIMY            ;DIMENSION COMMAND
*
       REF  EVAL,EVALS2       ;RECURSIVE EVALUATOR
       REF  EVARZ             ;EVALUATE VARIABLE
       REF  FIX               ;FIX ARGUMENT
       REF  NLIN              ;EXIT TO MULTIPLEXOR
       REF  VDT               ;VARIABLE DEFINITION TABLE INDEX
       REF  NVS               ;NEXT VARIABLE STORAGE
       REF  NVD               ;NEXT VARIABLE DEFINITION
       REF  DLIM              ;PROGRAM DELIMITER
       REF  C6                ;CONSTANT 6
       DEF  DIMY
*
ERROR  EQU  >2F80             ;XOP XX,14  (ERROR CALL)
ERROR2 EQU  ERROR+>20
*
NRV    EQU  4                 ;NUMBER OF RESERVED WORDS
       PAGE
*
*       DIMENSION A VARIABLE.  A DIMENSIONED
*       VARIABLE IS INDICATED IN THE VDT BY A
*       NEGATIVE VALUE.  THE VARIABLE
*       DEFINITION POINT TO THE INFORMATION
*       VECTOR.
*
*INFORMATION VECTOR FORMAT:
*
*       D1,X1,D2,X2,...DN,-1,...DATA...
*
*WHERE  D1,D2,...DN = DIMENSIONS
*       X1,X2,...   = MULTIPLIERS
*
*       X1 = D2+1 X D3+1 X ... DN+1
*       X2 = D3+1 X D4+1 X ... DN+1
*       ...
*       XN = 1
*
* CALLING SEQUENCE:
*
*       B @DIMY
*
* EXCEPTIONS AND CONDITIONS:
*
*       EVALUATION ERRORS, STORAGE OVERFLOW
*       EXPECTING DIMENSIONED VARIABLE
*       INVALID DELIMITER
*
       PAGE
DIMY   CLR  R0                ;READY R0
       MOVB *R8+,R0           ;GET BYTE
       CI   R0,>4300          ;'$' ?
       JNE  TSTVAR            ;N, CHECK IF ITS A VARIABLE
       MOVB *R8+,R0           ;Y, GET THE VARIABLE NAME BYTE
TSTVAR CI   R0,NRV*>100+>7000 ;VARIABLE?
       JL   ERR16             ;N
       MOV  @VDT,R14          ;Y, GET SYMBOL ADR
       SRL  R0,7
       A    R0,R14            ;INDEX
       AI   R14,->70*2        ;ELIMINATE DISPLACEMENT
       MOV  *R14,R1           ;DEFINED?
       JNE  DIM3              ;Y, SEE IF EXCEEDS PREVIOUS
*
       SETO R5                ;DO FULL EVALUATIONS
       BL   @EVALS2           ;DO INITIAL EVAL
       CLR  *R6+              ;ZERO COUNT
       JMP  DIM2
*
DIM1   BL   @EVAL             ;EVALUATE NEXT SUBSCRIPT
*
DIM2   BL   @FIX              ;FIX PARAMETER (R2)
       DECT R6                ;POP COUNT
       MOV  *R6,R3
       INC  R3                ;COUNT DIMENSION
       MOV  R1,*R6+           ;PUSH DIMENSION
       SETO *R6+              ;LEAVE SPACE FOR DEL MULTIPLIER
       MOV  R3,*R6+           ;PUSH COUNT AGAIN
       CI   R0,>3F00          ;,?
       JEQ  DIM1              ;Y
       CI   R0,>4B00          ;]?
       JNE  DIME37            ;N, ERROR
       DECT R6                ;RETRIEVE COUNT
       MOV  *R6,R0
       MOV  R0,R10            ;GET INFORMATION VECTOR LENGTH
       SLA  R10,2             ;COUNT X 4
       LI   R1,1              ;SET DEL(N) TO 1
       JMP  DIM5
*
DIME37 DATA ERROR2,37
       PAGE
DIM3   DEC  R8                ;BACKUP TO VARIABLE
       BLWP @EVARZ            ;EVALUATE
       JMP  DIM7
*
DIM4   AI   R6,-4             ;DO ANOTHER DIMENSION
       MOV  *R6,R2            ;GET DIMENSION
       INC  R2                ;INCREMENT
       MPY  R1,R2             ;GET PRODUCT
       MOV  R2,R2             ;OVERFLOW?
       JNE  DIME10            ;Y
       MOV  R3,R1             ;SET R1
       JLT  DIME10
       MOV  R1,@-2(6)         ;STORE DEL IN INFORMATION VECTOR
*
DIM5   DEC  R0                ;DONE?
       JNE  DIM4              ;N
       AI   R6,-4             ;FINISHED, CALCULATE VECTOR
       MOV  *R6,R0
       INC  R0                ;1ST DIMENSION + 1
       MPY  R0,R1             ;GET FINAL DIMENSION SIZE
       MPY  @C6,R2            ;X 6 BYTES
       MOV  R2,R2             CHECK FOR OVERFLOW
       JNE  DIME10            REPORT IT
       A    R10,R3            ;ADD INFORMATION VECTOR LENGTH
       MOV  @NVS,R2           ;DEFINE
       S    R3,R2
       C    R2,@NVD           ;OK?
       JL   DIME10            ;N, STORAGE OVERFLOW
       MOV  R2,*R14           ;Y, SET IN SYMBOL TABLE
       MOV  R2,@NVS           ;UPDATE NVS
*
DIM6   MOV  *R6+,*R2+         ;MOVE INFORMATION VECTOR
       INC  @-2(6)            ;DONE?
       JNE  DIM6              ;N
*
       CLR  R0
       MOVB *R8+,R0           ;GET NEXT BYTE
*
DIM7   CI   R0,>3F00          ;",?
       JEQ  DIMY              ;Y, DO NEXT VARIABLE
       MOV  R0,@DLIM          ;N, SAVE IN DLIM
DIM8   B    @NLIN             ;RETURN
*
DIME10 DATA ERROR+10          ;STORAGE OVERFLOW
*
ERR16  DATA ERROR+16          ;EXPECTING DIMENSIONED VARIABLE
       END
