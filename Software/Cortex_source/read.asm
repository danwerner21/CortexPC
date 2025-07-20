       TITL 'READ STATEMENT - CORTEX BASIC REV. 1.1'
       IDT  'READ'
*
*
*
       REF  BMVE              ;MOVE BYTE
       REF  CKEX              ;CHECK FOR EXPRESSION
       REF  EVARZ             ;GET VARIABLE
       REF  EVERZ             ;EVALUATE EXPRESSION
       REF  EVSDZ             ;EVALUATE STRING
       REF  NLIN              ;EXIT TO MULTIPLEXOR
       REF  NLIN0             ;EXIT TO MULTIPLEXOR
       REF  REST              ;RESTORE DATA PTR
       REF  SFSN              ;SEARCH FOR STATEMENT NUMBER
       DXOP EVFIX,11          ;EVALUATE AND FIX
ERROR  EQU  >2F80             ;XOP XX,14  (ERROR CALL)
*
       REF  BUS               ;BEGINNING OF USER STORAGE
       REF  DATXB             ;DATA INTERNAL BYTE CODE
       REF  DDM               ;DATA DELIMITER
       REF  DLC               ;DATA LINE COUNTER
       REF  DLIM              ;LINE DELIMITER
       REF  SLT               ;STATEMENT LOCATION TABLE
       REF  LNSZ
       DEF  RDDY,RNWY
*
* ABSTRACT:
*
*       READ COMMAND USES FDATA TO PICK
*       UP NEXT ITEM FROM DATA STATEMENTS.
*       SINCE THE EVALUATOR IS USED, ANY
*       EXPRESSION CAN APPEAR IN THE DATA
*       STATEMENT.
*
* CALLING SEQUENCE:
*
*       B @RDDY
*       B @RNWY
*
*       EXIT TO NLIN
*
* EXCEPTIONS AND CONDITIONS:
*
*       ERROR 24 FOR READING INTO STRING CONSTANT
*       ERROR 23 FOR READ WITHOUT DATA
       PAGE
*FIND DATA
*       BL @FDATA
*         STRING
*       #
*
FDATA  MOV  R8,R6             ;SAVE PBC
       MOV  @DLIM,R5          ;SAVE DLIM
       MOV  @DDM,R8           ;GET DELIMITER
       JEQ  FDATA1            ;MOVE TO NEXT LINE
       CLR  R0
       MOVB *R8+,R0           ;LOOK AT DELIMITER
       JEQ  FDATA1            ;EOF, MOVE TO NEXT
       CI   R0,>3F00          ;,?
       JEQ  FDATA2            ;Y, EVALUATE
*
ERR23  DATA ERROR+23          ;READ W/O DATA
*
FDATA1 MOV  @DLC,R8           ;GET DLC
       AI   R8,-4             ;MOVE TO NEXT LINE
       C    R8,@SLT           ;ANY MORE LINES?
       JL   ERR23             ;N, ERROR
       MOV  R8,@DLC           ;Y, SAVE DLC
       MOV  *R8,R8            ;GET PBC
       A    @BUS,R8           ;MAKE DISPLACEMENT INTO POINTER
       CB   *R8+,@DATXB       ;DATA STATEMENT?
       JNE  FDATA1            ;N, CONTINUE LOOKING
*
FDATA2 BLWP @EVSDZ            ;EVALUATE
       JMP  FDATA3            ;"
       JMP  FDATA3            ;$
       BLWP @EVERZ            ;#
       INCT R11               ;RETURN 2(11)
*
FDATA3 DEC  R8                ;BACKUP TO DELIMITER
       MOV  R8,@DDM           ;SAVE IN DELIMITER PTR
       MOV  R6,R8             ;RESTORE R8
       MOV  R5,@DLIM          ;RESTORE DLIM
       RT
       PAGE
*READ COMMAND
*
RDDY   BLWP @EVSDZ            ;SEE IF STRING
ERR24  DATA ERROR+24          ;", ERROR
       JMP  RDD2              ;$
       BLWP @EVARZ            ;WANTS #
       MOV  R2,R7             ;SAVE
       BL   @FDATA            ;FIND DATA
       JMP  ERR24             ;STRING, ERROR
       MOV  *R2+,*R7+         ;STORE NUMBER
       MOV  *R2+,*R7+
       MOV  *R2,*R7
*
RDD1   MOV  @DLIM,R0          ;LOOK AT DELIMITER
       CI   R0,>3F00          ;,?
       JEQ  RDDY              ;Y, DO AGAIN
       B    @NLIN             ;N
*
RDD2   MOV  R2,R7             ;SAVE ADR
       BL   @FDATA            ;LOOKING FOR STRING DATA
       JMP  RDD3              ;FOUND
       JMP  ERR24
*
RDD3   LI   R5,LNSZ           ;MAX # OF CHARACTERS
       BL   @BMVE             ;MOVE FROM *R2 TO *R7
       JMP  RDD1
       PAGE
*
*RESTOR COMMAND
*
RNWY   BL   @REST             ;RESTORE TO PROGRAM BEGINNING
       BL   @CKEX             ;CHECK FOR EXPRESSION
       JMP  RNWY1             ;NONE
       EVFIX R1               ;FIX PARAMETER
       MOV  R8,R2             SAVE PBC
       BL   @SFSN             ;LOOK FOR STATEMENT #
       C    *R8+,*R8+         ;MOVE BACK 1 LINE
       MOV  R8,@DLC           ;SAVE PLC
       MOV  R2,R8             RESTORE PBC
       B    @NLIN
RNWY1  B    @NLIN0
       END
