       TITL 'DECIMAL/BINARY CONVERSION - CORTEX BASIC REV. 1.1'
       IDT  'CVDB'
*
*       CVDIZ   ;CONVERT DECIMAL TO BINARY INTEGER
*       CVDIFZ  ;CONVERT DECIMAL TO FLOATING POINT
*       CVDB20  ;GET NEXT DIGIT
*
       REF  CVGCN1            ;GET FLOATING POINT CONSTANT
       REF  CVCH              ;CONVERSION HOLDING AREA
       REF  WPR2              ;SECONDARY WORKSPACE
       REF  CVC10             ;FLOATING POINT CONSTANTS
       REF  FPAC              FLOATING POINT ACCUMULATOR
       REF  B20               ;SPACE CHARACTER
       REF  C000A
*
       DEF  CVDIZ,CVDIFZ      ENTRY POINTS
       DEF  CVDB20
*
       DXOP LOADF,0           ;LOAD FPAC
       DXOP STORE,1           ;STORE FPAC
       DXOP FADD,2            ;ADD TO FPAC
       DXOP FSUB,3            ;SUBTRACT FROM FPAC
       DXOP FMUL,4            ;MULTIPLY FPAC
       DXOP FDIV,5            ;DIVIDE FPAC
       DXOP SCALE,6           ;SCALE FPAC
       DXOP NORMAL,7          ;NORMALIZE FPAC
       DXOP CLEAR,8           ;CLEAR FPAC
       DXOP NEGATE,9          ;NEGATE FPAC
       DXOP FLOATF,10         ;FLOAT FPAC
ERROR  EQU  >2F80             ;XOP XX,14  (ERROR CALL)
ERROR2 EQU  ERROR+>20
       PAGE
*       CVDIZ AND CVDIFZ CONVERT A STRING OF
*       ASCII DECIMAL DIGITS TO A BINARY NUMBER.
*       CVDIZ IS CALLED IF ONLY AN INTEGER WILL
*       BE ACCEPTED.  CVDIFZ IS CALLED IF A
*       FLOATING POINT NUMBER CAN BE ALLOWED.
*       CVDI IS FIRST EXECUTED TO GET AN
*       INTEGER.  IF IT FAILS, THEN A RETURN
*       IS MADE OR CVDIFZ IS EXECUTED DEPENDING
*       ON WHICH ROUTINE WAS INITIALLY CALLED.
*
*
* CALLING SEQUENCE:
*
*       BLWP @CVDIZ
*         OR
*       BLWP @CVDIFZ
*
*      IN - R7 =PTR
*
*      OUT -    R0 = DELIMITER
*               R1 = 16-BIT 2'S COMPLEMENT INTEGER
*               FPAC = FLOATING POINT NUMBER
*               R7 = NEW PTR
*
*      NORMAL EXIT - RTWP
*      ERROR EXIT
       PAGE
* CONVERT DECIMAL TO INTEGER
*       BLWP @CVDIZ
*         16-BIT OVERFLOW - FP # IN FPAC
*         NO NUMBER
*         (HEX ON CVDIF)
*         NUMBER
*
*  IN   R7 = PTR
*  OUT  R0 = DELIMITER
*       R1 = 16-BIT 2'S COMPLEMENT INTEGER
*       R7 = NEW PTR
*
CVDIZ  DATA WPR2,CVDI
CVDIFZ DATA WPR2,CVDIF
*
CVDI   CLR  R8                GET INTEGER ONLY
       JMP  $+4
CVDIF  SETO R8                ALLOW EXPONENT
*
       MOV  @14(13),R7        GET PTR
       CLR  R2                CLEAR RESULT
       CLR  R4                CLEAR SIGN FLAG
       BL   @CVDB25           LOOK FOR SIGN
       JMP  CVDI4             NO NUMBER, RETURN
       SETO R4                NEGATIVE
       MOV  R7,R6
*
CVDIH1 MOVB *R6+,R0
       SRL  R0,8              ;POSITION
       AI   R0,->30
       JLT  CVDIH4            ;NOT HEX
       CI   R0,>09
       JLE  CVDIH2            ;HEX 0-9
       AI   R0,->07
       CI   R0,>0A
       JLT  CVDIH4            ;NOT HEX
       CI   R0,>0F
       JH   CVDIH3            ;NOT HEX, "H MAYBE?
*
CVDIH2 SLA  R2,4              ;ADD NEW HEX DIGIT
       A    R0,R2
       JMP  CVDIH1
*
CVDIH3 AI   R0,->11           ;"H?
       JNE  CVDIH4            ;N
       MOVB *R6+,R0           ;Y, GET DELIMITER
       MOV  R6,R7             ;UPDATE R7
       JMP  CVDI3A
*
CVDIH4 CLR  R2                ;NOT HEX, CLEAR R2
*
CVDI1  BL   @CVDB20           GET DIGIT
       JMP  CVDI2             N
       MOV  R2,R1             SET FOR MULTIPLICATION
       MPY  @C000A,R1         R1,R2=R1*10
       MOV  R1,R1             OVERFLOW?
       JNE  CVDB              Y
       MOV  R2,R2             OVERFLOW?
       JLT  CVDB              ;Y
       A    R0,R2             ADD NEW DIGIT, OVERFLOW?
       JLT  CVDB              Y, OVERFLOW
       JMP  CVDI1             LOOP
*
CVDI2  CI   R0,>2E00          ".?
       JEQ  CVDI5             Y
       CI   R0,>4500          "E?
       JEQ  CVDI5             Y - SEE IF LEGAL
*
CVDI3  MOV  R8,R8             ;SKIP HEX RETURN?
       JEQ  CVDI3A            ;Y
       INCT R14               ;N
*
CVDI3A MOV  R4,R4             N, NEGATIVE?
       JEQ  $+4               N
       NEG  R2                Y, NEGATE
       MOV  R2,@2(13)         RETURN NUMBER IN R1
       MOV  R7,@14(13)        RETURN PTR IN R7
       INCT R14               RETURN 4(14)
CVDI4  INCT R14               RETURN 2(14) - NO NUMBER
       MOV  R0,*R13           RETURN DELIMITER IN R0
       RTWP RETURN            0(14) - OVERFLOW
*
CVDI5  MOV  R8,R8             EXPONENT ALLOWED?
       JEQ  CVDI3A            N
*
* FALL THRU TO CVDB
       PAGE
* CONVERT DECIMAL TO FP
*       BLWP @CVDBZ
*         NUMBER
*       NO NUMBER
*
*  IN   R7 = STRING ADDRESS
*  OUT  R0 = DELIMITER
*       R7 = NEW PTR
*       FPAC = NUMBER
*
CVDB   CLEAR 0                CLEAR FPAC
       MOV  @14(13),R7        GET STRING ADR
       SETO R2                SET DECIMAL FLAG
       CLR  R6                DECIMAL ADJUSTMENT=0
*
       CLR  R12               CLEAR SIGN FLAG
       CLR  R4                CLEAR SIGNIFICANT DIGITS COUNT
       BL   @CVDB25           PROCESS SIGN
       JMP  CVDI4             NO NUMBER
       SETO R12               NEG, SET TO -1
*
CVDB1  BL   @CVDB20           GET CHARACTER
       JMP  CVDB4             NOT NUMBER
       MOV  R2,R2             AFTER DECIMAL?
       JLT  $+4               N
       INC  R6                Y, COUNT
       MOV  R0,R0             '0'?
       JNE  CVDB1A
       MOV  R4,R4             Y - LEADING '0'?
       JEQ  CVDB1
CVDB1A INC  R4                Y - INCREMENT SIG DIGITS COUNT
       CI   R4,11             11 SIG DIGITS?
       JLE  CVDB1C
       CLR  R0                Y - SET DIGIT TO 0
CVDB1C FMUL @CVC10            ;OK, FPAC=FPAC*10
       SLA  R0,4              00X0
       JEQ  CVDB1             IGNOR IF ZERO
       AI   R0,>4100          41X0 (FP # NOW)
       MOV  R0,@CVCH          SAVE
       FADD @CVCH             ADD NEW DIGIT
       JMP  CVDB1
*
CVDB4  CI   R0,>2E00          ".?
       JNE  CVDB5             N
       MOV  R2,R2             Y - FIRST PERIOD?
       JEQ  CVDB10            N - END CONVERSION
       MOV  R8,R8             EXPONENT ALLOWED?
       JEQ  CVDB10            N - FINISH
       CLR  R2                Y, SET FLAG
       JMP  CVDB1
*
CVDB5  CI   R0,>4500          "E?
       JNE  CVDB10            N
       LI   R5,>0501          Y, (NEG R1)
       BL   @CVDB25           CHECK FOR SIGN
       JMP  CVDI4             PROBLEM, RETURN NO NUMBER
       SLA  R5,12             NEG, LOAD NOOP (1000)
       CLR  R1
*
CVDB6  BL   @CVDB20           GET DIGIT
       JMP  CVDB7             NO DIGIT, DONE
       MOV  R1,R3
       MPY  @C000A,R3         X 10
       A    R0,R4             ADD NEW DIGIT
       MOV  R4,R1             RESTORE R1
       JMP  CVDB6             LOOP AGAIN
*
CVDB7  X    R5                DO EXPONENT CHANGE
       A    R1,R6             ADD EXPONENT ADJUSTOR
*
CVDB10 MOV  R0,*R13           DONE, RETURN DELIMITER
       MOV  R7,@14(13)        RETURN NEW PTR
*
CVDB11 MOV  R6,R8             CHECK ADJUSTMENT
       JLT  CVDB12            DO ADJUSTMENT NECESSARY
       JEQ  CVDB13            "       "       "
       BL   @CVGCN1           GET CONSTANT
       S    R8,R6
       FDIV *R0               DIVIDE BY 10^R6
       JMP  CVDB11
*
CVDB12 NEG  R8
       BL   @CVGCN1           GET CONSTANT
       A    R8,R6
       FMUL *R0               MULTIPLY BY 10^R6
       JMP  CVDB11
*
CVDB13 MOV  R12,R12           CHECK FOR NEGATIVE #
       JEQ  CVDB14            POSITIVE
       NEGATE 0
*
* NOTE: -32768 IS EVALUATED AS +32768 (HAS TO GO TO FLOATING
* POINT FORMAT) AND THEN NEGATED.  IF FPAC CORRESPONDS TO
* >C480 >0000 >0000 THEN SET IT TO >0000 >8000 >0000
*
       LI   R12,FPAC          REF FPAC
       MOV  R12,R3
       LI   R6,CC480          REF -32768 (IN FL PT FORMAT)
       C    *R12+,*R6+        1ST WORD SAME?
       JNE  CVDB14
       C    *R12+,*R6+        Y - 2ND WORD SAME?
       JNE  CVDB14
       C    *R12,*R6+         Y - 3RD WORD SAME?
       JNE  CVDB14
       CLR  *R3+              Y - CLEAR 1ST WORD
       MOV  *R6,*R3+          2ND WORD TO >8000
       MOV  *R6,R2            R2=RESULT REGISTER FOR CVDI
       CLR  *R3               CLEAR 3RD WORD
       CLR  R4                CLEAR NEGATE FLAG
       JMP  CVDI3
CVDB14 RTWP
*
CC480  DATA >C480,0,0
       DATA >8000
       PAGE
* CVDB20:-  GETS THE NEXT DIGIT IF THERE
*           IS ONE.  LEADING BLANKS ARE SKIPPED.
*           IF THE NEXT CHARACTER IS A NON-DIGIT
*           THEN RETURN IS MADE TO THE FIRST
*           INSTRUCTION FOLLOWING THE CALL.  IF
*           THE NEXT CHARACTER IS A DIGIT, A
*           RETURN IS TAKEN TO THE SECOND
*           INSTRUCTION FOLLOWING THE CALL.
*
* CALLING SEQUENCE:
*
*       BL @CVDB20
*
*       IN -    R7 = POINTER
*       OUT -   R0 = CHARACTER
*
*       NORMAL EXIT - RETURN + 1 ON DIGIT
*                       RETURN ON NON-DIGIT
*
CVDB20 CLR  R0                GET DIGIT
       MOVB *R7+,R0           GET NEXT CHARACTER
       JEQ  CVDB22            EOL
       CI   R0,>2000          BLANK?
       JEQ  CVDB20            Y
       CI   R0,>3000          <"0?
       JL   CVDB22            Y
       CI   R0,>3900          >"9?
       JH   CVDB22            Y
       SLA  R0,4
       SRL  R0,12             RIGHT JUSTIFY CHARACTER
       INCT R11
CVDB22 RT   RETURN
       PAGE
* PROCESS SIGN
*       BL @CVDB25
*         NO NUMBER
*         -NUMBER
*       NUMBER OR +NUMBER
*
CVDB25 CB   *R7+,@B20         SPACE?
       JEQ  CVDB25            ?
       DEC  R7                BACKUP
       MOV  R7,R1             MARK
       MOV  R11,R10
       BL   @CVDB20           GET NUMBER
       JMP  CVDB28            NO NUMBER
CVDB26 INCT R10               NUMBER, RETURN 4(10)
       INCT R10               -NUMBER, RETURN 2(10)
CVDB27 MOV  R1,R7             RESTORE R7
       B    *R10              RETURN
*
CVDB28 CI   R0,>2B00          "+?
       JEQ  CVDB29            Y
       CI   R0,>2D00          N, "-?
       JEQ  CVDB30            Y
       CI   R0,>2E00          ".?
       JNE  CVDB27            N, NO NUMBER
       DEC  R7                Y, BACKUP OVER PERIOD
*
CVDB29 BL   @CVDB31           PROCESS POSITIVE NUMBER
       JMP  CVDB26            OK
*
CVDB30 BL   @CVDB31           PROCESS NEGATIVE NUMBER
       JMP  CVDB26+2          OK
*
CVDB31 MOV  R11,R3            SAVE RETURN
       MOV  R7,R1             MARK
       BL   @CVDB20           DIGIT?
       JMP  CVDB32            N, LOOK FOR PERIOD
       B    *R3               Y, NUMBER
*
CVDB32 CI   R0,>2E00          ".?
       JNE  CVDB27            N, NO NUMBER
       BL   @CVDB20           Y, LOOK FOR DIGIT
       JMP  CVDB27            NO NUMBER
       B    *R3               NUMBER OK
*
       END
