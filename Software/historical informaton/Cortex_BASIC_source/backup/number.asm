       TITL 'NUMBER - CORTEX BASIC REV 1.1'
       IDT  'NUMBER'
*
       DEF  NUMY,D100,D10,C000A
*
       REF  MODEOK,AINC,CRLF,LNUM
*
ERROR  EQU  >2F80             USER ERROR
ERROR2 EQU  ERROR+>20
       DXOP EVFIX,11
*
*  SET AUTO-INCREMENT
*
*
NUMY   BL   @MODEOK           ;ABORT IF RUNNING !
       MOVB *R8,R0            ;PARAMETERS
       JEQ  NUM2              ;N
       EVFIX @LNUM            ;Y, GET START
       CI   R0,>3F00          ;,?
       JNE  NUM3              ;N
       EVFIX R1               ;GET AUTO INCREMENT
NUM1   MOV  R1,@AINC          ;SAVE AINC
       S    R1,@LNUM          ;SUBTRACT INC FROM START VALUE
       B    @CRLF             ;RETURN
NUM2   LI   R1,100
D100   EQU  $-2
       MOV  R1,@LNUM          ;SET DEFAULT START #
NUM3   LI   R1,10
D10    EQU  $-2
C000A  EQU  D10
       JMP  NUM1
       END
