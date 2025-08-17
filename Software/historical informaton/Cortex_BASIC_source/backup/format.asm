       TITL 'FORMAT ROUTINE - CORTEX BASIC REV 1.1'
       IDT  'FORMAT'
*
*
*
       DEF  CVBF
       REF  CVBFR
       REF  B2A,B30
*
*PRINT FORMATTING
*
CVBF   MOV  R9,R8             MARK FORMAT
       MOV  R7,R6             MARK CHARACTER BUFFER
       CLR  R3                CLEAR DECIMAL FLAG
       CLR  R4                CLEAR BEFORE DECIMAL COUNT
       CLR  R5                CLEAR TOTAL DIGIT HOLDER COUNT
*
CVBF1  BL   @CVBFT            GET TYPE
       JMP  CVBF4             NULL, DONE
       JMP  CVBF2             DIGIT HOLDER
       SETO R3                DECIMAL
       JMP  CVBF3             CHARACTER
*
CVBF2  MOV  R3,R3             BEFORE DECIMAL?
       JNE  $+4               N
       INC  R4                Y, COUNT
       INC  R5
*
CVBF3  MOVB @B2A,*R6+         OUT "*
       JMP  CVBF1
*
CVBF4  AI   R10,-13           GET # OF LEADING DIGITS
       NEG  R10
       C    R10,R4            GREATER THAN DIGIT HOLDERS?
       JGT  CVBFR1            Y, ERROR RETURN
       S    R4,R5             GET NUMBER OF DIGITS PAST DECI
       A    R10,R5            INDEX TO ROUNDING DIGIT
       CI   R5,12             TOO LARGE?
       JLT  CVBF5             N
       LI   R5,12             Y, SET TO 12
*
CVBF5  MOV  R5,R0             SET TO ROUND
       DEC  R0
       BL   @CVBFR            ROUND
       INC  R10               ADD NEW DIGIT
       MOV  R9,R8             RESET FOR CVBFT
       CLR  R5                CLR # FLAG
*
CVBF6  BL   @CVBFT            GET FORMAT TYPE
CVBD7A JMP  CVBFR2            DONE
       JMP  CVBF8             DIGIT HOLDER
       JMP  CVBF15            ".
       CI   R0,>2C00          ",?
       JNE  CVBF7             N
       MOV  R5,R5             Y, DIGIT OUT?
       JEQ  CVBF11            N, OUT SPACE
CVBF7  MOVB R0,*R7+           Y, OUT CHARACTER
       JMP  CVBF6
       PAGE
CVBFR1 MOV  R6,R7             FORMATTING OVERFLOW
CVBFR2 MOV  R7,@14(13)        RETURN UPDATE PTR
       RTWP                   DONE
*
CVBF8  DEC  R4                DIGIT HOLDER
       C    R4,R10            TIME FOR DIGIT?
       JLT  CVBF13            Y
       JEQ  CVBF8A            N, CHECK FLOATER
       MOV  R4,R4             FLOATER?
       JNE  CVBF10            N
CVBF8A CI   R0,>2400          N, MAYBE FLOATER THOUGH, "$?
       JEQ  CVBF7             Y, INSERT
       CI   R0,>5300          "S?
       JNE  CVBF9             N
       MOV  R12,R12           Y, POSITIVE?
       JEQ  CVBF11            Y, INSERT BLANK
       LI   R0,>2D00          N, INSERT "-
       JMP  CVBF7
*
CVBF9  CI   R0,>3C00          "<?
       JNE  CVBF10            N
       MOV  R12,R12           Y, POSITIVE?
       JNE  CVBF7             N, INSERT "<
       JMP  CVBF11            Y, INSERT BLANK
*
CVBF10 CI   R0,>3000          "0?
       JEQ  CVBF12            Y
CVBF11 LI   R0,>2000          N, INSERT BLANK
CVBF12 MOVB R0,*R7+           INSERT
       JMP  CVBF6
*
CVBF13 BL   @CVBF30
       JMP  CVBF6
*
CVBF15 MOVB R0,*R7+           INSERT ".
*
CVBF16 BL   @CVBFT            GET BYTE
       JMP  CVBD7A            DONE
       JMP  CVBF20            DIGIT HOLDER
       JMP  $+2
       CI   R0,>3E00          ">?
       JNE  CVBF18            N
       MOV  R12,R12           Y, POSITIVE?
       JNE  CVBF18            N
       LI   R0,>2000          Y, OUT SPACE
CVBF18 MOVB R0,*R7+           OUT
       JMP  CVBF16
*
CVBF20 DEC  R4                COUNT
       C    R4,R10            TIME FOR DIGIT?
       JLT  CVBF21            Y
       LI   R0,>3000          N, OUT "0
       JMP  CVBF18
*
CVBF21 BL   @CVBF30           OUT DIGIT
       JMP  CVBF16
*
CVBF30 DEC  R10               COUNT DOWN
       SETO R5                ALLOW COMMA'S
       MOVB *R3+,*R7+         INSERT DIGIT
       JNE  CVBF31
       DEC  R7                NO DIGIT
       DEC  R3
       MOVB @B30,*R7+         INSERT "0
CVBF31 RT
       PAGE
*CHECK FORMAT TYPE
*      BL @CVBFT
*         NULL
*         DIGIT HOLDER  < $ S 0 9
*         DECIMAL
*       CHAR ,
*
CVBFT  CLR  R0                GET BYTE
       MOVB *R8+,R0
       JEQ  CVBFT3            NULL
       CI   R0,>3C00          "<?
       JEQ  CVBFT2            Y
       CI   R0,>2400          "$?
       JEQ  CVBFT2            Y
       CI   R0,>5300          "S?
       JEQ  CVBFT2            Y
       CI   R0,>3000          "0?
       JEQ  CVBFT2            Y
       CI   R0,>3900          "9?
       JEQ  CVBFT2            Y
B2E    EQU  $+2
       CI   R0,>2E00          ".?
       JEQ  CVBFT1            Y
       CI   R0,>4500          "E?
       JNE  CVBFT4            N
       LI   R0,>2000          Y, DEFAULT TO SPACE
       MOV  R12,R12           NEGATIVE?
       JEQ  $+6               N
       LI   R0,>2D00          Y, OUT "-
*
CVBFT4 CI   R0,>5E00          "^?
       JNE  CVBFT0            N
       LI   R0,>2E00          Y, REPLACE WITH ".
*
CVBFT0 INCT R11
CVBFT1 INCT R11
CVBFT2 INCT R11
CVBFT3 RT
       END
