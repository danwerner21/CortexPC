       TITL 'IF STATEMENT - CORTEX BASIC REV. 1.1'
       IDT 'IF'
*
*       IFY             ;IF COMMAND
*
        REF EVERZ       ;EVALUATE EXPRESSION
        REF EVSDZ       ;EVALUATE STRING
        REF LINE,NLIN   ;MULTIPLEXOR ENTRY POINTS
        REF ELSF        ;ELSE FLAG
        DEF IFY
*
        DXOP EVFIX,11   ;EVALUATE AND FIX
ERROR   EQU >2F80       ;XOP XX,14  (ERROR CALL)
ERROR2  EQU ERROR+>20
*
*       EXECUTE THE IF COMMAND.  THE FORM IS:
*
*       IF <EXP> THEN <STATEMENT>
*       IF <STRING> THEN <STATEMENT>
*       IF <STRING> <RELATION> <STRING> THEN <STATEMENT>
*       IF <STING> <RELATION> <STRING> , <EXP>
*               THEN <STATEMENT>
*
* CALLING SEQUENCE:
*
*       B @IFY
*
*       EXIT TO LINE OR NLIN
*
* EXCEPTIONS AND CONDITIONS:
*
*       ILLEGAL DELIMITER, SYNTAX ERROR
*
       PAGE
IFY     CLR @ELSF       ;CLEAR ELSE FLAG
        BLWP @EVSDZ     ;CHECK FOR " OR $
          JMP IF2
          JMP IF2
        BLWP @EVERZ     ;#
        MOV *R2+,R1
          JNE IFRT
        MOV *R2,R1
          JNE IFRT
        JMP IF13
*
IF2     CI R0,>3B00     ;THEN?
          JNE IF4       ;N
*
IF3     MOVB *R2,R1     ;Y, STRING?
          JNE IFRT      ;Y, CONTINUE
        JMP IF13                ;N, GOTO LINE
*
IF4     SWPB R0
        AI R0,->55      ;LEGAL?
          JGT IF5       ;Y
IFE37   DATA ERROR2,37  ;N
*
IF5     CI R0,>6
          JGT IFE37     ;N
        MOV R0,R6
        MOV R2,R3       ;STRINGS
        SETO R5
        BLWP @EVSDZ     ;GET SECOND OPERAND
          JMP IF7       ;"
          JMP IF7       ;$
        DATA ERROR+14   ;EXPECTING STRING
*
IFRT    B @NLIN
*
IF6     MOVB *R2+,R1    ;LOAD BYTE FROM SECOND STRING
          JNE $+4       ;OK
          SETO R1       ;NULL
        SB R4,R1        ;SAME BYTE?
          JNE IF10      ;N
        DEC R5          ;Y, DONE?
          JEQ IF11      ;Y
*
IF7     CI R0,>3F00     ;,?
          JNE IF8       ;N
        EVFIX R5        ;Y, GET COUNT
*
IF8     LI R0,4         ;GET MATCH
        MOVB *R3+,R4    ;SOURCE NULL?
          JNE IF6       ;N
*
IF9     MOVB *R2+,R1    ;LOAD BYTE FROM SECOND STRING
IF10      JGT IF12      ;POSITIVE
          JNE $+4       ;NEGATIVE
IF11    DEC R0          ;ZERO
        DECT R0
*
IF12    COC R0,R6       ;CONDITION MET?
          JEQ IFRT      ;Y
IF13    SETO @ELSF      ;N
        B @LINE
       END
