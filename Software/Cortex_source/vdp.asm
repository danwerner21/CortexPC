       TITL 'VDP ROUTINES - CORTEX BASIC REV. 1.1'
       IDT  'VDP'
*
*  NOTE :
*      ACCESSES TO THE VDP NOT REQUIRING VRAM ACCESS
*      MUST BE AT LEAST 3uS APART, ACCESSES REQUIRING
*      VRAM ACCESS MUST BE AT LEAST 8uS APART.
*
       DEF  UNPACK,LDCS,PIXON,PIXOFF,COLF
       DEF  PIXTST,TEXTY,GRAPHY,WAITY,LDCSY
       DEF  S$SM,MAGY,COLORY,SHAPEY,SPRITY
       DEF  EFIX$E,SPUTY,SGETY
       REF  XLOC,VDPWP0,VDPWP1,BITMAP,C2000
       REF  WAIT$,DELAY$,CNTDWN,CKEX,FPAC3
       REF  FPAC2,NLIN0,FBCOL
       REF  EVSFR$,SETTXT,SETGRA,SENDAD,LOADER
       REF  FGM$,VMODE,USERCS,PCHTB
       REF  FIX,EVARZ
*
       DXOP EVFIX,11
ERROR  EQU  >2F80
ERROR2 EQU  ERROR+>20
       COPY 'IODEFS.INC'
       PAGE
*
*              SGET STATEMENT
*             ================
*
*      FORMAT :-
*           SGET  ARG1 , ARG 2
*
*      TEXT MODE :-
*      THE VARIABLE SPECIFIED BY ARG 2 IS GIVEN THE ASCII
*      VALUE OF THE CHARACTER AT THE LINEAR CURSOR ADDRESS
*      SPECIFIED BY ARG1.
*
*      GRAPHIC MODE :-
*      THE SHAPE SPECIFIED BY ARG 2 IS SET TO THE PATTERN
*      AT THE LINEAR CURSOR ADDRESS SPECIFIED BY ARG1.
*
SGETY  EVFIX R1          GET ARG 1
       MOV  @VMODE,R0    TEXT MODE?
       JNE  GGET         N, DO GRAPHIC 'SGET'
*
       CI   R1,40*24     VALID CURSOR ADDRESS ?
       JHE  ERR15A       N, ERROR
       BLWP @EVARZ       GET VARIABLE ADDRESS IN R2
       MOV  R8,R15       SAVE PBC
       MOV  R1,R8        GET CURSOR ADDRESS IN R8
       AI   R8,NTBA      ADD IN NAME TABLE START
       BL   @SENDAD      POINT VDP AT IT
       CLR  *R2+         CLEAR 1ST WORD OF VAR
       CLR  *R2+         CLEAR 2ND WORD OF VAR
       CLR  *R2          CLEAR 3RD WORD OF VAR
       MOVB @VRAM,@-1(R2) READ CHARACTER CODE INTO VARIABLE
       JMP  PUTXIT       EXIT
*
GGET   CI   R1,32*24     VALID CURSOR ADDRESS ?
       JHE  ERR15A       N, ERROR
       EVFIX R2          Y, GET ARG 2
       MOV  R8,R15       SAVE PBC
       MOV  R1,R8        SET UP R8
       SLA  R8,3         FORM TABLE INDEX
       ASMIF PGTBA1
       AI   R8,PGTBA1    ADD IN PGT BASE IF NEEDED
       ASMEND
       BL   @SENDAD
       LI   R3,BITMAP    REF BITMAP
       MOV  R3,R4        SAVE FOR LATER
       SETO R0           DO 8 BYTES  (SLOWLY)
GREAD  MOVB @VRAM,*R3+   READ BYTE
       MOV  *R8,*R8      <<< SLOW THE BLOODY THING DOWN >>>
       SLA  R0,2         DONE?
       JNE  GREAD        N, LOOP
*
       MOV  R2,R8        GET SHAPE #
       SLA  R8,3         GET TABLE INDEX
       AI   R8,SPGBA1+>4000   ADD IN TABLE BASE
       BL   @SENDAD      POINT VDP AT IT
       SETO R0           DO 8 BYTES  (SLOWLY)
GWRIT  MOVB *R4+,@VRAM   WRITE BYTE
       MOV  *R8,*R8      <<< SLOW THE BLOODY THING DOWN >>>
       SLA  R0,2         DONE?
       JNE  GWRIT        N, LOOP
       JMP  PUTXIT       Y, EXIT
       PAGE
*
*              SPUT STATEMENT
*             ================
*
*      FORMAT :-
*           SPUT  ARG1 , ARG 2
*
*      TEXT MODE :-
*      THE CHARACTER AT THE LINEAR CURSOR ADDRESS
*      SPECIFIED BY ARG1 IS SET TO THE ASCII CHARACTER
*      GIVEN BY ARG2.
*
*      GRAPHIC MODE :-
*      THE CHARACTER AT THE LINEAR CURSOR ADDRESS
*      SPECIFIED BY ARG1 IS SET TO THE SHAPE SPECIFIED
*      BY ARG2.
*
SPUTY  EVFIX R1          GET ARG 1
       EVFIX R2          GET ARG 2
       MOVB R2,R2        ARG 2 VALID ?  0..255
       JNE  ERR15A       N, ERROR IT
       MOV  R8,R15       SAVE PBC
       MOV  @VMODE,R0    TEXT MODE ?
       JNE  GPUT         N, DO GRAPHICS 'SPUT'
*
       CI   R1,24*40     VALID CURSOR ADDRESS ?
       JHE  ERR15A       N, ERROR
       MOV  R1,R8        Y, SET UP R8
       AI   R8,NTBA+>4000   ADD TABLE START
       BL   @SENDAD      GIVE IT TO VDP
       SWPB R2           POSITION CHARACTER
       MOVB R2,@VRAM     WRITE IT TO VRAM
       JMP  PUTXIT       EXIT
*
GPUT   CI   R1,24*32     VALID CURSOR ADDRESS ?
       JHE  ERR15A       N, ERROR
       MOV  R2,R8        GET SHAPE #
       SLA  R8,3         FORM INDEX INTO SPRITE PAT. GEN TBL
       AI   R8,SPGBA1    ADD IN TABLE START
       BL   @SENDAD      POINT VDP AT IT
       LI   R8,8         READ 8 BYTES
       MOV  R8,R3        SAVE FOR LATER
       LI   R0,BITMAP    INTO 'BITMAP'
       MOV  R0,R4        SAVE FOR LATER
RSHAPE MOVB @VRAM,*R0+   READ SHAPE BYTE
       MOV  *R8,*R8      <<< SLOW THE BLOODY THINH DOWN >>>
       DEC  R8           DONE?
       JNE  RSHAPE       N, LOOP
*
       MOV  R1,R8        GET CURSOR ADDRESS IN R8
       SLA  R8,3         FORM INDEX INTO PGT
       AI   R8,PGTBA1+>4000 ADD IN TABLE START
       BL   @SENDAD      GIVE IT TO VDP
WSHAPE MOV  *R8,*R8      <<< SLOW THE BLOODY THING DOWN >>>
       MOVB *R4+,@VRAM   WRITE SHAPE TO VRAM
       DEC  R3           DONE ?
       JNE  WSHAPE       N, LOOP
*
       AI   R8,(CTBA1-PGTBA1) POINT TO COLOUR TABLE ENTRY
       BL   @SENDAD      POINT VDP AT IT
       SETO R5           8 LOOPS THE HARD WAY
WCOLOR MOVB @FBCOL,@VRAM WRITE FCOL/BCOL TO IT
       MOV  *R8,*R8      <<< SLOW THE BLOODY THING DOWN >>>
       SLA  R5,2         DONE ?   (SLOWLY !!)
       JNE  WCOLOR       N, LOOP
PUTXIT MOV  R15,R8       RESTORE PBC
       JMP  FNLIN0       AND EXIT
*
ERR15A JMP  ERR15        "DISP TOO BIG"
       PAGE
*
*  CODE FOR BASIC COMMANDS "TEXT" AND "GRAPH"
*
*   WORKSPACE       :    WPR1
*   ROUTINE(S)      :    LOADER,TEXTY,GRAPHY
*   REGISTER USAGE  :
*                                              GLOBAL DATA
*           ___________________________________   LABELS
*   R0      !                                 !
*   R1      !                                 !
*   R2      !                                 !
*   R3      !                                 !
*   R4      !                                 !
*   R5      !                                 !
*   R6      !                                 !
*   R7      !                                 !
*   R8      !         VRAM ADDRESS            !
*   R9      !                                 !
*   R10     !        PBC  STORE               !
*   R11     !        BL RETURN ADDRESS        !
*   R12     !    SAVED BL RETURN ADDRESS      !
*   R13     !          RETURN WP              !
*   R14     !          RETURN PC              !
*   R15     !          RETURN ST              !
*           -----------------------------------
*
*
* THIS ROUTINE SETS THE VDP INTO TEXT MODE AND
* LOADS THE CHARACTER SET. A CLEAR SCREEN IS THEN
* PERFORMED AND THE X & Y COUNTERS RESET.
*
*
TEXTY  DATA SETTXT            SET TEXT MODE
TGEXIT B    @NLIN0            NEXT LINE
*
*      THIS ROUTINE SETS UP THE VDP FOR GRAPHICS II MODE,
*      THE PNT AND PGT ARE INITIALIZED. THE X & Y COUNTERS
*      ARE RESET.
*
GRAPHY DATA SETGRA            SET GRAPHIC 2 MODE
       JMP  TGEXIT
       PAGE
*
*             CHAR STATEMENT
*            ================
*
*      FORMAT :
*           CHAR EXP1,EXP2,EXP3,EXP4
*      OR
*           CHAR
*
*      THIS STATEMENT RE-DEFINES A CHARACTER.
*           EXP1 - CHARACTER NUMBER   0..255
*           EXP2 - FIRST 16 BITS OF DEFINITION
*           EXP3 - SECOND 16 BITS OF DEFINITION
*           EXP4 - THIRD 16 BITS OF DEFINITION
*
*      IF THE STATEMENT IS ENTERED WITHOUT PARAMETERS
*      THEN THE CHARACTERS 0..127 ARE RE-LOADED WITH
*      THEIR INITIAL PATTERNS.
*
LDCSY  BL   @CKEX             EXPRESSIONS ?
       JMP  RELOAD            N, RELOAD CHARACTERS 0..127
       EVFIX R5               Y, GET CHARACTER #
       CI   R5,255            VALID CHARACTER ?
       JH   ERR35             N, ERROR IT
       LI   R4,3              DO 3 PARAMETERS
       STWP R6
       INCT R6                AND SAVE IN R1-R3
*
GETBIT CI   R0,>3F00          ',' ?
       JNE  ERR37             N, ERROR IT
       EVFIX *R6+             GET BIT PATTERN
       DEC  R4                COUNT IT
       JNE  GETBIT
*
       MPY  @C0006,R5         FORM INDEX INTO TABLE
       AI   R6,PCHTB          ADD IN TABLE START
       MOV  R1,*R6+           UPDATE TABLE ENTRY
       MOV  R2,*R6+
       MOV  R3,*R6+
EFIX$E EQU  $            <<< EXIT FOR AN EVFIX ROUTINE >>>
FNLIN0 DEC  R8                BACKUP TO DELIM
       JMP  TGEXIT            EXIT
*
*   RELOAD CHARACTERS 0..USERCS
*
RELOAD EQU  $
       ASMIF PIO
       LI   R12,2*PIO
       ASMELS
       CLR  R12
       ASMEND
       LI   R1,PCHTB          POINT TO START OF C. SET
*
COPY   LIMI 0                 CLOSE INTERRUPT WINDOW
       SBZ  ROMON-PIO         TURN ON ROM
       MOV  *R1,*R1+          COPY IT
       SBO  ROMON-PIO         TURN ROM OFF
       LIMI 15                OPEN WINDOW FOR INTERRUPTS
       CI   R1,USERCS         UPTO USER CHARACTER SET ?
       JL   COPY              N, CONTINUE COPY
       JMP  TGEXIT            EXIT
       PAGE
*
*              COLOUR STATEMENT
*             ==================
*
*      FORMAT :-
*           COLOUR ARG1<, ARG 2 >
*
*      THE FOREGROUND COLOUR IS SET TO ARG1,
*      THE BACKGROUND COLOUR IS SET TO ARG2 IF IT IS
*      PRESENT, IF NOT THE CURRENT BACKGROUND COLOUR
*      IS USED. IF THE STATEMENT IS ISSUED WITHOUT
*      PARAMETERS A DEFAULT OF 4,7 IS USED
*
COLORY BL   @CKEX             EXPRESSION?
       JMP  SET47             N, DEFAULT TO 4,7
       MOVB @FBCOL,R1         GET CURRENT COLOURS
       SWPB R1                PUT IT IN LSB
       EVFIX R2               GET FOREGROUND COLOUR
       CI   R0,>3F00          ',' FOLLOWING ?
       JNE  COLR1             N, USE DEFAULT BGND. COLOUR
       EVFIX R1               Y, GET BACKGROUND
COLR1  SRC  R1,4              MS NIBBLE =  BACKGROUND
       MOVB R1,R2             PUT IT IN R2
       SRC  R2,4              POSITION FOREGROUND
COLR2  MOVB R2,@FBCOL         R2 MSB = FCOL/BCOL
       MOV  @VMODE,R1         WHAT MODE AM I IN ?
       JNE  FNLIN0            GRAPHIC, ALL DONE THEN
       MOVB R2,@SFBCLR        TEXT, PUT IN CALL BLOCK
       BL   @LOADER           CALL VDP LOADER
SFBCLR BYTE >00,>80+R7        LOAD R7 WITH FCOL/BCOL
       DATA 0
       JMP  FNLIN0            EXIT
*
SET47  LI   R2,>4700          SET COLOUR 4,7
       INC  R8                ADJUST R8
       JMP  COLR2             & EXIT
       PAGE
*
*              COL FUNCTION
*             ==============
*
*      FORMAT :-
*           A = COL [ ARG 1 , ARG 2 ]
*
*      COL RETURNS THE COLOUR OF THE SPECIFIED PIXEL.
*      TESTING AN OFF SCREEN PIXEL RETURNS -1.
*         ARG 1 FORMS THE HORIZONTAL ORDINATE
*         ARG 2 FORMS THE VERTICAL ORDINATE
*
*     R1 CONTAINS ARG 2, *R2 CONTAINS ARG 1
*     R3 IS SPARE
*     EXIT IS OT EVSFR$
*
COLF   DATA FGM$              FORCE TO GRAPHIC
       MOV  @XLOC,R3          SAVE CURRENT X,Y
       MOV  R1,R10            COPY
       BL   @FIX              FIX
       CI   R10,192           'Y' VALID ?
       JHE  BADXY             N, RETURN INVALID COLOUR
       CI   R1,256            'X' VALID ?
       JHE  BADXY             N, RETURN INVALID COLOUR
       SWPB R1                PUT 'X' IN MSB
       MOVB R1,R10            R10 MSB = X , LSB = Y
       MOV  R10,@XLOC         SET CURSOR LOCATION
       BLWP @PIXTST           GET PIXEL COLOUR IN FPAC3
       DATA FPAC3
       MOV  R3,@XLOC          RESTORE CURSOR
COLXIT B    @EVSFR$           EXIT
BADXY  SETO @FPAC2            RETURN COLOUR = -1
       JMP  COLXIT
*
ERR15  DATA ERROR+15          ILLEGAL SCREEN COMMAND
ERR30  DATA ERROR+30          RANGE ERROR
ERR35  DATA ERROR2,35         PARAMETER ERROR
ERR37  DATA ERROR2,37         ILLEGAL DELIMETER
ERR48  DATA ERROR2,48         WRONG MODE !
       PAGE
************************************************************
*                                                          *
*               WAIT STATEMENT                             *
*                                                          *
*     FORMAT :-                                            *
*        WAIT  <ARG1>                                      *
*                                                          *
*                                                          *
*    IF ARG 1 IS PRESENT THEN IT IS USED AS A DELAY        *
*           COUNT AND THE ROUTINE WAITS FOR ARG 1 *10 MS   *
*           BEFORE RETURNING TO THE USER.                  *
*    IF ARG 1 IS NOT PRESENT THEN THE ROUTINE WAITS FOR    *
*           OUTPUT COMPLETION.                             *
*                                                          *
************************************************************
WAITY  BL   @CKEX             EXPRESSION ?
       JMP  WAITO             N, WAIT OUTPUT COMPLETION
       EVFIX @CNTDWN          Y, SET DELAY COUNT
       DATA DELAY$            WAIT FOR IT
       JMP  FNLIN0            THEN EXIT
*
WAITO  DATA WAIT$             WAIT OUTPUT COMPLETION
       JMP  TGEXIT            EXIT
       PAGE
*
*              SHAPE  STATEMENT
*             ==================
*
*      FORMAT :-
*           SHAPE SHAPE No.,WORD1,WORD2,WORD3,WORD4
*
*      THE SPRITE PATTERN REFERENCED BY THE SHAPE #
*      IS SET TO THE BIT PATTERN AS DEFINED BY THE
*      ARGUMENTS WORD1 TO WORD4.
*
SHAPEY DATA FGM$              FORCE GRAPHIC MODE
       EVFIX R1               N, GET SHAPE #
       MOVB R1,R1             VALID?
       JNE  ERR15             N, ERROR IT
       SLA  R1,3              R1=8* SHAPE#
       AI   R1,SPGBA1+>4000   POINT TO ITS STORAGE IN VRAM
       MOV  R8,R2             SAVE PBC
       MOV  R1,R8             SET R8 FOR SENDAD
       BL   @SENDAD           SEND ADDRESS TO VDP
       MOV  R2,R8             RESTORE PBC
       LI   R3,4              DO 4 WORDS
*
GETPAT CI   R0,>3F00          ',' ?
       JNE  ERR37             N, ERROR
       EVFIX R1               Y, GET NEXT WORD
       MOVB R1,@VRAM          WRITE MSB
       SWPB R1                POSITION LSB
       MOVB R1,@VRAM          SEND IT
       DEC  R3                DONE?
       JNE  GETPAT            N, LOOP
FNLIN  JMP  FNLIN0            Y, EXIT TO NLIN0
       PAGE
*
*              SPRITE STATEMENT
*             ==================
*
*      FORMAT :-
*           SPRITE PLANE,X,Y<,SHAPE#,COLOUR>
*
*      THE SPRITE ON THE SPECIFIED PLANE IS POSITIONED
*      AT X,Y. IF NO OTHER ARGUMENTS FOLLOW IT IS ASSUMED
*      THAT THE SPRITE CURRENTLY ON THAT PLANE IS TO BE
*      MOVED. IF OTHER ARGUMENTS ARE PRESENT THESE SPECIFY
*      THE NEW SHAPE # AND ITS COLOUR.
*
SPRITY DATA FGM$              FORCE GRAPHIC MODE
       EVFIX R15              Y, GET PLANE #
       MOV  R8,R14            SAVE PBC
       CI   R15,31            VALID ?
       JH   ERR15             N, ERROR IT
       SLA  R15,2             R15=4* PLANE#
       AI   R15,SNTBA1+2      REF ENTRY (LAST 2 BYTES ONLY)
       MOV  R15,R8            SET R8 FOR SENDAD
       BL   @SENDAD           SEND ADDRESS TO VDP
       MOV  *R8,*R8      <<< SLOW THE BLOODY THING DOWN >>>
       MOVB @VRAM,R2          GET SPRITE #
       SWPB R2
       AI   R8,>4000-2        REF START OF ENTRY (WRITE)
       MOVB @VRAM,R2          GET COLOUR
* R2 = COLOUR,SPRITE#
       MOV  *R8,*R8      <<< SLOW THE BLOODY THING DOWN >>>
       BL   @SENDAD           POINT VDP AT IT
       MOV  R14,R8            RESTORE PBC
       EVFIX R1               GET X
       EVFIX R3               GET Y
       SWPB R3                POSITION Y
       MOVB R3,R1             FORM R1=X,Y
       CI   R0,>3F00          ',' FOLLOWING ?
       JNE  PUTSAT            N, UPDATE ENTRY
*
       EVFIX R2               Y, GET SPRITE #
       EVFIX R3               GET COLOUR
       SWPB R3                N, POSITION IT
       MOVB R3,R2             FORM R2=COLOUR,SPRITE#
PUTSAT SWPB R2                POSITION COLOUR & SPRITE #
*
*            +-------------+
*    R1   =  |      Y      |    MSB
*            +-------------+
*            |      X      |    LSB
*            +-------------+
*    R2   =  |  SPRITE #   |    MSB
*            +-------------+
*            |   COLOUR    |    LSB
*            +-------------+
*
       STWP R3                GET WP
       INCT R3                POINT TO R1
       LI   R4,4              DO 4 BYTES
*
WRTIT  MOVB *R3+,@VRAM        WRITE IT BACK TO VRAM
       MOV  *R8,*R8      <<< SLOW THE BLOODY THING DOWN >>>
       DEC  R4                DONE ?
       JNE  WRTIT             N, LOOP
       JMP  FNLIN             Y, EXIT
       PAGE
*
*              MAG STATEMENT
*             ===============
*
*      FORMAT :-
*           MAG   SPRITE MAG,SPRITE SIZE
*
MAGY   DATA FGM$              FORCE GRAPHIC MODE
       EVFIX R2               GET SPRITE MAG
       EVFIX R1               GET SPRITE SIZE
       LI   R3,>00C0          DEFAULT TO SIZE=0,MAG=0
       MOV  R1,R1             BIG SPRITES ?
       JEQ  MAGY1             N, LEAVE R3
       INCT R3                Y, SET BIT
MAGY1  MOV  R2,R2             MAG THEM ?
       JEQ  MAGY2             N, LEAVE R3
       INC  R3                Y, SET BIT
MAGY2  SWPB R3                POSITION BYTE
       MOVB R3,@S$SM          SET UP LOADER TABLE
       BL   @LOADER           RE-ENABLE DISPLAY
S$SM   BYTE >C0,>80+R1        RELOAD VDP R1
       DATA 0
       JMP  FNLIN             EXIT
       PAGE
*      THESE ROUTINES UNPACK ENTRIES FROM THE
*      CHARACTER GENERATOR TABLE AND EITHER LOAD THEM
*      INTO THE VDP OR SAVE THEM IN A 8 BYTE STORAGE
*      AREA CALLED 'BITMAP'. LDCS ALSO SETS UP THE PNT
*
* CALLING SEQUENCE:
*      BLWP @UNPACK
*
* IN : R0   MSB=CHARACTER CODE TO BE UNPACKED
* OUT:      BIT PATTERN IN 'BITMAP'
*
*
* CALLING SEQUENCE:
*      BLWP @LDCS
* OUT:  CHARACTERS WRITTEN TO PGT
*       PNT INITIALIZED
*
*   WORKSPACE       :    VDPWP1
*   ROUTINE(S)      :    UNPACK,LDCS
*   REGISTER USAGE  :
*                                              GLOBAL DATA
*           ___________________________________   LABELS
*   R0      !                                 !
*   R1      !                                 !
*   R2      !                                 !
*   R3      !         BIT COUNTER             !
*   R4      !            ADDER                !
*   R5      !  2     UNPACKED DATA            !
*   R6      !         PACKED DATA             !
*   R7      !       COMPARE BIT MASK          !
*   R8      !      VRAM/STORAGE ADDRESS       !
*   R9      !      STORAGE INCREMENT          !
*   R10     !         BYTE COUNT              !
*   R11     !        BL RETURN ADDRESS        !
*   R12     !        TABLE POINTER            !
*   R13     !          RETURN WP              !
*   R14     !          RETURN PC              !
*   R15     !          RETURN ST              !
*           -----------------------------------
       PAGE
UNPACK DATA VDPWP1,$+2
       LI   R8,BITMAP         STORE IN 'BITMAP'
       LI   R9,1              STORAGE INCREMENT = 1
       MOV  *R13,R11          CHARACTER # FROM CALLERS R0
       SRL  R11,8             PUT IN LSB
       MPY  @C0006,R11        CALCULATE INDEX INTO TABLE
       AI   R12,PCHTB         ADD IN TABLE START
       LI   R10,8             UNPACK 8 BYTES WORTH
       JMP  MOD0              UNPACK
*
LDCS   DATA VDPWP1,$+2
* WRITE PATTERN NAMES TO PNT
       LI   R8,NTBA+>4000     REFERENCE PNT
       BL   @SENDAD           SEND ADDRESS TO VDP
       SETO R9                RESET COUNT
LDCS1  INC  R9                NEXT NAME
       SWPB R9                POSITION LSB
       MOVB R9,@VRAM          WRITE IT
       SWPB R9                RESTORE R9
       CI   R9,40*24          PNT FULL?
       JL   LDCS1             N, LOOP
* WRITE CHARACTER SET TO PGT
       LI   R8,PGBA+>4000     REFERENCE PGT
       BL   @SENDAD           SEND ADDRESS TO VDP
       LI   R8,VRAM           STORE IN VRAM
       CLR  R9                STORAGE INCREMENT = 0
       LI   R12,PCHTB         START AT BEGINNING OF TABLE
       LI   R10,256*8         CALCULATE UNPACKED SIZE
*
*  FALL THROUGH TO 'MOD0'
*
*
* UNPACK CHARACTER GENERATOR TABLE
*
MOD0   LI   R7,>0101          SET SHIFT MASK
       SETO R6                INIT. HOLDING REGISTER
MOD1   CLR  R5                INIT. DATA REGISTER
       LI   R4,>0001          INIT. ADDER
       LI   R3,6              DO 6 BITS
C0006  EQU  $-2
MOD2   SRC  R4,1              POSITION ADDER
       SRC  R7,1              POSITION MASK, NEW BYTE?
       JNC  MOD3              N, SKIP
       MOVB *R12+,R6          Y, GET NEW BYTE
MOD3   COC  R7,R6             HAVE WE A '1' HERE ?
       JNE  MOD4              N, VALUE OK
       AB   R4,R5             Y, ADD IN ADDER
MOD4   DEC  R3                COUNT BIT
       JGT  MOD2              LOOP TILL 6 BITS DONE
       MOVB R5,*R8            STORE UNPACKED BYTE
       A    R9,R8             UPDATE STORAGE POINTER
       DEC  R10               COUNT BYTE
       JGT  MOD1              LOOP TILL ALL BYTES DONE
       RTWP                   DONE, RETURN TO CALLER
       PAGE
*      THIS ROUTINE SETS,RESETS OR TESTS A SPECIFIED
*      PIXEL,DEPENDING ON ENTRY POINT. IF REFERENCE IS MADE
*      TO A PIXEL OFF SCREEN THEN AN IMMEDIATE RETURN IS
*      MADE.
*
* CALLING SEQUENCE:
*      BLWP @PIXOFF           RESET PIXEL
*           OR
*      BLWP @PIXON            SET PIXEL
*           OR
*      BLWP @PIXTST           TEST PIXEL
*      DATA <BYTE REPLY ADDRESS>
*
* IN : XLOC = X CO-ORDINATE
*      YLOC = Y CO-ORDINATE
*      FBCOL= FGND./BGND. COLOURS
*
* OUT  ST2  = BIT STATUS (PIXTST ONLY)
*
*   WORKSPACE       :    VDPWP0
*   ROUTINE(S)      :    PIXON,PIXOFF,SENDAD
*   REGISTER USAGE  :
*                                              GLOBAL DATA
*           ___________________________________   LABELS
*   R0      ! TMP. STORAGE  !  SHIFT COUNT    !
*   R1      !     X-LOC     !    Y-LOC        !  XLOC,YLOC
*   R2      !                                 !
*   R3      !                                 !
*   R4      !      **** RESERVED ****         !  BITMAP 0,1
*   R5      !      **** RESERVED ****         !         2,3
*   R6      !      **** RESERVED ****         !         4,5
*   R7      !      **** RESERVED ****         !         6,7
*   R8      !  Y ORDINATE / VRAM ADDRESS      !
*   R9      !  X ORDINATE / BIT MASK          !
*   R10     !        CELL DOT ROW             !
*   R11     !        BL RETURN ADDRESS        !
*   R12     !   SOCB/SZCB/COC INSTRUCTION     !
*   R13     !          RETURN WP              !
*   R14     !          RETURN PC              !
*   R15     !          RETURN ST              !
*           -----------------------------------
*
PIXTST DATA VDPWP0,$+2
       LI   R12,>2009         R12='COC R9,R0'
TMASK  EQU  $-2
       JMP  PIX1
*
PIXOFF DATA VDPWP0,$+2
       LI   R12,>5009         R12='SZCB R9,R0'
RMASK  EQU  $-2
       JMP  PIX1
*
PIXON  DATA VDPWP0,$+2
       LI   R12,>F009         R12='SOCB R9,R0'
*
PIX1   MOVB R1,R9             GET X ORDINATE
       SRL  R9,8              POSITION IT
       MOV  R1,R8             GET CO-ORDINATES
*  CALCULATE 32 x CELL ROW
       SLA  R8,2
       ANDI R8,>03E0
* CHECK TO SEE IF Y-ORDINATE IS LEGAL (X MUST BE!!)
       CI   R8,192*4          VALID?
       JHE  NBUMP             N, EXIT
*
       MOV  R1,R10            GET CO-ORDINATES
       ANDI R10,>0007         CALCULATE CELL DOT ROW
*  CALCULATE CELL DOT COLUMN, KEEP AS SHIFT COUNT IN R0
       MOV  R9,R0
       ANDI R0,>0007
*
       SRL  R9,3              CALCULATE CELL COLUMN
*  CALCULATE CELL # ( =CELL COLUMN+[32xCELL ROW] )
       A    R9,R8
       SLA  R8,3              FORM PGT INDEX
       A    R10,R8            REFERENCE REQUIRED ROW
*
*  IF THE PGT IS NOT LOCATED AT 0 THEN THIS NEEDS TO BE
*  ADDED TO R8 !!!
       ASMIF PGTBA1
       AI   R8,PGTBA1         ADD IN PGT BASE ADDRESS
       ASMEND
*
       BL   @SENDAD           SEND ADDRESS TO VDP
       CLR  R9                READY MASK REGISTER
       AI   R0,MASKTB         ADD IN MASK TABLE BASE
       MOVB *R0,R9            FETCH MASK
       MOVB @VRAM,R0          GET CURRENT VRAM DATA
       ORI  R8,>4000          SET WRITE BIT IN ADDRESS
       BL   @SENDAD           SEND ADDRESS TO VDP
       X    R12               SET/RESET/TEST BIT
       STST R9                SAVE STATUS
       C    R12,@TMASK        TEST ?
       JNE  WVRAM             N, UPDATE VRAM
*
*  TEST BIT EXIT ROUTINE
*
       ANDI R8,>BFFF          SET READ BIT IN ADDRESS
       AI   R8,CTBA1-PGTBA1   GET ADDRESS OF PCT ENTRY
       BL   @SENDAD           SEND ADDRESS TO VDP
       C    *R12+,*R12+       DUMMY DELAY FOR VDP
       MOVB @FBCOL,R8         GET COLOUR TABLE ENTRY
       COC  @C2000,R9         Y, 'EQ' SET?
       JNE  BITOFF            N, BIT IS BACKGROUND
       SRL  R8,4              Y, POSITION FGND COLOUR
BITOFF ANDI R8,>0F00          ISOLATE COLOUR
       MOV  *R14+,R9          PICK UP STORAGE ADDRESS
       MOVB R8,*R9            SAVE COLOUR
NBUMP  RTWP                   RETURN
*
*  SET/RESET EXIT ROUTINE
*
WVRAM  MOVB R0,@VRAM          WRITE UPDATED VRAM DATA
       C    R12,@RMASK        RESET PIXEL ?
       JEQ  NBUMP             Y, LEAVE COLOUR TABLE
*  NOW UPDATE COLOUR TABLE ENTRY
       AI   R8,CTBA1-PGTBA1   GET ADDRESS OF PCT ENTRY
       BL   @SENDAD           SEND ADDRESS TO VDP
       C    *R12+,*R12+       DUMMY DELAY FOR VDP
       MOVB @FBCOL,@VRAM      UPDATE COLOUR ENTRY
       RTWP                   EXIT
*
*   BIT MASK TABLE
*
*  PIXEL -   0 1   2 3   4 5   6 7
MASKTB DATA >8040,>2010,>0804,>0201
       END
