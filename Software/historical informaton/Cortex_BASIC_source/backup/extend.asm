       TITL 'EXTENDED COMMAND HANDLER - CORTEX BASIC REV. 1.1'
       IDT  'EXTEND'
*
*  DEFINE FLOATING POINT XOPS
*
       DXOP LOADF,0           LOAD FPAC
       DXOP STORE,1           STORE FPAC
       DXOP FADD,2            ADD TO FPAC
       DXOP FSUB,3            SUBTRACT FROM FPAC
       DXOP FMUL,4            MULTIPLY FPAC
       DXOP FDIV,5            DIVIDE FPAC
       DXOP SCALE,6           SCALE PFAC
       DXOP NORMAL,7          NORMALIZE FPAC
       DXOP CLEAR,8           CLEAR FPAC
       DXOP NEGATE,9          NEGATE FPAC
       DXOP FLOATF,10         FLOAT FPAC
       DXOP EVFIX,11          EVALUATE AND FIX
       DXOP OUTFP,12          OUT FLOATING POINT #
       DXOP OUTINT,13         OUT INTEGER
ERROR  EQU  >2F80             XOP XX,14  (ERROR CALL)
ERROR2 EQU  ERROR+>20
*
       COPY 'IODEFS.INC'
       DEF  XFRPM,EXTNDY
*
       REF  WPR2,EXTWP,C10,EXT$ID,LINE,CONFIG
*
*   TRANSFER PHYSICAL MEMORY
*
*     BLWP @XFRPM
*
*   IN:  R1   PAGE #
*        R2   PAGE INDEX (WORD ALIGNED)
*        R3   TRANSFER LENGTH
*        R4   LOAD POINT (WORD ALIGNED)
*
XFRPM  DATA WPR2,$+2
       MOV  @CONFIG,R12       GET CONFIGURATION FLAG
       SRL  R12,1             MAPPER PRESENT ?
       JNC  ERR47             N, ERROR
       CKON                   Y, ENABLE MAPPER
       LI   R12,M$REG2        POINT TO MAP REG 2
       MOV  *R12,R0           SAVE IT
       MOV  @2*R1(R13),*R12   SET NEW PAGE
       MOV  @2*R2(R13),R2     GET INDEX PAGE
       MOV  @2*R3(R13),R3     GET # BYTES TO XFER
       MOV  @2*R4(R13),R4     GET LOAD ADDRESS
       ANDI R2,>0FFF          ENSURE INDEX IS VALID
       INC  R3                FORCE TRANSFER LENGTH TO WORDS
       SRL  R3,1    
*
XF1    MOV  @>2000(R2),*R4+   COPY THE WORD
       DEC  R3                COUNT IT
       JEQ  XF2               0, DONE
       INCT R2                UPDATE INDEX
       CI   R2,>1000          OFF PAGE?
       JL   XF1               N, LOOP
       CLR  R2                Y, RESET IT
       INC  *R2               GET NEXT PAGE
       JMP  XF1               LOOP
*
XF2    CKOF                   DISABLE MAPPER
       MOV  R0,*R12           RESTORE MAP REG 2
       RTWP                   EXIT
       PAGE
*
*      EXTENDED COMMAND HANDLER
*
*
*      EXTENDE COMMAND WORKSPACE :-
*      IN   R8       PBC
*           R13-R15  RETURN CONTEXT
*
*      NORMAL EXIT :
*           RTWP
*
*      ERROR EXIT :
*           LI   R0,????        ERROR # (RIGHT JUSTIFIED)
*           INCT R14
*           RTWP
*
*      EXTENDED COMMAND HANDLER FORMAT
*
*           1000 *COMMAND_NAME parameters
*                 ^^^^^^^^^^^^
*      EPROM FORMAT
*
*           DATA >9C42        IDENTIFIER WORD
*           DATA LINK1        LINK TO NEXT DIR. ENTRY
*           DATA ENTRY1       ENTRY PT. OF COMMAND_1 ROUTINE
*           TEXT 'COMMAND_1'  NAME
*           BYTE 0            TERMINATOR FOR NAME
*   LINK1   DATA LINK2        LINK TO NEXT DIR. ENTRY
*           DATA ENTRY2       ENTRY PT. OF COMMAND_2 ROUTINE
*           TEXT 'CPMMAND_2'  NAME
*           BYTE 0            TERMINATOR FOR NAME
*   LINK2   DATA 0            LINK TERMINATOR !!
*
*   EXTENDED COMMAND HANDLERS ARE LOCATED ON 4K BYTE
*   BOUNDARIES STARTING FROM >010000 AND ARE MAPPED INTO
*   THE CPU ADDRESS SPACE FROM >2000 TO >3FFF. ALL EXTENDED
*   COMMAND HANDLER EPROMS MUST BE INSTALLED BETWEEN
*   >01 0000 AND >0F FFFF
*
EXTNDY MOV  @CONFIG,R1        GET CONFIG FLAG
       SRL  R1,1              MAPPER PRESENT ?
       JOC  EXT1              Y, GO LOOK FOR HANDLER
ERR47  DATA ERROR2,47         REQ. H/W NOT FOUND
*
EXT1   LI   R1,M$REG2         POINT TO MAPPER R2 (>2000)
       LI   R6,M$REG3         POINT TO MAPPER R3 (>3000)
       MOV  *R1,R15           SAVE ITS CONTENTS
       MOV  *R6,R14           SAVE ITS CONTENTS
       MOV  @C10,*R1          START FROM ADDRESS  010000 HEX
       MOV  R8,R10            SAVE PBC
       CKON                   MAPPER ON
*
EXT2   LI   R2,>2000          POINT TO START OF WINDOW
       C    *R2+,@EXT$ID      EXPANSION FOUND?
       JNE  EXT7              N, TRY NEXT 4K
       MOV  *R1,*R6           Y, SET UP 2ND HALF OF WINDOW
       INC  *R6
EXT3   MOV  *R2+,R3           Y, GET LINK POINTER
       JEQ  EXT7              0, END OF TABLE FOR THIS ROM
       MOV  *R2+,R4           GET ITS ENTRY POINT
EXT4   MOVB *R8+,R0           GET SEARCH STRING BYTE
       JEQ  EXT5              0, DONE SEARCH
       CB   *R2+,R0           MATCH ?
       JEQ  EXT4              Y, CONTINUE
EXT4A  MOV  R3,R2             N, PICK UP LINE
       JMP  EXT3              AND LOOP
EXT5   MOVB *R2,R0            SOURCE NULL, MATCH ?
       JNE  EXT4A             N, CONTINUE SEARCH
EXT6   LI   R3,EXTWP          MATCH, SET EXTERNAL WP
       MOV  R8,@2*8(R3)       COPY OVER R8
       BLWP R3                EXECUTE HANDLER
       JMP  EXT6A             <NORMAL RETURN>
       LI   R2,ERROR2         <ERROR  RETURN>
       MOV  *R3,R3            PICK UP ERR #
       ANDI R3,>003F          ISOLATE ERROR CODE
       JNE  EXT6B             NOT 0, OK
       SETO R3                0, MAKE ILLEGAL
EXT6B  B    R2                CALL THE ERROR HANDLER
*
EXT6A  CKOF
       MOV  R15,*R1           RESTORE MAPPER REGISTER
       MOV  R14,*R6           RESTORE MAPPER REGISTER
       B    @LINE             EXIT TO NEXT LINE
EXT7   INC  *R1               MOVE TO NEXT 4K
       MOVB @1(R1),R2         HAVE WE REACHED 10 0000
       JNE  EXT2              N, LOOK FOR NEXT EPROM
*
*  EXPANSION NOT IMPLEMENTED - ERROR
*
ERR41  CKOF                   TURN MAPPER OFF
       MOV  R15,*R1           RESTORE MAPPER REGISTER
       MOV  R14,*R6           RESTORE MAPPER REGISTER
       DATA ERROR2,41         EXPANSION EPROM NOT FOUND
       END
