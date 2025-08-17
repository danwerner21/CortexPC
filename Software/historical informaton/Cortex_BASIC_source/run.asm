       TITL 'RUN COMMAND - CORTEX BASIC REV 1.1'
       IDT  'RUN'
*
* THIS MODULE CAUSES THE STORED BASIC PROGRAM TO BE
* INTERPRETED. ON ENTRY IT CAUSES THE USER VARIABLE
* SPACE TO BE CLEARED.
*
* A LIST OF ALL BASIC STATEMENT ENTRY POINTS IS MAINTAINED
* IN THIS MODULE.
*
* ALSO CONTAINED ARE THE ROUTINES: CLRV (CLEAR USER VARIABLE
* SPACE), REST (RESET THE DLC AND DDM SYSTEM POINTERS) AND
* THE BASIC STATEMENT HANDLER 'ELSE'.
*
       DEF  LINE,LINE0,LINE2,LINE5
       DEF  NLIN,NLIN0,RUNP,RUNP1,CLRV,REST
       DEF  NXTXB,DATXB
       REF  B2E,PLF,ESCFLG,POPY,ENTERY
       REF  MAGY,LST,BAUD,GOTY
       REF  EXTNDY
       REF  SLN,RENUMS,BASY,BITY,CLLY,CRBY
       REF  SGETY,SPUTY,CRFY,DEFY,DIMY
       REF  SPRITY,SHAPEY,ERRY,ESCY,FORY
       REF  BOOTY,GOSY,IFY,INPY,LDPY
       REF  SWAPY,LETY,MEMY,NEWY,NOEY
       REF  NXTY,ONY,PRTY
       REF  RANY,RDDY,RNWY,RTNY
       REF  SAVY,STPY,TIMY,UNTY
       REF  MOTORY,RANDS
       REF  NUMY,PLOTY,UPLOTY,COLORY
       REF  GRAPHY,TEXTY,WAITY,LDCSY
       REF  CRLF,TYPC$,STPYA,MWDY
       REF  NVS,GSS,VNT,VDT,DDM,DLC,GSC
       REF  BUS,DLIM,MODE,PLC,SLT,ELSF
       REF  PURGY,STACNT,IOB,R8STOR,TRAFLG
       REF  TOFY,TONY,TYPBE$,MOVEB,MOVEL
       REF  F$WHO
*
* XOP EQUATES
*
ERROR  EQU  >2F80
ERROR2 EQU  ERROR+>20
       DXOP OUTINT,13
       PAGE
*
* CLRV - CLEAR R1 THRU EUS
*
CLRV   MOV  @IOB,@NVS         ;SET NVS TO IOB POINTER
       MOV  @GSS,@GSC         ;RESET GOSUB STACK
       CLR  *R1+              ;CLEAR MEMORY
       CI   R1,RANDS          ;REACHED EUS YET?
       JL   $-6               ;N - BACK FOR NEXT WORD
       CLR  @R8STOR           ;CLEAR ENTER'S R8 STORE
*
* REST - RESET DATA PTR
*
REST   MOV  @VNT,@DLC         ;RESET DLC TO VNT
       CLR  @DDM              ;ZERO DDM
       RT
       PAGE
*
* RUN STATEMENT
*
RUNP   DATA TYPC$             ;OUT CRLF
*
RUNP1  SETO @MODE             ;SET MODE TO RUN
       CLR  @ESCFLG           ;ENABLE ESCAPE
       CLR  @PLF              ;FLAG AS NOT LOAD/SAVE
       MOV  @VNT,@PLC         ;SET PLC TO START
       MOV  @VDT,R1           ;CLEAR VDT THRU EUS
       BL   @CLRV
       JMP  LINE
*
* MULTIPLEXOR
*
NLIN1  CI   R0,>3C00          ;':' ?
       JNE  NLIN1A            ;N, TRY OTHER DELIMITERS
       INC  @STACNT           ;Y, COUNT
       JMP  LINE2             ;GET REST OF LINE
NLIN1A CI   R0,>3B00          ;'THEN' ?
       JEQ  LINE2             ;Y, GET REST OF LINE
       CI   R0,>4700          ;'!' ?
       JEQ  LINE              ;Y, NEW LINE
*
ERR37  DATA ERROR2,37         ;N - ILLEGAL DELIMITER
*
* NEXT LINE OR ":
*
NLIN0  MOVB *R8+,@DLIM        ;SAVE & SKIP DELIMITER
*
NLIN   MOV  @DLIM,R0          ;LOOK AT DELIMITER
       JNE  NLIN1             ;NOT EOL, CHECK IT
*
* NEXT LINE
*
LINE   MOV  @MODE,R0          ;LOOK AT MODE
       JEQ  LINE3             ;IDLE - THEN FINISHED
LINE5  EQU  $
       MOV  @PLC,R8           ;GET PROGRAM LINE COUNTER
       AI   R8,-4             ;MOVE TO NEXT LINE
       C    R8,@SLT           ;ANY MORE STATEMENTS?
       JL   LINE4             ;N
LINE0  MOV  R8,@PLC           ;Y - UPDATE PLC
       MOV  @-2(R8),@SLN      ;SAVE LINE #
       MOV  *R8,R8            ;GET PBC
       A    @BUS,R8           ;GET POINTER TO BOL
       CLR  @STACNT           ;ZERO STATEMENT COUNTER
*
LINE2  CLR  @F$WHO            ;FLAG IN BASIC
NOTO   CLR  R2
       MOVB *R8+,R2           ;GET STATEMENT CODE
       SRL  R2,7              ;GET INDEX
       CI   R2,2*MAXSTA       ;VALID ?
       JH   SYSERR            ;N, ERROR !!!
       MOV  @MXLST-2(2),R9    ;Y, GET STATEMENTS ENTRY PT
       JEQ  SYSERR            ;'0' SYSTEM ERROR
*
       MOV  @TRAFLG,R11       ;IN TRACE ?
       JEQ  NOTRC             ;N, DONT OUT TRACE INFO
       MOV  @MODE,R11         ;Y, IDLE?
       JEQ  NOTRC             ;Y, DONT BOTHER THEN
*
*  OUT TRACE INFORMATION IN THE FORM '  {line No.}  '
*
       BL   @MOVEB            ;GET IOB & ENTER TEXT
       TEXT ' Statement No. '
       BYTE 0
       MOV  @PLC,R11          GET PLC
       OUTINT @-2(R11)        OUT LINE #
       MOV  @STACNT,R11       GET STATEMENT INDENT COUNT
       JEQ  OTRC1             0, LEAVE LINE No.
       MOVB @B2E,*R7+         OUT '.'
       OUTINT R11             OUT INDENT COUNT
OTRC1  BL   @MOVEL            ADD IN REST OF TEXT
       TEXT '   '
       BYTE 0
       DATA TYPBE$            OUT I/O BUFFER
       DATA TYPC$             OUT CRLF
*
*      !!!!! WARNING !!!!!
*
*    R2 MUST STILL CONTAIN 2*PCODE
*
NOTRC  B    *R9               ;GOTO STATEMENT HANDLER
*
SYSERR DATA ERROR2,-1         ;SYSTEM ERROR
*
* ONLY HAD 1 LINE TO EXECUTE - ORIGINALLY ENTERED VIA EDIT
*
LINE3  B    @CRLF
*
*     FINISHED PROGRAM
*
LINE4  B    @STPY             ;ESCAPE
       PAGE
*
* ELSE STATEMENT
*
ELSY   MOV  @ELSF,R0          ;CHECK ELSE FLAG
       JNE  LINE2             ;SET - CONTINUE TO EXECUTE
       JMP  LINE              ;RESET - IGNORE LINE
       PAGE
*
* RUN TABLE
*
       DATA STPY                            00 TRAP NULLS
MXLST  DATA GOTY              GOTO          01
       DATA GOSY              GOSUB         02
       DATA ELSY              ELSE          03
       DATA LINE              REM           04
       DATA FORY              FOR           05
       DATA LETY              (LET)         06
       DATA LINE              DATA          07
DATB   EQU  $-MXLST/2
       DATA NXTY              NEXT          08
NXTB   EQU  $-MXLST/2
       DATA ERRY              ERROR         09
       DATA PRTY              PRINT         0A
       DATA CLLY              CALL          0B
       DATA LDPY              LOAD          0C
       DATA INPY              INPUT         0D
       DATA RDDY              READ          0E
       DATA RNWY              RESTOR        0F
       DATA RTNY              RETURN        10
       DATA STPYA             STOP          11
       DATA UNTY              UNIT          12
       DATA TIMY              TIME          13
       DATA SAVY              SAVE          14
       DATA BASY              BASE          15
       DATA ESCY              ESCAPE        16
       DATA NOEY              NOESC         17
       DATA RANY              RANDOM        18
       DATA BAUD              BAUD          19
       DATA ENTERY            ENTER         1A
       DATA PLOTY             PLOT          1B
       DATA UPLOTY            UNPLOT        1C
       DATA COLORY            COLOUR        1D
       DATA PURGY             PURGE         1E
       DATA GRAPHY            GRAPH         1F
       DATA TEXTY             TEXT          20
       DATA WAITY             WAIT          21
       DATA LDCSY             CHAR          22
       DATA NUMY              NUMBER        23
       DATA LST               LIST          24
       DATA RENUMS            RENUM         25
       DATA SPRITY            SPRITE        26
       DATA SHAPEY            SHAPE         27
       DATA SPUTY             SPUT          28
       DATA SGETY             SGET          29
       DATA BOOTY             BOOT          2A
       DATA SWAPY             SWAP          2B
       DATA >0000                           2C
       DATA MOTORY            MOTOR         2D
       DATA >0000                           2E
       DATA >0000                           2F
       DATA >0000                           30
       DATA >0000                           31
       DATA >0000                           32
       DATA >0000                           33
       DATA >0000                           34
       DATA >0000                           35
       DATA >0000                           36
       DATA >0000                           36
       DATA >0000                           38
       DATA MAGY              MAG           39
       DATA TOFY              TOF           3A
       DATA TONY              TON           3B
       DATA POPY              POP           3C
       DATA DIMY              DIM           3D
       DATA LETY              LET           3E
       DATA PRTY              (;)           3F
       DATA ONY               ON            40
       DATA IFY               IF            41
       DATA DEFY              DEF           42
       DATA NEWY              NEW           43
       DATA STPY              END           44
       DATA PRTY              (?)           45
       DATA EXTNDY            (*) (EXTEND)  46
       DATA BITY              BIT(          47
       DATA CRBY              CRB(          48
       DATA CRFY              CRF(          49
       DATA MEMY              MEM(          4A
       DATA MWDY              MWD(          4B
*
LASTSM EQU  $
MAXSTA EQU  $-MXLST/2         MAX. STATEMENT CODE
*
* INTERNAL DATA
*
NXTXB  BYTE NXTB
DATXB  BYTE DATB
       END
