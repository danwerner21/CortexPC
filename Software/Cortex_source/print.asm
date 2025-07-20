       TITL 'PRINT STATEMENT - CORTEX BASIC REV. 1.1'
       IDT 'PRINT'
       DEF PRTY,HOUT,HOUT$,HTXT
       REF B20,B3F,XLOC,YLOC,B4C,B40
       REF BMVE,CCNT,FFLG,VMODE,TYP0$,TYP11$
       REF CVDIZ,EVERZ,EVSDZ
       REF CURFLG,LNSZ,MODE,IOB
       REF OUTL1,PRTEND,JMPR0A
       DXOP EVFIX,11
       DXOP OUTFP,12
*
ERROR  EQU  >2F80
ERROR2 EQU  ERROR+>20
*
*    PRINT @
*
PRTAT  BLWP @EVSDZ            ;LOOK FOR STRING
       JMP  SCRNQ             ;"
       JMP  SCRNQ             ;$
       CB   *R8+,@B4C         ;(?
       JNE  ERR15             ;N
       EVFIX R3               ;EVALUATE X
       EVFIX R1               ;EVALUATE Y
       CI   R0,>4D00          ;)?
       JNE  ERR15             ;N, ERROR
       CI   R1,23             ;VALID Y?
       JH   ERR15             ;N
       CI   R3,39             ;VALID X?
       JH   ERR15             ;N
       MOV  @VMODE,R10        ;TEXT MODE?
       JEQ  UCL0              ;Y, EVERYTHING OK
       CI   R3,31             ;N, CHECK VALID X
       JH   ERR15             ;N
       SLA  R1,3              ;OK, FORM Y PIXEL CURSOR
       SLA  R3,3              ;    FORM X PIXEL CURSOR
UCL0   SWPB R1                ;PUT IN MSB
       SWPB R3                ;PUT IN MSB
*
*  UPDATE CURSOR LOCATION
*
       MOV  @CURFLG,R0        ;GET CURSOR FLAG
       JNE  UCL1              ;OFF, LEAVE IT
       DATA TYP11$,>1D00      ;ON, TURN IT OFF THEN
UCL1   MOVB R3,@XLOC          ;UPDATE X
       MOVB R1,@YLOC          ;UPDATE Y
       MOV  R0,@CURFLG        ;RESTORE CURSOR FLAG
       JNE  PRT2              ;WAS OFF, LEAVE IT
       DATA TYP11$,>1C00      ;WAS ON, PUT IT BACK ON
       JMP  PRT2              ;CONTINUE PRINTING
*
*   SCREEN COMMANDS IN A STRING
*
SCRNQ  MOV  R7,R3             ;SAVE IOB POINTER
       MOV  R2,R7             ;SET FOR CONVERSION
*
SCRNQ1 LI   R1,1              ;DEFAULT TO 1
       BLWP @CVDIZ            SEE IF THERE IS A NUMBER
       JMP  ERR15             FP - ERROR
       INC  R7                NO NUMBER, INC OVER DELIMITER
       MOV  R1,R5
*
       MOVB R0,R0             DONE ?
       JEQ  SCRNQ5            Y, EXIT
       LI   R2,SCRNQ6         GET TABLE ADDRESS
SCRNQ2 MOV  *R2+,R4           GET CODE
       JEQ  ERR15             0, NOT FOUND - ERROR
       CB   R0,R4             FOUND ?
       JNE  SCRNQ2            N, LOOP
       SWPB R4                Y, POSITION IT
SCRNQ3 MOVB R4,R0             GET BYTE
       DATA TYP0$             OUTPUT IT
       DEC  R5                DONE?
       JNE  SCRNQ3            N, LOOP
       JMP  SCRNQ1            Y, CONTINUE CMD STRING
*
SCRNQ5 DEC  R8                BACKUP OVER DELIMITER
       MOV  R3,R7             RESTORE IOB POINTER
       JMP  PRT2              ;CONTINUE PRINTING
ERR15  DATA ERROR+15          ;INVALID SCREEN COMMAND
       PAGE
*
*PRINT COMMAND
*
PRTY    LI R14,LNSZ     ;GET LINE SIZE
        MOV @MODE,R0 ;LOOK AT MODE
          JNE PRT0      ;RUN
        SRL R14,3       ;IDLE, LOAD COMMA SIZE
*
PRT0   MOV @IOB,R7      ;SET R7  (WSBC)
*
PRT1    SETO R15        ;SET FOR CRLF
*
PRT2    INC R15
        BL @JMPR0A      ;SWITCH BOARD
PRTTB     BYTE PRTE-PRTTB/2,>00   ;NULL
          BYTE PRTE-PRTTB/2,>3C   ;::
          BYTE PRTE-PRTTB/2,>47   ;!
          BYTE PRTF-PRTTB/2,>3E   ;#
          BYTE PRTC-PRTTB/2,>3F   ;,
          BYTE PRTSC-PRTTB/2,>40  ;;
          BYTE PRTAB-PRTTB/2,>39  ;TAB
          BYTE PRTAT-PRTTB/2,>3D  ;@
          DATA 0
        DEC R8          ;TRY STRINGS
        BLWP @EVSDZ
          JMP PRTQ      ;"
          JMP PRTQ      ;$
        BLWP @EVERZ     ;NEITHER, TRY NUMBER
        MOV @FFLG,R4 ;FORMATTING?
          JNE $+6       ;Y, IGNOR SPACE
        MOVB @B20,*R7+  ;N, OUT SPACE
        OUTFP *R2       ;OUTPUT #
*
PRT4    DEC R8          ;BACKUP TO DELIMITER
        JMP PRT1
*
PRTE   B @PRTEND         ;END PRINT
        PAGE
*PRINT FORMATTING
*
PRTF    BLWP @EVSDZ     ;EVALUATE STRING
          JMP PRTF1     ;"
          JMP PRTF1     ;$
        LI R2,5         ;MAYBE HEX
        CB *R8,@B3F     ;#,?
          JNE PRTH1     ;N
        INC R8
        EVFIX R1        ;Y, GET #
        JMP PRTH2
*
PRTH1   CB *R8,@B40     ;#,?
          JNE PRTH3     ;N
        INC R8
        EVFIX R1        ;Y, GET #
        DECT R2
        SLA R1,8        ;LEFT JUSTIFY
*
PRTH2   BL @HOUTE       ;OUT #
        DEC R7          ;BACKUP ON "H
        JMP PRT4
*
PRTH3   EVFIX R1        ;FORMAT FREE
        MOVB @B20,*R7+  ;OUT SPACE
        BL @HOUT        ;OUT #
        JMP PRT4
*
PRTF1   MOV R2,@FFLG    ;SET FLAG
        JMP PRT4
*
PRTQ    MOV R14,R5      ;GET NUMBER OF BYTES AVAILABLE
        A @IOB,R5
        S R7,R5
        BL @BMVE        ;MOVE INTO OUTPUT STRING
          JMP PRTQ1     ;OK
        BL @OUTL1       ;LIST LINE
        A R3,@CCNT      ;UPDATE BACKSPACES
        JMP PRTQ
*
PRTQ1   A R3,@CCNT      ;UPDATE COLUMN COUNT
        JMP PRT4
*
*COMMA
*
PRTC    BL @PRTCK       ;GET CURRENT # OF BYTES
        ANDI R0,>7      ;MOD 8
        AI R0,-8        ;GET -# OF BLANKS
        BL @PRTSP       ;OUT SPACES
*
*SEMI-COLON OPERATER
*
PRTSC   BL @PRTCK       ;CHECK BUFFER OVERFLOW
        S R14,R0        ;EXCEEDED?
          JLT PRT2      ;N
        BL @OUTL1       ;Y, PRINT LINE
        JMP PRT2
        PAGE
*TAB FUNCTION
*
PRTAB   EVFIX R1        ;GET TAB
        DEC R8          ;BACKUP OVER DELIMITER
        ANDI R1,>7F     ;LIMIT TABS TO 127
        BL @PRTCK       ;GET CURRENT CHARACTER POSITION
        S R1,R0         ;NEED SPACES?
          JGT PRT2      ;N, PAST POINT
        MOV  @MODE,R1     ;CHECK MODE
        JEQ  PRT2             ;KEYBOARD MODE, IGNORE
        BL @PRTSP       ;Y, OUT SPACES
        JMP PRT2
*
*OUT -R0 SPACES
*
PRTSP   INC R0          ;DONE?
          JGT PRTCK1    ;Y
        MOVB @B20,*R7+  ;N, OUT SPACE
        JMP PRTSP
*
PRTCK   MOV @CCNT,R0 ;GET LINE SIZE
        A R7,R0
        S @IOB,R0        ;GET CURRENT CHARACTER POSITION
PRTCK1  RT
        PAGE
*PRINT HEX
*
HOUT$   SETO R0         ;SET ZERO FLAG
       JMP HOUT$1
HOUT    CLR R0          ;RESET ZERO FLAG
HOUT$1  MOVB @HTXT,*R7+ ;OUT "0
        LI R2,4         ;DO 4 DIGITS
*
HOUT1   MOV R1,R4
        SLA R1,4        ;ISOLATE DIGIT
        SRL R4,12
          JNE HOUT2     ;NON-ZERO
        MOV R0,R0       ;FLAG SET?
          JEQ HOUT3     ;N
HOUT2   MOVB @HTXT(4),*R7+
*
HOUTE   SETO R0         ;SET FLAG
*
HOUT3   DEC R2          ;DONE?
          JGT HOUT1     ;N
        MOVB @B48,*R7+  ;Y, OUT "H
        RT
*
       EVEN
SCRNQ6 BYTE 'U',>0B           UP
       BYTE 'D',>0A           DOWN
       BYTE 'L',>08           LEFT
       BYTE 'R',>09           RIGHT
       BYTE 'B',>0D           BEGINNING
       BYTE 'C',>0C           CLEAR SCREEN
B48    BYTE 'H',>1E           HOME
       DATA 0
*
HTXT    TEXT '0123456789ABCDEF'
*B40     BYTE >40
*B48     BYTE >48
        EVEN
       END
