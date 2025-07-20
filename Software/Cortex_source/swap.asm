       TITL 'COLOUR SWAP ROUTINE - CORTEX BASIC REV. 1.1'
       IDT  'SWAP'
*
       DEF  SWAPY
*
       REF  VMODE,NLIN0,SWPTBL,SENDAD
*
ERROR  EQU  >2F80
ERROR2 EQU  ERROR+>20
       COPY 'IODEFS.INC'
*
*      THIS ROUTINE READS THE COLOUR TABLE OF
*      THE VDP AND SWAPS THE FGND & BGND COLOURS
*      WITH THE COLOUR SET UP IN THE 16 BYTE
*      COLOUR TABLE 'SWPTBL'
*      THIS TABLE IS COPIED INTO INTERNAL RAM FOR SPEED
*
SWAPY  MOV  @VMODE,R0         GRAPHICS MODE?
       JNE  SWAP1             Y, EVERYTHING OK
*
       DATA ERROR2,48         ILLEGAL IN CURRENT MODE
*
*      COPY THE SWAP TABLE INTO INTERNAL RAM
*      TO SPEED UP THE ROUTINE (12K ACCESSES TO IT !!)
*
SWAP1  LI   R1,SWPTBL         REF THE SWAP TABLE
       LI   R2,>F000          PUT IT IN INTERNAL RAM
       SETO R3                SET COUNT FOR 16 BYTES
SWAP1A MOV  *R1+,*R2+         COPY IT
       SRL  R3,2              DONE?
       JNE  SWAP1A            N, LOOP
*
       MOV  R8,R1             SAVE PBC
       LI   R8,CTBA1          REF THE COLOUR TABLE
       LI   R2,>1800          SET UP TABLE SIZE
*
SWAP2  BL   @SENDAD           GIVE VDP THE ADDRESS
SWAP2A INC  R8                ADJUST R8
SWAP2B MOVB @VRAM,R3          READ THE BYTE
       MOV  R3,R5             SAVE FOR LATER
       SRL  R3,8              PUT IT IN LSB
       MOV  R3,R4             SAVE IT
       ANDI R3,>000F          ISOLATE BACKGROUND COLOUR
       MOVB @>F000(3),R0      GET ITS NEW COLOUR
       SRL  R0,4              POSITION IT
       SRL  R4,4              POSITION FGND COLOUR CODE
       MOVB @>F000(4),R0      GET ITS NEW COLOUR
       SLA  R0,4              POSITION COLOUR BYTE
       CB   R0,R5             SAME?
       JEQ  SWAP2A            Y, DONT UPDATE
       AI   R8,>4000-1        SET WRITE BIT
       BL   @SENDAD           GIVE ADDRESS TO VDP
       AI   R8,-(>4000-1)     SET VDP FOR READ OF NEXT BYTE
       MOVB R0,@VRAM          WRITE IT BACK TO THE VDP
       DEC  R2                DONE?
       JNE  SWAP2             N,LOOP
*
       MOV  R1,R8             RESTORE PBC
       B    @NLIN0            NEXT LINE
       END
