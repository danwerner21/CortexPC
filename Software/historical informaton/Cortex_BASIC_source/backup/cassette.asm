       TITL 'CASSETTE LOAD/SAVE - CORTEX BASIC REV. 1.1'
       IDT  'CASSETTE'
       DEF  MC$LOD,MOTORY,MC$SAV,SAVY,LDPY,YORN
       REF  DH$VNT,DH$SLT,PRDY,DMYHDR
       REF  B43,CMON$,CMOF$,DH$HCS,DH$END,DH$PLL,ERR3$M
       REF  UNIT,C0004,D100,CNTDWN,TYP11$,TYP0$,DELAY$
       REF  MODEOK,IDRUN,AUTORN,TYPSN$,CASRDY,PLF,GETC$
       REF  MODE,GTLN,IOB,BUS,GETCR$
       REF  NEWP,NVD,RUNP,SLT,STRTC,TAPERR,VDT,VNT
       REF  TYPS$,DH$SIZ,DH$ARF,PGMFND,CALLWP
       REF  ESCFLG,EFIX$E,IDTEQ,WPR1
*
ERROR  EQU  >2F80
ERROR2 EQU  ERROR+>20
*
       DXOP EVFIX,11
SYN    EQU  >1600             TAPE SYNC CHARACTER
STX    EQU  >0200             TAPE START OF TEXT CHARACTER
ETX    EQU  >0300             TAPE END OF TEXT CHARACTER
************************************************************
*
*          TAPE/EPROM HEADER BLOCKS
*
*          BASIC               MACHINE CODE
*   ********************   ******************** RUN  =>A5A5
*   *  AUTO RUN FLAG   *   *  AUTO RUN FLAG   * NORUN=>5A5A
*   ********************   ********************
*   *   8 BYTE NAME    *   *   8 BYTE NAME    * NULL FILLED
*   ********************   ********************
*   *   DISP TO SLT    *   *       >0000      *
*   ********************   ********************
*   *   DISP TO VNT    *   *   LOAD ADDRESS   *
*   ********************   ********************
*   *     NVD - VDT    *   *   ENTRY POINT    * (OFFSET)
*   ********************   ********************
*   *   LOAD LENGTH    *   *   LOAD LENGTH    * (BYTES)
*   ********************   ********************
*   *     CHECKSUM     *   *     CHECKSUM     * HEADER ONLY
*   ********************   ********************
*
*DMYHDR EQU  $
*DH$ARF BSS  2            RUN/NORUN
*DH$PGN BSS  8            PROGRAM NAME
*DH$SLT BSS  2            DISP TO SLT
*DH$VNT BSS  2            DISP TO VNT
*DH$SIZ BSS  2            NVD-VDT
*DH$PLL BSS  2            LENGTH
*DH$HCS BSS  2            HEADER CHECKSUM
*HDRSIZ EQU  $-DMYHDR     HEADER BLOCK SIZE
*DH$END EQU  $            END OF DUMMY HEADER
****************
       COPY 'IODEFS.INC'
*
*   WORKSPACE       :
*   ROUTINE(S)      :
*   REGISTER USAGE  :
*                                              GLOBAL DATA
*           ___________________________________   LABELS
*   R0      !        SCRATCH                  !
*   R1      !        SCRATCH                  !
*   R2      !        SCRATCH                  !
*   R3      !      UNIT SAVE                  !
*   R4      !        SCRATCH                  !
*   R5      !        PTR TO DMYHDR            !
*   R6      !                                 !
*   R7      !         CHECKSUM                !
*   R8      !   POINTER TO PROG NAME          !
*   R9      !   POINTER TO PROG START         !
*   R10     !   SAVED RETURN                  !
*   R11     !        BL RETURN ADDRESS        !
*   R12     !         CASSETTE CRUBASE        !
*   R13     !          RETURN WP              !
*   R14     !          RETURN PC              !
*   R15     !          RETURN ST              !
*           -----------------------------------
*
************************************************************
*                                                          *
* MONITOR DUMP ROUTINE -- 'D' COMMAND                      *
*                                                          *
* DUMP RAM IMAGE TO CASSETTE TAPE IN IMAGE FORMAT          *
*     R0 = START   R1=STOP    R2=ENTRY POINT               *
*                                                          *
************************************************************
MC$SAV SZC  R8,R2             WORD ALIGN ENTRY POINT
*
* START ADDRESS > STOP ADDRESS--ERROR
*
       C    R0,R1             START <= STOP ?
       JLE  ADDROK            YES
       B    @ERR3$M           ERROR EXIT TO MONITOR
ADDROK LI   R4,DH$SLT         REF HDR SLT ENTRY
       CLR  *R4+              CLEAR IT (FLAGS AS M/C)
       MOV  R0,*R4+           SAVE LOAD ADDRESS
       MOV  R0,R9             SAVE IT FOR SAV0
       MOV  R2,*R4+           SAVE ENTRY POINT
       S    R0,R1             GET LENGTH
       MOV  R1,*R4            SAVE LENGTH
       MOV  R11,R10           SAVE RETURN ADDRESS
       DATA TYPSN$,IDTEQ      PROMPT FOR IDT
       SETO @MODE             STOP GETLIN USING ^E
       CLR  @ESCFLG           MAKE SURE HE CAN BOM OUT
       BL   @GTLN             GET A LINE
       MOV  @IOB,R8           GET ITS START
       JMP  SAV0              DO SAVE
       PAGE
*
*
*     ENTRY POINT FOR BASIC 'SAVE' COMMAND
*
SAVY   BL   @MODEOK           CHECK NOT RUNNING
       LI   R1,DH$SLT         OK, REF THE SLT ENTRY
       LI   R10,PRDY          RETURN TO 'READY*'
       MOV  @BUS,R9           GET BUS
       MOV  @SLT,*R1          COPY IN THE SLT
       S    R9,*R1+           SUBTRACT BUS
       MOV  @VNT,*R1          COPY IN THE VNT
       S    R9,*R1+           SUBTRACT BUS
       MOV  @NVD,*R1          COPY IN THE NVD
       S    @VDT,*R1+         CALCULATE NVD-VDT (VAR NAMES)
       MOV  @NVD,*R1          CALCULATE LOAD LENGTH FROM
       S    R9,*R1                  NVD-BUS
       INC  R8                SKIP THE QUOTE ON "NAME"
*
*      SET UP DUMMY HEADER
*
SAV0   LI   R1,DMYHDR         POINT TO DUMMY HEADER
       MOV  R1,R4             SAVE FOR LATER
       MOV  R1,R5             SAVE FOR LATER
       LI   R2,IDRUN          SET FOR AUTO-RUN
SAV1   DATA TYPSN$,AUTORN     AUTO-RUN PROMPT
       BL   @YORN             GET REPLY
       JMP  SAV1              LOOP IF NOT A 'Y' OR 'N'
       JMP  SAV2              Y, R2 SET UP OK
       INV  R2                N, SET FOR NO AUTORUN
SAV2   MOV  R2,*R1+           STORE AUTO-RUN FLAG
       LI   R2,8              MAX OF 8 CHARACTERS
SAV3   MOVB *R8,R0            GET CHARACTER
       JEQ  SAV4              0, END OF NAME
       INC  R8                MOVE TO NEXT CHARACTER
SAV4   MOVB R0,*R1+           COPY NAME
       DEC  R2                LIMIT?
       JNE  SAV3              N, LOOP
*
* NOW WORK OUT THE CHECKSUM
*
       LI   R1,DH$HCS         REF HEADER CHECKSUM WORD
       CLR  *R1               CLEAR CHECKSUM
SAV5   A    *R4+,*R1          ADD IN HEADER WORD
       C    R4,R1             DONE
       JL   SAV5              N, LOOP
*
*      NOW DUMP TO TAPE
*
SAV6   DATA TYPSN$,CASRDY     CASSETTE READY PROMPT
       BL   @YORN             GET RESPONSE
       JMP  SAV6              NEITHER
       JMP  SAV7              Y
       JMP  SAV6              N, LOOP
*
SAV7   SETO @PLF              FLAG AS SAVING (-VE)
       BL   @SET02            GO SET UP 9902
       DATA CMON$             TURN TAPE MOTOR ON
       MOV  @UNIT,R3          SAVE UNIT FLAGS
       MOV  @C0004,@UNIT      SET UNIT FOR CASSETTE ONLY
       MOV  @D200,@CNTDWN     SET FOR 2 SEC. START UP
SNDSYN DATA TYP11$,SYN        SEND SYNC CHARACTER
       MOV  @CNTDWN,R0        DONE STARTUP ?
       JGT  SNDSYN            LOOP TILL ENOUGH SENT
       DATA TYP11$,STX        SEND STX
       LI   R1,DMYHDR         POINT TO HEADER BLOCK
DUMP   MOVB *R5+,R0           GET BYTE
       DATA TYP0$             SEND IT
       CI   R5,DH$END         DONE HEADER?
       JNE  DUMP              N, LOOP
*
* NOW DUMP THE PROGRAM
*
       MOV  @DH$PLL,R1        GET LENGTH
       CLR  R7                RESET CHECK COUNTER
DUMP1  MOVB *R9+,R0           GET BYTE
       DATA TYP0$             SEND IT
       SRL  R0,8              PUT IN LSB
       A    R0,R7             ADD IN TO CHECKSUM
       DEC  R1                DONE?
       JNE  DUMP1             N, LOOP
*
       DATA TYP11$,ETX        SEND 'ETX' CHARACTER
       MOV  R7,R0             GET CHECK WORD
       DATA TYP0$             SEND MSB
       SWPB R0                POSITION LSB
       DATA TYP0$             SEND LSB
*
       MOV  @D100,@CNTDWN     SET FOR 1 SECOND DELAY
       DATA DELAY$            AND WAIT
       DATA CMOF$             TURN TAPE OFF
       MOV  R3,@UNIT          RESTORE UNIT FLAGS
       CLR  @PLF              RESET PROGRAM LOAD FLAG  
       B    *R10              AND EXIT
*
D200   DATA 200               TWO SECOND WAIT FOR TAPE M/C
       PAGE
*
*   WORKSPACE       :
*   ROUTINE(S)      :
*   REGISTER USAGE  :
*                                              GLOBAL DATA
*           -----------------------------------   LABELS
*   R0      !          SCRATCH                !
*   R1      !                                 !
*   R2      !                                 !
*   R3      !                                 !
*   R4      !                                 !
*   R5      !       POINTER TO DMYHDR         !
*   R6      !          CHECKSUM               !
*   R7      !        AUTO-RUN FLAG            !
*   R8      !       POINTER TO NAME           !
*   R9      !                                 !
*   R10     !                                 !
*   R11     !                                 !
*   R12     !         CASSETTE CRUBASE        !
*   R13     !          RETURN WP              !
*   R14     !          RETURN PC              !
*   R15     !          RETURN ST              !
*           -----------------------------------
*
************************************************************
*                                                          *
* MONITOR 'L' COMMAND                                      *
*                                                          *
*                                                          *
************************************************************
MC$LOD MOV  R11,R10           SAVE RETURN ADDRESS
       DATA TYPSN$,IDTEQ      PROMPT FOR IDT
       SETO @MODE             STOP GTLN USING ^E
       CLR  @ESCFLG           MAKE SURE HE CAN BOM OUT
       BL   @GTLN             GET THE NAME
       CLR  @MODE             RESET MODE
       MOV  @IOB,R8           POINT TO NAME
       JMP  LDPY0             AND DO LOAD
*
*      ENTRY POINT FOR BASIC 'LOAD' COMMAND
*
LDPY   LI   R10,PRDY          EXIT TO '*READY'
       BL   @MODEOK           ;CHECK MODE
       INC  R8                SKIP LEADING QUOTE
LDPY0  DATA TYPSN$,CASRDY     'CASSETTE READY' PROMPT
       BL   @YORN             GET A RESPONSE
       JMP  LDPY0             NEITHER
       JMP  LDPY0A            Y
       JMP  LDPY0             N
LDPY0A INC  @PLF              PLF=1 NON DESTRUCTIVE ESC.
       BL   @SET02            GO SET UP 9902
       DATA CMON$             TURN MOTOR ON
*
*  LOOK FOR SOME 'SYN' CHARACTERS
*
NOSYN  CLR  R7                RESET 'SYN' COUNT
FNDSYN DATA GETCR$            GET A CHARACTER
       CI   R0,STX            'STX' CHARACTER
       JEQ  SYN$1             Y, CHECK IF THIS IS OK
       CI   R0,SYN            N, IS IT A 'SYN'
       JNE  NOSYN             N, RESET COUNTER
       INC  R7                Y, COUNT IT
       JMP  FNDSYN            LOOK FOR SOME MORE
*
* 'STX' FOUND - IS THIS OK ?
*
SYN$1  CI   R7,6              HAVE WE FOUND ENOUGH ?
       JL   NOSYN             N, KEEP LOOKING
*                             Y, IS THE NEXT WORD THE HEADER
       DATA GETCR$            GET BYTE
       MOVB R0,R7             SAVE IT
       SRL  R7,8              POSITION
       DATA GETCR$            GET 2ND BYTE
       MOVB R0,R7             ADD IT IN
       CI   R7,IDRUN          IS IT A RUNID?
       JEQ  SYN$2             Y, FOUND HEADER
       INV  R7                N, MAYBE NOT AUTO-RUN
       CI   R7,IDRUN          IS IT OK?
       JNE  NOSYN             N, NOT THE HEADER BLOCK
       INV  R7                Y, RESTORE ID WORD
*
*   HEADER BLOCK FOUND, R7 CONTAINS THE AUTO-RUN FLAG
*
SYN$2  LI   R5,DMYHDR         POINT TO THE DMYHDR
       MOV  R5,R1             SAVE IT
       MOV  R7,*R5+           SAVE THE AUTO RUN FLAG
       MOV  R5,R2             SAVE FOR LATER
       DATA TYPS$,PGMFND      OUTPUT 'FOUND "'
       MOV  R8,R4             SAVE SEARCH NAME START
LOAD1  DATA GETCR$            GET A CHARACTER
       MOVB R0,*R5+           SAVE IT
       JEQ  LOAD1A            NULL, DON'T PRINT IT THEN
       CI   R5,DH$SLT         GOT NAME?
       JH   LOAD1A            Y, SKIP
       DATA TYP0$             OUTPUT CHARACTER
LOAD1A CI   R5,DH$END         DONE HEADER ?
       JL   LOAD1             N, LOOP
       DATA TYP11$,'"'*256    Y, NOW OUT CLOSING "
*
*      IS IT THE ONE WE ARE LOOKING FOR ?
*
       LI   R0,8              MAX 8 BYTES
LOAD1B CB   *R4,*R2+          MATCH ?
       JNE  NOSYN             N, LOOK FOR NEW HEADER
       MOVB *R4,*R4+          WAS IT A NULL
       JEQ  LOAD1C            Y, MATCH FOUND
       DEC  R0                DONE ?
       JNE  LOAD1B            N, LOOP
*
*  NOW CALCULATE THE HEADER CHECKSUM
*
LOAD1C CLR  R6                RESET CHECKSUM COUNTER
LOAD2  A    *R1+,R6           ADD IN WORD OF HEADER
       CI   R1,DH$HCS         DONE?
       JL   LOAD2             N, LOOP
       C    R6,*R1            Y, DO THEY MATCH ?
       JNE  LDERR             N, ERROR
*
*     FIND OUT WHERE TO LOAD THE PROGRAM
*
LOAD3  MOV  @DH$VNT,R4        GET M/C LOAD ADDRESS
       MOV  @DH$PLL,R3        GET LOAD LENGTH
       CLR  R6                RESET CHECKSUM
       MOV  @DH$SLT,R15       M/C LOAD?
       JEQ  LOAD4             Y, R4 SET UP
       MOV  @BUS,R4           N, GET BASIC LOAD ADDRESS
       INC  @PLF              PLF=2, DESTRUCTIVE ESCAPE
*
*      NOW LOAD THE PROGRAM
*
LOAD4  DATA GETCR$            GET CHARACTER
       MOVB R0,*R4+           STORE IT
       SRL  R0,8              PUT IN LSB
       A    R0,R6             ADD INTO CHECKSUM
       DEC  R3                DONE?
       JNE  LOAD4             N, LOOP
*
       DATA GETCR$            Y, GET CHARACTER
       CI   R0,ETX            'ETX' CHARACTER ?
       JNE  ABORT             N, ABORT
*
*      NOW READ IN CHECKSUM
*
       DATA GETCR$            GET MSB CHECKSUM
       MOV  R0,R3             COPY IT
       DATA GETCR$            GET LSB CHECKSUM
       SRL  R0,8              POSITION IT
       A    R0,R3             ADD IT
       C    R3,R6             CHECKSUM OK?
       JEQ  LOAD9             Y, SET POINTERS ETC
*
*  TAPE LOAD ERROR - ERROR & ABORT
*
ABORT  MOV  R15,R15           M/C LOAD?
       JEQ  LDERR             Y, JUST ERROR IT
       LI   R10,NEWP          N, DO A NEW
*
*  TAPE LOAD ERROR - ERROR & EXIT
*
LDERR  DATA CMOF$             TURN MOTOR OFF
       DATA TYPSN$,TAPERR     OUT TAPE ERROR
       CLR  @PLF              RESET PROGRAM LOAD FLAG
       B    *R10              EXIT
*
*      SET UP VECTORS AND EXECUTE IF NEEDED
*
LOAD9  DATA CMOF$             TURN TAPE OFF
       MOV  R15,R15           M/C LOAD ?
       JNE  LOAD6             N, BASIC
       MOV  @DH$SIZ,R14       Y, SET PC
       LI   R13,CALLWP        SET UP WP
LOAD8  CLR  @PLF              RESET LOADING FLAG
       C    @DH$ARF,@STRTC    AUTO-RUN?
       JEQ  LOAD7             Y, DO AUTO-RUN
       B    *R10              N, EXIT
LOAD7  RTWP                   DO AUTO-RUN
*
*      BASIC LOAD
*      R4 POINTS AFTER END OF PGM
*
LOAD6  LI   R2,DH$SLT         REF SLT DISP ENTRY
       MOV  @BUS,R1           GET BUS
       A    R1,*R2            MAKE SLT ADDRESS
       MOV  *R2+,@SLT         GET SLT
       A    R1,*R2            MAKE VNT ADDRESS
       MOV  *R2+,@VNT         GET VNT
       MOV  R4,@VDT           SET VDT TO FOLLOW PGM.
       S    *R2+,@VDT         ADJUST FOR VARIABLE SPACE
       MOV  R4,@NVD           SET NVD
       LI   R14,RUNP          SET RUN ADDRESS
       LI   R13,WPR1          SET WORKSPACE
       STST R15               SAVE STATUS
       JMP  LOAD8
       PAGE
*
*
*      BASIC 'MOTOR' STATEMENT
*   FORMAT
*      MOTOR <ARG>
*
*      IF ARG =  0 THEN TURN CASSETTE MOTOR OFF
*      IF ARG <> 0 THEN TURN CASSETTE MOTOR ON
*
MOTORY EVFIX R1               GET ARG
       MOV  R1,R1             OFF ?
       JNE  MTRON             N, GO TURN IT ON
       DATA CMOF$             Y, TURN IT OFF
MOTXIT B    @EFIX$E           AND BACK TO BASIC
MTRON  DATA CMON$             TURN CASSETTE MOTOR ON
       JMP  MOTXIT            AND EXIT
*
*  GET CHAR FROM KEYBOARD AND CHECK FOR Y OR N
*
*      BL @YORN
*        NEITHER Y NOR N RETURN
*        Y RETURN
*        N RETURN
*
YORN    EQU  $
        DATA GETC$      GET CHAR FROM KEYBOARD
        JMP  YORN       NO CHAR
        JMP  YORN2      VALID CHAR
        JMP  ESCAPE     ESCAPE CHAR
*
YORN2   DATA TYP0$       ECHO THE CHARACTER
        CI   R0,>5900   Y?
        JEQ  YORN3
        CI   R0,>4E00   N?
        JNE  YORN4
        INCT R11        'N' DOUBLE SKIP
YORN3   INCT R11        'Y' SINGLE SKIP
YORN4   B    *R11       RETURN
ESCAPE  B   @PRDY        EXIT
*
*      SET UP CASSETTE 9902 FOR TAPE I/O
*
SET02  LI   R12,2*CASS02      SET UP CRUBASE
       SBO  31                RESET 9902
       MOV  *R11,*R11         DELAY
       LDCR @B43,8            2 STOP,NO PAR,8 BITS,/3 CLOCK
       SBZ  13                NO TIMER
       LDCR @CBRATE,12        LOAD XMT/REC BAUD RATE
       RT                     EXIT
*
CBRATE DATA >04D0             300 BAUD CASSETTE
       END
