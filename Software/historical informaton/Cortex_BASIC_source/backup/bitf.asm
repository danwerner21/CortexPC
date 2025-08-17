       TITL 'BIT ROUTINES - CORTEX BASIC REV. 1.1'
       IDT  'BITF'
*
*
* ROUTINE LIST:
*
*       BITF                   BIT FUNCTION
*       BITY                   BIT COMMAND
*
       REF  EVSFR$             RELOAD R2 & EXIT TO EVALUATOR
       REF  NLIN               EXIT TO MULTIPLEXOR
       REF  EVERZ              EVALUATE EXPRESSION
       REF  GPRM1              GET-PARAMETER ENTRY
       REF  FPAC2              
       REF  B4A
       DXOP STORE,1            STORE FPAC
       DXOP EVFIX,11           EVALUATE AND FIX
       DXOP OUTFP,12           OUT FLOATING POINT #
ERROR  EQU  >2F80              XOP XX,14  (ERROR CALL)
       DEF  BITY               ENTRY POINT FOR BIT STATEMENT
       DEF  BITF               ENTRY POINT FOR BIT FUNCTION
       DEF  C8000
       PAGE
*       THE BIT STATEMENT ALLOWS A BASIC PROGRAM
*       TO ALTER ANY BIT WITHIN A BASIC
*       VARIABLE. THE FORM IS:
*
*       BIT[ <VAR> , <EXP1> ] = <EXP2>
*
*       WHERE:  <VAR>  = VARIABLE TO BE ALTERED
*               <EXP1> = BIT POSITION WITHIN VARIABLE
*               <EXP2> = 0 OR <>0 TO RESET OR SET
* BIT RANGES FROM 0 UPWARDS (IF -VE ASSUMES BIT 0)
*
* CALLING SEQUENCE:
*
*       B @BITY
*
*       EXIT TO NLIN
*
* EXCEPTIONS AND CONDITIONS:
*
*       EVALUATION ERRORS
*       ERROR 1 IF BRACKETS MISSING OR NO ','
*
BITY   CB   *R8+,@B4A         ;RIGHT BRACKET?
       JNE  ERR1              ;N, ERROR
       BLWP @EVERZ            ;GET ADRRESS OF VARIABLE
       CI   R0,>3F00          ;',' FOLLOWING ?
       JNE  ERR1              ;NO, ERROR IT
       BL   @GPRM1            ;GET PARAMETERS
       MOV  R1,R1             ;TEST SIGN OF BIT #
       JGT  BITY0             ;+VE, LEAVE ALONE
       CLR  R1                ;-VE, MAKE 0
BITY0  MOV  R1,R0             ;GET WORD INDEX
       SRL  R1,3
       A    R1,R2             ;INDEX
       LI   R1,>8000          ;SET MASK
C8000  EQU  $-2
       SRC  R1,0              ;POSITION MASK
       SZC  R1,*R2            ;SET BIT TO ZERO
       MOV  R3,R3             ;NEED 1?
       JEQ  BITY1             ;N
       SOC  R1,*R2            ;Y, SET BIT TO ONE
BITY1  B    @NLIN             ;RETURN
*
ERR1   DATA ERROR+1
       PAGE
*       THE BIT FUNCTION WILL DISPLAY ANY BIT VALUE OFFSET
*       FROM A BASIC VARIABLE. THE FORM IS:-
*
*       BIT[ <EXP1> , <EXP> ]
*
*       WHERE   <EXP1> = BASIC VARIABLE
*               <EXP2> = BIT POSITION
*
* CALLING SEQUENCE:
*
*       B @BITF
*
*       EXIT TO EVSFR$
*
BITF   MOV  R1,R1             ;TEST SIGN OF BIT #
       JGT  BITF0             ;+VE, OK
       CLR  R1                ;-VE, MAKE 0
BITF0  MOV  R1,R0
       SRL  R1,3              ;GET WORD INDEX
       A    R1,R2             ;INDEX
       LI   R1,>8000          ;GET INITIAL MASK
       SRC  R1,0              ;SHIFT
       MOV  *R2,R2            ;GET WORD
       COC  R1,R2             ;BIT SET?
       JNE  BITF1             ;N, RETURN 0
       INC  @FPAC2            ;Y, RETURN 1
*
BITF1  B    @EVSFR$           ;RETURN ADR
       END
