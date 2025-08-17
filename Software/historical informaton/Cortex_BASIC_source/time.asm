       TITL 'TIME STATEMENT - CORTEX BASIC REV. 1.1'
       IDT  'TIME'
*
* THIS MODULE CONTAINS THE ROUTINE THAT SETS UP THE
* TIME STORAGE AREA FOR BASIC AND ALSO RETURNS TO BASIC THE
* CURRENT TIME
*
*      TIME                   - OUTPUT THE TIME
*      TIME <EXP>,<EXP>,<EXP> - SET THE TIME
*      TIME <VAR>             - GET REAL TIME CLOCK IN <VAR>
*
* MODULE DEFINITIONS:
*
       DEF  TIMY
*
* EXTERNAL ROUTINES:
*
       REF  CKEX,EVSDZ,TYPB$,NLIN
*
* EXTERNAL DATA:
*
       REF  B20,CLKADR,FPWP,IOB
       REF  TIMC,BFA,B3A
*
* XOP EQUATES
*
       DXOP EVFIX,11
       PAGE
*
* TIME STATEMENT
*
TIMY   LI   R3,CLKADR         ;GET CLOCK'S WP ADDRESS
       LI   R12,>1EE2         ;SELECT 9995'S FLAG1
*
* DETEMINE TYPE
*
       BLWP @EVSDZ            ;LOOK FOR STRING
       JMP  TIM5              ;" OR ' - PROBLEM
       JMP  TIM3              ;$
       BL   @CKEX             ;LOOK FOR EXPRESSION
       JMP  TIM2              ;NONE
*
* SET REAL TIME CLOCK WITH A 40MS INTERVAL
*
TIM0   LI   R4,3              ;ONLY 3 <EXP>S ALLOWED
       LIMI 0                 ;NO INTERRUPTS
TIM0A  EVFIX *R3+             ;STORE RESULT
       CI   R0,>3F00          ;,?
       JNE  TIMR              ;N - SET 9901 INTERVAL
       DEC  R4                ;Y - ANY MORE <EXP>S ALLOWED?
       JNE  TIM0A             ;Y - GET NEXT
       JMP  TIMR              ;N, EXIT
*
TIM2   INC  R8                ;INCREMENT OVER DLIM
       MOV  @IOB,R2            ;GET IOB ADDRESS
       MOVB @B20,*R2+         ;OUT SPACE
       SETO R6                ;SET TO PRINT
       JMP  TIM4
*
TIM3   CLR  R6                ;STORE IN VARIABLE
*
TIM4   BLWP @TIMVEC           ;GET TIME
       BL   @TIM6             ;DO HOURS
       MOV  R4,R0
       BL   @TIM6             ;DO MINUTES
       MOV  R5,R0
       BL   @TIM6             ;DO SECONDS
       DEC  R2                ;END STRING
       SB   *R2,*R2
       MOV  R6,R6             ;TYPE?
       JEQ  TIMR              ;N - RETURN
*
TIM5   DATA TYPB$             ;Y - OUTPUT BUFFER
TIMR   LIMI 15
       B    @NLIN
*
* FIX R0 TO XX:
*
TIM6   CLR  R1                ;CLEAR COUNTER
*
TIM7   AI   R0,-10            ;NEGATIVE
       JLT  TIM8              ;Y
       INC  R1                ;N, COUNT
       JMP  TIM7
*
TIM8   SWPB R1
       MOVB R1,R0             ;APPEND
       AI   R0,>2F3A          ;ADD ASCII CODES
       MOVB R0,*R2+           ;MOVE INTO OUTPUT STREAM
       SWPB R0
       MOVB R0,*R2+
       MOVB @B3A,*R2+         ;OUT ":
       RT
*
TIMVEC DATA FPWP,TIM9
*
TIM9   LIMI 0                 ;MASK INTRPTS
       LI   R3,CLKADR         ;GET SOURCE
       MOV  *R3+,*R13         ;SET HOURS
       MOV  *R3+,@8(R13)      ;SET MIN
       MOV  *R3,@10(13)       ;SET SECONDS
       RTWP                   ;RETURN
       END
