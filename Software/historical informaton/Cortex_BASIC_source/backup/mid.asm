       TITL 'MID HANDLER - CORTEX BASIC REV. 1.1'
       IDT  'MID'
************************************************************
* THIS MODULE HANDLES THE LEVEL 2 INTERRUPT
* DECIDING WETHER IT IS AN ARITHMETIC OVERFLOW
* OR A MID. FOR A MID THE OPCODE IS CHECKED
* TO SEE IF IT IS A SYSTEM MID, IF SO THE
* REQUIRED HANDLER IS EXECUTED.
*
*  MID0      ADDRESS FOR A MID OPCODE >0000 HANDLER
*  OVINT     ADDRESS FOR AN ARITHMETIC OVERFLOW HANDLER
*  ILLMID    ADDRESS FOR AN ILLEGAL MID HANDLER
*
* IF NO HANDLER IS AVAILIABLE FOR AN ARITHMETIC OVERFLOW
* OR NON SYSTEM MID AN 'INTERRUPT WITHOUT TRAP' ERROR OCCURS
*
* HANDLERS ARE ENTERED WITH ALL INTERRUPTS DISABLED AND EXIT
* IS VIA AN 'RTWP'
*
*  REGISTER USAGE ( ~ = RESERVED REGISTER )
*    R0-3   NOT USED
*    R4     GENERAL FLAG, CLEARED ON ENTRY
*    R5-11  NOT USED
*    R12    TEMP. REGISTER
*  ~ R13-15 RETURN VECTORS
*
       DEF  MIDPC,TYP0$,TYPC$,TYPB$,TYPE$,LOOK$
       DEF  TYP11$,TYPBE$,GETC$,GETCR$,TYPS$
       DEF  GCLEAR,TYPSN$,TYPEN$,DELAY$,WAIT$
       DEF  SETTXT,SETGRA,LOADER,SENDAD,STXT,SGRA
       DEF  CMON$,CMOF$,FLUSH$,FTM$,FGM$
*
       REF  FBCOL,LDCS,XLOC,VMODE,CCSAVE
       REF  OVINT,ERROR2,MID0,ILLMID
       REF  S$SM,B00,B20,OUTDSR,C1
       REF  MCRLF,TY0,UNIT,OUTBUF,BUFEND
       REF  DSRCNT,LOADPT
       REF  RECPTR,INPTR,RBUF
       REF  CNTDWN,B1B,IOB,IOSTOR,DEVTBL
       REF  B62,XREGS,WHXETY,RHENTY,WHENTY
       REF  EREGS,ECHOEN,IREGS,WENTRY,RENTRY
       REF  MREGS,MENTRY,XOPENT
       COPY 'IODEFS.INC'
MBIAS  EQU  >0000             NORMAL MID
*
MIDPC  LIMI 3                 NO I/O INTERRUPTS
       CLR  R4                RESET FLAG
       ORI  R15,>000F         MAKE SURE IMASK=15 ON EXIT
       LI   R12,>1FDA         POINT TO MID FLAG
       TB   0                 MID?
       JEQ  TSTMID            Y, CHECK FOR SYSTEM MID
       ANDI R15,>F7FF         N, KILL OVERFLOW BIT
       MOV  @OVINT,R12        GET A.O. HANDLER
       JMP  VERIFY            VALIDATE
*
TSTMID SBZ  0                 KILL MID FLAG
       MOV  @-2(14),R12       GET INSTRUCTION
       JNE  SYSMID            NOT 0, CHECK IF SYSTEM MID
       MOV  @MID0,R12         0, GET HANDLER ADDRESS
*
VERIFY JNE  SERV              HANDLER, EXECUTE
       DATA ERROR2,44         NO HANDLER -ILLEGAL OPCODE
*
BADMID MOV  @ILLMID,R12       GET ILLEGAL MID HANDLER
       JMP  VERIFY            CHECK IT
DECODE A    R12,R12           GET WORD INDEX INTO TABLE
       MOV  @MIDTAB(12),R12   GET HANDLER
SERV   B    *R12              EXECUTE HANDLER
*
SYSMID CI   R12,MAXMID        SYSTEM MID ?
       JLE  DECODE            Y, SERVICE IT
       LIMI 15                N, RE-ENABLE INTERRUPTS
       AI   R12,->0E00        SUBTRACT MONITOR MID BASE
       JLT  BADMID            ILLEGAL, TRY ERROR TRAP
       CI   R12,>01FF         TOO BIG ?
       JH   BADMID            Y, TRY ERROR TRAP
*
*   EVALUATE OPERAND
*
       MOV  R12,R11           COPY OPCODE
       ANDI R11,>000F         ISOLATE REGISTER FIELD
       S    R11,R12           REMOVE REG. FIELD FROM OPCODE
       SRL  R12,3             ALIGN Ts IN BITS 13&14
       MOV  R12,R8            SAVE IT
       ANDI R8,>0006          ISOLATE (2 * Ts)
       S    R8,R12            SUBTRACT IT FROM OPCODE
       SRL  R12,1             R12 =(4 * FUNCTION #)
       A    R11,R11           R11 = 2 * REGISTER #
       A    R13,R11           R11 = ADDRESS OF REGISTER
       MOV  @FMTTYP(8),R8     GET ADDRESS OF Ts HANDLER
       B    *R8               GOTO ROUTINE
*
FMTTYP DATA Z$REG,Z$IND,Z$SYMB,Z$AINC
*
Z$IND  MOV  *R11,R11          FETCH REGISTER CONTENTS
       JMP  Z$REG             EXIT
*
Z$SYMB MOV  *R11,R8           GET REGISTER CONTENTS
       C    R11,R13           USING R0 ?
       JNE  NOTR0             N, OK TO INDEX WITH IT
       CLR  R8                Y, DON'T INDEX
NOTR0  A    *R14+,R8          ADD IN IMEDIATE OPERAND
       JMP  Z$REG8            GO PUT IT IN R11
*
Z$AINC MOV  *R11,R8           GET REGISTER CONTENTS
       INCT *R11              INC REGISTER
Z$REG8 MOV  R8,R11            PUT RESULT IN R11
*
Z$REG  EQU  $
*
*  EFFECTIVE ADDRESS OF OPERAND IS IN R11
*
*
       MOV  @MONTBL(12),R8    PICK UP TARGET WP
       MOV  R11,@22(8)        PASS OVER EFFECTIVE ADDRESS
       MOV  R13,@26(8)        SAVE CALLERS WP
       MOV  R14,@28(8)        SAVE CALLERS PC
       MOV  R15,@30(8)        SAVE CALLERS ST
*
       MOV  R8,R13            SET UP WP FOR TRANSFER
       MOV  @MONTBL+2(12),R14 SET UP PC FOR TRANSFER
       RTWP                   EXECUTE UTILITY
*
*     VECTORS FOR MONITOR UTILITIES
*
MONTBL DATA XREGS,WHXETY      1 NIBBLE HEX OUTPUT
       DATA XREGS,RHENTY      READ HEX INPUT
       DATA XREGS,WHENTY      4 NIBBLE HEX OUTPUT
       DATA EREGS,ECHOEN      CHARACTER ECHO
       DATA IREGS,WENTRY      CHARACTER WRITE
       DATA IREGS,RENTRY      CHARACTER READ
       DATA XREGS,MENTRY      MESSAGE OUTPUT
       DATA MREGS,XOPENT      BREAKPOINT ENTRY
       PAGE
MIDTAB EQU  $-2                       0 - RESERVED MID
*
TYP0$  EQU  ($-MIDTAB/2)+MBIAS        1 - OUT R0
       DATA ETYP0
TYPC$  EQU  ($-MIDTAB/2)+MBIAS        2 - OUT 'CRLF'
       DATA ETYPC
TYPB$  EQU  ($-MIDTAB/2)+MBIAS        3 - OUT IOB(0)
       DATA ETYPB
TYPE$  EQU  ($-MIDTAB/2)+MBIAS        4 - OUT *R1 STRING(0)
       DATA ETYPE
TYPEN$ EQU  ($-MIDTAB/2)+MBIAS        5 - OUT *R1 STRING(-)
       DATA ETYPEN
TYPS$  EQU  ($-MIDTAB/2)+MBIAS        6 - OUT <INLINE ADR> S
       DATA ETYPS
TYPSN$ EQU  ($-MIDTAB/2)+MBIAS        7 - OUT <INLINE ADR> S
       DATA ETYPSN
DELAY$ EQU  ($-MIDTAB/2)+MBIAS        8 - DELAY LOOP
       DATA EDELAY
TYP11$ EQU  ($-MIDTAB/2)+MBIAS        9 - OUT INLINE CHARACT
       DATA ETYP11
TYPBE$ EQU  ($-MIDTAB/2)+MBIAS        A - TERM & OUT IOB
       DATA ETYPBE
GETC$  EQU  ($-MIDTAB/2)+MBIAS        B - TEST FOR CHARACTER
       DATA EGETC
GETCR$ EQU  ($-MIDTAB/2)+MBIAS        C - GET CHARACTER
       DATA EGETCR
LOOK$  EQU  ($-MIDTAB/2)+MBIAS        D - TEST INPUT BUFFER
       DATA E$LOOK
WAIT$  EQU  ($-MIDTAB/2)+MBIAS        E - WAIT FOR OUTPUT TO
       DATA EWAIT
SETTXT EQU  ($-MIDTAB/2)+MBIAS        F - SET TEXT MODE
       DATA STXT
SETGRA EQU  ($-MIDTAB/2)+MBIAS       10 - SET GRAPHICS MODE
       DATA SGRA
FTM$   EQU  ($-MIDTAB/2)+MBIAS       11 - FORCE TEXT MODE
       DATA EFTM$
FGM$   EQU  ($-MIDTAB/2)+MBIAS       12 - FORCE GRAPHIC MODE
       DATA EFGM$
FLUSH$ EQU  ($-MIDTAB/2)+MBIAS       13 - FLUSH INPUT BUFFER
       DATA EFLUSH
CMON$  EQU  ($-MIDTAB/2)+MBIAS       14 - CASSETTE ON
       DATA ECMON
CMOF$  EQU  ($-MIDTAB/2)+MBIAS       15 - CASSETTE OFF
       DATA ECMOF
MAXMID EQU  $-MIDTAB-2/2      HIGHEST SYSTEM MID
       PAGE
*
* THIS SECTION PROVIDES THE OUTPUT ROUTINES
* FOR BASIC I/O.
*
* TYP11$  - OUT INLINE CHARACTER
* TYP0$   - OUT R0
* TYPC$   - OUT 'CRLF'
* TYPBE$  - TERMINATE & OUTPUT I/O BUFFER
* TYPB$   - OUT I/O BUFFER(0)
* TYPE$   - OUT *R1 STRING(0)
* TYPEN$  - OUT *R1 STRING(-)
* TYPS$   - OUT <INLINE ADR> STRING(0)
* TYPSN$  - OUT <INLINE ADR> STRING(-)
* WAIT$   - WAIT FOR OUTPUT TO COMPLETE
* DELAY$  - DELAY FOR CNTDWN * 40mS
*
*     REGISTER USAGE
*   R0      UNUSED
*   R1      UNUSED
*   R2      SCRATCH
*   R3      SCRATCH
*   R4      TERMINATOR TYPE FLAG
*   R5      UNIT FLAGS
*   R6      SAVED RETURN ADDRESS
*   R7      BUFFER START POINTER
*   R8      BUFFER LOAD POINTER
*   R9      SYSTEM POINTER
*   R10     TEXT POINTER
*   R11     RETURN ADDRESS
*   R12     TEMP. REGISTER
*   R13     )
*   R14     ) RTWP VECTORS
*   R15     )
*
ETYP11 MOV  *R14+,R12         GET INLINE CHARACTER
       JMP  FORTY0            SKIP R0 FETCH
ETYP0  MOV  *R13,R12          GET R0
FORTY0 LI   R10,TY0           GET CHARACTER BUFFER
       ANDI R12,>FF00         ZERO LSB
       MOV  R12,*R10          SAVE CHARACTER IN BUFFER
       JMP  TYPE0             & OUTPUT
ETYPC  LI   R10,MCRLF         POINT TO 'CRLF'
       JMP  TYPE0             & OUTPUT
ETYPBE MOV  @14(13),R12       GET IOB POINTER
       SB   *R12,*R12         TERMINATE IOB
ETYPB  MOV  @IOB,R10          GET IOB START
       JMP  TYPE0             & OUTPUT
ETYPEN SETO R4                SET -VE TERMINATOR FLAG
ETYPE  MOV  @2(13),R10        GET CALLERS R1
       JMP  TYPE0             & OUTPUT
ETYPSN SETO R4                SET -VE TERMINATOR FLAG
ETYPS  MOV  *R14+,R10         GET INLINE ADDRESS & BUMP PC
*   FALL THROUGH TO ETYP0
*
* NOW OUTPUT STRING *R10 (TERMINATED BY A NULL)
*
TYPE0  BL   @WAIT             GET BUFFER
SETPTR LI   R8,OUTBUF         GET BUFFER START
       MOV  R8,R7             SAVE FOR DSRGO
FILBUF MOVB *R10+,*R8+        COPY OVER BYTE
       JEQ  ENDX1             NULL, EXIT
       JGT  FILB1             +VE, CONTINUE COPY
       MOV  R4,R4             -VE, LOOKING FOR -VE TERM. ?
       JEQ  FILB1             N, CONTINUE COPY
       CLR  R11               Y, READY HOLDING REGISTER
       DEC  R8                BACKUP TO -VE CHARACTER
       MOVB *R8,R11           GET BYTE BACK
       NEG  R11               CORRECT IT
       MOVB R11,*R8           PUT IT BACK
       JMP  ENDXFR            AND EXIT
ENDX1  DECT R8                BACKUP TO LAST CHARACTER
ENDXFR C    R8,R7             WAS IT A SINGLE NULL?
       JHE  ENDX2             N, OUT AS NORMAL
       INC  R8                Y, BUMP END POINTER
ENDX2  BL   @DSRGO            START DSR'S
       RTWP
*
FILB1  CI   R8,BUFEND         BUFFER FULL?
       JL   FILBUF            N, CONTINUE
       DEC  R8
       BL   @DSRGO            Y, OUT BUFFER SO FAR
       JMP  TYPE0             & CONTINUE
       PAGE
*
*    SYSTEM WAIT ROUTINES
*
*
* WAIT FOR BUFFER
*
WAIT   LI   R8,DSRCNT
       JMP  WAIT1
*
WAIT0  IDLE
WAIT1  LIMI 15                ENABLE INTERRUPTS
       MOV  *R8,*R8           GET ACTIVE DSR COUNT
       JGT  WAIT0             ACTIVE, IDLE
       B    *R11              AVAILIABLE, RETURN
*
* WAIT FOR OUTPUT TO COMPLETE
*
EWAIT  BL   @WAIT             GET BUFFER
       RTWP                   EXIT WHEN AVAILIABLE
*
*  WAIT TILL CNTDWN HAS GONE TO 0 (OR -VE)
*
EDELAY LI   R8,CNTDWN         POINT TO CNTDWN
       BL   @WAIT1            DELAY
       RTWP                   & EXIT
       PAGE
*
* THIS ROUTINE HANDS THE OUTPUT BUFFER TO A PRE-PROCESSOR
* IF REQUIRED AND THEN STARTS THE OUTPUT TRANSFER
* ON 9902'S SELECTED BY 'UNIT' AND ANY NON-STANDARD DEVICE
* HANDLERS ARE BID VIA THE BLWP VECTOR IN THE DEVICE TABLE
* R7 POINTS TO THE 1ST BYTE OF THE MESSAGE
* R8 POINTS TO THE LAST BYTE OF THE MESSAGE
*
DSRGO  MOV  R11,R6            SAVE RETURN
       LIMI 15                RE-ENABLE INTERRUPTS
       MOV  R8,@LOADPT        SAVE MSG END FOR DSR'S
       MOV  @UNIT,R5          GET UNIT FLAGS
       MOV  @OUTDSR,R11       PRE-PROCESSOR ?
       JEQ  OUTIT             N, SKIP
       BLWP *R11              Y, EXECUTE IT
OUTIT  CLR  @DSRCNT           RESET DSR COUNT
       LI   R11,DEVTBL        POINT TO DEVICE TABLE
       LI   R2,IOSTOR         REF I/O STOREAGE AREA
       LI   R3,16             DO 16 DEVICES
*
DSRGO0 MOV  *R11+,R12         GET DEVICE ENTRY
       JEQ  DSRGO1            0, NO DEVICE INSTALLED
       COC  @C1,R12           IS IT A 9902?
       JNE  DSRGO2            N, CALL IT'S HANDLER
       ANDI R12,>7FFE         Y, ISOLATE CRUBASE
       MOV  R7,*R2            SET UNLOAD POINTER
       SETO @2(2)             RESET CR DELAY
       COC  @C1,R5            DEVICE SELECTED?
       JNE  DSRGO1            N, SKIP IT
       INC  @DSRCNT           Y, COUNT IT
       SBO  19                ENABLE XMIT INTERRUPTS
DSRGO1 SRC  R5,1              SHIFT UNIT BIT
       JEQ  DGOXIT            0, NO MORE UNITS
       C    *R2+,*R2+         STEP TO NEXT STORAGE AREA
       DEC  R3                DONE ALL DEVICES?
       JNE  DSRGO0            N, LOOP
DGOXIT B    *R6               Y, RETURN
*
DSRGO2 COC  @C1,R5            IS THE DEVICE SELECTED?
       JNE  DSRGO1            N, NEXT ENTRY
*
*      DEVICE HANDLER INSTALLED, PASS OVER POINTERS
*     HANDLER WP:
*         R9   = OUTPUT BUFFER START
*         R10  = OUTPUT BUFFER END 
*         R11  = POINTER TO LOCAL STORAGE 
*         R13-15 RETURN CONTEXT
*
       MOV  R12,R0            SAVE HANDLER ADDRESS
       MOV  *R12,R12          GET ITS WP
       MOV  R7,@2*R9(12)      SET BUFFER START 
       MOV  R8,@2*R10(12)     SET BUFFER END 
       MOV  R2,@2*R11(12)     SET LOCAL STORAGE PTR
       BLWP *R0               CALL HANDLER
       JMP  DSRGO1            NEXT ENTRY
       PAGE
*
*   THIS MODULE HANDLES THE CHARACTER INPUT FOR BASIC.
* THE INPUT BUFFER IS FILLED BY THE 9902 DSR WHEN A
* RECEIVE INTERRUPT IS ENCOUNTERED. THESE ROUTINES UNLOAD
* THIS BUFFER AND RETURN THE CHARACTERS TO BASIC.
*
*
*   WAIT FOR CHARACTER ROUTINE
*
* CALLING SEQUENCE :
*      DATA GETCR$            CALL GET CHARACTER MID
*
* CHARACTER RETURNED IN R0 EXCEPT WHEN AN ESCAPE IS
* ENCOUNTERED WHILE ESCAPE IS ENABLED. IN THIS CASE
* PROGRAM EXECUTION IS RETURNED TO PRDY IN KEYBOARD MODE
* OR STPY IN RUN MODE.
* IF AN ESCAPE IS ENCOUNTERED AN HAS BEEN DISABLED VIA
* A 'NOESC' STATEMENT THE ESCAPE CHARACTER IS RETURNED
* IN THE MSB OF R0 AS WITH ANY OTHER CHARACTER.
*
*****************
*
*        TEST FOR CHARACTER INPUT
*
* CALLING SEQUENCE :
*      DATA GETC$             CALL TEST INPUT MID
*      <NO CHARACTER>         )
*      < CHARACTER  >         )RETURN POINTS
*      <   ESCAPE   >         )
*
*   THIS ROUTINE TESTS THE INPUT BUFFER FOR A CHARACTER AND
* RETURNS TO ONE OF THE FOLLOWING THREE WORDS DEPENDING ON
* THE RESULTS OF THE TEST.
*
* NO CHARACTER    EXECUTION RESUMES AT THE WORD FOLLOWING
*                 THE CALL.
* CHARACTER       EXECUTION RETURNS TWO WORDS AFTER THE
*                 CALL WITH THE CHARACTER IN THE MSB OF R7.
* ESCAPE          IF THE ESCAPE HAS BEEN DISABLED VIA THE
*                 'NOESC' COMMAND THE RETURN IS AS FOR A
*                 NORMAL CHARACTER. OTHERWISE EXECUTION
*                 RESUMES THREE WORDS AFTER THE CALL.
*
GETC1  LIMI 15                ALLOW INTERRUPTS
       IDLE                   WAIT FOR ONE
*
EGETCR SETO R4                SET WAIT FLAG
*
EGETC  LIMI 0                 DISABLE INTERRUPTS
       MOV  @RECPTR,R8        GET UNLOAD PTR
       C    R8,@INPTR         BUFFER EMPTY?
       JL   GETC2             N, UNLOAD
*
*      FLUSH THE INPUT BUFFER ( R4 IS CLEARED )
*
EFLUSH LI   R8,RBUF           Y, GET START OF BUFFER
       MOV  R8,@INPTR         RESET LOAD PTR
       MOV  R8,@RECPTR        RESET UNLOAD PTR
       MOV  R4,R4             WAIT ?
       JNE  GETC1             Y, LOOP
       RTWP                   N, EXIT
*
GETC2  CLR  *R13              READY CALLERS R0
       MOVB *R8+,*R13         COPY OVER BYTE
       MOV  R8,@RECPTR        UPDATE UNLOAD POINTER
       MOV  R4,R4             WAS IT A WAIT CALL?
       JNE  GETC5             Y, EXIT WITH CHARACTER
       CB   *R13,@B1B         N, WAS IT AN ESCAPE CHARACTER?
       JNE  GETC4             N, EXIT 2(14) - NORMAL CHAR.
       INCT R14               4(14) - ESCAPE
GETC4  INCT R14               2(14) - CHARACTER IN R0
GETC5  RTWP                   0(14) - NO CHARACTER
*
*      CASSETTE SUPPORT ROUTINES
*
ECMON  LI   R12,2*CASS02      SET CRUBASE
       SBO  16                TURN MOTOR ON
       SBO  18                ENABLE IT'S RECEIVE INTERRUPTS
       JMP  EFLUSH            FLUSH BUFFER & EXIT
*
ECMOF  LI   R12,2*CASS02      SET CRUBASE
       SBZ  18                KILL IT'S RECEIVE INTERRUPTS
       SBZ  16                TURN MOTOR OFF
       SBO  14                SET TO LOAD CONTROL REG.
       LDCR @B62,8            2 STOP,EV PAR,7 BITS,/3 CLOCK
       JMP  EFLUSH            FLUSH BUFFER & EXIT
       PAGE
************************************************************
*                                                          *
*           TEST THE INPUT BUFFER ROUTINE                  *
*                                                          *
*      THIS ROUTINE TESTS TO SEE IF THERE IS ANYTHING      *
*      IN THE INPUT BUFFER, IF NOT AN IMMEDIATE RETURN     *
*      IS MADE. IF THERE IS A CHARACTER IN THE BUFFER      *
*      THEN IT IS RETURNED IN THE CALLERS R0(MSB) WITH     *
*      THE LSB CLEARED.                                    *
*      NB.  THE CHARACTER IS NOT REMOVED FROM THE BUFFER.  *
*                                                          *
*      CALLING SEQUENCE :                                  *
*           DATA LOOK$                                     *
*           <EMPTY RETURN>                                 *
*           <CHARACTER RETURN>                             *
*                                                          *
************************************************************
E$LOOK LIMI 0                 MASK INTERRUPTS
       MOV  @RECPTR,R8        FETCH RECEIVE POINTER
       C    R8,@INPTR         UPTO THE LOADING POINTER ?
       JHE  EFLUSH            Y, NOTHING THERE - EXIT
       CLR  *R13              N, CLEAR CALLERS R0
       MOVB *R8,*R13          COPY THE CHARACTER OVER
       JMP  GETC4             AND EXIT @2(R14)
       PAGE
************************************************************
*                                                          *
*             VDP UTILITY ROUTINES                         *
*                                                          *
************************************************************
EFTM$  MOV  @VMODE,R0         TEXT MODE ALREADY?
       JEQ  STMXIT            Y, EXIT
*
*              SET VDP FOR TEXT MODE
*
STXT   EQU  $
       LIMI 15                ALLOW INTERRUPTS
       MOVB @FBCOL,@SFBC      SET FGND/BGND COLOUR
       BL   @LOADER           LOAD VDP REGISTERS
       BYTE >00,>80+R0   TEXT MODE, EXT. VIDEO OFF
       BYTE >90,>80+R1   16K,NO DISPLAY,NO INT,TXT,SIZ&MAG=0
       BYTE >01,>80+R2   PNTBA=>400
       BYTE >01,>80+R4   PGTBA=>800
SFBC   BYTE >00,>80+R7   FGND/BGND COLOURS
       DATA 0
       BLWP @LDCS             LOAD DEFAULT CHARACTER SET
       LI   R8,NTBA+>4000     REF PNT
       BL   @SENDAD           SEND ADDRESS TO VDP
       LI   R11,40*24         SET SCREEN SIZE
*
CLS    CLR  @XLOC             XLOC=0, YLOC=0
       MOVB @B20,@VRAM        WRITE SPACE CHARACTER
       DEC  R11               COUNT IT
       JNE  CLS               LOOP TILL ALL DONE
       MOVB @B20,@CCSAVE      SET SAVED CHARACTER TO A ' '
       CLR  @VMODE            FLAG VDP IN TEXT MODE
*
       BL   @LOADER           RE-ENABLE DISPLAY
       BYTE >D0,>80+R1   16K,DISPLAY ON,NO INT,TXT,SIZ&MAG=0
       DATA 0
*
STMXIT RTWP                   EXIT
*
EFGM$  MOV  @VMODE,R0         IN TEXT MODE?
       JNE  STMXIT            N, EXIT
*
*              SET VDP FOR GRAPHIC 2 MODE
*
*
SGRA   EQU  $
       LIMI 15                ALLOW INTERRUPTS
       MOVB @FBCOL,@BDCOL
       BL   @LOADER           LOAD VDP REGISTERS
       BYTE >02,>80+R0        GRAPHICS 2,NO EXT. VIDEO
       BYTE >80,>80+R1        DISABLE DISPLAY
       BYTE >06,>80+R2        PNT=>1800
       BYTE >FF,>80+R3        CTBA=>2000
       BYTE >03,>80+R4        PGT=>0000
       BYTE >36,>80+R5        SAT=>1B00
       BYTE >07,>80+R6        SPG=>3800
BDCOL  BYTE >00,>80+R7        BACKDROP=BGND COLOUR
       DATA 0
*
*  CLEAR DOWN THE SNT & SPG TABLES
*
       CLR  R6                WRITE NULLS
       LI   R7,VRAM           TO VRAM
       LI   R8,SPGBA1+>4000   REF SPG
       BL   @SENDAD           POINT VDP AT IT
       LI   R8,256*8          DO 256 8 BIT PATTERNS
*
KILSPG MOVB R6,*R7            NULL OUT THE PATTERN
       MOV  R8,R8             WASTE SOME TIME
       DEC  R8                DONE ?
       JNE  KILSPG
*
       LI   R8,SNTBA1+>4000   REF SNT
       BL   @SENDAD           POINT VDP AT IT
       LI   R8,32*8           KILL OFF ALL 32 PLANES
       LI   R6,>D000          FILL IT FULL OF SPRITE TERM.
*
KILSNT MOVB R6,*R7            CLEAR DOWN SNT
       MOV  R8,R8             WASTE SOME TIME
       DEC  R8                DONE?
       JNE  KILSNT            N, LOOP
*
*     SET UP PATTERN NAME TABLE
GCLEAR LI   R8,NTBA1+>4000    REF PNT
       BL   @SENDAD           SEN ADDRESS TO VDP
       SETO R11               RESET COUNT
*
SGRA1  INC  R11               NEXT PATTERN
       SWPB R11               POSITION LS BYTE
       MOVB R11,@VRAM         WRITE IT
       SWPB R11               RESTORE R11
       CI   R11,3*256         DONE ALL ENTRIES ?
       JL   SGRA1             N, LOOP
*     SET UP PATTERN GENERATOR TABLE
       LI   R8,PGTBA1+>4000   REF PGT
       BL   @SENDAD           SEN ADDRESS TO VDP
       LI   R11,3*256*8       RESET COUNT
*
SGRA2  CLR  @XLOC             RESET XLOC & YLOC
       MOVB @B00,@VRAM        RESET ENTRY
       DEC  R11               DONE ALL ENTRIES ?
       JNE  SGRA2             N, LOOP
*     SET UP PATTERN COLOUR TABLE
       LI   R8,CTBA1+>4000    REF PCT
       BL   @SENDAD           SEN ADDRESS TO VDP
       LI   R11,3*256*8       RESET COUNT
*
SGRA3  MOVB @FBCOL,@VRAM      SET ENTRY
       MOV  R8,R8             WASTE SOME TIME
       DEC  R11               DONE ALL ENTRIES ?
       JNE  SGRA3             N, LOOP
       SETO @VMODE            FLAG IN GRAPHICS MODE
*
       MOVB @S$SM,@S$$SM      RELOAD SPRITE SIZE & MAG
       BL   @LOADER           RE-ENABLE DISPLAY
S$$SM  BYTE >C0,>80+R1
       DATA 0
       RTWP
*
BD0    BYTE >D0
       EVEN
       PAGE
*
*    THIS ROUTINE LOADS THE VDP REGISTERS
*    FROM AN INLINE DATA TABLE.
*
LOADER MOVB *R11+,@VDPREG     WRITE REGISTER DATA
       C    *R11,*R11         DUMY DELAY FOR VDP
       MOVB *R11+,@VDPREG     WRITE REGISTER NUMBER
       MOV  *R11,*R11         END OF TABEL?
       JNE  LOADER            N, LOOP
       INCT R11               Y, SKIP TERMINATOR
       RT
*
*    THIS ROUTINE SENDS THE ADDRESS IN R8
*    TO THE VDP, EXIT IS VIA A NORMAL 'RT'
*
SENDAD SWPB R8                POSITION LSB
       MOVB R8,@VDPREG        SEND LSB
       SWPB R8                POSITION MSB
       MOVB R8,@VDPREG        SEND MSB
       C    *R11,*R11         DUMY DELAY FOR VDP
       RT                     RETURN TO CALLER
       END
