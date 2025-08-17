       TITL 'BINARY/DECIMAL CONVERSION - CORTEX BASIC REV. 1.1'
       IDT  'CVBD'
*
*
*
       DEF  CVBD,CVBI,CVBFR,B30
       REF  FPAC,FPAC2
       REF  CVHD,CVHD01,CVHD15,CVHD12
       REF  FFLG
       REF  FADDI,FSUBI
       REF  CVGCN,B01,B05
       REF  CVBF
       REF  B2D,B2E,B31,B3A,B45
*
*  DEFINE FLOATING POINT XOPS FOR THIS MODULE
*
       DXOP LOADF,0           ;LOAD FPAC
       DXOP STORE,1           ;STORE FPAC
       DXOP FADD,2            ;ADD TO FPAC
       DXOP FSUB,3            ;SUBTRACT FROM FPAC
       DXOP FMUL,4            ;MULTIPLY FPAC
       DXOP FDIV,5            ;DIVIDE FPAC
       DXOP SCALE,6           ;SCALE PFAC
       DXOP NORMAL,7          ;NORMALIZE FPAC
       DXOP CLEAR,8           ;CLEAR FPAC
       DXOP NEGATE,9          ;NEGATE FPAC
       DXOP FLOATF,10         ;FLOAT FPAC
*
       DXOP EVFIX,11          ;EVALUATE AND FIX
       DXOP OUTFP,12          ;OUT FLOATING POINT #
       DXOP OUTINT,13         ;OUT INTEGER
ERROR  EQU  >2F80             ;XOP XX,14  (ERROR CALL)
ERROR2 EQU  ERROR+>20
       PAGE
ERR34  DATA ERROR2,34         ;UNNORMALIZED #
*
*OUTPUT INTEGER
*
CVBI   CLEAR 0                ;CLEAR FPAC
       MOV  *R11,@FPAC2
       JMP  CVBD0
*
*OUTPUT FLOATING POINT #
*
CVBD   LI   R10,FPAC          ;GET FPAC ADDRESS
       MOVB *R11+,*R10+
       MOVB *R11+,*R10+
       MOVB *R11+,*R10+
       MOVB *R11+,*R10+
       MOVB *R11+,*R10+
       MOVB *R11,*R10
*
CVBD0  FLOATF 0               ;FLOAT FPAC IF NECESSARY
       CLR  R12               ;CLEAR SIGN FLAG
       LI   R7,CVHD           ;GET HOLD ADR
       SB   *R7,*R7+          ;CLEAR FIRST BYTE
       MOV  @FPAC,R1          ;CHECK FOR ZERO
       JLT  CVBD1             ;NEGATIVE
       JGT  CVBD2             ;POSITIVE
       MOVB @B30,*R7+         ;ZERO, OUT "0
       LI   R10,12            ;SET DIGIT COUNT
       JMP  CVBD12
*
CVBD1  NEGATE 0               ;NEGATE FPAC
       SETO R12               ;SET FLAG
*
CVBD2  ANDI R1,>00F0          ;NORMALIZED?
       JEQ  ERR34             ;N
       CLR  R10               ;CLEAR DECIMAL ADJUST COUNTER
*
CVBD3  MOVB @FPAC,R1          GET EXPONENT
       SRL  R1,8
       LI   R8,>4A            GET EXP=4A (16^10)
       S    R1,R8
       JEQ  CVBD6             EXP=4A
       JLT  CVBD4             EXP<4A   (MUL)
*
       BL   @CVGCN            FIX
       A    R8,R10            UPDATE DECIMAL ADJUSTOR
       FMUL *R0               MULTIPLY
       JMP  CVBD3
*
CVBD4  NEG  R8                R8=-R8
       BL   @CVGCN
       S    R8,R10            UPDATE DECIMAL ADJUSTOR
       FDIV *R0               DIVIDE
       JMP  CVBD3
*
CVBD5  BL   @FADDI            WENT NEG, ADD BACK
       INC  R10
       JMP  CVBD7
*
CVBD6  STORE R0               LOAD FPAC IN R0,R1,R2
       ANDI R0,>00FF          CLEAR EXPONENT
       LI   R3,CVTB0          GET TABLE ADR
       LI   R9,>30            "0
B30    EQU  $-1
*
CVBD7  MOV  *R3+,R4           GET 1ST #
       MOV  *R3+,R5
       MOV  *R3+,R6
       BL   @FSUBI            R0,R1,R2=R0,R1,R2-R4,R5,R6
       JLT  CVBD5             WENT NEG, ADD BACK
*
CVBD8  INC  R9                COUNT
       BL   @FSUBI            ;R0,R1,R2=R0,R1,R2-R4,R5,R6
       JEQ  CVBD8
       JGT  CVBD8
       BL   @FADDI            WENT NEGATIVE, ADD BACK
       SWPB R9                READY BYTE FOR STORING
       MOVB R9,*R7+           MOVE INTO BUFFER
       LI   R9,>2F            RELOAD R9
       MOV  *R3+,R4           GET NEXT CONSTANT
       MOV  *R3+,R5
       MOV  *R3+,R6
       CI   R3,CVTB0E         DONE?
       JLE  CVBD8             N
       PAGE
*CONVERSION DONE, ROUND AND SUPPRESS TRAILING ZERO'S
*
*       R10 = F = .001
*             E = .01
*             D = .1
*             C = 1
*             B = 10
*             A = 100
*
CVBD12 SB   *R7,*R7+          OUT NULL
       CI   R7,CVHD15         DONE?
       JL   CVBD12            N
*
       MOV  @14(13),R7        GET OUTPUT BUFFER PTR
       LI   R0,10             GET ROUNDING DIGIT COUNT
       MOV  @FFLG,R9          FORMATTING?
       JNE  CVBFP             Y
       BL   @CVBFR            N, ROUND
       DEC  R10               NEW DIGIT
       CLR  R4                CLEAR TRAILING ZEROES
*
CVBD16 CB   @B30,*R5          "0?
       JNE  CVBD17            N
       MOVB R4,*R5            Y, MAKE NULL
       DEC  R5                BACKUP 1 DIGIT
       JMP  CVBD16
*
CVBFP  B    @CVBF             DO FORMATTING
*
*SET SIGN AND GET INITIAL PTRS
*
CVBD17 MOV  R12,R12           NEGATIVE?
       JEQ  $+6               N
       MOVB @B2D,*R7+         Y, OUT "-
       PAGE
*PROCESS FORMAT FREE #
*
       DECT R10               >E11?
       JLT  CVBD27            Y
       CI   R10,16            N, <E05?
       JGT  CVBD27            Y
       AI   R10,-11           N, "0.?
       JGT  CVBD22            Y
       JEQ  CVBD22            Y
*
CVBD19 MOVB *R3+,*R7+         N, MOVE NUMBER
       JEQ  CVBD20            DONE
       INC  R10               TIME FOR ".?
       JLT  CVBD19            N
       MOVB *R3,R0            Y, ". NEEDED?
       JEQ  CVBD25            N, DONE
       MOVB @B2E,*R7+         Y, OUT ".
       JMP  CVBD24
*
CVBD20 DEC  R7
*
       MOVB @B30,*R7+         OUT "0 UNTIL R10=0
       INC  R10               DONE?
       JLT  $-6               N
       JMP  CVBD25            Y
*
CVBD22 MOVB @B30,*R7+         OUT "0
       MOVB @B2E,*R7+         OUT ".
*
CVBD23 DEC  R10               TIME FOR DIGIT?
       JLT  CVBD24            Y
       MOVB @B30,*R7+         N, OUT "0
       JMP  CVBD23
*
CVBD24 MOVB *R3+,*R7+         MOVE REST OF STRING
       JNE  $-2
       DEC  R7                BACKUP OVER NULL
*
CVBD25 MOV  R7,@14(13)        RETURN UPDATE PTR
       RTWP DONE
*
CVBD26 MOV  R6,R7             FORMATTING OVERFLOW
       JMP  CVBD25
       PAGE
*EXPONENTIAL FORM
*
CVBD27 MOVB *R3+,*R7+         OUT FIRST DIGIT
       MOVB *R3,R4            ANOTHER DIGIT?
       JEQ  $+6               N, NO ".
       MOVB @B2E,*R7+         Y, ".
*
       MOVB *R3+,*R7+         MOVE #
       JNE  $-2
       DEC  R7
       MOVB @B45,*R7+         "E
       AI   R10,-10
       NEG  R10
       JGT  CVBD28            POSITIVE
       MOVB @B2D,*R7+         -, OUT "-
       NEG  R10
*
CVBD28 CLR  R9                ;CLEAR UPPER PART
       DIV  @C000A,R9         ;R9,R10/10
       AI   R9,>30            ;ADD BITS
       AI   R10,>30
       SWPB R9                ;POSITION
       SWPB R10
       MOVB R9,*R7+           ;MOVE INTO STREAM
       MOVB R10,*R7+
       JMP  CVBD25            ;RETURN
       PAGE
*
*ROUND CHARACTER STRING TO R0 TH POSITION
*
CVBFR  LI   R6,CVHD01         GET STRING ADR
       MOVB @B30,@CVHD12      FORCE 12TH DIGIT TO '0'
       A    R0,R6             INDEX TO ROUNDING POSITION
       MOVB R0,R0             ABLE TO ROUND?
       JNE  CVBFR3            N
       MOV  R6,R5             Y, MARK
       INC  R6
*
       MOVB *R6,R3            GET ROUNDING DIGIT
       JEQ  CVBFR3            DONE
       MOVB R0,*R6            SET TO NULL
       AB   @B05,R3           ADD 5
       CB   @B3A,R3           CARRY?
       JH   CVBFR3            N, DONE
*
CVBFR1 DEC  R6                Y, HANDLE CARRY
       MOVB *R6,R0            EOS?
       JEQ  CVBFR2            Y, INSERT NEW DIGIT
       AB   @B01,*R6          N, ADD CARRY
       CB   @B3A,*R6          CARRY?
       JH   CVBFR3            N, DONE
       MOVB @B30,*R6          Y, INSERT "0
       JMP  CVBFR1            CONTINUE
*
CVBFR2 X    *R11              NEW DIGIT, ADJUST R10
       MOVB @B31,*R6          INSERT "1
*
CVBFR3 LI   R3,CVHD           GET INITIAL PTR
       MOVB *R3+,R6           NULL?
       JEQ  CVBFR4            Y
       DEC  R3                N, MOVE BACK
CVBFR4 B    @2(11)            RETURN
*
*
CVTB0  DATA >00E8,>D4A5,>1000
       DATA >0017,>4876,>E800
       DATA >0002,>540B,>E400
       DATA >0000,>3B9A,>CA00
       DATA >0000,>05F5,>E100
       DATA >0000,>0098,>9680
       DATA >0000,>000F,>4240
       DATA >0000,>0001,>86A0
       DATA >0000,>0000,>2710
       DATA >0000,>0000,>03E8
       DATA >0000,>0000,>0064
C000A  EQU  $+4
       DATA >0000,>0000,>000A
       DATA >0000,>0000,>0001
CVTB0E EQU  $
       END
