       TITL 'ERROR HANDLER - CORTEX BASIC REV 1.1'
       IDT  'ERROR'
*
*
*
       DEF  ERRS,STPY
       DEF  STPYA,ERRY
*
       REF  CRLF,EFLG,ENUM,RTSTOR,TBEND
       REF  EVSKB,TYPS$,ESCFLG,PLF,GOSB1,MODE,B2E,NLIN
       REF  TYP11$,MOVEB,MOVEL,PLC,SLN,CPYST
       REF  TYPSN$,TYPBE$,TYPC$,TYPEN$,IOB
       REF  NOERR,CRLF1T,CRLF2T,SYSERR,D10
       REF  ERR00T,ERR01T,ERR02T,ERR03T,ERR04T,ERR05T
       REF  ERR06T,ERR07T,ERR08T,ERR09T,ERR10T,ERR11T
       REF  ERR12T,ERR13T,ERR14T,ERR15T,ERR16T,ERR17T
       REF  ERR18T,ERR19T,ERR20T,ERR21T,ERR22T,ERR23T
       REF  ERR24T,ERR25T,ERR26T,ERR27T,ERR28T,ERR29T
       REF  ERR30T,ERR31T,ERR32T,ERR33T,ERR34T,ERR35T
       REF  ERR36T,ERR37T,ERR38T,ERR39T,ERR40T,ERR41T
       REF  ERR42T,ERR43T,ERR44T,ERR45T,ERR46T,ERR47T
       REF  ERR48T,ERR49T
       DEF  ERRLS2
       REF  SLT,FFLG,STACNT
       REF  D$RTWP,CRASH$,WAIT$
       DXOP OUTINT,13
       DXOP EVFIX,11
       COPY 'IODEFS.INC'
       PAGE
*
* ERROR HANDLER CALLED FROM EDER IN GETLINE MODULE
*
ERRLS2 MOV  R11,@RTSTOR       SAVE RETURN
       CKOF
       DATA TYPSN$,CRLF1T     OUT 'CRLF** '
       CLR  @ESCFLG           ENABLE ESCAPE
       MOV  R3,R0             GET ASCII ERROR NUMBER
       SWPB R0                RJ TENS DIGIT
       ANDI R0,>000F          MASK
       MPY  @D10,R0           CONVERT
       MOV  R3,R10            COPY ERROR NUMBER
       ANDI R10,>000F         MASK
       A    R10,R1            ADD IN
       MOV  @MODE,R10         IDLE ?
       JEQ  GETRR             Y, OK
       MOV  R1,R2             N, PUT ERROR # IN R2
       DATA TYP11$,>0D00      OUT A 'CR' TO COVER '** '
       JMP  ERRS0             AND DO PROPPER ERROR
GETRR  CI   R1,MAXERR         VALID?
       JLE  GETER4            Y, LOOKUP
       LI   R2,SYSERR         N, GET 'SYTEM ERROR'
       JMP  GETER3
GETER4 A    R1,R1             GET WORD INDEX INTO TABLE
       MOV  @ERRTBL(1),R2     GET ERROR TEXT ADDRESS
GETER3 LI   R7,EVSKB          PUT THE MSG IN THE EVAL STACK
       BL   @MOVE$R           GO COPY IT
       SBO  ROMON-PIO         TURN ROM OFF
       SB   *R7,*R7           PUT NULL ON END OF ERR MSG
       DATA TYPS$,EVSKB       OUTPUT IT
       LI   R1,CRLF2T         POINT TO ' ** '
       DATA TYPEN$            OUT STRING
       DATA TYPC$             OUT CRLF
       MOV  @RTSTOR,R11       GET RETURN ADDRESS
       B    *R11              RETURN
       PAGE
*
* ERROR RECOVERY ROUTINE
*
ERRS   MOV  @-2(14),R2        GET INSTRUCTION
       ANDI R2,>3F            ISOLATE ERROR #
       CKOF
*
* COPY BACK THE ERROR MESSAGES
*
       LI   R1,CPYST          REF START
       ASMIF PIO
       LI   R12,PIO
       ASMELS
       CLR  R12
       ASMEND
*
CPY1   LIMI 0                 NO INTERRUPTS
       SBZ  ROMON-PIO         ROM ON
       MOV  *R1,*R1+          COPY WORD
       SBO  ROMON-PIO         ROM OFF
       LIMI 15                ENABLE INTERRUPTS
       CI   R1,TBEND          DONE COPY ?
       JL   CPY1              N, LOOP
*
ERRS0  MOV  R2,@ENUM          SAVE IT
       CLR  @FFLG             KILL FORMATTING FLAG
       CLR  @PLF              KIL LOAD/SAVE FLAG
       MOV  @EFLG,R1          ERROR COMMAND EXECUTED?
       JEQ  ERRS01
       CLR  @EFLG             Y - RESET ERROR HANDLER
       B    @GOSB1            DO SYSTEM GOSUB
ERRS01 MOV  R2,R1             GET ERROR NUMBER
       MOV  @IOB,R7           GET I/O BUFFER
       LI   R2,CRLF1T         POINT TO 'CRLF** '
       BL   @MOVE             COPY INTO BUFFER
       CI   R1,MAXERR         VALID ERROR ?
       JLE  ERRL1             Y, LEAVE ALONE
       LI   R2,SYSERR         N, GET SYSTEM ERROR
       JMP  ERRL2             & OUTPUT
ERRL1  A    R1,R1             WORD INDEX
       MOV  @ERRTBL(1),R2     GET ERROR TEXT
ERRL2  BL   @MOVE             OUT TEXT
       LI   R2,CRLF2T         OUT ' ** '
       BL   @MOVE             OUTPUT IT
ERRS02 MOV  @MODE,R0          CHECK MODE
       JEQ  ERRS2             IDLE
       MOV  @SLN,R4           RUN. PICK UP LINE #
       JMP  ERRS1
       PAGE
*
*    ENTRY POINT FOR STOP STATEMENT
*
STPYA  MOV  @PLC,R2
       AI   R2,-4             CHECK TO SEE IF THERE IS A
       C    R2,@SLT           STATEMENT AFTER THE STOP
       JL   STPY
       MOV  @SLN,R4
       MOV  @-2(R2),@SLN      SAVE LINE # FOR CONT
       JMP  STPY1
*
* ENTRY POINT FOR ESCAPE KEY OR END
*
STPY   MOV  @SLN,R4
STPY1  BL   @MOVEB
       DATA >0A0D
       TEXT 'Stop '
       BYTE 0
*
ERRS1  SETO @ENUM             NON-FATAL ERROR
       BL   @MOVEL
       TEXT 'at '
       BYTE 0
ERRS1A OUTINT R4              OUT CURRENT LINE #
       MOV  @STACNT,R1        GET STATEMENT INDENT COUNT
       JEQ  ERRS2             0, LEAVE LINE No.
       MOVB @B2E,*R7+         OUT '.'
       OUTINT R1              OUT INDENT COUNT
*
ERRS2  MOV  @ENUM,R1          GET ERROR NUMBER
       JNE  ERRS2A            NON-FATAL, SKIP
       LI   R1,D$RTWP         FATAL, POINT TO A RTWP
       MOV  R1,@>FFFE         KILL OFF NMI
ERRS2A DATA TYPBE$            OUTPUT BUFFER
       MOV  @ENUM,R1          GET ERROR NUMBER
       JEQ  ERRS2B            0, CRASH
       B    @CRLF             <>0, RETURN TO CRLF
ERRS2B DATA WAIT$             WAIT FOR I/O TO COMPLETE
       B    @CRASH$           CALL SYSTEM CRASH HANDLER
*
MOVE$R LIMI 0                 NO INTERRUPTS
       ASMIF PIO
       LI   R12,2*PIO         LOAD CRUBASE
       ASMELS
       CLR  R12
       ASMEND
       SBZ  ROMON-PIO         TURN ROM ON
MOVE   EQU  $
       MOVB *R2+,*R7+         MOVE CHARACTER
       JGT  MOVE              +VE, COPY
       JLT  NTERM             -VE, SPECIAL EXIT PATH
*                             0,   EXIT
       DEC  R7                BACKUP BUFFER POINTER
DONIT  B    *R11              RETURN
*
NTERM  CLR  R9                -VE,READY HOLDING REG
       MOVB @-1(R7),R9        GET BACK BYTE
       NEG  R9                RESTORE IT
       MOVB R9,@-1(R7)        PUT IT BACK
       JMP  DONIT             EXIT
       PAGE
*
*   ERROR COMMAND
*
ERRY   MOV  @MODE,R1          IDLE?
       JEQ  ERRY1             Y, TOGGLE ENABLE FLAG
       EVFIX @EFLG            SAVE ERROR HANDLER LINE NUMBER
       B    @NLIN             NEW LINE
ERRY1  INV  @NOERR            TOGGLE FLAG
       B    @CRLF             EXIT
       PAGE
ERRTBL DATA ERR00T
       DATA ERR01T
       DATA ERR02T
       DATA ERR03T
       DATA ERR04T
       DATA ERR05T
       DATA ERR06T
       DATA ERR07T
       DATA ERR08T
       DATA ERR09T
       DATA ERR10T
       DATA ERR11T
       DATA ERR12T
       DATA ERR13T
       DATA ERR14T
       DATA ERR15T
       DATA ERR16T
       DATA ERR17T
       DATA ERR18T
       DATA ERR19T
       DATA ERR20T
       DATA ERR21T
       DATA ERR22T
       DATA ERR23T
       DATA ERR24T
       DATA ERR25T
       DATA ERR26T
       DATA ERR27T
       DATA ERR28T
       DATA ERR29T
       DATA ERR30T
       DATA ERR31T
       DATA ERR32T
       DATA ERR33T
       DATA ERR34T
       DATA ERR35T
       DATA ERR36T
       DATA ERR37T
       DATA ERR38T
       DATA ERR39T
       DATA ERR40T
       DATA ERR41T
       DATA ERR42T
       DATA ERR43T
       DATA ERR44T
       DATA ERR45T
       DATA ERR46T
       DATA ERR47T
       DATA ERR48T
       DATA ERR49T
MAXERR EQU  ($-ERRTBL-2)/2    HIGHEST USED BASIC ERROR
       END
