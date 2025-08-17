       TITL 'LET STATEMENT - CORTEX BASIC REV. 1.1'
        IDT 'LET'
*
* ROUTINE LIST:
*
*       LETY            ;LET COMMAND
*
        REF EVARZ       ;EVALUATE VARIABLE
        REF EVERZ       ;EVALUATE EXPRESSION
        REF EVSDZ       ;EVALUATE STRING
        REF CKEX        ;CHECK FOR EXPRESSION
        REF CVDIFZ      ;CONVERT DECIMAL TO INTEGER/FP
        REF NLIN        ;EXIT ENTRY TO MULTIPLEXOR
        REF FFLG        ;FORMATTING FLAG
        REF IOB         ;I/O BUFFER PTR
        DXOP LOADF,0    ;LOAD FPAC
        DXOP STORE,1    ;STORE FPAC
        DXOP SCALE,6    ;SCALE FPAC
        DXOP NORMAL,7   ;NORMALIZE FPAC
        DXOP EVFIX,11   ;EVALUATE AND FIX
        DXOP OUTFP,12   ;OUT FLOATING POINT #
        DXOP OUTINT,13  ;OUT INTEGER
ERROR   EQU >2F80       ;XOP XX,14  (ERROR CALL)
       PAGE
* ABSTRACT:
*
*       THE LET COMMAND IS THE ASSIGNMENT STATEMENT
*       AND CAN HAVE ANY OF THE FOLLOWING FORMS:
*
*       A[-]=B[-]               NUMERIC ASSIGNMENT
*       $A[-]=$B[-]             ASSIGNMENT
*       $A[-]=$B[-],N           PICK
*       $A[-]=$B[-];N           REPLACE
*       $A[-]=$B[-]+$C[0]       CONCATENATE
*       $A[-]=/N                DELETE
*       $A[-]=/$B[-]            INSERT
*       $A[-]=N                 CONVERT # TO ASCII
*       $A[-]=#$B[-],N          CONVERT # TO ASCII FORMATTED
*       $A[-]=%N                CONVERT BYTE
*       N=$A[-],E               CONVERT ASCII TO #
*
* CALLING SEQUENCE:
*
*       B @LETY
*
*       EXIT TO NLIN
*
* EXCEPTIONS AND CONDITIONS:
*
*       MISSING ASSIGNMENT OPERATOR
*       EVALUATION ERRORS
*
       PAGE
* ENTRY POINT:
*
       DEF LETY
*
LETY    BLWP @EVSDZ
          DATA ERROR+1  ;"
          JMP LET3      ;$
        BLWP @EVARZ     ;EVALUATE VARIABLE
        CI R0,>5600     ;=?
          JNE ERR36     ;N, ERROR
        MOV R2,R3
        BL @CKEX        ;CHECK FOR EXPRESSION
          JMP LET1      ;PROBLEM
        BLWP @EVERZ     ;EVALUATE
*
*  [VAR]=[EXP]
*
        MOV *R2+,*R3+   ;STORE RESULTS
        MOV *R2+,*R3+
        MOV *R2,*R3
LETR    B @NLIN         ;GOTO NEXT LINE
*
*  [VAR]=_
*
LET1    BLWP @EVSDZ     ;LOOK FOR STRING
          JMP LET2      ;"
          JMP LET2      ;$
        DATA ERROR+1    ;SYNTAX ERROR
*
*  [VAR]=[$VAR]_
*
LET2    MOV R2,R4       ;SAVE ADR
        CI R0,>3F00     ;,?
          JEQ LET6      ;Y, CONVERT ASCII TO BINARY
*
ERR7    DATA ERROR+7    ;N, EXPECTING OPERATER
*
*  $_           STRING ENTRY
*
LET3    CI R0,>5600     ;=?
          JEQ LET3A     ;Y
ERR36   DATA ERROR+>20  ;MISSING ASSIGNMENT OPERATER
          DATA 36
        PAGE
*BYTE ASSIGNMENT
*
*       $A[-]=%N...
*
LET11   EVFIX R1        ;GET BYTE
        SWPB R1
*
LET11A  MOVB R1,*R7+    ;SAVE #
        CI R0,>4200     ;%?
          JEQ LET11     ;Y
        JMP LETR        ;RETURN
*
*MULTIPLEXOR FOR STRING ASSIGNMENTS
*  $[VAR]=_
*
LET3A   MOV R2,R7
LET3E   BLWP @EVSDZ     ;GET STRING OR $VAR
          JMP LET4      ;"
          JMP LET4      ;$
        MOVB *R8+,R0    ;GET BYTE
        CI R0,>4200     ;%?
          JEQ LET11     ;Y
        CI R0,>5E00     ;/?
          JEQ LET12     ;Y, INSERT
        CI R0,>3E00     ;#?
          JNE LET3C     ;N
*
*CONVERT NUMBER TO ASCII
*
*       $A[-]=N
*       $A[-]=#$B[-],N
*
        BLWP @EVSDZ     ;Y, GET FORMAT
          JMP LET3B
          JMP LET3B
ERR14   DATA ERROR+14   ;ERROR, EXPECTING STRING
*
LET3B   CI R0,>3F00     ;,?
          JNE ERR7      ;N, ERROR
        MOV R2,@FFLG    ;Y, SET TO FORMAT
        JMP LET3D
*
LET3C   DEC R8          ;N, BACKUP
*
LET3D   BLWP @EVERZ     ;EVALUATE EXPRESSION
        OUTFP *R2       ;CONVERT #
        CLR @FFLG       ;CLEAR FORMATTING
        SB *R7,*R7      ;END STRING
        JMP LETR
        PAGE
*ASSIGN,PICK,REPLACE,CONCATENATE
*
*       $A[-]=$B[-]
*       $A[-]=$B[-],N
*       $A[-]=$B[-];N
*       $A[-]=$B[-]+$C[-]
*
LET4    SETO R5         ;NO NULL
        CI R0,>4000     ;;?
          JEQ LET4A     ;Y, REPLACEMENT
        CLR R5          ;SET TO NULL
        CI R0,>3F00     ;,?
          JNE LET4B     ;N, DIRECT ASSIGNMENT
        INC R5
*
LET4A   EVFIX R1        ;EVALUATE COUNT
        MOV   R1,R1     ;CHECK CONDITION
        JGT   LET4C     ; >0 THEN DO PICK
        JMP   LETR      ;ELSE IGNORE
*
LET4B   MOV @IOB,R1      ;GET MAX MOVE
        S R7,R1
        DEC  R1
*
LET4C   MOV R7,R3       ;MARK
*
LET4D   MOVB *R2+,R4    ;MOVE
          JNE LET4I     ;CHARACTER
        MOV R5,R5       ;NULL, ASSIGNMENT?
          JEQ LET4F     ;Y
LET4I   MOVB R4,*R7+
        C R2,R7         ;IS S<D?
          JHE LET4E     ;N
        C R2,R3         ;Y, IS S>=D0?
          JHE LET4F     ;Y, ABORT
LET4E   DEC R1          ;DONE?
          JNE LET4D     ;N
*
LET4F   SRL R5,1        ;DONE, NEED NULL?
          JNE LET4G     ;N
        SB *R7,*R7      ;Y
*
LET4G   CI R0,>5D00     ;+?
          JNE LETRR     ;N, RETURN
        JMP LET3E       ;Y, GET STRING
        PAGE
*ASCII TO DECIMAL
*
*       N=$A[-],E
*
LET6    BLWP @EVARZ     ;GET VARIABLE ADR
        CLR *R2
        MOV R4,R7
        BLWP @CVDIFZ    ;CONVERT TO #
          JMP LET6A
          CLR R1        ;NOTHING
          NOP
        MOVB R0,*R2     ;SAVE DELIMITER
        CLR *R3+        ;MOVE IN RESULTS
        MOV R1,*R3+
        CLR *R3
        JMP LETRR
*
LET6A   MOVB R0,*R2     ;SAVE DELIMITER
        STORE *R3       ;STORE RESULTS
        JMP LETRR       ;RETURN
        PAGE
*INSERT STRING
*
*       $A[-]=/$B[-]
*       (R7)=DESTINATION
*
LET12   BLWP @EVSDZ     ;LOOK AT NEXT STRING
          JMP LET13A
          JMP LET13A
        EVFIX R1        ;GET #
*
*DELETE CHARACTERS
*
*       $A[-]=/N
*       (R2)=(R7)=STRING
*       R1=COUNT
*
LET12A  DEC R1          ;MOVE BY HOLE
          JLT LET12B    ;DONE
        MOVB *R2+,R0    ;SKIP 1 CHARACTER
          JNE LET12A    ;CONTINUE
        DEC R2          ;EOL
*
LET12B  MOVB *R2+,*R7+  ;MOVE REST OF STRING
          JNE LET12B
        JMP LETRR
*
*INSERT STRING
*
LET13A  MOV R2,R3       ;COUNT INSERT STRING
        SETO R1
*
LET13B  INC R1          ;COUNT
        MOVB *R3+,R0
          JNE LET13B
        CLR R3          ;MOVE AND COUNT TO END OF DES STRING
*
LET13C  INC R3          ;COUNT
        MOVB *R7+,R0
          JNE LET13C
*
        MOV R7,R4       ;MOVE DES STRING
        A R1,R4
*
LET13D  DEC R7
        DEC R4
        MOVB *R7,*R4    ;MOVE CHARACTERS
        DEC R3          ;TO HOLE?
          JGT LET13D    ;N
*
LET13E  MOVB *R2+,*R7+  ;Y, INSERT STRING
        DEC R1          ;DONE?
          JGT LET13E    ;N
LETRR   B @NLIN         ;Y
       END
