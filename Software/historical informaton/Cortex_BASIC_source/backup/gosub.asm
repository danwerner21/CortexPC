        TITL 'GOSUB/GOTO COMMANDS - CORTEX BASIC REV. 1.1'
        IDT  'GOSUB'
*
* ROUTINE LIST:
*
*       GOTY            ;GOTO COMMAND
*       GOSY            ;GOSUB COMMAND
*       RTNY            ;RETURN COMMAND
*       POPY            ;POP COMMAND
*
* EXTERNAL ROUTINES:
*
        REF SFSN,CRLF,SLN
        REF LINE0,LINE2,LINE5,NLIN0
        DXOP EVFIX,11
ERROR   EQU >2F80        XOP XX,14  (ERROR CALL)
ERROR2  EQU ERROR+>20
*
* EXTERNAL DATA:
*
        REF ELNM        ;ERROR LINE NUMBER
        REF FNS         ;FOR/NEXT STACK
        REF GSC         ;GOSUB STACK COUNTER
        REF GSS         ;GOSUB STACK
        REF PLC         ;PROGRAM LINE COUNTER
        REF SLT         ;STATEMENT LOCATION TABLE
        REF BUS         ;BEGINNING USER STORAGE
        REF C4          ;>0004
*
* ABSTRACT:
*
* EXECUTE GOTO, GOSUB AND SYSTEM GOSUB COMMANDS.
* PROVIDE FOR ON COMMAND
*
*       STACK FORMAT:   PLC
*                       PBC
*
*                       PLC
*                       PBC
*
*                       ...
*
* IF PBC = 0, GOTO NEXT LINE, OTHERWISE CONTINUE EXECUTION ON
* THE SAME LINE AS THE CALL WAS MADE (NEEDED FOR ON COMMAND TO
* ENSURE YOU DO NOT JUMP BACK INTO THE PARAMETER LIST.)
*
* CALLING SEQUENCE:
*
*       B @GOSB1
*       B @GOSY
*       B @GOTY
*
*       EXIT TO LINE0 IN DEMULTIPLEXOR (RUN)
*
* EXCEPTIONS AND CONDITIONS:
*
*       STACK OVERFLOW
*       STACK UNDERFLOW
*       NO SUCH LINE #
*
        PAGE
*
* ENTRY POINT:
*
        DEF GOSY        ;GOSUB COMMAND
        DEF GOS1,GOS2
        DEF GOSB1       ;SYSTEM ENTRY
        DEF GOSON       ;ON COMMAND ENTRY
        DEF GOTY        ;GOTO ENTRY
*
* ENTRY FOR INPUT ERROR HANDLER/ERRECOVY
*
GOSB1   MOV  @PLC,R5          GET ADDRESS OF CURRENT LINE
        MOV  @-2(5),@ELNM     GET ERROR LINE #
        CLR  R6               SET FOR FIRST OF NEXT LINE
        JMP  GOS1A
*
* ENTRY FOR CONT (EDIT)
*
GOS2    MOV  @PLC,R5          GET ADDRESS OF CURRENT LINE
        JMP  GOS2A
*
* INTERRUPT ENTRY POINT (RUN)
*
GOS1    MOV  @PLC,R5          GET ADDRESS OF CURRENT LINE
        JMP  GOS1A
*
* GOTO ENTRY - CHECK SYNTAX (VALID TERMINATOR?)
*
GOTY    CLR  R3         INDICATE A GOTO             JG 12/1/82
        JMP  GOSYA
*
* GOSUB ENTRY - CHECK SYNTAX (VALID TERMINATOR?)
*
GOSY    SETO R3         INDICATE A GOSUB            JG 12/1/82
GOSYA   MOV R8,R6             ;SET TO SAVE PBC
        INCT R6
        CLR R0
        MOVB *R6+,R0          ;GET DELIMITER
          JEQ GOSY1           ;EOL
        CI R0,>3C00           ;::?
          JEQ GOSON1    ;Y                          JG 02/3/82
        CI R0,>4700           ;!?
        JNE  GOSYE            ;N - ERROR
*
* ON ENTRY POINT - IGNORE REST OF THE LINE
*
GOSON   EQU  $                                      JG 12/1/82
GOSY1   MOV  @PLC,R5  SAVE ADD OF CURRENT STMT # JG 12/1/   82
        JEQ  GOSON1      ENTERED FROM KEYBOARD MODE? JG 15/1/82
        MOV  R3,R3       N - GOTO?                  JG 12/1/82
        JEQ  GOSON1      Y                          JG 12/1/82
*
* GOSUB - NOTHING FOLLOWING STATEMENT - GET PBC OF NEXT LINE
*
        MOV  R5,R6      GOTO NEXT STATMENT          JG 12/1/82
        AI R6,-4
        C R6,@SLT    ;DONE?
          JL GOS6A       Y - SET PBC=0              JG 12/1/82
        MOV R6,@PLC  ;N, UPDATE
        MOV *R6,R6
        A @BUS,R6
GOSON1  MOVB *R8+,R1     GET LINE NUMBER
        SWPB R1
        MOVB *R8+,R1
        SWPB R1
        SRL  R2,2        GOTO?
        JEQ  GOS2A
GOS1A   MOV  @GSC,R3  N - GOSUB
        C R3,@FNS    ;ROOM?
          JHE ERR11     ;N, ERROR
        MOV R6,*R3+     ;Y, SAVE PBC OR NULL AND PLC
        MOV @PLC,*R3+
        MOV R3,@GSC  ;UPDATE GSC
GOS2A   BL  @SFSN        SEARCH FOR STATEMENT #
        B   @LINE0
*
GOS6A   CLR  R6          PBC=0                      JG 12/1/82
        JMP  GOSON1                                 JG 12/1/82
*
ERR11   DATA ERROR+11   ;STACK OVERFLOW
ERR12   DATA ERROR+12   ;STACK UNDERFLOW
GOSYE   DATA ERROR2,37  ILLEGAL DELIMITER
        PAGE
*
* ABSTRACT:
*
* POPS THE RETURN ADDRESS FROM THE GOSUB STACK (GSS) AND
* EFFECTS A TRANSFER TO THAT POPPED ADDRESS. IF THE
* UNSTACKED PLC = 0 THEN IT INDICATES THAT THE GOSUB WAS
* ENTERED DIRECTLY FROM KEYBOARD MODE.  IF THE PBC <> 0
* THEN AN EXIT IS MADE TO THE LINE DEMULTIPLEXER; OTHERWISE
* A NEW STATMENT LINE IS INTERPRETED.
*
* CALLING SEQUENCE:
*
*       B @RTNY
*
*       EXIT TO MULTIPLEXOR
*
* EXCEPTIONS AND CONDITIONS:
*
*       STACK UNDERFLOW
*
* ENTRY POINT:
*
        DEF RTNY
*
RTNY    MOV  @GSC,R3  ;SEE IF STACK EMPTY
        C    R3,@GSS  ;EMPTY?
        JLE  ERR12        Y
        DECT R3           N, POP PLC
        MOV  *R3,R5     
        DECT R3           POP PBC
        MOV  R3,@GSC      UPDATE GSC
        MOV  R5,@PLC      UPDATE THE PLC
        JEQ  RTRN3        0 - KEYBOARD MODE WHEN STACKED 
        MOV  @-2(R5),@SLN UPDATE SLN
        MOV  *R3,R8       ;GET PBC
        JEQ  RTRN2        ;0, GOTO NEXT LINE (ON)
        B    @LINE2
*
RTRN2   B    @LINE5
*
RTRN3   B    @CRLF       RETURN TO INTERPRETER LOOP 
        PAGE
*
* ABSTRACT:
*
* POP THE PLC AND PBC FROM THE GOSUB STACK AND THROW
* THE VALUES AWAY.
*
* CALLING SEQUENCE:
*
*       B @POPY
*
*       EXIT TO NLIN0
*
* EXCEPTIONS AND CONDITIONS:
*
*       STACK UNDERFLOW
*
* ENTRY POINT:
*
        DEF POPY
*
POPY   C    @GSC,@GSS         SOMETHING ON THE STACK ?
       JLE  ERR12             N, ERROR
       S    @C4,@GSC          Y, BACKUP STACK POINTER
       B    @NLIN0            CONTINUE
*
       END
