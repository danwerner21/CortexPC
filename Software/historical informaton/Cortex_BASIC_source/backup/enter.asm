       TITL 'ENTER STATEMENT - CORTEX BASIC REV. 1.1'
       IDT  'ENTER'
*
* 
*
       REF  EDIT,EVSDZ,LNSZ,IOB,LINE,C1,ESCFLG
       REF  PLC,SLN,SFSN
       DEF  ENTERY
*
ERROR  EQU  >2F80
ERROR2 EQU  ERROR+>20
*
ENTERY BLWP @EVSDZ            ;LOOK FOR STRING
       JMP  ENT1              ; " OR $
       JMP  ENT1
       DATA ERROR+14          ;EXPECTING STRING VARIABLE
*
ENT1   MOV  @IOB,R7           ;GET I/O BUFFER START
       LI   R5,LNSZ           ;GET MAX. SIZE
*
ENT2   MOVB *R2+,*R7+         ;COPY BYTE OVER
       JEQ  ENT3              ;NULL, END OF STRING
       DEC  R5                ;CHARACTER, STILL ROOM?
       JNE  ENT2              ;Y, CONTINUE COPY
       DATA ERROR2,35         ;N, PARAMETER ERROR
*
ENT3   MOV  @PLC,R5           ;GET CURRENT PLC
       MOV  @-2(R5),@SLN      ;SAVE CURRENT LINE #
       BL   @EDIT             ;EDIT I/O BUFFER
       SZC  @C1,@ESCFLG       ;REMOVE NOESC
       MOV  @SLN,R1           ;GET BACK MY LINE #
       BL   @SFSN             ;GO FIND IT
       MOV  R8,@PLC           ;SAVE MY NEW PLC
       B    @LINE             ;CONTINUE EXECUTION
       END
