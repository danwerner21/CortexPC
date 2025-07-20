       TITL  'ON STATEMENT - CORTEX BASIC REV. 1.1'
       IDT   'ON'
*
* ROUTINE LIST:
*
*       ONY             ;ON COMMAND
*
* EXTERNAL ROUTINES:
*
        REF GOSON       ;GOSUB ENTRY
        REF LINE        ;EXIT ENTRY TO MULTIPLEXOR
        DXOP EVFIX,11   ;EVALUATE AND FIX
ERROR   EQU >2F80               ;XOP XX,14  (ERROR CALL)
*
* EXTERNAL DATA:
*
        REF B3F         ;>3F
*
* ABSTRACT:
*
*       ON COMMAND
*
* CALLING SEQUENCE:
*
*       B @ONY
*
*       EXIT TO LINE OR GOSON
*
* EXCEPTIONS AND CONDITIONS:
*
*       SYNTAX ERROR
*       EVALUATION ERRORS
*
       PAGE
*
* ENTRY POINT:
*
       DEF ONY
ONY     EVFIX R1        ;EVALUATE & FIX
        CI R0,>3B00     ;THEN?
          JNE ERR1      ;N, SYNTAX ERROR
ON1     CLR R2          ;CLEAR DESTINATION
        MOVB *R8+,R2    ;GET TYPE CODE
        SRL R2,7        TYPE CODE TO LOW BYTE & *2
*
* TYPE CODE SHOULD BE EITHER 'GOTO' OR 'GOSUB' - IN THE
* MODULE GOSUB, 'GOTO' IS IDENTIFIED BY R3=0 AND 'GOSUB'
* IS IDENTIFIED BY R3=>FFFF
*
        MOV  R2,R3                               JG 12/1/82
        DECT R3         GOTO?                    JG 12/1/82
        JEQ  ON2                                 JG 12/1/82
        AI   R3,-3      N - GOSUB?               JG 12/1/82
        JGT  ERR1       N - THEN ERROR           JG 12/1/82
*
ON2     DEC R1          ;COUNT
          JLT ON3       ;DONE
          JEQ ON4       ;DO GOTO
        INCT R8         ;MOVE TO NEXT
        CB *R8+,@B3F    ;,?
          JEQ ON2       ;Y, CONTINUE
ON3     B @LINE         ;DROP THRU
*
ON4     B @GOSON
*
ERR1    DATA ERROR+1    ;SYNTAX ERROR
       END
