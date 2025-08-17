       TITL 'DEBUG MONITOR - CORTEX BASIC REV. 1.1'
       IDT  'MONITOR'
*
       DEF  DEBUG$,MONTOP
       DEF  WHXETY,RHENTY,WHENTY
       DEF  ECHOEN,WENTRY,RENTRY
       DEF  MENTRY,XOPENT
       DEF  ERR3$M,D$LDWP,D$LDPC
*
       REF  HALTO$,BEGN1,MC$LOD,MC$SAV
       REF  WORKS,ZZZZZ$
       REF  INITFG,BPT0V,BPTSET,BPTADD
       REF  BPTDTA,ASMOPC,MREGS,MREG3
       REF  MREG13,PC
       REF  B0D,GETCR$,TYP0$
       REF  UNIT,MCRLF,TYPEN$,TYPC$
       REF  BRAM,BADTER,BADADR
       REF  BELL,SPACE5,SPACE2,BPMSG
       REF  LOGON,SPR,REGSTR,ASKBP,EXMSG,SPBP
       REF  SETVEC,DSRCNT,STPMSG,SYMBLC,MODE
       REF  PROMPT,WS,EQUSGN,GTLN,BLSTOR,ASMPTR,IOB
       REF  ESCFLG,F$WHO,PADIT,TYPS$,WPR1
*
       COPY 'MACRO.INC'            
       PAGE   
************************************************************
*                                                          *
* MENU OF CORTEX COMMANDS (ALPHABETICAL)                   *
*                                                          *
*      ?    WHERE AM I                                     *
*      A    LINE BY LINE ASSEMBLER                         *
*            PC ADDRESS                                    *
*      B    BREAKPOINT SETTING                             *
*            BREAKPOINT NUMBER (CR=DISPLAY ALL BK. PTS.)   *
*            '-' = CLEAR BREAKPOINT LIST                   *
*      C    CRU INSPECT CHANGE                             *
*            BASE ADDRESS                                  *
*            NO. OF BITS                                   *
*      D    TAG DUMP                                       *
*            START ADDRESS                                 *
*            STOP ADDRESS                                  *
*            ENTRY ADDRESS                                 *
*            PROMPT FOR IDT                                *
*      E    EXECUTE - MESSAGES FOLLOW -                    *
*            PC                                            *
*      F    FIND ('CR' =WORD, '-' =BYTE)                   *
*            START ADDRESS                                 *
*            STOP ADDRESS                                  *
*            PATTERN                                       *
*      G    GOTO BASIC  (WARM START)                       *
*      I    INITIALIZE MEMORY                              *
*            START ADDRESS                                 *
*            STOP ADDRESS                                  *
*            VALUE                                         *
*      L    LOADER                                         *
*            PROMPT FOR IDT                                *
*      M    MEMORY INSPECT CHANGE                          *
*            START ADDRESS (CR-MODIFY)                     *
*            STOP ADDRESS                                  *
*      N    NEGATIVE FIND (FIND ANYTHING BUT PATTERN)      *
*            AS 'FIND'                                     *
*      P    OUTPUT PORT TOGGLE                             *
*      R    INSPECT/CHANGE WP,PC, AND ST REGISTERS         *
*             (- GIVES PREVIOUS REGISTER)                  *
*      S    SINGLE/MULTIPLE STEPPING                       *
*            VALUE LESS THAN >80=                          *
*             NO. OF STEPS (NULL OR 0 =1 )                 *
*            VALUE GREATER THAN >80=                       *
*             SINGLE STEP UNTIL MEM ADDR REACHED           *
*      T    TRACE-SINGLE STEP WITH PRINTOUT                *
*            NO. OF STEPS (NULL OR 0 =1 )                  *
*      U    UN-ASSEMBLER                                   *
*            START ADDRESS (CR-SINGLE LINE)                *
*            STOP ADDRESS                                  *
*      W    WORKSPACE REGISTER INSPECT/CHANGE              *
*            REGISTER NUMBER (CR-DUMP ALL REGS.)           *
*      X    TRANSFER (XFER) MEMORY                         *
*            START BYTE ADDRESS                            *
*            STOP  BYTE ADDRESS                            *
*            DESTINATION START BYTE ADDR                   *
*                                                          *
************************************************************
       PAGE
************************************************************
*  MODIFICATIONS DONE SINCE FIRST OPERATIONAL              *
*                                                          *
* BREAKPOINT HANDLER CHANGED SO AS NOT TO DELETE B.P.'S    *
*   WHEN THEY ARE 'HIT', ALSO CHANGED SO AS NOT TO CLEAR   *
*   BREAKPOINTS ON SYSTEM RESET.                           *
* OUTPUT ROUTINE: ESCAPE DETECT WRITTEN IN, SO AS TO GIVE  *
*  RETURN TO COMMAND SCANNER WHEN ESCAPE IS PRESSED        *
* ASSEMBLER AND DISASSEMBLER RE-WRITTEN SO AS TO USE       *
*  COMBINED TEXT/DATA TABLES.                              *
* NEGATIVE FIND ROUTINE ADDED                              *
* SINGLE STEP TO VALUE ADDED                               *
* MID, AO, & UNDEFINED XOP HANDLERS ADDED.                 *
* PRINT FORMAT IN 'R' & 'M' COMMANDS ALTERED SO AS TO USE  *
*  40 CHARACTER VDU'S                                      *
* BREAKPOINT HANDLING CHANGED SO AS TO REMOVE BPTS FROM    *
* USER RAM IN THE MONTOP. ALSO CHANGED SO THAT A SINGLE    *
* STEP IS EXECUTED BEFORE THE BPTS ARE SET.                *
* SINGLE STEP ROUTINES RE-WRITTEN SO AS TO USE LESS CODE   *
* USER VECTOR FOR BREAKPOINT ZERO INTRODUCED.              *
* PRINT FORMAT FOR 'F' & 'N' COMMANDS CHANGED              *
* SINGLE STEP/TRACE CHANGED SO AS NOT TO OUTPUT IN XOP'S   *
************************************************************
       PAGE
*
* MONITOR RE-ENTRY POINT
*
MONTOP LWPI MREGS
LOADWP EQU  $-2
D$LDWP EQU  LOADWP
       SETO @F$WHO
       BL   @RMVBPT           REMOVE BREAKPOINTS IF NEC.
BPTSOK EQU  $
*
* INITIALIZE LOAD ETC
*
       CLR  R0
       LI   R1,>FFFC          LOAD POINTER
       MOV  @LOADWP,*R1+      COPY WORKSPACE POINTER
D$LDPC EQU  $+2
       LI   R2,LOAD           LOAD 2ND POINTER
       MOV  R2,*R1+           COPY LOAD ENTRY POINT
       MSG  @PROMPT
*
* WAIT FOR A COMMAND ENTRY
*
CMDIN  READ R5
       CLR  R2                CLEAR KEY
       CLR  R3                CLEAR COUNT
       LI   R8,1              WORD BOUNDRY REG (LSB= 1)
       LI   R9,SPACE2
       LI   R10,MCRLF
       BL   @SRCH
       PAGE
*
* COMMAND SEARCH TABLE
*
       TEXT 'M'               MEMORY INSPECT/CHANGE
       BYTE 3                  (START,STOP)
       DATA M
       TEXT 'D'               DUMP MEMORY TO CASSETTE
       BYTE 7                  (START,STOP,ENTRY)
       DATA MC$SAV              (NOW IN BASIC)
       TEXT 'I'               INITIALIZE MEMORY COMMAND
       BYTE 7                  (START,STOP,VALUE)
       DATA I
       TEXT 'A'               ZERO LABEL ASSEMBLER
       BYTE 1                  (START PC)
       DATA ZLABGN
       TEXT 'U'               UNASSEMBLER
       BYTE 3                  (START,STOP)
       DATA U
ADREVN TEXT 'W'               USER WORKSPACE INSPECT/CHANGE
       BYTE 1                  (REG NO.)
       DATA W
       TEXT 'E'               EXECUTE
       BYTE 0                  (PC ADDR-OPTIONAL)
       DATA E
       TEXT 'B'               BREAKPOINT
       BYTE 1                  (BREAKPOINT NO.)
BPCMD  DATA B
       TEXT 'S'               EXECUTE SINGLE STEPS
       BYTE 3                  (VARIOUS)
       DATA S
       TEXT 'L'               LOAD MEMORY FROM CASSETTE
       BYTE 0
       DATA MC$LOD               (NOW IN BASIC)
       TEXT 'C'               CRU INSPECT/CHANGE
       BYTE 3                  (CRU BASE, # OF BITS)
       DATA C
       TEXT 'R'               WP,PC,ST INSPECT/CHANGE
       BYTE 0
       DATA R
       TEXT 'F'               FIND BYTE/WORD
       BYTE 7                  (START,STOP,VALUE)
       DATA F
       TEXT 'N'               NEGATIVE FIND BYTE/WORD
       BYTE 7                  (START,STOP,VALUE)
       DATA N
       TEXT 'T'               TRACE STEP AND PRINT WP,PC,ST
       BYTE 1                  (# OF STEPS)
       DATA T
       TEXT 'X'               TRANSFER (XFER) MEMORY
       BYTE 7                  (START,STOP,DEST.)
       DATA X
       TEXT 'P'               TOGGLE OUTPUT PORT
       BYTE 1                 (UNIT #)
       DATA P
       TEXT 'G'               GOTO BASIC (WARM START)
       BYTE 0
       DATA G
       TEXT '?'               WHERE AM I?
       BYTE 0
       DATA QUERY
       BYTE 0,0,0,0           SPARE ENTRY
       BYTE 0,0,0,0           SPARE ENTRY
       DATA 0                 END OF TABLE
       TITL 'COMMAND SEARCH AND ERROR HANDLER'
*
* COMMAND SEARCH ROUTINE
*
SRCHLP AI   R11,3             UPDATE POINTER
SRCH   MOV  *R11,R7           SEARCH FAIL?
       JEQ  ERR4              YES, ERROR
       CB   *R11+,R5          DOES INPUT MATCH A TABLE ENTRY
       JNE  SRCHLP            NO, TRY NEXT TABLE ENTRY
       WRIT R5
       WRIT *R9
       MOVB *R11+,R6          NUMBER OF HEX INPUTS TO ICOUNT
*
* ICOUNT SPECIFIES NUMBER OF HEX INPUT FIELDS
*
       SRL  R6,8              ALIGN ICOUNT
       STWP R7                POINT TO R0
INLOOP SRL  R6,1              DONE?
       JNC  CEXIT             YES, TO COMMAND PROCESSOR
       RHXW R4                ACCEPT HEX ENTRY
       DATA NULL,ERR2
       MOV  R4,*R7+           SAVE HEX INPUT VIA POINTER
CNT    INC  R3                COUNT THE NUMBER OF ENTRIES
       CB   R5,*R10           END OF INPUT?
       JNE  INLOOP            YES, TO COMMAND PROCESSOR
CEXIT  CI   R11,ADREVN        EVEN BOUNDARY REQUIRED
       JH   BRCMD             NO - JUMP
       SZC  R8,R0
       SZC  R8,R1
BRCMD  MOV  *R11,R11          GET ENTRY ADDRESS
       BL   *R11              YES, TO COMMAND PROCESSOR
       JMP  MONTOP            RETURN FROM COMMANDS
NULL   INCT R7                UPDATE POINTER
       CB   R5,*R10           NO INPUT?
       JEQ  CEXIT             Y, GOT COMMAND
       CI   R11,BPCMD         TEST FOR BREAKPOINT
       JNE  CNT               NO - DEFAULT PARAMETER
       CI   R5,'-'*256        TEST FOR '-'
       JNE  CNT               NO - DEFAULT
       LI   R8,BPTADD
       JMP  CLRBPT            YES, CLEAR BREAKPOINTS
*
* ERROR HANDLER
*
ERR2   LI   R1,BADTER         TERM. CHARACTER ERROR
       JMP  OUTERR
ERR3$M EQU  $
ERR3   LI   R1,BADADR         DUMP ADDRESS ERROR
*
OUTERR DATA TYPC$             OUT 'CRLF'
       DATA TYPEN$            OUT ERROR TEXT
JMMONT JMP  MONTOP            RETURN TO MONITOR
ERR4   MSG  @BELL
       JMP  CMDIN
*
*      INITIAL ENTRY POINT
*
DEBUG$ LWPI MREGS
BANNER MSG  @LOGON            OUTPUT BANNER
       LI   R8,INITFG         POINT TO INIT FLAG
       C    *R8,@INITIZ       TEST INIT. FLAG
       JEQ  JMMONT            IF SET, TO MONTOP
       MOV  @INITIZ,*R8+      SET FLAG
CLRBPT CLR  *R8+              CLEAR BOTH TABLES
       CI   R8,BPTADD+64
       JL   CLRBPT
       JMP  JMMONT            TO TOP OF MONITOR
************************************************************
*
*                EXIT TO BASIC
*
************************************************************
G      BLWP @BVEC1            WARMSTART VECTORS
BVEC1  DATA WPR1,BEGN1
       PAGE
************************************************************
*                                                          *
* TRANSFER (XFER) COMMAND -- 'X'                           *
*                                                          *
*  CALLING SEQUENCE:          BL   X                       *
*                       R0=START BYTE ADDRESS              *
*                       R1=STOP BYTE ADDRESS               *
*                       R2=DESTINATION START ADDR          *
*                                                          *
*  RETURN                     RT                           *
*                                                          *
* IF THE DESTINATION ADDRESS IS BETWEEN THE START AND STOP *
*  ADDRESS, THE TRANSFER IS BOTTOM UP TO AVOID COPYING     *
*  OVER THE ORIGINAL BEFORE IT IS TRANSFERED.              *
*                                                          *
************************************************************
*
X      C    R0,R1             START < STOP
XERR   JH   ERR3               N, EXIT
       C    R2,R0             DEST. LESS THAN START?
       JLE  TOPDN              Y, TOP DOWN
       C    R2,R1             DEST. GREATER THEN STOP?
       JH   TOPDN              Y, TOP DOWN
       MOV  R1,R3              N, BOTTOM UP
       S    R0,R3
       A    R3,R2             DEST BOT= DEST+(START-STOP)
BOTUP  MOVB *R1,*R2
       DEC  R1                BUMP POINTERS
       DEC  R2
       C    R1,R0             PAST START?
       JHE  BOTUP              N, CONTINUE
       RT                      Y, EXIT TO MONITOR
TOPDN  MOVB *R0+,*R2+
       C    R0,R1             PAST STOP?
       JLE  TOPDN              N, CONTINUE
       RT                      Y, EXIT TO MONITOR
*
       TITL '*** INITIALIZE COMMAND ***'
       PAGE
************************************************************
* INITILIZE MEMORY COMMAND -- 'I'                          *
*                                                          *
* FILL MEMORY WITH THE KEY FROM 'START' TO 'STOP' BY WORDS *
*                                                          *
* CALLING SEQUENCE:           BL    I                      *
*                        R0=START                          *
*                        R1=STOP                           *
*                        R2=KEY                            *
*                                                          *
* RETURN                      B     *R11                   *
*                                                          *
************************************************************
*
I      C    R0,R1             TEST FOR VALID START/STOP
       JH   XERR
I11    MOV  R2,*R0+           DATA TO MEMORY
       C    R0,R1             DONE?
       JNE  I11               NO, LOOP
       MOV  R2,*R0+
       RT                     EXIT
*
       TITL '*** FIND COMMAND ***'
       PAGE
************************************************************
*                                                          *
* FIND COMMAND -- 'F'                                      *
*                                                          *
* LOOK FROM START ADDRESS TO STOP ADDRESS FOR              *
* THE SPECIFIED DATA PATTERN.                              *
*      R0-START ADDRESS                                    *
*      R1-STOP ADDRESS                                     *
*      R2-PATTERN                                          *
* THE TERMINATION CHARACTER DETERMINES THE SEARCH          *
* INCREMENT.                                               *
*      CARRAIGE RETURN - WORD                              *
*      MINUS SIGN      - BYTE                              *
*                                                          *
************************************************************
*
F      LI   R6,SKIP2-SKIP11/2-1+>1600  JUMP NOT EQUAL
FIND   LI   R3,>8402          LOAD COMPARE- C  R2,*R0
       LI   R4,>5C0           LOAD - INCT  R0
       CB   R5,*R10           TCHAR='CR'?
       JEQ  SKIP0             YES,LEAVE AS C, INCT
       AI   R3,>1000          CHANGE TO- CB
       AI   R4,->40           CHANGE TO- INC
       SLA  R2,8              ALIGN PATTERN
       JMP  SKIP1
SKIP0  SZC  R8,R0
       SZC  R8,R1
       JMP  SKIP1
NXTBLK MOV  R7,R7             DONE 4 ADDRESSES?
       JNE  NOCRTN            NO - JUMP
SKIP1  LI   R7,4
       MSG  *R10
NOCRTN X    R3                EXECUTE THE COMPARE
SKIP11 X    R6                JNE (FIND) JEQ (-VE FIND)
       WHXW R0                OUTPUT ADDRESS
       MSG  *R9               TWO SPACES
       DEC  R7
SKIP2  X    R4                EXECUTE THE INCREMENT
       JOC  FEXIT             AVOID >FFFF TRAP!
       C    R0,R1             DONE?
       JLE  NXTBLK            NO, LOOP
FEXIT  RT
************************************************************
*                                                          *
* NEGATIVE FIND COMMAND.                                   *
*  THE OPERATION OF THIS COMMAND IS THE SAME AS THE 'FIND' *
*  COMMAND, BUT WILL OUTPUT ADDRESS OF DATA WHICH IS NOT   *
*  THE SAME AS THE INPUT PATTERN                           *
*                                                          *
************************************************************
N      LI   R6,SKIP2-SKIP11/2-1+>1300         =JUMP EQUAL
       JMP  FIND
       TITL '***MEMORY INSPECT/CHANGE***'
       PAGE
************************************************************
*                                                          *
* INSPECT/CHANGE MEMORY - 'M' COMMAND                      *
*                                                          *
* OPTIONS:                                                 *
*          1) START ADDRESS, CARRIAGE RETURN --            *
*             DISPLAY ADDRESS, CONTENTS, AND               *
*             OPEN THE MEMORY LOCATION FOR A CHANGE.       *
*                                                          *
*          2) CARRIAGE RETURN -- SAME AS 1) BUT THE        *
*             DEFAULT START ADDRESS IS 0000.               *
*                                                          *
*          3) START ADDRESS, BLANK (OR COMMA), STOP        *
*             STOP ADDRESS, CARRIAGE RETURN -- OUTPUT      *
*             MEMORY CONTENTS FROM START ADDRESS TO        *
*             STOP ADDRESS. DEFAULT VALUES FOR BOTH        *
*             ADDRESSES ARE 0000.                          *
*                                                          *
************************************************************
*
M      DEC  R3                1 INPUT?
       JEQ  MIC               YES, TO MEMORY INSPECT/CHANGE
*
* MEMORY DUMP ROUTINE
*
MLOOP1 LI   R3,4
       MSG  *R10              NEXT LINE
       WHXW R0                PRINT FIRST ADDRESS
       MSG  @EQUSGN           DELIMITER
MLOOP2 WHXW *R0+              PRINT MEMORY CONTENTS
       MOV  R0,R0             AVOID THE >FFFF TRAP!
       JEQ  MEXIT
       C    R0,R1             DONE?
       JH   MEXIT             YES, BACK TO THE MONITOR
       DEC  R3                DONE WITH LINE?
       JEQ  MLOOP1            YES, NEW LINE
       WRIT *R9               NO, OUTPUT SPACE
       JMP  MLOOP2
       PAGE
*
* MEMORY INSPECT/CHANGE ROUTINE
*
MIC1   DECT R0                LAST ADDRESS?
       CI   R5,'-'*256
       JEQ  MIC               YES
       AI   R0,4              POINT TO NEXT ADDRESS
MIC    DATA TYPC$             NEXT LINE
       WHXW R0                PRINT MEMORY ADDRESS
       MSG  @EQUSGN           PRINT '='
       MOV  *R0,R4            GET DATA
       WHXW R4                PRINT MEMORY CONTENTS
       MSG  *R9               DELIMITER
       RHXW R4                ACCEPT NEW INPUT
       DATA MNULL,ERR2
       MOV  R4,*R0            UPDATE CONTENTS
MNULL  CB   R5,*R10           RETURN TO COMMAND SCANNER?
       JNE  MIC1
MEXIT  RT                     EXIT TO MONITOR
*
       TITL '***WORKSPACE REGISTER INSPECT/CHANGE***'
       PAGE
************************************************************
*                                                          *
* INSPECT/CHANGE USER WORKSPACE REGISTER -- 'W' COMMAND    *
*                                                          *
* OPTIONS: 1) 'W' FOLLOWED BY CARRIAGE RETURN --           *
*              DISPLAY THE CONTENTS OF ALL CURRENT USER    *
*              WORKSPACE REGISTERS AND RETURN TO THE       *
*              COMMAND SCANNER.                            *
*                                                          *
*          2) 'W', REGISTER NUMBER IN HEX, CARRIAGE        *
*              RETURN -- DISPLAY THE CONTENTS OF THE       *
*              DESIGNATED REGISTER. USER MAY ALTER         *
*              THE CONTENTS FOLLOWED BY A TERMINATION      *
*              CHARACTER OR MERELY ENTER A TERMINATION     *
*              CHARACTER. THE TERMINATION CHARACTER        *
*              SIGNIFIES WHAT IS TO BE DONE NEXT:          *
*                                                          *
*     SPACE -- DISPLAY THE CONTENTS OF THE NEXT REG.       *
*     MINUS -- DISPLAY THE CONTENTS OF THE PREVIOUS REG.   *.
*     CARRIAGE RETURN -- TO THE COMMAND SCANNER.           *
*                                                          *
* NOTE THAT THIS ROUTINE IS ALSO USED BY THE INSPECT/CHANGE*
* BREAKPOINTS.                                             *
************************************************************
B      LI   R7,BPTADD         SET UP POINTER
       LI   R8,SPBP           SET UP BREAKPOINT INITIALS
       JMP  BPOINT            USE THE INS/CH WS REGR ROUTINE
*
W      MOV  R13,R7            GET WORKSPACE POINTER
       LI   R8,SPR            SET TO PRINT ' R'
BPOINT MOV  R3,R3             NULL INPUT?
       JEQ  WNULL1            YES, TO FORMATTED DUMP
*
* INSPECT/CHANGE A WORKSPACE REGISTER OR BREAKPOINT
*
       ANDI R0,>F             ISOLATE NUMBER
       MOV  R0,R6             SAVE R/BPT NUMBER
       SLA  R0,1
       A    R0,R7             FORM R/BPT ADDRESS
ICLOOP MSG  *R10              NEXT LINE
       MSG  *R8
       WNBL R6                OUTPUT R/BPT NUMBER
       MSG  @EQUSGN           PRINT '='
       MOV  *R7,R4
       WHXW R4                PRINT RR/BPT CONTENTS
       WRIT *R9               DELIMITER
       RHXW R4                NEW CONTENTS?
       DATA WNULL2,ERR2
       MOV  R4,*R7            UPDATE R/BPT
WNULL2 CB   R5,*R10           RETURN TO COMMAND SCANNER?
       JNE  SKIP              NO, CHECK FOR ' '
WEXIT  RT                     TO SCANNER
SKIP   CB   R5,*R9            NEXT R/BPT?
       JEQ  NREG              YES
*
* CHECK FOR REGISTER OR BREAKPOINT ZERO
*
       MOV  R6,R6             AT REGISTER 0?
       JEQ  WEXIT             YES, TO SCANNER
       DEC  R6                UPDATE R/BPT NUMBER
       DECT R7                UPDATE ADDRESS
       JMP  ICLOOP
*
* CHECK FOR REGISTER OR BREAKPOINT >F
*
NREG   CI   R6,>F
       JEQ  WEXIT             R/BPT >F, TO SCANNER
       INC  R6                UPDATE R/BPT NUMBER
       INCT R7                UPDATE ADDRESS
       JMP  ICLOOP
*
* FORMATTED REGISTER OR BREAKPOINT DISPLAY
*
WNULL1 LI   R6,->10
NLINE  MSG  *R10              NEXT LINE
WLOOP  MSG  *R8
       WNBL R6                R/BPT NUMBER
       MSG  @EQUSGN           PRINT '='
       WHXW *R7+              PRINT CONTENTS
       INC  R6                TO NEXT R/BPT
       JEQ  WEXIT             DONE?
       CZC  @C3,R6            4 ENTRIES ON THE LINE?
       JEQ  NLINE             YES, NEXT LINE
       JMP  WLOOP             NO, CONTINUE ON THIS LINE
*
       TITL '***CRU INSPECT/CHANGE***'
       PAGE
************************************************************
*                                                          *
* CRU INSPECT/CHANGE -- 'C' COMMAND                        *
*                                                          *
* INPUT THE CRU BASE ADDRESS FOLLOWED BY THE BIT COUNT.    *
* ALL INPUT AND OUTPUT TO THE CRU IS RIGHT JUSTIFIED IN    *
* THE 16 BIT INPUT/OUTPUT DATA FIELDS.                     *
*                                                          *
* INPUT OF A CARR. RET. AS A TERMINATION CHARACTER RETURNS *
* CONTROL TO THE COMMAND SCANNER. A ' ' AS TERMINATION     *
* CHARACTER CAUSES THE CRU INPUT BITS TO BE OUTPUT AGAIN   *
* WELL AS THE CRU OUTPUT BITS TO BE CHANGED.               *
*                                                          *
************************************************************
*
C      MOV  R0,R12            UPDATE CRU BASE REGISTER
       CLR  R7                RESET WORD FLAG
       SLA  R1,12             ISOLATE BIT COUNT
       JEQ  SETFG             SET FLAG FOR 16-BIT TRANS.?
       CI   R1,>9000          BYTE JUSTIFIED I/O?
       JL   CSKIP1            YES, SKIP
SETFG  INC  R7                WORD JUSTIFIED I/O FLAG
*
* FORM 'STCR' COMMAND AND READ CRU
*
CSKIP1 SRL  R1,6              POSITION BIT COUNT
CLOOP  LI   R8,>3404          'STCR R4' OP CODE
       SOC  R1,R8             COMBINE WITH BIT COUNT
       X    R8                EXECUTE 'STCR'
*
* OUTPUT STATE OF CRU
*
       MSG  *R10              NEXT LINE
       WHXW R12               PRINT BASE ADDRESS
       MSG  @EQUSGN           PRINT '='
       MOV  R7,R7             WORD I/O?
       JNE  CSKIP2            YES, SKIP ALIGN
       SRL  R4,8              ALIGN BYTE INPUT TO WORD
CSKIP2 WHXW R4                PRINT CRU DATA
       MSG  *R9               DOUBLE SPACE
*
* ACCEPT INPUT FOR ALTERATION OF CRU
*
       RHXW R4                CRU OUTPUT?
       DATA CNULL,ERR2
       MOV  R7,R7             WORD I/O?
       JNE  CSKIP3            YES, SKIP ALIGN
       SLA  R4,8              ALIGN BYTE FOR OUTPUT
CSKIP3 ANDI R8,>F3FF          CHANGE 'STCR' TO 'LDCR'
       X    R8                EXECUTE 'LDCR'
CNULL  CB   R5,*R10           EXIT?
       JNE  CLOOP             NO LOOP
       RT                     EXIT TO MONITOR
       TITL '***WP,PC,ST INSPECT/DISPLAY***'
       PAGE
************************************************************
*                                                          *
* DISPLAY WP, PC, ST REGISTERS                             *
*                                                          *
* TERMINATION CHARACTERS:                                  *
*     SPACE -- TO NEXT REGISTER                            *
*     CARRIAGE RETURN -- TO MONITOR SCANNER                *
*     MINUS -- INSPECT PREVIOUS REGISTER                   *
*                                                          *
* ORDER OF DISPLAY: WP, PC, ST.                            *
*                                                          *
************************************************************
*
R      LI   R6,WS+4           INIT. MESSAGE POINTER
       INC  R8                SET LOOP COUNT
       LI   R7,MREG13+2       POINT TO WP+2
BACKWP AI   R6,-4
       DECT R7
       INC  R8
       CI   R8,4
       JHE  REXIT
RLOOP  DATA TYPC$
       MSG  *R6               OUTPUT REGISTER SLOGAN
       MOV  *R7,R4
       WHXW R4                PRINT CONTENTS
       MSG  *R9               DELIMITER
       RHXW R4                NEW DATA?
       DATA RNULL,ERR2
       MOV  R4,*R7
RNULL  CI   R5,'-'*256        PREVIOUS REGISTER?
       JEQ  BACKWP            YES, THEN GO BACK
       CB   R5,*R10           TO SCANNER?
       JEQ  REXIT             YES, EXIT
       C    *R6+,*R6+         UPDATE MESSAGE POINTER BY 4
       INCT R7                POINT TO NEXT OUTPUT
       DEC  R8                DONE?
       JNE  RLOOP             NO
REXIT  RT                     EXIT TO MONITOR
*
       TITL 'SINGLE STEPPING, BKPT ZERO SUBROUTINE'
       PAGE
************************************************************
*                                                          *
* SINGLE STEP COMMAND -- 'S'                               *
*                                                          *
*    THE SINGLE STEP ENTRY HAS BEEN MODIFIED TO ALLOW FOR  *
* MULTIPLE STEPPING.  IT REQUIRES AN ENTRY WHILE IN THE    *
* SCANNER.                                                 *
*                                                          *
************************************************************
*
S      LI   R7,>9999          SET CODE
       DEC  R3                STEP TO VALUE?
       JGT  VALADR            YES, JUMP
*
       DECT R7                NEXT CODE (>9997)
       CI   R0,>80            STEPS OR MEM ADR?
       JL   SINGLE            STEPS - JUMP
*
* STEP UNTIL MEMORY ADDRESS CONTAINS VALUE
*
VALADR SZC  R8,R0             WORD ALIGN ADDRESS
       JMP  STEP              GO TO STEPPER
*
*  SINGLE/MULTIPLE STEPS
*
SINGLE DECT R7                NEXT CODE (>9995)
LDCNT  MSG  *R10              GO TO NEW LINE
DECSTP DEC  R0                COUNTING REGISTER
STEP   LREX                   INITIATE THE LOAD VECTOR
GO     RTWP                   GOTO USER PROGRAM
*
STPCHK MOV  R0,R0             DONE STEPPING?
       JGT  DECSTP            NO, DECREMENT AND LREX AGAIN
       JMP  WPS10             YES, OUTPUT ONCE
**********************************************************
*        USER SUBROUTINE ON BREAKPOINT ZERO              *
**********************************************************
CONTIN BL   *R0               EXECUTE USER ROUTINE
       MOV  @BPTDTA,*R14      INSERT USER INSTRUCTION
       MOV  R14,R0            SAVE LOCATION
       LI   R2,BPTLOD         GET LOAD ADDRESS
       JMP  SETLOD            DO A SINGLE STEP
BPTLOD MOV  @COD15,*R0        BREAKPOINT TO RAM
       RTWP
       TITL 'LOAD ENTRY POINT, TRACE'
       PAGE
*
* LOAD INTERRUPT ENTRY POINT
*
*      POWER UP:      R7=RANDOM VALUE
*      SINGLE STEP:   R7=>9995
*      SS TO ADR:     R7=>9997
*      SS TO VALUE:   R7=>9999
*      TRACE:         R7=>9900
*
LOAD   CI   R7,>9997          MEM ADDRESS STEP?
       JNE  CHKVAL            NO, JUMP
       C    R0,R14            REACHED REQUIRED ADDRESS?
CHKDUN JNE  STEP              DONE?
PRTSTP MSG  @STPMSG           GO PRINT DATA
       JMP  WPS10
CHKVAL CI   R7,>9999          TEST FOR VALUE STEP
       JNE  CHKSGL            IF NOT, JUMP
       C    *R0,R1            CHECK IF ADR=VAL
       JMP  CHKDUN            GO FIX IT
CHKSGL CI   R7,>9995          SINGLE STEP?
INITIZ EQU  $-2
       JEQ  STPCHK
       CI   R7,>9900          TRACE STEP?
       JEQ  WPS10             YES, OUTPUT WP, PC, ST, INSTR
*
* IF NOT SINGLE STEP OR TRACE, IT IS A SYSTEM INITIALIZATION
*
       CLR  @DSRCNT           CLEAR DSR COUNTER
       BL   @SETVEC           RESET ALL IMPORTANT THINGS
       B    @DEBUG$           GOTO MONITOR
************************************************************
*                                                          *
*  TRACE COMMAND -- 'T'                                    *
*                                                          *
* MULTIPLE STEPPING WITH WP, PC, ST, AND INSTRUCTION       *
*  PRINTED AFTER EACH STEP                                 *
*                                                          *
************************************************************
*
T      LI   R7,>9900          SET TRACE FLAG
       MOV  R11,R5            SAVE RETURN ADDR
       BL   @LDCNT            GO TO THE STEPPER
*
* TRACE COMES HERE AFTER PRINTING WP, PC, ST, AND INSTR
*
* CHECK TRACE COUNT
*
       MOV  R0,R0             IF COUNT=0 DO ONLY ONE
       JGT  LDCNT             IF >0 DECREMENT AND STEP
       B    *R5               RETURN TO MONITOR
       TITL '***EXCEUTE***'
       PAGE
************************************************************
*                                                          *
* EXECUTE COMMAND -- 'E'                                   *
*                                                          *
*  ASK USER IF BREAKPOINTS ARE TO BE SET.  IF SO SET ALL   *
*  BREAKPOINTS WHICH DO NOT HAVE A ZERO ADDRESS.           *
*  OUTPUT THE WP,PC,ST DATA.  WAIT FOR USER INPUT.         *
*  IF A PARAMETER IS ENTERED IT REPLACES THE PC BEFORE     *
*  THE RTWP.  OTHERWISE ONLY THE RTWP IS EXECUTED.         *
*                                                          *
************************************************************
*
E      MSG  @ASKBP            ASK IF BP'S TO BE SET
       EKO  R4                ANSWER IN R4
       CI   R4,>1B00          ESCAPE?
ESC    EQU  $-2
       JEQ  TOPRTN            YES, EXIT
       DATA TYPC$             NO, OUT CRLF
       MSG  @EXMSG            'EXECUTE'
       LI   R7,MREG13         GET WP POINTER
       LI   R6,WS             POINT TO MESSAGE
WSPLP  MSG  *R9               OUTPUT 2 SPACES
       CI   R7,MREG13+6       DONE?
       JEQ  INPC              YES, JUMP
       MSG  *R6+              OUTPUT MESSAGE & INC POINTER
       WHXW *R7+              OUTPUT DATA & INC. POINTER
       INCT R6                POINT TO NEXT MESSAGE
       JMP  WSPLP             LOOP BACK
INPC   RHXW R1                GET NEW PC (IF ANY)
       DATA PCOK,ERR2
       MOV  R1,R14            TRANSFER NEW PC
PCOK   CI   R4,'Y'*256        SET BK PTS?
       JNE  GO                IF NOT, BYPASS
       MOV  @INITIZ,@BPTSET   SET BREAKPOINT FLAG
       LI   R2,EBPTLP         GET LOAD ADDRESS
SETLOD LI   R1,>FFFE          SET POINTER ( & COUNTER)
       MOV  R2,*R1            MOVE TO LOAD VECTORS
       JMP  STEP              GO TO STEPPER
*
EBPTLP INCT R1                UPDATE BPT NUMBER
       CI   R1,32             DONE?
       JH   GO                YES - EXECUTE
       MOV  R1,R2             GET NUMBER
       SRL  R2,1              DIVIDE BY TWO
       MOV  @BPTADD(R1),R3    GET BP ADDRESS
       JEQ  EBPTLP            ZERO - BYPASS
       AI   R2,>0FC0          ADD BKPT MID OPCODE
COD15  EQU  $-2
       MOV  *R3,@BPTDTA(R1)   SAVE DATA
       MOV  R2,*R3            INSTALL BP
       JMP  EBPTLP            AND LOOP FOR NEXT
*
       TITL '*** BREAKPOINT HANDLER ***'
       PAGE
*
* MID >0F80 ENTRY POINT (BREAKPOINT)
*
XOPENT LIMI 0                 GIVE LOBUG COMPLETE CONTROL
       DECT R14               UPDATE USER PC
       MOV  *R14,R1           GET BREAKPOINT OPCODE
       ANDI R1,>F             ISOLATE THE NUMBER
       JNE  OPBPMS            NOT ZERO, JUMP
       MOV  @BPT0V,R0         GET USER VECTORS (IF ANY)
       JNE  CONTIN            ZERO, NO VECTORS
OPBPMS MSG  @BPMSG            'BP:'
       WNBL R1
       SLA  R1,1              WORD ALIGN
       MOV  @BPTDTA(R1),*R14  REPLACE INSTRUCTION
       BL   @RMVBPT           REMOVE BREAKPOINTS FROM RAM
       LI   R11,MONTOP        RESTORE MONITOR RETURN
*
* OUTPUT USER WP, PC, AND ST AT BREAKPOINT
*
WPS10  CI   R14,ZZZZZ$        INTERNAL ROUTINE?
       JL   STEP              YES, IGNORE AND STEP AGAIN
*
* '?' (WHERE AM I) COMMAND ENTRY POINT
*
QUERY  EQU  $
WPS20  LI   R8,MREG13         USER WP, PC, ST
       LI   R6,WS             MSG POINTER
BLOOP  MSG  *R9               PRINT 2 SPACES
       CI   R8,MREG13+6       DONE?
       JEQ  BINST             YES, PRINT INSTRUCTION NEXT
       MSG  *R6               PRINT 'WP=,PC=,ST=' MSG
       WHXW *R8+              GET WP, PC, ST
       C    *R6+,*R6+         ADD 4 TO MSG POINTER
       JMP  BLOOP
*
* OUTPUT DATA AND INSTRUCTION AT PC ADDRESS
*
BINST  DATA TYPS$,PADIT       OUT CRLF AND 8 SPACES
       BLWP @INSTR            CALL UNASSEMBLER
       RT                     EXIT TO MONITOR (OR TRACE)
       TITL 'TOGGLE OUTPUT PORT'
       PAGE
************************************************************
*                                                          *
*  OUTPUT PORT TOGGLE ----- 'P'                            *
*                                                          *
************************************************************
P      MOV  R0,@UNIT
TOPRTN B    @MONTOP
*
* SUBROUTINE TO REMOVE BREAKPOINTS FROM USER PROGRAM
*
RMVBPT LI   R1,BPTSET         SET POINTER
       C    *R1,@INITIZ       INITIALIZED?
       JNE  RMVRTN            NO - EXIT
       CLR  *R1+              CLEAR FLAG
REPLP  MOV  *R1,R3            GET ADDRESS (IF ANY)
       JEQ  NEXTBP            IF ZERO, IGNORE
       MOV  @32(R1),*R3       REPLACE INTRUCTION
NEXTBP INCT R1
       CI   R1,BPTADD+32      FINISHED?
       JL   REPLP
RMVRTN RT
*
       TITL '***CHARACTER ECHO***'
       PAGE
************************************************************
*                                                          *
* ECHO A CHARACTER TO THE TERMINAL (EKO)    **USES EREGS** *
*                                                          *
* --WARNING-- SYSTEM FLAGS ARE KEPT IN R0-R7 OF EREGS      *
*                                                          *
*      CALLING SEQUENCE:      EKO   Register               *
*                                                          *
*      RETURN                 RTWP                         *
*                                                          *
* THE CHARCTER IS RETURNED IN THE MOST SIGNIFICANT BYTE    *
*  OF THE REGISTER WITH THE LOWER BYTE ZEROED.             *
************************************************************
*
ECHOEN READ *R11              READ CHARACTER
       WRIT *R11              ECHO THE CHARACTER
       RTWP
*
       TITL '***READ CHARACTER***'
       PAGE
************************************************************
* READ CHARACTER  (READ)      ** USES IREGS **             *
*                                                          *
*   CALLING SEQUENCE:         READ   Register              *
*                                                          *
*   RETURN                    RTWP                         *
*                                                          *
* READ WAITS FOR A CHARACTER TO BE ASSEMBLED IN            *
* THE UART. THE CHARACTER IS PLACED IN THE LEFT            *
* BYTE OF USER REGISTER. THE RIGHT BYTE IS ZEROED          *
*                                                          *
************************************************************
*
RENTRY MOV  R0,R12            SAVE R0
       DATA GETCR$            GET A CHARACTER
       MOV  R0,*R11           RETURN CHARACTER TO CALLER
       MOV  R12,R0            RESTORE R0
       RTWP                   EXIT
*
       TITL 'WRITE'
       PAGE
************************************************************
* WRITE CHARACTER  (WRIT)     ** USES IREGS **             *
*                                                          *
*   CALLING SEQUENCE:         WRIT   Register              *
*                                                          *
*   RETURN:                   RTWP                         *
*                                                          *
* TRANSMIT THE CHARACTER IN THE LEFT BYTE OF THE USER      *
* REGISTER.   IF THE CHARACTER IS A CARRIAGE RETURN, AND   *
* THE BAUD RATE IS 1200 OR LESS, THE ROUTINE DELAYS 200MS  *
* TO ALLOW THE CARRIAGE TO RETURN.                         *
*     IF THE TERMINAL IS A 733 ASR, THEN EACH CHARACTER    *
* IS PADDED WITH 25 MSEC TO REDUCE THE TRANSFER RATE TO    *
* 300 BAUD.                                                *
*    NOTE THAT EACH TIME A CARRIAGE RETURN IS OUTPUT, A    *
* TEST IS MADE TO SEE IF AN 'ESCAPE' HAS BEEN RECEVIED.    *
* IF IT HAS, THEN CONTROL IS RETURNED TO THE COMMAND       *
* SCANNER.  USER PROGRAMMES REQUIRING AN 'ESCAPE' INPUT    *
* INTO THEM SHOULD THEREFORE AVOID USING THIS XOP IF       *
* INPUT IS REQUIRED WHILST OUTPUTTING.                     *
************************************************************
*
WENTRY MOV  R0,R12            SAVE R0
       MOVB *R11,R0           FETCH BYTE TO BE SENT
       DATA TYP0$             SEND IT
*
       CB   *R11,@B0D         'CR' ?
       JNE  W40               N, EXIT
       BLWP @HALTO$           Y, CHECK FOR HALT OUTPUT
W40    MOV  R12,R0            RESTORE R0
       RTWP                   N, EXIT
       TITL '***ASCII MESSAGE OUTPUT***'
       PAGE
************************************************************
*                                                          *
* MESSAGE OUTPUT  (MSG)       ** USES XREGS **             *
*                                                          *
*    CALLING SEQUENCE:        MSG   @Message Address       *
*                                                          *
*    RETURN                   RTWP                         *
*                                                          *
* THE ASCII STRING POINTED TO BY MESSAGE ADDRESS IS        *
* OUTPUT UNTIL A ZERO BYTE IS ENCOUNTERED.                 *
*                                                          *
************************************************************
*
MLOOP  WRIT R12               O/P THE CHARACTER
MENTRY MOVB *R11+,R12         GET CHARACTER
       JNE  MLOOP             IF NOT ZERO, LOOP BACK
       RTWP                   RETURN
*
       TITL '***HEX INPUT***'
       PAGE
************************************************************
* HEX INPUT ROUTINE  (RHXW)                                *
*                                                          *
*     CALLING SEQUENCE:       RHXW    @Register no.        *
*                             DATA    NULL                 *
*                             DATA    ERROR                *
*     RETURN                  RTWP                         *
*                                                          *
* RETURNS A 16-BIT NUMBER INPUT FROM TERMINAL. DIGITS      *
* ARE ACCEPTED UNTIL A TERMINATION CHARACTER IS FOUND.     *
*                                                          *
* TERMINATION CHARACTERS: SPACE, COMMA, CAR. RETURN, MINUS *
* THE TERMINATION CHARACTER IS RETURNED IN THE LEFT        *
* BYTE OF THE REGISTER FOLLOWING 'R'.                      *
*                                                          *
* RETURN IS TO THE NULL RETURN ADDRESS IF INPUT IS         *
* A TERMINATION CHARACTER ONLY. (Rn IS UNCHANGED)          *
*                                                          *
* IF A FAULTY TERMINATION CHARACTER IS FOUND,              *
* RETURN IS TO THE ERROR ENTRY. (Rn,Rn+1 ARE UNCHANGED)    *
************************************************************
*
RHENTY CLR  R9                RESET NUMBER INPUT FLAG
       CLR  R12               CLEAR ACCUMULATOR
LOOP   EKO  R10               GET A CHARACTER INPUT
*
* CHECK FOR VALID HEX INPUT
*
       CI   R10,'0'*256       MIN NUMERIC
       JL   NOTHEX
       CI   R10,'9'*256       MAX NUMERIC
       JLE  GOTONE
       CI   R10,'A'*256       MIN ALPHA
       JL   NOTHEX
       CI   R10,'F'*256       MAX ALPHA
       JH   NOTHEX
       AI   R10,>900          ALPHA ADJUST
GOTONE SLA  R10,4             ISOLATE DIGIT
       SRL  R10,12            WORD ALIGN DIGIT
*
* DIGIT TO ACCUMULATOR
*
       SLA  R12,4
       A    R10,R12
       INC  R9                SET INPUT FLAG
       JMP  LOOP
*
* CHECK FOR TERMINATION CHARACTER
*
NOTHEX CI   R10,' '*256       ' '?
       JEQ  SPCK
       CI   R10,'-'*256       '-'?
       JEQ  SPCK
       CI   R10,>D00          CARRIAGE RETURN?
       JEQ  SPCK
       CI   R10,','*256       COMMA?
       JNE  ERR               NO, TERMINATION CHAR ERROR
       LI   R10,' '*256       CHANGE TO SPACE
SPCK   MOV  R9,R9             NULL INPUT?
       JEQ  NEXIT             YES, SKIP
       MOV  R12,*R11+         RETURN VALUE
       MOV  R10,*R11          RETURN TERMINATOR
       C    *R14+,*R14+       BUMP PAST NULL POINTER
       RTWP
NEXIT  INCT R11
       MOV  R10,*R11          RETURN TERMINATOR
EXIT1  MOV  *R14,R14          GET POINTER
       RTWP
ERR    INCT R14               POINT TO ERROR POINTER
       JMP  EXIT1
*
       TITL      '***HEX OUTPUT***'
       PAGE
************************************************************
*                                                          *
* HEX OUTPUT ROUTINES  (WNBL AND WHXW)                     *
*                                                          *
*                                                          *
* ROUTINE 1   (WHXW)                                       *
*      CALLING SEQUENCE:      WHXW    Register             *
*                                                          *
*      RETURN                 RTWP                         *
*                                                          *
* OUTPUT THE BINARY CONTENTS OF 'R' AS                     *
* 4 HEXADECIMAL DIGITS.                                    *
*                                                          *
*                                                          *
* ROUTINE 2   (WNBL)                                       *
*      CALLING SEQUENCE:      WNBL     Register            *
*                                                          *
*      RETURN:                RTWP                         *
*                                                          *
* OUTPUT RIGHT MOST HEX DIGIT IN R.                        *
*                                                          *
************************************************************
*
* WHX1 ENTRY POINT
*
WHXETY MOV  *R11,R12          GET VALUE TO PRINT
       SLA  R12,12
       LI   R9,1              SET COUNT FOR 1 DIGIT
       JMP  LOOP1
*
* WHEX ENTRY POINT
*
WHENTY MOV  *R11,R12          GET THE VALUE
       LI   R9,4              SET COUNT FOR 4 DIGITS OUT
LOOP1  MOV  R12,R10
       SRL  R10,12            ISOLATE HEX DIGIT
       SLA  R10,8             BYTE ALIGN
       CI   R10,>900          NUMERIC?
       JLE  NUM               YES, SKIP
       AI   R10,>700          ALPHA ADJUST
NUM    AI   R10,'0'*256       NUMERIC TO ASCII
       WRIT R10               WRITE CHARACTER
       SRC  R12,12            ALIGN NEXT DIGIT
       DEC  R9                DONE?
       JNE  LOOP1             NO, LOOP
       RTWP
       TITL 'ASSEMBLER'
       PAGE
************************************************************
* TITLE: ZERO LABEL ASSEMBLER                              *
*                                                          *
* REVISION: 9/04/79  REPRINT ADDR FOR CRT'S                *
*           21/2/78  MODIFIED FOR HIBUG                    *
*           22/6/81  MODIFIED FOR LOBUG                    *
*           8/7/81   MODIFIED FOR TMS 9995                 *
*           10/12/81 MODIFIED TO USE DIS-ASM TABLE         *
* ABSTRACT: PROVIDES LIMITED ASSEMBLER                     *
*           CAPABILITY.  MOST FEATURES OF                  *
*           THE 990/4 ASSEMBLER ARE INCLUDED               *
*           EXCEPT LABEL DEFINITION/REFERENCE.             *
* CALLING SEQUENCE: ENTER 'A' COMMAND TO                   *
*                   LOBUG                                  *
*                                                          *
* THE ENTRY ADDRESS IS AT ZLABGN                           *
************************************************************
*
* GET ONE CHARACTER FROM IO BUFFER
* CHARACTER RETURNED RIGHT JUSTIFIED IN R4
*
INPT   MOV  R11,@BLSTOR       SAVE RETURN ADDRESS
       MOV  @ASMPTR,R11       FETCH IO POINTER
       MOVB *R11,R4           FETCH BYTE
       JEQ  INPTX             SKIP IF NULL
       INC  @ASMPTR           INCREMENT IO POINTER
INPTX  SRL  R4,8              RIGHT JUSTIFY
       MOV  @BLSTOR,R11       FETCH RETURN ADDRESS
       RT
*
* BRANCH TABLE FOR OPERANDS
*
* 0 - N/A
* 1 - S OR D
* 2 - W OR C
* 3 - IOP
* 4 - N (SHIFT COUNT)
* 5 - DIS
* 6 - BIT
*
OPER   DATA 0,OPA,OPF,OPE
       DATA OPD,OPG,OPH
*
*
* FORMAT CODE CONVERSION TABLE
*
FMCONV BYTE 0,9,5,>A,>A,>14,8,0,>13,>A,6,3,>10
*
       EVEN
*
* HEX, BINARY, OR DECIMAL INPUT
*
HEX    MOV  R11,R1            SAVE RETURN
       LI   R8,16             PRESET BASE
       JMP  DEC5
BIN    LI   R8,2              PRESET BASE
       BL   *R15
       JMP  DEC5
DEC    MOV  R11,R1            SAVE RETURN
DEC1   LI   R8,10             PRESET BASE
DEC5   CLR  R7                PRESET VALUE
DEC10  MOV  R4,R6             PUT CHAR IN R6
       AI   R6,->30           REMOVE ASCII BIAS
       JLT  DEC30             NOT VALID
       CI   R6,10
       JLT  DEC20             O.K.
       AI   R6,-7
       CI   R6,10
       JLT  DEC30             NOT VALID
DEC20  C    R6,R8             IF NOT LT BASE - NOT GOOD
       JLT  DEC40
DEC30  MOV  R1,R11            RESTORE EXIT
       MOV  R7,R1             R1=ANS.
       B    *R11              EXIT
DEC40  MOV  R6,R0
       MOV  R7,R6
       MPY  R8,R6
       A    R0,R7
       BL   *R15
       JMP  DEC10
*
* GET REGISTER NAME
*
GETR   MOV  R11,R1            SAVE RET
       BL   *R15
       MOV  R1,R11            TEMP. RESET OF R11
GETRA  MOV  R11,R13           SAVE RET
       CI   R4,'R'
       JNE  GETR20
       BL   *R15
GETR20 BL   @DEC              GET X
       CI   R1,15             TEST RANGE
       JH   GETR30
       B    *R13              EXIT
GETR30 LI   R4,'*R'           ISSUE RANGE ERROR
       JMP  TYPE1
*
* GET ADDRESS
*
GETL   MOV  R11,R1            SAVE RET
       BL   *R15
       JMP  GETL10
GETLA  MOV  R11,R1            SAVE RETURN
GETL10 CI   R4,'%'            CHECK FOR BINARY
       JEQ  BIN
       CI   R4,>27            CHECK FOR STRING (')
       JEQ  GETL20
       CI   R4,'>'            CHECK FOR HEX
       JNE  DEC1              MUST BE DEFAULT
       BL   *R15              MUST BE HEX
       JMP  HEX+2
GETL20 CLR  R7                PRESET STRING
GETL30 BL   *R15              GET A CHAR
       CI   R4,>27            IF ', DONE
       JEQ  GETL40
       SLA  R7,8
       SOC  R4,R7
       JMP  GETL30
GETL40 BL   *R15              GET TERM.
       JMP  DEC30             EXIT
*
* CONTROL LOOP - REQUEST ADDRESS,
* PRINT TRANSLATED OPCODES
*
ZLABGN EQU  $                 *** ENTRY ***
       LWPI WORKS             SET WORKSPACE
       LI   R1,BRAM           DEFAULT START PC
       MOV  @MREG3,R2         WAS A PC ENTERED?
       JEQ  PT100              NO, USE DEFAULT
       MOV  @MREGS,R1          YES, USE ENTRY
PT100  LI   R15,INPT          SET R15 FOR INPT CALL
       LI   R9,GETL
PT110  MOV  R1,@PC            SAVE PC
PT120  MOV  @PC,R2            R2=PC
       CLR  R3                R3=WORD COUNT
PT130  DATA TYPC$             PRINT LINE FEED
PT135  WHXW R2                PRINT (R2) IN HEX
       LI   R5,'  '           SPACE OVER ONE
       WRIT R5
       MOV  R3,R3             IF WORD COUNT NONZERO
       JEQ  PT150             DISPLAY INST. WORDS
       WHXW *R2+
       MOV  R2,@PC            UPDATE PC
       DECT R3                REDUCE WORD COUNT
       JMP  PT130             CONT. TILL ALL DONE
PT150  MSG  @SPACE5           TAB OVER 6 PLACES
*
* ACCEPT THE OP-CODE MNEMONIC
*
       SETO @MODE             STOP USER USING ^E ( EDIT )
       CLR  @ESCFLG           MAKE SURE USER CAN ESCAPE
       LWPI WPR1              USE BASIC'S WP
       BL   @GTLN             CALL GET LINE
       LWPI WORKS             RESTORE WP
       MOV  @IOB,@ASMPTR      SET UP IO POINTER
       LI   R10,ASMOPC        POINT TO BUFFER
       MOV  R5,*R10           SET 2 SPACES
       MOV  R5,@ASMOPC+2      SET 2 MORE SPACES
GETASC BL   *R15              GET A CHARACTER
       CI   R4,'A'            TEST IF VALID ALPHA
       JLT  NOTOPC            NOT OPCODE
       CI   R4,'Z'            TEST UPPER ALPHA
       JGT  NOTOPC            NOT OPCODE
GOTASC SWPB R4
       MOVB R4,*R10+          STORE THE CHARACTER
       CI   R10,ASMOPC+4      CHECK NOT TOO MANY CHARS.
       JH   PAT90             TOO MANY - JUMP
       JMP  GETASC            GET ANOTHER CHAR
*
NOTOPC CI   R4,' '            IF SPACE, FINISHED OPCODE
       JEQ  GOTOPC
       MOV  R4,R4             IF NULL, FINISHED OPCODE
       JEQ  GOTOPC   
       CI   R10,ASMOPC        TEST IF FIRST CHARACTER
       JNE  PAT90             IF NOT, ERROR IT
       CI   R4,'$'            CHECK FOR $(STRING)
       JEQ  PT220
       CI   R4,'/'            CHECK FOR ADDR RESET
       JNE  PAT90
       BL   *R15              GET ANOTHER CHARACTER
       BL   @HEX              GET NEW ADDRESS
       JMP  PT110
PAT90  LI   R4,'*S'           ERROR - SNATCH CONTROL
TYPE1  LI   R5,>0700          SET UP BELL AND ZERO BYTE
       MSG  R4                OUTPUT R4
       JMP  PT120             DON'T CHANGE PC
*
* HANDLE STRING ENTRIES.  COLLECT CHARACTERS
* UNTIL A CR.  THEN FORCE ADDRESS EVEN AND
* EXIT
*
PT220  BL   *R15              GET A CHAR.
       MOV  R4,R4             IF NULL - EXIT
       JEQ  PT230
       SLA  R4,8              SAVE THE CHAR.
       MOVB R4,*R2+
       INC  R3
       JMP  PT220
PT230  MOV  R3,R0             IF ODD-INST. SPACE
       SRA  R0,1
       JNC  PT240
       MOVB R5,*R2            PAD WITH SPACE
       INC  R3
PT240  MOV  @PC,R2            RESET PC
       JMP  PT300             GO PRINT RESULTS
*
* GET SPECIAL OPCODES FROM TABLE
*
SPECOC LI   R5,SOPSTT-6       GET TABLE START
       INCT R10
SPEC1  AI   R5,6
       DECT R10
       MOV  *R5+,R0
       JEQ  PAT90             NOT FOUND, ERROR
       C    R0,*R10+
       JNE  SPEC1
       C    *R5,*R10
       JNE  SPEC1
       MOV  @4(R5),R1
       JMP  PT270
*
* THE INPUT OPCODE IS NOW IN ASMOPC. SEARCH THE TABLE
*  UNTIL IT IS LOCATED.
*
GOTOPC LI   R5,CODSTT-4       GET TABLE ADDRESS
       LI   R10,ASMOPC+2      INPUT OPCODE PTR
OCSH1  AI   R5,4              POINT TO NEXT OPCODE
       DECT R10               BACK TO OPCODE START
       MOV  *R5+,R0           GET DATA
       JEQ  SPECOC            ZERO, END OF TABLE
       C    R0,*R10+          CHECK FOR EQUALITY
       JNE  OCSH1             NO, -LOOP BACK
       C    *R5,*R10          CHECK LAST TWO LETTERS
       JNE  OCSH1
*
* THE OPCODE HAS BEEN LOCATED . NOW
* COLLECT THE OPERANDS.
*
PT280  MOV  @2(R5),R10        R10=INST&PARSING INST.
       MOV  R10,R0            PRESET THE INST.
       ANDI R0,>FFF0
       MOV  R0,*R2
       INCT R3                COUNT=2
       MOV  R10,R1            SAVE
       ANDI R1,>F             REMOVE OPCODE, LEAVE FORMAT
       JEQ  GETDTA
       MOVB @FMCONV(R1),R1    GET TRANSLATION
       SRL  R1,8              TO LSB
       MOV  R1,R12            STORE FOR FUTURE USE
       SRL  R1,2
       ANDI R1,>6
       MOV  @OPER(R1),R1      R1=OPERAND INDEX
       JEQ  PT290             SKIP IF NO FIRST ONE
       BL   *R1               COLLECT FIRST ONE
PT290  SLA  R12,13            COLLECT SECOND OPERAND.
       SRL  R12,12
       MOV  @OPER(R12),R6
       JEQ  PT300             JUMP IF NONE
       CLR  R10               SET FLAG
       BL   *R6
*
* THE ENTIRE STATEMENT HAS BEEN
* ACCEPTED - PRINT TRANSLATION
* AND UPDATE P.C.
*
PT300  MOV  R4,R4             EOL ?
       JEQ  PT310             YES,JUMP
       BL   *R15              NO,FETCH NEXT CHAR
       JMP  PT300             LOOP
PT310  WRIT @MCRLF            PRINT CR
       B    @PT135            GO DISPLAY ADDR AND OBJECT
*
* RANGE ERROR
*
RNGERR LI   R4,'*D'
       JMP  TYPE1
*
* HANDLE S OR D
*     N
*     *N
*     *N+
*     @X(N)
*     @X
*
OPA    MOV  R11,R14           SAVE RETURN ADDRESS
       BL   *R15              GET CHAR
       CI   R4,'*'            CHECK FOR *N OR *N+
       JEQ  OPB               JUMP IF YES
       CI   R4,'@'            CHECK FOR @X OR @X(N)
       JNE  OPC               JUMP IF NOT
       BL   *R9
       MOV  R3,R6             ADD TO MEMORY
       A    R2,R6
       MOV  R1,*R6            SAVE X
       INCT R3                UPDATE COUNT
       LI   R1,>20            ADDRESS MODE 2
       MOV  R4,R4             IF NULL OR ',' DONE
       JEQ  OPA10
       CI   R4,','
       JEQ  OPA10
       CI   R4,' '            IF SPACE - DONE
       JEQ  OPA10
       CI   R4,'('            IF NOT ( - ERROR
       JNE  PAT90
       BL   @GETR             GET REG. N
       ORI  R1,>20            SET MODE 2
       CI   R4,')'            IF NOT ) - ERROR
       JNE  PAT90
       BL   *R15
OPA10  MOV  R10,R0            REPOS. IT
       JNE  OPA15
       SLA  R1,6
OPA15  SOC  R1,*R2            INSERT IT
OPA20  B    *R14              EXIT
OPB    BL   @GETR             GET N(FOR *N)
       LI   R0,>10            SET MODE = 1
       CI   R4,'+'            IF TERM. BY +
       JNE  OPB10             CHANGE MODE
       BL   *R15
       LI   R0,>30            SET MODE = 3
OPB10  SOC  R0,R1             R1=REG&MODE
       JMP  OPA10
OPC    BL   @GETRA            GET N(FOR N)
       JMP  OPA10             MODE=0 - GO INSERT
*
* HANDLE DATA ENTRIES.
*
GETDTA BL   *R15              GET NEXT CHARACTER
       CLR  R10               DEFAULT TO +
       CI   R4,'+'
       JEQ  DTASUB
       CI   R4,'-'
       JNE  NOSIGN
       SETO R10               SET FLAG
DTASUB BL   *R9
       JMP  TSTSGN
NOSIGN BL   @GETLA
TSTSGN MOV  R10,R10
       JEQ  PT270
       NEG  R1
PT270  MOV  R1,*R2            SAVE IT
       LI   R3,2              SET R3
       JMP  PT300             GO PRINT
*
* HANDLE SHIFT COUNT
*
OPD    MOV  R11,R14           SAVE RETURN
       BL   @GETR             GET COUNT
       SLA  R1,4              REPOSITION
       JMP  OPA15             INSERT
*
* HANDLE IMMEDIATE OPERANDS
*
OPE    MOV  R11,R14           SAVE RETURN
       BL   *R9               GET IOP
       MOV  R3,R6             ADD TO MEMORY
       A    R2,R6
       MOV  R1,*R6
       INCT R3                ADJUST COUNT
       B    *R14              CONTINUE
*
* HANDLE W
*
OPF    MOV  R11,R14
       BL   @GETR
       JMP  OPA10
*
* HANDLE DISPLACEMENTS
* + DIS
* - DIS
* ADDRESS  (CALCULATE DISPLACEMENT)
*
OPG    MOV  R11,R14           SAVE RETURN
       BL   *R15              GET LEADER ($)
       CI   R4,'$'
       JNE  OPG5
       BL   *R15              GET FIRST CHAR
       CI   R4,'+'            CHECK FOR +DIS
       JEQ  OPG30
       CI   R4,'-'            CHECK FOR -DIS
       JEQ  OPG40
OPG5   BL   @GETLA
       MOV  R2,R0             MUST BE ADDRESS
       INCT R0                DIS*2=ADDRESS-(PC+2)
       S    R0,R1
OPG10  SRA  R1,1              DISP=BYTE STUFF/2
       CI   R1,>7F            CHECK RANGE
       JGT  RNGERR
       CI   R1,>FF80
       JLT  RNGERR
OPG15  ANDI R1,>FF            RANGE O.K. SO
       SOC  R1,*R2            INSERT IT
       LI   R1,2              RESET R3
       B    *R14              EXIT
OPG30  BL   *R9               +DIS
OPG35  DECT R1                ADJUST DIS FOR CUR. INST
       JMP  OPG10
OPG40  BL   *R9
       NEG  R1                -DIS
       JMP  OPG35
*
* HANDLE BIT
*
OPH    MOV  R11,R14           SAVE RETURN
       BL   *R9
       JMP  OPG15             GO PROCESS IT
       TITL 'UNASSEMBLER FOR LOBUG. MJS & CRH'
       PAGE
************************************************************
**                                                        **
**          TMS 9900  DISASSEMBLER                        **
**                                                        **
**          PROGRAMMED BY:  M. J. STEWART                 **
**          MODIFIED BY:    C.R. HINSON  19TH JUNE 1981   **
**          COPYRIGHT 1978                                **
**                                                        **
************************************************************
*
* INTERNAL REGISTER USAGE:
*      R0   DISASSEMBLY ADDRESS
*      R1   OP CODE DATA / MNEMONIC ADDRESS
*      R2   PRGM CALCULATIONS
*      R3   PRGM CALCULATIONS
*      R4   PRGM CALCULATIONS
*      R5   DUMP MODE END ADDRESS / FLAG
*      R6   ASCII DATA
*      R7   PGM CALCULATIONS
*      R8   ADDR. OPERAND ADDR. MODES SUBR.
*      R9   ADDR. PRINT REG. NO. SUBR.
*      R10  PGM CALCULATIONS
*      R11  LINK ADDRESS
*      R12  SAVED LINK / UART CRU BASE ADDR.
*      R13  CALLERS WP
*      R14  CALLERS PC
*      R15  CALLERS ST
*
* MONITOR LINKAGE
*
U      EQU  $
       BLWP @UNVEC            CALL UNASSEMBLER
       RT                     RETURN TO MONITOR
*
UNVEC  DATA WORKS             VECTORS FOR UNASSEMBLER
       DATA UENTRY
*
INSTR  DATA WORKS,TRCENT
*
TRCENT MOV  @28(R13),R0       GET START ADDRESS
       MOV  R0,R5             COPY FOR STOP ADDRESS
       CLR  R12
       JMP  U2                DO DISASSEMBLY
       PAGE
*
* TRANSFER START AND STOP ADDRESSES FROM CALLER
*
UENTRY EQU  $                 *** ENTRY ***
       MOV  @2(R13),R5        LOAD STOP ADDR
       MOV  *R13,R0           LOAD START ADDR
       JNE  BEGIN             IF NON-ZERO, GO ON
       MOV  @6(R13),R6        ANY PARMS ?
       JNE  BEGIN             YES , SKIP DEFAULT
       LI   R0,BRAM           OTHERWISE LOAD DEFAULT
*
* BEGIN DISASSEMBLY
*
BEGIN  MSG  @MCRLF            SEND CR AND LF
       ANDI R0,>FFFE          WORD ALIGN ADDRESS
       WHXW R0                PRINT MEM ADDRESS
U2     EQU  $                 *** ALTERNATE ENTRY FOR TRACE
       LI   R6,' ,'           ASCII SPACE AND COMMA
       WRIT R6                PRINT SPACE
       MOV  *R0,R1            GET INSTRUCTION OP CODE
       WHXW R1                PRINT OP CODE DATA
       WRIT R6                PRINT SPACE
       CI   R12,>0400
       JEQ  DATAOP
LDREG  LI   R8,AMODE          ADDR FOR ADDRESSING MODES SUBR
       LI   R9,REGNO          ADDR FOR PRINT REG NUMBER SUBR
       LI   R10,SOPSTT-2
SOPLP  AI   R10,8
       CI   R10,SOPEND
       JH   NOTSOP
       C    R1,*R10
       JNE  SOPLP
       MOV  R10,R1
       AI   R1,-6
       BL   @PRTOP
       JMP  CMDSCN
NOTSOP LI   R10,CODEND        GET TABLE POINTER
GETOPC MOV  *R10,R2           GET OPCODE VALUE
       JEQ  DATAOP
       ANDI R2,>FFF0          REMOVE FORMAT
       JEQ  NODIR
       C    R1,R2             CORRECT OPCODE?
       JHE  OPCFND            YES - EXIT
NODIR  AI   R10,-6            NEXT OPCODE
       JMP  GETOPC            LOOP BACK
OPCFND MOV  *R10,R7           STORE VALUE
       MOV  R10,R1            STORE VALUE
       AI   R1,-4             GET TEXT PNTR
       BL   @PRTOP
*
       ANDI R7,>F             GET THE FORMAT
       MOVB @BTABL(R7),R7     GET THE ADDRESS
       SRA  R7,7
       B    @BTABL(R7)        GO EXECUTE AS REQUIRED
*
*  FORMAT JUMP TABLE
*
BTABL  BYTE PRTDAT-BTABL/2
       BYTE FRMAT1-BTABL/2
       BYTE FRMAT2-BTABL/2
       BYTE FRMAT3-BTABL/2
       BYTE FRMAT4-BTABL/2
       BYTE FRMAT5-BTABL/2
       BYTE FRMAT6-BTABL/2
       BYTE CMDSCN-BTABL/2
       BYTE FRMAT8-BTABL/2
       BYTE FRMAT3-BTABL/2
       BYTE BITFMT-BTABL/2
       BYTE FRMATB-BTABL/2
       BYTE FRMATC-BTABL/2
       EVEN
*
FRMAT1 BL   *R8               PRINT SOURCE OPERAND
       SRL  R1,6
       JMP  TST42
FRMAT4 BL   *R8               PRINT OPERAND
       SRL  R1,6
       JMP  TST81
FRMAT3 BL   *R8               PRINT SOURCE OPERAND
       SLA  R1,6
       SRL  R1,12
TST42  WRIT R6                PRINT COMMA
FRMAT6 BL   *R8               PRINT DESTN OPERAND
       JMP  CMDSCN
FRMAT2 SWPB R1
       SRA  R1,7              SIGNED DISPLACEMENT
       A    R0,R1             NEW PC ADDRESS
       JMP  PRTDAT
DATAOP LI   R1,DTATXT
       BL   @PRTOP            PRINT MNEMONIC
PRTDAT WRIT @SYMBLC+1         PRINT ">"
       WHXW R1                PRINT DATA WORD
       JMP  CMDSCN
FRMAT5 MSG  @REGSTR
       BL   *R9               PRINT REG NUMBER
       SRL  R1,4
TST81  WRIT R6                PRINT COMMA
TST82  BL   *R9               PRINT NUMBER
JCMD   JMP  CMDSCN
FRMAT8 ANDI R1,>F
       BL   *R8               PRINT DESTINATION REGISTER
       WRIT R6                PRINT COMMA
FRMATB MOV  *R0+,R1           GET IMMEDIATE OPERAND
       JMP  PRTDAT
FRMATC ANDI R1,>F
       JMP  FRMAT6
*
* SUBROUTINE WHICH PRINTS THE MNEMONIC BASED ON
*      THE TABLE OFFSET COMPUTED IN REG 1
*
PRTOP  LI   R3,4              LOAD CHAR COUNT
PRT1   MOVB *R1+,R2           GET CHARACTER FROM TABLE
       WRIT R2                PRINT CHARACTER
       DEC  R3                DECREMENT COUNT
       JNE  PRT1              REPEAT FOR FOUR CHAR
       WRIT R6                PRINT SPACE
       SWPB R6                ASCII ',' IN UPPER BYTE
       MOV  *R0+,R1           RELOAD OPCODE DATA, INC ADDR
       RT
       PAGE
*
* ROUTINE FOR PRINTING THE CRU BIT DISPLACEMENT AS
*      A SIGNED DECIMAL NUMBER (127 TO -128)
*
BITFMT LI   R3,'-1'           ASCII DATA FOR MINUS AND ONE
       SWPB R1                ISOLATE BIT DISPLACEMENT
       SRA  R1,8               AND SIGN EXTEND
       JEQ  BIT5              JUMP IF ZERO
       JGT  BIT1              JUMP IF POSITIVE
       WRIT R3                OTHERWISE PRINT MINUS SIGN
       NEG  R1                AND MAKE POSITIVE
BIT1   CI   R1,100            ONE HUNDRED OR GREATER?
       JLT  BIT2              NO, JUMP
       SLA  R3,8              MOVE OVER ASCII ONE
       WRIT R3                PRINT A ONE
       AI   R1,-100           SUBTRACT 100 FROM DISPLACEMENT
BIT2   LI   R4,10             LOAD DIVISOR
       MOV  R1,R2             RIGHT JUSTIFY DISPLACEMENT
       CLR  R1                 IN REG PAIR 1 AND 2
       DIV  R4,R1             DIVIDE BY TEN
       SLA  R3,8              WAS ONE HUNDRED PRINTED?
       JEQ  BIT3              YES, PRINT BOTH TENS AND ONES
       MOV  R1,R1             NO, THEN TEST TENS VALUE
       JEQ  BIT4              IF ZERO PRINT ONLY ONES
BIT3   BL   *R9               PRINT TENS DIGIT
BIT4   MOV  R2,R1             LOAD ONES DIGIT
BIT5   BL   *R9               PRINT ONES DIGIT
       PAGE
*
* COMMAND SCANNER: TWO MODES
*   1) DUMP MODE:  DISASSEMBLES FROM START TO END
*        ADDRESS.
*   2) SINGLE STEP MODE:  AT THE END OF EACH LINE,
*     A) ESCAPE CAUSES A RETURN TO LOBUG CMD SCANNER
*     B) ANY OTHER CHARACTER CONTINUES DIS-ASSEMBLY AT THE
*        NEXT INSTRUCTION
*
CMDSCN MOV  R5,R5             TEST FOR DUMP MODE
       JNE  CMD5               Y,GO TO DUMP HANDLER
*
       MSG  @SPACE2           OUTPUT TWO SPACES
CMD2   READ R12
       CI   R12,>1B00         ESCAPE?
       JEQ  CMD6               Y, EXIT DISASSEMBLER
CMD4   B    @BEGIN            CONTINUE DISASSEMBLY
*
CMD5   C    R0,R5             TEST DUMP END ADDRESS
       JLE  CMD4               N, CONTINUE DISASSEMBLY
CMD6   RTWP                    Y, EXIT DISASSEMBLER
       PAGE
*
* SUBROUTINE WHICH PRINTS THE DECIMAL VALUE OF
*      THE LOWER FOUR BITS OF REGISTER 1
*
REGNO  MOV  R1,R3             MOVE DATA FROM R1 TO R3 AND
       SLA  R3,12              ISOLATE LOWER 4 BITS RIGHT
       SRL  R3,4               JUSTIFIED IN UPPER BYTE
       CI   R3,>900           COMPARE TO NINE
       JLE  REG1              EQUAL OR LESS, SINGLE DIGIT
       SWPB R3                OTHERWISE TWO DIGITS
       AI   R3,>0126          BCD CORRECT ONES, MAKE ASCII
REG1   AI   R3,>3000          MAKE UPPER BYTE ASCII
REG2   WRIT R3                PRINT ASCII NUMBER
       SLA  R3,8              CHECK FOR SECOND DIGIT
       JNE  REG2              IF THERE, PRINT IT
REGRTN RT                     RETURN
*
* SUBROUTINE TO HANDLE THE MULTIPLE ADDRESSING MODES:
*      WORKSPACE REGISTER
*      WORKSPACE REGISTER INDIRECT
*      SYMBOLIC AND INDEXED
*      WORKSPACE REG. INDIRECT AUTO INCREMENT
*
AMODE  LI   R4,'*R'           ACSII DATA FOR REG FORMATS
       MOV  R1,R2             GET OP CODE AND
       SLA  R2,10              ISOLATE ADDR MODE (T) FIELD
       SRL  R2,14              RIGHT JUSTIFIED IN R2
*
*      WORKSPACE REGISTER
*
       JNE  AMD2              JUMP IF T FIELD NOT ZERO
AMD1   SWPB R4                PUT ASCII "R" IN UPPER BYTE
       WRIT R4                PRINT "R"
       JMP  REGNO             PRINT REG. NO. AND RETURN
*
*      WORKSPACE REGISTER INDIRECT
*
AMD2   DEC  R2                WAS T FIELD = 1?
       JNE  AMD3              NO, JUMP
       WRIT R4                PRINT "*"
       JMP  AMD1              GO HANDLE REG FORMAT
*
*      SYMBOLIC AND INDEXED
*
AMD3   DEC  R2                WAS T FIELD = 2?
       JNE  AMD6              NO, JUMP
       MSG  @SYMBLC           ASCII DATA FOR SYMBOLIC
       WHXW *R0+              PRINT SYMBOLIC WORD
       MOV  R1,R2             GET OP CODE AGAIN AND
       SLA  R2,12              CHECK IF REG. NO. IS ZERO
       JEQ  REGRTN            YES, RETURN
       LI   R2,'()'           NO, INDEXED MODE
       WRIT R2                PRINT "("
AMD4   SWPB R4                PUT ASCII R IN UPPER BYTE
       WRIT R4                PRINT "R"
       MOV  R11,R12           SAVE LINK ADDR
       BL   *R9               PRINT REG NO.
       SWPB R2                NEXT ASCII BYTE
       WRIT R2                PRINT ")" OR PRINT "+"
AMD5   B    *R12              RETURN
*
*      WORKSPACE REG. INDIRECT AUTO INCREMENT
*
AMD6   WRIT R4                PRINT "*"
       LI   R2,'+'            LOAD ASCII "+"
       JMP  AMD4              GO HANDLE REST OF FORMAT
       TITL 'OPCODE TABLES FOR ASSEMBLER/DIS-ASSEMBLER'
       PAGE
*
****************************************
*       BASIC OP-CODE TABLE            *
*                                      *
* EACH ENTRY HAS THE OP CODE, OPERAND  *
* ONE AND OPERAND TWO DESCRIPTION.     *
*                                      *
*       BRANCH TABLE FOR OPERANDS      *
*                                      *
*        0 - N/A                       *
*        1 - S OR D                    *
*        2 - IOP                       *
*        3 - W                         *
*        4 - C                         *
*        5 - DIS                       *
*        6 - BIT                       *
*        7 - N (SHIFT CNT)             *
****************************************
FM1    EQU  >1           FORMAT 1 - S,D
FM2    EQU  >2           FORMAT 2 - DIS
FM3    EQU  >3           FORMAT 3 - S,W
FM4    EQU  >4           FORMAT 4 - S,C
FM5    EQU  >5           FORMAT 5 - W,N
FM6    EQU  >6           FORMAT 6 - S
FM7    EQU  >7           FORMAT 7 - N/A
FM8    EQU  >8           FORMAT 8 - W,IOP
FM9    EQU  >3           FORMAT 9 - S,W
FMA    EQU  >A           FORMAT A - BIT
FMB    EQU  >B           FORMAT B - IOP
FMC    EQU  >C           FORMAT C - W
FMD    EQU  >D           FORMAT D - LMF
*
*      COMBINED TEXT AND OPCODE TABLE
*
*      TEXT 'AB  '       ;TEXT FOR MNEMONICS
*      DATA >B000+FM1    ;OPCODE AND FORMAT INFORMATION
*
CMBTAB DATA 0            *** TABLE TERMINATOR ***
*
CODSTT TEXT 'LST '
       DATA >0080+FMC    LST
       TEXT 'LWP '
       DATA >0090+FMC    LWP
DTATXT TEXT 'DATA'
       DATA >00A0
       TEXT 'DIVS'
       DATA >0180+FM6    DIVS
       TEXT 'MPYS'
       DATA >01C0+FM6    MPYS
       TEXT 'LI  '
       DATA >0200+FM8    LI
       TEXT 'AI  '
       DATA >0220+FM8    AI
       TEXT 'ANDI'
       DATA >0240+FM8    ANDI
       TEXT 'ORI '
       DATA >0260+FM8    ORI
       TEXT 'CI  '
       DATA >0280+FM8    CI
       TEXT 'STWP'
       DATA >02A0+FMC    STWP
       TEXT 'STST'
       DATA >02C0+FMC    STST
       TEXT 'LWPI'
       DATA >02E0+FMB    LWPI
       TEXT 'LIMI'
       DATA >0300+FMB    LIMI
       TEXT 'DATA'
       DATA >0320
       TEXT 'IDLE'
       DATA >0340+FM7    IDLE
       TEXT 'RSET'
       DATA >0360+FM7    RSET
       TEXT 'RTWP'
       DATA >0380+FM7    RTWP
       TEXT 'CKON'
       DATA >03A0+FM7    CKON
       TEXT 'CKOF'
       DATA >03C0+FM7    CKOF
       TEXT 'LREX'
       DATA >03E0+FM7    LREX
       TEXT 'BLWP'
       DATA >0400+FM6    BLWP
       TEXT 'B   '
       DATA >0440+FM6    B
       TEXT 'X   '
       DATA >0480+FM6    X
       TEXT 'CLR '
       DATA >04C0+FM6    CLR
       TEXT 'NEG '
       DATA >0500+FM6    NEG
       TEXT 'INV '
       DATA >0540+FM6    INV
       TEXT 'INC '
       DATA >0580+FM6    INC
       TEXT 'INCT'
       DATA >05C0+FM6    INCT
       TEXT 'DEC '
       DATA >0600+FM6    DEC
       TEXT 'DECT'
       DATA >0640+FM6    DECT
       TEXT 'BL  '
       DATA >0680+FM6    BL
       TEXT 'SWPB'
       DATA >06C0+FM6    SWPB
       TEXT 'SETO'
       DATA >0700+FM6    SETO
       TEXT 'DATA'
       DATA >0780
       TEXT 'ABS '
       DATA >0740+FM6    ABS
       TEXT 'SRA '
       DATA >0800+FM5    SRA
       TEXT 'SRL '
       DATA >0900+FM5    SRL
OP8    TEXT 'SLA '
       DATA >0A00+FM5    SLA
       TEXT 'SRC '
       DATA >0B00+FM5    SRC
       TEXT 'DATA'
       DATA >0C00        MID OPCODES >0C00 - >0DFF
       TEXT 'WNBL'
       DATA >0E00+FM6    WRITE HEX NIBBLE
       TEXT 'RHXW'
       DATA >0E40+FM6    READ HEX WORD
       TEXT 'WHXW'
       DATA >0E80+FM6    WRITE HEX WORD
       TEXT 'EKO '
       DATA >0EC0+FM6    ECHO CHARACTER
       TEXT 'WRIT'
       DATA >0F00+FM6    WRITE BYTE XOP
       TEXT 'READ'
       DATA >0F40+FM6    READ XOP
       TEXT 'MSG '
       DATA >0F80+FM6    MESSAGE
       TEXT 'BKPT'
       DATA >0FC0+FM6    BKPT
*
       TEXT 'JMP '
       DATA >1000+FM2    JMP
       TEXT 'JLT '
       DATA >1100+FM2    JLT
       TEXT 'JLE '
       DATA >1200+FM2    JLE
       TEXT 'JEQ '
       DATA >1300+FM2    JEQ
       TEXT 'JHE '
       DATA >1400+FM2    JHE
       TEXT 'JGT '
       DATA >1500+FM2    JGT
       TEXT 'JNE '
       DATA >1600+FM2    JNE
       TEXT 'JNC '
       DATA >1700+FM2    JNC
       TEXT 'JOC '
       DATA >1800+FM2    JOC
       TEXT 'JNO '
       DATA >1900+FM2    JNO
       TEXT 'JL  '
       DATA >1A00+FM2    JL
       TEXT 'JH  '
       DATA >1B00+FM2    JH
       TEXT 'JOP '
       DATA >1C00+FM2    JOP
       TEXT 'SBO '
       DATA >1D00+FMA    SBO
       TEXT 'SBZ '
       DATA >1E00+FMA    SBZ
       TEXT 'TB  '
       DATA >1F00+FMA    TB
       TEXT 'COC '
       DATA >2000+FM3    COC
       TEXT 'CZC '
       DATA >2400+FM3    CZC
       TEXT 'XOR '
       DATA >2800+FM3    XOR
       TEXT 'XOP '
       DATA >2C00+FM4    XOP
       TEXT 'LDCR'
       DATA >3000+FM4    LDCR
       TEXT 'STCR'
       DATA >3400+FM4    STCR
       TEXT 'MPY '
       DATA >3800+FM9    MPY
       TEXT 'DIV '
       DATA >3C00+FM9    DIV
       TEXT 'SZC '
       DATA >4000+FM1    SZC
       TEXT 'SZCB'
       DATA >5000+FM1    SZCB
       TEXT 'S   '
       DATA >6000+FM1    S
       TEXT 'SB  '
       DATA >7000+FM1    SB
       TEXT 'C   '
       DATA >8000+FM1    C
       TEXT 'CB  '
       DATA >9000+FM1    CB
       TEXT 'A   '
       DATA >A000+FM1    A
       TEXT 'AB  '
       DATA >B000+FM1    AB
       TEXT 'MOV '
       DATA >C000+FM1    MOV
       TEXT 'MOVB'
       DATA >D000+FM1    MOVB
       TEXT 'SOC '
       DATA >E000+FM1    SOC
       TEXT 'SOCB'
CODEND DATA >F000+FM1    SOCB
       DATA 0
*
* SPECIAL OPCODE TABLE
*
*      TEXT 'xxxx'        MNEMONIC
*      DATA OP$x          SPECIAL FORMAT TYPE
*      DATA >xxxx         UNIQUE OP-CODE
*
SOPSTT TEXT 'SPIN'
       DATA 0,>10FF
CONOP  TEXT 'NOP '
       DATA 0,>1000
CORT   TEXT 'RT  '
       DATA 0,>045B
SOPEND DATA 0
*
*
*
C3     DATA >0003
       END
       