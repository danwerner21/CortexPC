       TITL 'PUT BYTE IN BUFFER - CORTEX BASIC REV. 1.1'
       IDT  'PUTB'
       REF  UFT               ;USER FUNCTION TABLE
       REF  CCNT              ;COLUMN COUNTER
       REF  IOB               ;IO BUFFER
       REF  TYP11$            ;OUTPUT IMMEDIATE BYTE
*
*       PUTB TRANSFERS LEFT BYTE IN R0 INTO
*       BUFFER (R7).  A CHECK IS MADE FOR
*       CARRIAGE RETURN AND CONTROL CHARACTERS.
*
* CALLING SEQUENCE:
*
*       BL @PUTB
*         CARRIAGE RETURN
*         CONTROL CHARACTER
*       CHARACTER
*
*       IN  (R7) = BUFFER
*            R0  = CHARACTER
*       OUT  R0  = CHARACTER OR BACKSPACE OR BELL
       PAGE
*
* ENTRY POINT:
*
       DEF  PUTB
*
PUTB   ANDI R0,>FF00          ;REMOVE LS BYTE
       CI   R0,>0D00          ;CR?
       JNE  PUTB1             ;N
       SB   *R7,*R7           ;Y,NULL TERMINATE
       RT
*
PUTB1  CI   R0,>2000          ;CONTROL?
       JHE  PUTB2             ;N
       B    @2(11)            ;Y,EXIT
*
PUTB2  CI   R0,>7F00          ;RUBOUT?
       JNE  PUTB4             ;N
       INC  R4                ;Y,ALLOW 1 MORE EMPTY CHARACTER
       C    R7,@IOB           ;RUBOUT, EMPTY?
       JH   PUTB5             ;N
PUTB3  LI   R0,>0700          ;Y, OUT BELL
       JMP  PUTB6             ;EXIT
*
PUTB4  C    R7,@UFT           ;BUFFER FULL?
       JHE  PUTB3             ;Y
       MOVB R0,*R7+           ;N,STORE BYTE
       JMP  PUTB6             ;EXIT
*
PUTB5  DEC  R7                ;BACKUP
       INC  R4                ;ALLOW CHARACTER
       DEC  @CCNT             ;ADJUST BACKSPACE COUNT
       DATA TYP11$,>0800      ;OUT BS
       DATA TYP11$,>2000      ;OUT SP
       LI   R0,>0800          ;OUT BS
PUTB6  B    @4(11)            ;EXIT
       END
