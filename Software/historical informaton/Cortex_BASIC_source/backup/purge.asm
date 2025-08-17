       TITL 'PURGE STATEMENT - CORTEX BASIC REV. 1.1'
       IDT  'PURGE'
*
*
*
       DEF  PURGY
       REF  EMV,PLC,PRDY,SLN,SLT,VNT
       REF  GETC$,MODEOK,ESCFLG
       DXOP EVFIX,11
*
ERROR  EQU  >2F80
ERROR2 EQU  ERROR+>20
*
PURGY  BL   @MODEOK           ABORT IF RUNNING
       MOV  @PLC,R2           SAVE CURRENT LINE + 1
       JEQ  PRG0
       MOV  @-6(2),@SLN
PRG0   EVFIX R1               GET START LINE #
       CI   R0,>3800          'TO' ?
       JNE  ERR37             N, ILLEGAL DELIMITER
       EVFIX R3               GET STOP LINE #
       MOV  R3,R15            SAVE END
PRG2   MOV  @VNT,R14          GET TABLE ADR
       DECT R14               BACKUP
PRG1   C    R14,@SLT          DONE?
       JLE  PRG3              Y
       AI   R14,-4            N, MOVE TO NEXT ENTRY
       C    R1,*R14           SAME ?
       JGT  PRG1              N, CONTINUE
       C    R14,@SLT          DONE?
       JL   PRG3              Y
       C    *R14,R15          REACHED END?
       JGT  PRG3              Y
       CLR  R0
       MOV  *R14,R1           GET LINE #
       SETO @ESCFLG           DISABLE ESCAPE KEY
       BL   @EMV              DELETE LINE
       DATA GETC$             CHARACTER ?
       JMP  PRG2              N, CONTINUE
       JMP  PRG2              Y, NORMAL CHARACTER - IGNORE
PRG3   B    @PRDY             ESCAPE, EXIT
*
ERR37  DATA ERROR2,37         ILLEGAL DELIMITER
       END
