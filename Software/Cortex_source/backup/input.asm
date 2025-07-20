       TITL 'INPUT STATEMENT - CORTEX BASIC REV. 1.1'
       IDT  'INPUT'
*
*       INPY            ;INPUT ROUTINE
*
        REF INPEND      ;QUIT INPUT
        REF OUTL1       ;OUT LINE
        REF CVDIFZ      ;CONVERT DECIMAL TO INTEGER/FP
        REF EVARZ       ;EVALUATE VARIABLE
        REF EVSDZ       ;EVALUATE STRING
        REF GOSB1       ;GOSUB ENTRY
        REF BMVE        ;BYTE MOVE
        REF JMPR0A      ;BYTE MULTIPLEXOR JUMP
        REF PUTB        ;PUT BYTE IN BUFFER
        REF GETCR$      ;GET CHARACTER
        REF TYP0$       ;ECHO R0
        REF FFLG        ;FORMATTING FLAG
        REF HFLG        ;HELP FLAG
        REF IFLG        ;INPUT FLAG
       REF NIC,LNSZ      ;IO BUFFER SIZE
       REF  IOB          ;I/O BUFFER
       REF  INM1,INM2,INM3    PROMPTS
        DEF INPY
*
        DXOP STORE,1    ;STORE FPAC
        DXOP EVFIX,11   ;EVALUATE AND FIX
       PAGE
*
*       EXECUTE THE INPUT COMMAND
*
* CALLING SEQUENCE:
*
*       B @INPY
*
*       EXIT TO INPEND, GOSB1
*
*       R7 = OBC
*       R8 = PBC
*       R15 = CR/LF FLAG
*
INPY    CLR @HFLG       ;CLEAR HELP FLAG
        CLR @IFLG       ;CLEAR INPUT FLAG
        JMP INP1
*
*HELP QUERY SETUP
*
INPQM   EVFIX @HFLG     ;GET LINE #
*
INP0    DEC R8          ;BACKUP TO DELIMITER
*
INP1    SETO R15        ;SET FOR CR
*
INPSC   INC R15         ;SET FOR NO CRLF
        BL @JMPR0A      ;SWITCH BOARD
INPTB     BYTE INPE-INPTB/2,>00   ;NULL
          BYTE INPE-INPTB/2,>3C   ;::
          BYTE INPE-INPTB/2,>47   ;!
          BYTE INP1-INPTB/2,>3F   ;,
          BYTE INPSC-INPTB/2,>40  ;;
          BYTE INPFM-INPTB/2,>3E  ;#
          BYTE INPQM-INPTB/2,>41  ;?
          BYTE INPFL-INPTB/2,>42  ;%
          DATA 0
        DEC R8          ;NONE OF ABOVE, TRY STRING
        BLWP @EVSDZ
          JMP INPQ      ;STRING
          JMP INPD      ;$VAR
        BLWP @EVARZ     ;NEITHER, TRY VARIABLE
        MOV R2,R6       ;SAVE
        LI R2,INM1      ;'? '
*
INP3    BL @INPP        ;PROMPT AND GET STRING
        BLWP @CVDIFZ    ;CONVERT
          JMP INP4      ;FP
          CLR R1        ;NO NUMBER
          NOP
        MOV R0,R0       ;CHECK DELIMITER
          JNE INP5      ;PROBLEM
        CLR *R6+        ;INTEGER
        MOV R1,*R6+
        CLR *R6+
INP3A   CLR  @IFLG      CLEAR % FLAG (EXACT # OF CHARS)
        CLR  @FFLG      CLEAR NUMBER OF CHARS FLAG
        JMP INP0        ;LOOK AT DELIMITER
        PAGE
*GET NUMERIC INPUT
*
INP4    MOV R0,R0       ;EOL?
          JNE INP5      ;N, PROBLEM
        STORE *R6       ;MOVE NUMBER
        JMP INP3A
*
*ERROR IN NUMERIC INPUT
*
INP5    LI R2,INM2      ;OUT '?? '
        MOV @HFLG,R1 ;HELP WANTED?
          JEQ INP3      ;N
        SETO R0         ;YES, RETURN -1
        JMP INPP6
*
*% FORMATTING
*
INPFL   SETO @IFLG   ;SET FLAG
*
*# FORMATTING
*
INPFM   EVFIX @FFLG  ;GET FLAG
        JMP INP0
*
*OUT LITERAL
*
INPQ    LI R11,INP0     ;GET RETURN ADR
*
*OUT STRING
*       R2 = STRING ADR
*
OUTL    MOV R11,R13     ;SAVE RETURN
        MOV @IOB,R7      ;SET R7 (WSBC)
        LI R5,LNSZ      ;GET MAX
        BL @BMVE        ;MOVE TO OUTPUT BUFFER
          NOP
        MOV R13,R11
        B @OUTL1
        PAGE
*GET STRING
*
INPD    MOV R2,R6       ;SAVE ADR
        LI R2,INM3      ;GET ': '
        BL @INPP        ;PROMPT AND GET CHARACTERS
        MOV R7,R2       ;SET FOR MOVE
        MOV R6,R7
        LI R5,NIC       ;MAXIMUM OF EIGHTY CHARACTERS
        BL @BMVE        ;MOVE INTO VARIABLE
       JMP  INP3A     ;OK
       JMP  INP3A     ;OK
*
*END INPUT LINE
*
INPE    B @INPEND
        PAGE
*PROMPT AND GET CHARACTERS
*
INPP    MOV R11,R14     ;SAVE RETURN
        MOV R15,R15     ;SUPPRESS PROMPT?
          JNE INPP1     ;Y
        BL @OUTL        ;N, LIST
*
INPP1   MOV @IOB,R7      ;GET IOB  (WSBC)
        MOV @FFLG,R4 ;GET INPUT COUNTER
          JNE INPP2     ;OK
        LI R4,>80       ;SET MAX      ??????????????????????
*
INPP2   DATA GETCR$     ;GET CHARACTER
        BL @PUTB        ;STORE
          JMP INPP3     ;CR
          JMP INPP4     ;CONTROL CHARACTER
       DATA TYP0$       ;ECHO
        DEC R4          ;MORE CHARACTERS?
          JNE INPP2     ;Y
        JMP INPP5       ;N
*
INPP3   MOV  @IFLG,R0 EXACT NUMBER REQUIRED?
        JNE  INPP2      Y - CONTINUE
*
INPP5   SB *R7,*R7      ;TERMINATE
        MOV @IOB,R7      ;RESET BYTE COUNTERS  (WSBC)
        B *R14          ;RETURN
*
INPP4   MOV @HFLG,R1 ;HELP WANTED?
          JEQ INPP2     ;N, CONTINUE
        SWPB R0         ;Y
*
INPP6   MOV R0,@HFLG ;SAVE CONTROL CHARACTER
        CLR @FFLG    ;CLEAR FORMATTING
        CLR  @IFLG  CLEAR % FLAG
        B @GOSB1        ;DO SYSTEM GOSUB
*
       END
