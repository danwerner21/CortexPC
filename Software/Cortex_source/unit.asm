       TITL 'UNIT STATEMENT - CORTEX BASIC REV. 1.1'
       IDT  'UNIT'
*
*
*
       REF  NLIN0             ;RETURN TO MULTIPLEXOR
       DXOP EVFIX,11          ;EVALUATE AND FIX
       REF  UNIT              ;INDEX TO UNIT
       REF  ERROR2
       DEF  UNTY
*
*       THE UNIT COMMAND IS USED TO SELECT OUTPUT DEVICES
*       ACCORDING TO BIT POSITIONS WITHIN @UNIT.
*
*   BIT:   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
*  PORT:  16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1
*
*  FORMAT :
*      UNIT <ARG 1>
*
*     IF ARG 1 IS 0 THEN ALL PORTS ARE DISABLED
*     IF ARG 1 IS -VE THEN THE SPECIFIED PORT IS DISABLED
*     IF ARG 1 IS +VE THEN THE SPECIFIED PORT IS ENABLED
*
*     VALID RANGE OF ARG 1 IS 1..16
* CALLING SEQUENCE:
*
*       B @UNTY
*
*       NORMAL EXIT TO NLIN0
*
       PAGE
UNTY   EVFIX R1               GET THE UNIT #
       LI   R2,UNIT           POINT TO STORAGE
       LI   R4,>E481          R4='SOC R1,*R2'
       MOV  R1,R0             PUT UNIT # IN R0
       JEQ  NOUNIT            0, RESET UNIT FLAG
       ABS  R0
       JGT  UNITON            +VE, R4 OK TO SET BIT
       LI   R4,>4481          -VE, R4='SZC R1,*R2'
*
UNITON LI   R1,>0001          SET BIT MASK
       DEC  R0                MAKE 0..15 FOR SHIFT
       JEQ  UON1              0, MASK OK
       CI   R0,16             VALID?
       JHE  ERR               N, ERROR IT
       SLA  R1,0              Y, POSITION MASK
UON1   X    R4                SET BIT/RESET BIT
*
UEXIT  DEC  R8                BACKUP POINTER
       B    @NLIN0            EXIT
*
NOUNIT CLR  *R2               RESET ALL UNIT FLAGS
       JMP  UEXIT             AND EXIT
*
ERR    DATA ERROR2,46
       END
