       TITL 'MOVE ROUTINES - CORTEX BASIC REV. 1.1'
        IDT  'MOVE'
*
*       BMVE            ;BYTE MOVE
*
*       MOVE BYTES FROM (R2) TO (R7) WATCHING FOR
*       BACKSPACES AND HEX CHARACTERS IN
*       ANGLE BRACKETS.  ALSO WATCH FOR
*       BUFFER OVERFLOW.
*
* CALLING SEQUENCE:
*
*       BL @BMVE
*         MOVE OK
*       OVERFLOW
*
*       IN  R2 = SOURCE
*           R5 = MAX COUNT
*           R7 = DESTINATION
*       OUT R3 = -(# OF <10>)*2
*
       DEF BMVE
       REF  IOB,B08,B3C
*
BMVE    CLR R3          ;RESET BS COUNT
*
BMVE0   MOVB *R2+,*R7   ;MOVE BYTE
          JEQ BMVE2     ;DONE
        CB *R7,@B3C     ;"<?
          JEQ BMVE3     ;Y
BMVE1   INC R7          ;N
        MOV @IOB,R10     ;GET IOB
        DEC R10         ;BACKUP TO LAST BYTE
        C R7,R10        ;ABOUT TO STORE IN LAST BYTE?
          JEQ BMVE6     ;Y, QUIT
        DEC R5          ;MORE ROOM?
          JGT BMVE0     ;Y
        INCT R11        ;N, RETURN 2(11)
*
BMVE6   SB *R7,*R7      ;MARK
*
BMVE2  B    *R11
*
BMVE3   CLR R1          ;BUILD # IN R1
        MOV R2,R10      ;SAVE R2
*
BMVE4   CLR R0
        MOVB *R10+,R0   ;GET NEXT CHARACTER
        AI R0,->3000
          JLT GHEX2
        CI R0,>0900
          JLE GHEX1
        AI R0,->0700
        CI R0,>0A00
          JLT GHEX2
        CI R0,>0F00
          JH GHEX2
GHEX1   SLA R1,4        ;SHIFT R1
        AB R0,R1        ;ADD NEW DIGIT
        JMP BMVE4
*
GHEX2   EQU $
BMVE5   CI R0,>3E00->3700       ;">?
          JNE BMVE1     ;N, DISREGUARD #
       MOVB R1,R1        ;ARE WE GOING TO MOVE IN A NULL?
       JEQ  BMVE1        Y, IGNORE
        MOV R10,R2      ;Y, SET R2
        MOVB R1,*R7     ;MOVE INTO STRING
        CB R1,@B08      ;BS?
          JNE BMVE1     ;N
        DECT R3         ;Y, COUNT
        JMP BMVE1
       END
