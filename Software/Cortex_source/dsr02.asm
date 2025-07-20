       TITL 'DSR FOR TMS9902 UARTS - CORTEX BASIC REV 1.1'
       IDT  'DSR02'
       RORG
       COPY 'IODEFS.INC'
*
       DEF  INT4PC,D$RTWP,C10
       REF  MREG13
       REF  BELCNT,RBUFE,CRDELY
       REF  F$WHO,B00,B0D,BFA,CCNT,FFLG
       REF  UNIT,B1B,MODE,STPY,PRDY,ESCFLG
       REF  DEVTBL,IOSTOR,FDCDON,MONTOP
       REF  CMOF$,NEWP,PLF,C1,WPR1
*
*   R0
*   R1
*   R2
*   R3      SCRATCH
*   R4      'LOADPT' OUTPUT BUFFER LOAD PTR
*   R5
*   R6      'INPTR ' RECEIVE BUFFER LOAD PTR
*   R7      'DSRCNT' ACTIVE DSR COUNTER
*   R8      'RECPTR' RECEIVE UNLOAD POINTER
*   R9      'TRAP4 ' BAD LEVEL 4 TRAP
*   R10     POINTER TO UNLOAD POINTER STORAGE
*   R12     CRUBASE
*   R13-R15 RTWP VECTORS
*
       PAGE
************************************************************
*                                                          *
*            LEVEL 4 INTERRUPT SERVICE ROUTINE             *
*                                                          *
************************************************************
*
*  THIS ROUTINE CHECKS FOR :-
*      1.   KEYBOARD INTERRUPT.
*      2.   FDC INTERRUPT
*      3.   ALL 9902 INTERRUPTS
*      4.   USER LEVEL 4 INTERRUPT TRAP
*
INT4PC LI   R12,2*KEYBRD      SET CRUBASE TO KEYBOARD DATA
************************************************************
*                                                          *
*            CHECK FOR KEYBOARD INTERRUPT                  *
*                                                          *
************************************************************
       SBO  KBDACK-KEYBRD     MAKE SURE IT IS ENABLED !!!
       TB   KBDINT-KEYBRD     KEYBOARD?
       JEQ  FDC1              N, CHECK THE FDC
       CLR  R11               Y, READY HOLDING REGISTER
       STCR R11,8             GET THE KEYBOARD DATA
       SBZ  KBDACK-KEYBRD     ACKNOWLEDGE IT
       SBO  KBDACK-KEYBRD
       CI   R11,>FF00         GRAPHIC RUBOUT
       JNE  KBDIN             N, NORMAL KEYBOARD INPUT
       SETO R7                Y, KILL DSRCNT FLAG
       MOV  @C1,@UNIT         AND RESET UNIT FLAG TO 1
       RTWP                   AND EXIT 
*
KBDIN  MOV  @PLF,R12          LOAD/SAVE?
       JEQ  CHKESC            N, DECODE AS NORMAL
       CB   R11,@B1B          Y, ESCAPE?
       JEQ  ABRTIO            Y, ABORT
       RTWP                   N, THROW IT AWAY
************************************************************
*                                                          *
*            CHECK FOR FDC INTERRUPT                       *
*                                                          *
************************************************************
FDC1   TB   FDCINT-KEYBRD     WAS IT THE FDC?
       JEQ  POLL02            N, POLL 9902'S
       MOVB @B00,@FDC         Y, RESET FDC
       SETO @FDCDON           SET FDC DONE FLAG
       RTWP
************************************************************
*                                                          *
*            ALL 9902'S FOR INTERRUPT                      *
*                                                          *
************************************************************
POLL02 LI   R10,IOSTOR        POINT TO I/O STORAGE AREA
       LI   R11,DEVTBL        POINT TO DEVICE TABLE
       LI   R3,16             DO 16 DEVICES
C10    EQU  $-2
NXTENT MOV  *R11+,R12         GET DEVICE ENTRY
       COC  @C1,R12           IS IT A 9902?
       JNE  NXTDEV            N, DO NEXT DEVICE
       ANDI R12,>7FFE         Y, ISOLATE CRUBASE
       TB   31                THIS ONE?
       JEQ  SERV              Y, SERVICE IT
NXTDEV C    *R10+,*R10+       SKIP STORAGE AREA
       DEC  R3                COUNT DEVICE
       JNE  NXTENT            LOOP TILL ALL DEVICES DONE
*
* NON-STANDARD DEVICE INTERRUPT
*
       MOV  R9,R9             TRAP SET?
       JEQ  EXIT4             N, IGNORE IT
       BLWP *R9               Y, GOTO ROUTINE
EXIT4  RTWP                   EXIT
*
* SERVICE 9902 INTERRUPT
*
SERV   TB   16                RECEIVE INTERRUPT
       JNE  TRYTIM            N, TRY NEXT INTERRUPT SOURCE
       STCR R11,8             Y, GET CHARACTER
       SBO  18                RESET RBRL
       TB   9                 WAS THERE AN ERROR ?
       JEQ  QUIT              Y, IGNORE IT !
       MOV  @PLF,R10          N, TEST LOAD/SAVE ?
       JNE  LOADIN            Y, DO SPECIAL CHECKS
*
CHKESC MOV  @ESCFLG,R12       RUNING WITH NOESC ?
       JNE  SERV1             Y, DONT CHECK FOR ESCAPE
       CB   R11,@B1B          N, WAS IT AN ESCAPE?
       JNE  SERV1             N, BUFFER IT 
       CLR  @FFLG             Y, RESET FORMATTING FLAG
*
       MOV  @F$WHO,R0         WHO IS IN CONTROL ?
       JEQ  ESC0              BASIC, TAKE BASIC EXIT
       LI   R0,MREG13         LOAD POINTER TO R13
       MOV  R13,*R0+          COPY CONTEXT
       MOV  R14,*R0+          COPY CONTEXT 
       MOV  R15,*R0+          COPY CONTEXT
       B    @MONTOP           MONITOR, EXIT TO MONTOP
*
*      ABORT !     IF PLF > +1 THEN DO A NEW
*
ABRTIO MOV  @C1,@UNIT         <>0 SET UNIT TO 1
       DEC  R12               ABORT ?
       JGT  DONEW             +VE, DO A 'NEW'
*
ESC0   LI   R14,STPY          DEFAULT TO 'STOP'
       MOV  @MODE,R12         KEYBOARD MODE?
       JNE  ESC2              N, EXIT SET UP
ESC1   LI   R14,PRDY          Y, EXIT TO PRDY
ESC2   LI   R13,WPR1          SET MAIN WORKSPACE
       CKOF                   MAKE SURE MAPPER IS OFF
       DATA CMOF$             TURN OFF THE TAPE
       RTWP
DONEW  LI   R14,NEWP          DO A NEW
       JMP  ESC2
*
*    9902  CHARACTER RECEIVED DURING LOAD/SAVE
*
LOADIN JLT  QUIT                 SAVE   - THROW IT AWAY
       CI   R12,2*CASS02         LOAD   - FROM CASSETTE ?
       JNE  QUIT                          N, THROW IT THEN !
*                                         Y, STORE IT
*
SERV1  CI   R6,RBUFE          OFF BUFFER?
       JHE  BEEP              Y, THROW CHARACTER AWAY
       MOVB R11,*R6+          N, SAVE IT
       RTWP                   TERMINATE
BEEP   SETO @BELCNT           BEEP !!
       RTWP
*
TRYTIM TB   19                TIMER?
       JNE  TRYDSC            N, TRY NEXT INTERRUPT SOURCE
       SBZ  20                Y, DISABLE TIMER INTERRUPTS
       TB   23                XSRE ?
       JNE  ENBTIM            N, DON'T START COUNT YET
       TB   22                Y, XBRE ?
       JNE  ENBTIM            N, DON'T COUNT YET
       DEC  @2(10)            DECREMENT CR DELAY
       JLT  DONE              TIME UP, CONTINUE OUTPUT
ENBTIM SBO  20                RE-ENABLE TIMER
QUIT   RTWP                   EXIT
*
TRYDSC TB   20                DSCH INTERRUPT
       JNE  TRYXMT            N, TRY TRANSMITTER INTERRUPT
       SBZ  21                Y, DISABLE DSCH INTERRUPTS
       JMP  SETREQ            SET UP PTRS AND CHECK DSR  
*
DSR    TB   27                ONLINE?
       JEQ  OUTPUT            Y, OUT CHARACTER
       SBO  21                N, ENABLE DSCH INTERRUPTS
       RTWP                   EXIT
*
*
TRYXMT TB   22                XBRE INTERRUPT ?
       JNE  QUIT              N, GLITCH
       SBZ  19                DISABLE XBRE INTERRUPTS
SETREQ MOV  R11,R3            Y, SAVE DEVTBL INDEX REG.
       MOV  @2(10),R11        CR DELAY SET?
       JGT  SETTIM            Y, SET TIMER
*                             N, FALL THROUGH TO DONE
DONE   MOV  *R10,R11          GET UNLOAD POINTER
       C    R11,R4            DONE
       JLE  DSR               N, CHECK FOR ONLINE
       DEC  R7                Y, FLAG DSR TERMINATED
D$RTWP RTWP                   EXIT
*
SETTIM SBO  13                SET LDIR FLAG
       LDCR @BFA,8            LOAD TIMER
       SBO  20                Y, START TIMER
       RTWP                   EXIT
*
OUTPUT SBO  16                RTS ON
       LDCR *R11,R8           SEND CHARACTER
       CI   R12,2*CASS02      IS IT THE CASSETTE PORT ?
       JEQ  OPT1              Y, LEAVE RTS ALONE
       SBZ  16                N, TURN RTS OFF
OPT1   CB   *R11+,@B0D        CR ?
       JNE  XMTEN             N, UPDATE POINTER
       CLR  @CCNT             Y, RESET COLUMN COUNTER
       MOV  @-2(R3),R3        FETCH DEVICE ENTRY
       JGT  XMTEN             +VE, NO CR DELAY
       MOV  @CRDELY,@2(10)    Y, LOAD CR DELAY COUNT
XMTEN  MOV  R11,*R10          UPDATE UNLOAD POINTER
       SBO  19                ENABLE XBRE INTERRUPTS
       RTWP                   EXIT
       END
