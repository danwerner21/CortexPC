       TITL 'SIZE COMMAND - CORTEX BASIC REV. 1.1'
       IDT  'SIZE'
*
* DEFINE FLOATING POINT XOPS
*
       DXOP LOADF,0           LOAD FPAC
       DXOP STORE,1           STORE FPAC
       DXOP FADD,2            ADD TO FPAC
       DXOP FSUB,3            SUBTRACT FROM FPAC
       DXOP FMUL,4            MULTIPLY FPAC
       DXOP FDIV,5            DIVIDE FPAC
       DXOP SCALE,6           SCALE FPAC
       DXOP NORMAL,7          NORMALIZE FPAC
       DXOP CLEAR,8           CLEAR FPAC
       DXOP NEGATE,9          NEGATE FPAC
       DXOP FLOATF,10         FLOAT FPAC
       DXOP EVFIX,11          EVALUATE AND FIX
       DXOP OUTFP,12          OUT FLOATING POINT #
       DXOP OUTINT,13         OUT INTEGER
ERROR  EQU  >2F80             XOP XX,14  (ERROR CALL)
ERROR2 EQU  ERROR+>20
*
       DEF  SIZE,EVENRT
       DEF  OUTR1U,MOVEB,MOVEL
       DEF  MOVEBN,MOVELN
       REF  VDT,BUS,NVS,NVD,VNT
       REF  CRLF,TYPBE$,RTSTOR
       REF  FPAC,C4A00,IOB
*
* SIZE ROUTINE
*
SIZE   BL   @MOVEBN           ;OUT 'PRGM:'
       DATA >0A0D
       TEXT -'PRGM:'
       MOV  @NVD,R1           ;PRGM=((NVD-VDT)+VNT)-BUS
       S    @VDT,R1
       A    @VNT,R1
       S    @BUS,R1
       BL   @OUTR1U           OUT R1 UNSIGNED
       BL   @MOVBY            ;OUT 'BYTES'+'VARS:'
       TEXT -'VARS:'
       MOV  @IOB,R1           ;VARS=IOB-NVS+NVD-VDT
       S    @NVS,R1
       A    @NVD,R1
       S    @VDT,R1
       AI   R1,-8             ;REMOVE EPROM OVERHEAD
       BL   @OUTR1U           OUR R1 UNSIGNED
       BL   @MOVBY
       TEXT -'FREE:'
       MOV  @NVS,R1           ;FREE=NVS-NVD
       S    @NVD,R1
*
SIZE1  BL   @OUTR1U           ;OUT R1 UNSIGNED
       BL   @MOVBY
       BYTE 0                 ;NO FOLLOWING TEXT
       EVEN
       DATA TYPBE$            ;OUTPUT BUFFER
       B    @CRLF             ;EXIT
*
*       OUT R1 UNSIGNED     USES R0
*
OUTR1U LI   R0,FPAC           GET THE FPAC
       MOV  @C4A00,*R0+       SET EXPONENT
       CLR  *R0+              RESET 2ND WRD
       MOV  R1,*R0            LOAD UP R1
       NORMAL R0              NORMALISE IT
       OUTFP @FPAC            OUT THE FPAC
       RT                     EXIT
*
* OUT 'Bytes<CR><LF>'    THEN INLINE TEXT
*                             (-VE/NULL TERMINATOR)
*
MOVBY  MOV  R11,R1            SAVE RETURN
       BL   @MOVELN           OUT 'BYTES'
       TEXT ' Bytes'
       BYTE >0D,->0A
       MOV  R1,R11            RESTORE RETURN ADDRESS
       JMP  MOVELN            & OUT INLINE TEXT
       PAGE
*
* MOVE STRING ROUTINES - NULL TERMINATOR
*
*       BL @MOVEB       MOVE INTO IOB
*       BL @MOVEL       MOVE INTO R7
*
*       IN  0(11) = STRING
*              R7 = OBC
*       OUT    R7 = UPDATED OBC
*
MOVEB  MOV  @IOB,R7           GET BUFFER ADR
MOVEL  MOVB *R11+,*R7+        MOVE CHARACTER
       JNE  $-2               UNTIL NULL
NTERM  DEC  R7                BACKUP OVER NULL
*
* FORCE THE RETURN ADDRESS TO AN EVEN WORD BOUNDARY
*
EVENRT INC  R11
       ANDI R11,>FFFE
       RT
*
* MOVE STRING ROUTINES  - NULL/NEGATED CHARACTER TERMINATION
*
*       BL @MOVEBN      MOVE INTO IOB
*       BL @MOVELN      MOVE INTO R7
*
*       IN  0(11) = STRING
*              R7 = OBC
*       OUT    R7 = UPDATED OBC
*
MOVEBN MOV  @IOB,R7           GET BUFFER ADR
MOVELN MOVB *R11+,*R7+        MOVE CHARACTER
       JEQ  NTERM             NULL, TERMINATE
       JGT  MOVELN            +VE, CONTINUE COPY
* R7 NOW POINTS TO THE BYTE AFTER THE STRING END
       MOV  R11,@RTSTOR       SAVE R11
       CLR  R11               READY R11
       MOVB @-1(R7),R11       GET BACK THE CHARACTER
       NEG  R11               NEGATE IT TO GET REAL VALUE
       MOVB R11,@-1(R7)       RE-STORE IT
       MOV  @RTSTOR,R11       GET RETURN ADDRESS
       JMP  EVENRT            EXIT
       END
