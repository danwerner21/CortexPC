       TITL 'PLOT/UNPLOT ROUTINES - CORTEX BASIC REV. 1.1'
       IDT  'PLOT'
*
*
*
ERROR  EQU  >2F80
ERROR2 EQU  ERROR+>20
       DXOP EVFIX,11
*
       DEF  PLOTY,UPLOTY,PLOT,UNPLOT
       REF  VDPWP1,PIXON,PIXOFF,V1R5L,V1R6L
       REF  V1R3L,V1R4L
       REF  XLOC,YLOC,NXLOC,NYLOC,NLIN,JMPR0A
       REF  FGM$
       PAGE
*
*         PLOT AND UNPLOT POINT/LINE ROUTINES
*
*  FORMAT:
*    PLOT  A,B             PLOT POINT (A,B)
*    PLOT  TO A,B          PLOT LINE FROM CURSOR TO (A,B)
*    PLOT  A,B TO C,D      PLOT LINE FROM (A,B) TO (C,D)
*    UNPLOT A,B            UNPLOT POINT (A,B)
*    UNPLOT TO A,B         UNPLOT LINE FROM CURSOR TO (A,B)
*    UNPLOT A,B TO C,D     UNPLOT LINE FROM (A,B) TO (C,D)
*
*   FOLLOWING ONE OF THESE STATEMENTS THE CURSOR IS
*   POSITIONED AT THE LAST CO-ORDINATE PAIR SPECIFIED.
*
*   AFTER THE LAST CO-ORDINATE PAIR A 'TO' MAY BE USED
*   TO EXTEND THE LINE (UN)PLOTTING WITHIN THE ONE
*   STATEMENT.  EG:-
*
*   PLOT TO A,B TO C,D TO E,F     THIS WILL PLOT THE 3 LINES
*                                 1.  FROM CURSOR TO (A,B)
*                                 2.  FROM (A,B) TO (C,D)
*                                 3.  FROM (C,D) TO (E,F)
*
*
       PAGE
UPLOTY SETO R13               FLAG AS 'UNPLOT'
       JMP  PLOTE
PLOTY  CLR  R13               FLAG AS 'PLOT'
*
PLOTE  DATA FGM$              FORCE GRAPH MODE
       BL   @JMPR0A           Y, GET BYTE & CHECK IT
EOLTST BYTE PLTO-EOLTST/2,>38 'TO' - PLOT LINE
       BYTE PLTE-EOLTST/2,>00 NULL - EXIT
       BYTE PLTE-EOLTST/2,>3C ':'  - EXIT
       BYTE PLTE-EOLTST/2,>47 '!'  - EXIT
       DATA 0
*
*     NO LEADING 'TO' - MUST BE AN XY PAIR
*
       DEC  R8                BACKUP CODE PTR
       BL   @GETXY            GET X,Y PAIR
       MOVB R15,@XLOC         SET CURSOR X
       MOVB R14,@YLOC         SET CURSOR Y
       CI   R0,>3800          'TO' ?
       JEQ  PLTO              Y, LINE
       MOV  R13,R13           N, PIXEL SET ?
       JEQ  SETIT             Y,
       BLWP @PIXOFF           N, TURN PIXEL OFF
       JMP  PLTE              AND QUIT
*
SETIT  BLWP @PIXON            TURN PIXEL ON
PLTE   B    @NLIN             EXIT TO NLIN
*
*
PLTO   BL   @GETXY            'TO' FOUND - GET X,Y PAIR
       MOVB R15,@NXLOC        SET STOP X
       MOVB R14,@NYLOC        SET STOP Y
       MOV  R13,R13           PLOT ?
       JNE  UNPLT             N, DO UNPLOT LINE
       BLWP @PLOT             Y, CALL PLOT LINE
*
*      CHECK FOR FOLLOWING 'TO'
*
LOOKTO CI   R0,>3800          'TO' FOLLOWING ?
       JEQ  PLTO              Y, CONTINUE
       JMP  PLTE              N, EXIT
*
UNPLT  BLWP @UNPLOT           CALL UNPLOT LINE
       JMP  LOOKTO            CHECK FOR 'TO'
*
*      GET A FOLLOWING  X,Y PAIR AND STORE THEM IN THE
*      MS BYTE OF R15 & R14 RESPECTIVLY.
*      RANGE CHECKING IS DONE IN PIXON/PIXOFF ROUTINES
*
GETXY  EVFIX R15              GET X
       SWPB R15               POSITION IT
       CI   R0,>3F00          ',' ?
       JNE  ERR37             N, ERROR IT
       EVFIX R14              Y, GET Y
       SWPB R14               POSITION IT
       RT
*
ERR37  DATA ERROR2,37         INVALID DELIMITER
ERR48  DATA ERROR2,48         NOT ALLOWED IN CURRENT MODE
       PAGE
*   WORKSPACE       :    VDPWP1
*   ROUTINE(S)      :    PLOT,UNPLOT
*   REGISTER USAGE  :
*                                              GLOBAL DATA
*           ___________________________________   LABELS
*   R0      !  PLOT/UNPLOT POINTER            !
*   R1      !                                 !
*   R2      !                                 !
*   R3      !                                 ! LSB=V1R3L
*   R4      !                                 ! LSB=V1R4L
*   R5      !                                 ! LSB=V1R5L
*   R6      !                                 ! LSB=V1R6L
*   R7      !                                 !
*   R8      !                                 !
*   R9      !                                 !
*   R10     !                                 !
*   R11     !        BL RETURN ADDRESS        !
*   R12     !                                 !
*   R13     !          RETURN WP              !
*   R14     !          RETURN PC              !
*   R15     !          RETURN ST              !
*           -----------------------------------
*
UNPLOT DATA VDPWP1,$+2
       LI   R0,PIXOFF
       JMP  PLOT1
PLOT   DATA VDPWP1,$+2
       LI   R0,PIXON
PLOT1  SETO R11
       SETO R10
       SETO R9
       CLR  R3
       CLR  R4
       MOVB @XLOC,@V1R4L
       MOVB @NXLOC,@V1R3L
       S    R3,R4
       ABS  R4
       JLT  INITY
       NEG  R10
INITY  MOVB @YLOC,@V1R3L
       CLR  R5
       MOVB @NYLOC,@V1R5L
       S    R5,R3
       ABS  R3
       JLT  XYORYX
       NEG  R9
XYORYX C    R4,R3
       JGT  LEEVEM
       XOR  R3,R4
       XOR  R4,R3
       XOR  R3,R4
       MOVB @NYLOC,R5
       MOVB @NXLOC,R6
       JMP  SLOPE
LEEVEM MOVB @NXLOC,R5
       MOVB @NYLOC,R6
       XOR  R9,R10
       XOR  R10,R9
       XOR  R9,R10
       INCT R11
SLOPE  SRL  R5,8
       SRL  R6,8
       SLA  R3,5
       CLR  R2
       DIV  R4,R2
       MOV  R2,R7
       MOV  R3,R2
       CLR  R3
       JNO  DIVLSB
       JMP  SETREM
DIVLSB DIV  R4,R2
       MOV  R2,R3
       MOV  R7,R2
SETREM LI   R7,>10
       CLR  R8
       CLR  R12
PLOTLP C    R12,R4
       JH   EXIT
       INC  R12
       MOV  R11,R11
       JLT  XLESS
       MOVB @V1R5L,@XLOC
       MOVB @V1R6L,@YLOC
       JMP  SCNDRW
XLESS  MOVB @V1R6L,@XLOC
       MOVB @V1R5L,@YLOC
SCNDRW BLWP *R0
       A    R3,R8
       JNC  NOCARY
       INC  R7
NOCARY A    R2,R7
       CI   R7,>20
       JL   EXPLOT
       AI   R7,->20
       A    R10,R6
EXPLOT A    R9,R5
       JMP  PLOTLP
EXIT   MOVB @NXLOC,@XLOC
       MOVB @NYLOC,@YLOC
       RTWP
       END
