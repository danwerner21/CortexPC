       TITL 'SEARCH FOR STATEMENT ROUTINE - CORTEX BASIC REV. 1.1'
       IDT  'SFSN'
*
*       SFSN            ;SEARCH FOR STATEMENT #
*
ERROR  EQU  >2F80             ;XOP XX,14  (ERROR CALL)
       REF  SLT               ;STATEMENT LOCATION TABLE
*
*       SEARCH IN STATEMENT-LOCATION-TABLE FOR THE LINE NUMBER IN R1.
*       THE SLT IS ORGANIZED AS FOLLOWS FOR STATEMENTS 100,200,300:
*
*                       SLT     300
*                               PLC
*
*                               200
*                               PLC
*
*                               100
*                               PLC
*
*                               0
*
* CALLING SEQUENCE:
*
*       BL @SFSN
*
*      IN  - R1 = #
*      OUT - R8 = PLC
*
*      NORMAL EXIT - RETURN
*      ERROR EXIT TO ERROR ROUTINE
*
* EXCEPTIONS AND CONDITIONS:
*
*       ERROR 13 RESULTS IF LINE IS NOT FOUND
       PAGE
* ENTRY POINT:
*
       DEF  SFSN
*
SFSN   MOV  @SLT,R8           ;START AT TOP
*
SFSN1  MOV  *R8+,R12          ;DONE?
       JEQ  SFSN3             ;Y
       C    R12,R1            ;FOUND?
       JEQ  SFSN2             ;Y
       INCT R8                ;N, CONTINUE
       JMP  SFSN1
*
SFSN2  B    *R11              ;RETURN
*
SFSN3  DATA ERROR+13          ;NO SUCH LINE #
       END
