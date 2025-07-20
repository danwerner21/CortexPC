       TITL 'TEXT & CONSTANTS - CORTEX BASIC REV. 1.1'
       IDT  'TEXT'
       COPY 'IODEFS.INC'
       DEF  B4A,B0D,B1B,B2E,B56,B62,CRDELY
       DEF  IDRUN,MBEGN,MCRLF,MPRDY,STRTC,B00
       DEF  EXT$ID,CPYST,B3C,B3A,BFA,BF1,B4C,B47
       DEF  CASRDY,CRLF1T,CRLF2T,SYSERR
       DEF  ERR00T,ERR01T,ERR02T,ERR03T,ERR04T
       DEF  ERR05T,ERR06T,ERR07T,ERR08T,ERR09T
       DEF  ERR10T,ERR11T,ERR12T,ERR13T,ERR14T
       DEF  ERR15T,ERR16T,ERR17T,ERR18T,ERR19T
       DEF  ERR20T,ERR21T,ERR22T,ERR23T,ERR24T
       DEF  ERR25T,ERR26T,ERR27T,ERR28T,ERR29T
       DEF  ERR30T,ERR31T,ERR32T,ERR33T,ERR34T
       DEF  ERR35T,ERR36T,ERR37T,ERR38T,ERR39T
       DEF  ERR40T,ERR41T,ERR42T,ERR43T,ERR44T
       DEF  ERR45T,ERR46T,ERR47T,ERR48T,ERR49T
       DEF  FP5E5,TAPERR,AUTORN,INM1,INM2,INM3
       DEF  TIMC,TMPBUF,TBEND
       DEF  C0004,C4600,C4A00,B3F,B7F
       DEF  BFF,B2D,B45,B31,B2A,B40,CF0,C7F,CFF80
       DEF  BADTER,BADADR
       DEF  BELL,SPACE5,SPACE2,BPMSG
       DEF  LOGON,SPR,REGSTR,ASKBP,EXMSG,SPBP
       DEF  STPMSG,SYMBLC
       DEF  PROMPT,WS,EQUSGN
       DEF  START,PADIT,SPACE8,PGMFND,IDTEQ
*
       REF  P$EUS,P$FNS,P$GSS,P$UFT,P$IOB,CRASH$,WP10L
       REF  CONFIG,PRAM,BRAM,MEMSIZ,DEVTBL,DTEND
       REF  SRATE,NEWP,SETVEC,C1,IOB
*
* DECREMENTER DECREMENTED EVERY 4TH CLKOUT CYCLE RUNNING
* AT 3 MHZ
*
TIMC   DATA 7500              = 10 MS FOR DECREMENTER
*
C4A00  DATA >4A00
C4600  DATA >4600
C0004  DATA 4
IDRUN  EQU  >A5A5
STRTC  DATA IDRUN             AUTO RUN CODE
CRDELY DATA 25                250MS CR DELAY COUNT
EXT$ID DATA >94C2             EXTENDED CMD HEADER ID
B45    EQU  $
FP5E5  DATA >457A,>1200,>0000  FLOATING POINT  500,000
CF0    DATA >00F0
C7F    DATA >007F
BFF    EQU  $
CFF80  DATA >FF80
       EVEN
B47    BYTE >47               '!'   BLUE ON CYAN COL. CODE
PGMFND BYTE >0D,>0A
       TEXT 'Found "'
       BYTE 0
*
B4C    BYTE >4C
B31    BYTE >31               '1'
B1B    BYTE >1B               'ESC'
BFA    BYTE >FA               =16MS FOR 9902 TIMER
B3C    BYTE >3C               '<'
B4A    BYTE >4A               'J'
B56    BYTE >56               'V'
B62    BYTE >62               'b'
BF1    BYTE >F1
B7F    EQU  C7F+1             CURSOR CHARACTER
*
B0D    EQU  $
MBEGN  BYTE >0D,>0A           ;PROMPT
       TEXT 'CORTEX BASIC Rev'
B2E    EQU  $                 '.'
       TEXT '. 1.1  '
       BYTE >88
       TEXT ' 1982'
MPRDY  BYTE >0D,>0A
       TEXT '*Ready'
MCRLF  BYTE >0D,>0A
B00    BYTE 0
*
TAPERR BYTE >0D,>0A
       TEXT '** Tape read error'
CRLF2T TEXT -' ** '
CASRDY BYTE >0D,>0A
       TEXT 'Cassette ready '
B3F    EQU  $                 '?'
       TEXT -'? (Y/N) '
AUTORN BYTE >0D,>0A
       TEXT 'Auto'
B2D    EQU  $                 '-'
       TEXT -'-Run ? (Y/N) '
CRLF1T BYTE >0D,>0A
B2A    EQU  $
       TEXT -'** '
INM2   TEXT '?'
INM1   TEXT '? '
       BYTE 0
B3A    EQU  $                 ':'
INM3   TEXT ': '
       BYTE 0
       PAGE
************************************************************
*
* MONITOR MESSAGES
*
BELL   BYTE >07,0
       EVEN
PADIT  DATA >0D0A
SPACE8 TEXT '   '           !!! MUST BE WORD ALIGNED !!!
SPACE5 TEXT ' '
       TEXT '  '
SPACE2 TEXT ' '
       TEXT ' '
       BYTE 0
BPMSG  BYTE >D,>A,7
       TEXT 'BP'
       BYTE 0
LOGON  BYTE >D,>A
       TEXT 'Monitor Rev. 1.1 '
       BYTE >88
       TEXT ' 1982'
       BYTE 0
SPR    TEXT ' R'
REGSTR EQU  $-1
       BYTE 0
ASKBP  TEXT ' Set BPs?'
       BYTE 0
EXMSG  TEXT ' Execute'
       BYTE 0
SPBP   TEXT ' BP'
       BYTE 0
STPMSG BYTE >D,>A
       TEXT 'SS:-'
       BYTE 7,0
B40    EQU  $
SYMBLC TEXT '@>'
       BYTE 0
PROMPT BYTE >D,>A
       TEXT '[]'
       BYTE 0
WS     TEXT 'WP='
       BYTE 0
       TEXT 'PC='
       BYTE 0
       TEXT 'ST'
EQUSGN TEXT '='
       BYTE 0
IDTEQ  TEXT 'IDT='
       BYTE 0
BADTER TEXT -'Illegal terminator'
BADADR TEXT -'Invalid address'
       PAGE
***********************************************************
***********************************************************
***********************************************************
***                                                     ***
***                   W A R N I N G                     ***
***                 =================                   ***
***                                                     ***
***  THE AREA BETWEEN 'TMPBUF' AND 'TBEND' IS USED      ***
***  DURING NORMAL PROGRAM EXECUTION. ACCESS TO THIS    ***
***  AREA MUST THEREFORE BE MADE WITH THE EPROM TURNED  ***
***  ON.                                                ***
***                                                     ***
***********************************************************
***********************************************************
***********************************************************
*
TMPBUF EQU  $                 <====== START OF SCRATCH AREA
*
PTRTBL DATA P$IOB,P$EUS,P$FNS,P$GSS,P$UFT
************************************************************
*                                                          *
*                 SYSTEM COLDSTART                         *
*                                                          *
************************************************************
*
*    THIS ROUTINE MUST :-
*      1.   CHECKSUM THE EPROMS TO MAKE SURE THEY ARE OK
*      2.   COPY THE EPROMS INTO RAM AND TURN THEM OFF
*      3.   RESET THE KEYBOARD INTERRUPT LATCH
*      4.   INITIALISE THE MEMORY MAPPER REGISTERS
*      5.   AUTOSIZE THE RAM
*
*
************************************************************
*                                                          *
*      CHECKSUM THE EPROMS AND GENERATE A SYSTEM           *
*      CRASH IF THIS FAILS. THIS ALSO COPIES THE           *
*      EPROMS INTO RAM.                                    *
*                                                          *
************************************************************
START  EQU  $                 RESET ENTRY POINT
       CLR  R1                START FROM 0
       LI   R2,CHKWRD         POINT TO THE CHECKWORD
       MOV  R2,R0             INITIALISE CHECKSUM
L0     MOV  R1,R4             SAVE THE COPY POINTER
       ANDI R4,>FFFE          KILL LS BIT
       C    R4,R2             ARE WE AT THE CHECKWORD?
       JEQ  L2                Y, DONT ADD IT IN
       AB   *R1,@WP10L        ADD INTO R0 LSB
       JOP  L1                ODD PARITY, MODIFY CHECKWORD
       XOR  R1,R0             MODIFY THE CHECKSUM
L1     SRC  R0,0              FRIG THE CHECKWORD
L2     MOVB *R1,*R1+          COPY THE ROM WORD INTO RAM
       CI   R1,>6000          DONE?
       JL   L0                N, LOOP
       CLR  R1
       ASMIF PIO
       LI   R12,2*PIO         POINT TO PARALLEL I/O
       ASMELS
       CLR  R12
       ASMEND
       C    R0,*R2            DOES IT MATCH?
       JEQ  RAMST             Y, GO TURN ROM OFF <<<<<<<
       SBO  BELLON-PIO        N, TURN THE BELL ON
       B    @CRASH$               GO DIE!!!!
CHKWRD DATA >1B70             REV 1.1  CHECSUM WORD
************************************************************
*                                                          *
*      RESET THE I/O  AND TURN THE ROMS OFF                *
*                                                          *
************************************************************
RAMST  LI   R0,>0360               R0='RSET'
       LI   R1,>1D00+(KBDACK-PIO)  R1='SBO KBDACK-PIO'
       LI   R2,>1D00+(ROMON-PIO)   R2='SBO ROMON-PIO'
       LI   R3,>045B               R3='RT'
       BL   R0                TURN OFF EPROMS
************************************************************
*                                                          *
*      NOW INITIALISE THE MAPPER & SET CONFIG FLAGS        *
*                                                          *
************************************************************
       LI   R7,>0001          SET FOR MAPPER PRESENT
       LI   R12,MAPPER        POINT TO THE MAPPER
       SETO R8                FUDGE FOR NEXT INSTRUCTION
SETMAP INC  R8                NEXT REGISTER
       MOV  R8,*R12+          INIT. REGISTER
       SWPB R8                TEST JUST IT'S MSB
       CB   @-1(12),R8        WAS IT WRITTEN OK?
       JEQ  MAPOK             Y, LEAVE MAPPER FLAG
       CLR  R7                N, RESET MAPPER FLAG
MAPOK  SWPB R8                RESTORE R8
       CI   R12,M$REGF        HAVE WE DONE WITH THE MAPPER?
       JLE  SETMAP            N, LOOP
NOPOP  MOV  R7,@CONFIG        SETUP CONFIGURATION FLAGS
************************************************************
*                                                          *
*                     AUTO-SIZE RAM                        *
*                                                          *
************************************************************
       STWP R2                GET START OF AUTOSIZE
       LI   R4,>AAAA          SET TEST PATTERN
       LI   R5,PRAM           SET PRAM POINTER
START1 DECT R2                BACKUP POINTER
       MOV  R4,*R2            WRITE
       C    R4,*R2            O.K. ?
       JNE  START6            N, END OF RAM
       CLR  *R2               CLEAR RAM
       CI   R2,BRAM           DOWN TO INTERPRETER
       JH   START1            N, CONTINUE
       DECT R2
************************************************************
*                                                          *
*                 INITIALIZE SYSTEM POINTERS               *
*                                                          *
************************************************************
*
*       R2=BRAM-2
*       R5=PRAM
*
START6 LI   R1,IOB            GET POINTER
       LI   R5,PTRTBL         REF THE SYSTEM PTR TABLE
       MOV  R2,@MEMSIZ         ;SAVE MEMORY SIZE
       AI   R2,1024            ;RESERVE ROOM FOR HEADER BLOC
*                                    AND USER AREA
       MOV  *R5+,*R1+          ;SAVE IOB
       MOV  *R5+,*R1+          ;SAVE EUS
       MOV  *R5+,*R1+          ;SAVE FNS
       MOV  *R5+,*R1+          ;SAVE GSS
       MOV  *R5,*R1+           ;SAVE UFT
       MOV  R2,*R1+            ;SAVE BUS
       MOV  R2,*R1+            ;SAVE EORBUS
************************************************************
*                                                          *
*      INITIALISE ALL 9902'S FOR 300 BAUD OPERATION        *
*                                                          *
************************************************************
       LI   R7,DEVTBL         REF. DEVICE TABLE
       LI   R2,>04D0          SET TO 300 BAUD
*
NXTDEV CI   R7,DTEND          DONE ALL ENTRIES
       JHE  SETUP             Y, SET SYSTEM POINTERS
       MOV  *R7+,R12          N,GET ENTRY
       COC  @C1,R12           LS BIT SET?
       JNE  NXTDEV            N, NOT A 9902
       ANDI R12,>7FFE         KILL OFF LS & MS BITS
       TB   18                IS IT INSTALLED
       JEQ  NODEV             N, DELETE TABLE ENTRY
       BL   @SRATE            Y, INITIALISE IT
       JMP  NXTDEV            DO NEXT DEVICE
*
NODEV  CLR  @-2(R7)           DELETE TABLE ENTRY
       JMP  NXTDEV            DO NEXT DEVICE
*
SETUP  EQU  $
       LI   R11,NEWP          SET EXIT TO 'NEW'
       B    @SETVEC           SET CRITICAL SYSTEM POINTERS
       PAGE
CPYST  EQU  $
ERR00T TEXT 'FATAL '
SYSERR TEXT -'SYSTEM ERROR'
ERR01T TEXT -'Syntax Error'
ERR02T TEXT -'Unmatched delimiter'
ERR03T TEXT -'Invalid line number'
ERR04T TEXT -'Illegal variable name'
ERR05T TEXT -'Too many variables'
ERR06T TEXT -'Illegal character'
ERR07T TEXT -'Expecting operator'
ERR08T TEXT -'Illegal function name'
ERR09T TEXT -'Illegal function argument'
ERR10T TEXT -'Out of memory'
ERR11T TEXT -'Stack overflow'
ERR12T TEXT -'Stack underflow'
ERR13T TEXT -'No such line number'
ERR14T TEXT -'Expecting string variable'
ERR15T TEXT -'Invalid screen command'
ERR16T TEXT -'Expecting dimensioned variable'
ERR17T TEXT -'Subscript out of range'
ERR18T TEXT -'Too few subscripts'
ERR19T TEXT -'Too many subscripts'
ERR20T TEXT -'Expecting simple variable'
ERR21T TEXT -'Digits out of range'
ERR22T TEXT -'Expecting variable'
ERR23T TEXT -'Read out of Data'
ERR24T TEXT -'Read & Data types differ'
ERR25T TEXT -'Square root of negative number'
ERR26T TEXT -'Log of non-positive number'
ERR27T TEXT -'Expression too complex'
ERR28T TEXT -'Division by zero'
ERR29T TEXT -'Floating point overflow'
ERR30T TEXT -'Range error'
ERR31T TEXT -'Missing NEXT'
ERR32T TEXT -'Missing FOR'
ERR33T TEXT -'EXP has invalid argument'
ERR34T TEXT -'Corrupted number'
ERR35T TEXT -'Parameter error'
ERR36T TEXT -'Missing assignment operator'
ERR37T TEXT -'Illegal delimiter'
ERR38T TEXT -'Undefined function'
ERR39T TEXT -'Undimensioned variable'
ERR40T TEXT -'Undefined variable'
ERR41T TEXT -'Expansion eprom not found'
ERR42T TEXT -'Interrupt without trap'
ERR43T TEXT -'Invalid baud rate'
ERR44T TEXT -'Illegal opcode'
ERR45T TEXT -'Eprom verify error'
ERR46T TEXT -'Invalid device number'
ERR47T TEXT -'Required hardware not found'
ERR48T TEXT -'Illegal in current mode'
ERR49T EQU  BADADR
TBEND  EQU  $                 <===  END OF SCRATCH AREA
*
TBSIZE EQU   ((TBEND-TMPBUF)-1024)
       ASMIF TBSIZE&>8000
 SPLAT !!! - SCRATCH BUFFER IS TOO SMALL
       ASMEND
       END
