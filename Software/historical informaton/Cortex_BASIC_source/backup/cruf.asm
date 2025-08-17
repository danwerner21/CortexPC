       TITL 'CRU FUNCTIONS - CORTEX BASIC REV. 1.1'
       IDT 'CRUF'
*
*       BASY            ;BASE COMMAND
*       CRBY            ;CRB COMMAND
*       CRBF            ;CRB FUNCTION
*       CRFY            ;CRF COMMAND
*       CRFF            ;CRF FUNCTION
*
* THIS MODULE ALSO CONTAINS THE SYSTEM CRASH ROUTINE.
*
        REF FIX         ;INTEGER FIX
        REF EVSFR$      ;EXIT TO EVALUATOR WITH R2 RELOAD.
        REF NLIN        ;ENTRY EXIT TO MULTIPLEXOR
        REF GPRM        ;GET PARAMETER
        REF FPAC2       ;FLOATING POINT ACCUMULATOR
        REF BCRU        ;BASE CRU INDEX
*
        DEF CRFY         ;CRF STATEMENT ENTRY POINT
        DEF CRBY         ;CRB STATEMENT ENTRY POINT
        DEF CRFF         ;CRF FUNCTION ENTRY POINT
        DEF CRBF         ;CRB FUNCTION ENTRY POINT
        DEF  BASY         ;BASE STATEMENT ENTRY POINT
*
       DEF  CRASH$   <<< SYSTEM CRASH ROUTINE >>>
*
        DXOP OUTINT,13          ;OUT INTEGER
        DXOP EVFIX,11           ;EVALUATE AND FIX
*
************************************************************
*                                                          *
*               SET USER CRU BASE.                         *
*                                                          *
************************************************************
BASY   EVFIX @BCRU            ;SAVE IN BCRU
       JMP  CRUXIT            ;EXIT TO NLIN
ERROR  EQU >2F80              ;XOP XX,14  (ERROR CALL)
       PAGE
************************************************************
*                                                          *
*               MULTIPLE BIT CRU WRITE.                    *
*                                                          *
************************************************************
*
* CALLING SEQUENCE:
*
*       B @CRFY
*
*       EXIT TO NLIN
*
CRFY    BL   @GPRM            GET PARAMETERS
        ANDI R1,>F            MASK COUNT
        JEQ  CRF1             0=16 BITS
        CI   R1,8             BYTE?
        JGT  CRF1             N
        SWPB R3               Y
*
CRF1    SLA  R1,6             POSITION
        AI   R1,>3003         MAKE 'LDCR R3,X'
        JMP  CRBY1
************************************************************
*                                                          *
*                 SINGLE BIT CRU WRITE.                    *
*                                                          *
************************************************************
*
* CALLING SEQUENCE:
*
*       B @CRBY
*
*       EXIT TO NLIN
*
*
CRBY    BL   @GPRM            GET DISPLACEMENT
        ANDI R1,>FF           MASK
        AI   R1,>1D00         GET SBO
        MOV  R3,R3            SET TO 1?
        JNE  CRBY1            N
        AI   R1,>0100         Y, CHANGE TO SBZ
*
CRBY1   MOV  @BCRU,R12
D$XR1   X    R1               DO CRU INSTRUCTION
CRUXIT  B    @NLIN            RETURN
        PAGE
************************************************************
*                                                          *
*               MULTIPLE BIT CRU READ                      *
*                                                          *
************************************************************
*
* CALLING SEQUENCE:
*
*       B @CRFF
*
*       EXIT TO EVSFR$
*
CRFF   BL   @FIX              GET PARAMETER
       ANDI R1,>F             MASK
       MOV  R1,R0             SAVE
       SLA  R1,6              POSITION
       AI   R1,>3401          MAKE 'STCR R1,X'
       MOV  @BCRU,R12         GET BASE
       X    R1                DO STCR
       MOV  R0,R0             16?
       JEQ  CRFF1             Y
       CI   R0,8              N, BYTE?
       JGT  CRFF1             N
       SRL  R1,8              Y, POSITION RESULTS
*
CRFF1  MOV  R1,@FPAC2
*  RELOAD R2 WITH FPAC & RETURN TO EVSFR
CRFF2  B    @EVSFR$           RETURN ADR
************************************************************
*                                                          *
*                 SINGLE BIT CRU READ                      *
*                                                          *
************************************************************
*
* CALLING SEQUENCE:
*
*       B @CRBF
*
*       EXIT TO EVSFR$
*
*
CRBF   BL   @FIX              GET DISPLACEMENT
       ANDI R1,>FF            MASK
       AI   R1,>1F00          MAKE 'TB XX'
       MOV  @BCRU,R12         GET BASE
EXECR1 X    R1                DO TEST
       JNE  CRFF2             0, RT
       INC  @FPAC2            1
       JMP  CRFF2
       PAGE
************************************************************
*                                                          *
*                SYSTEM CRASH ROUTINE                      *
*      THIS ROUTINE IS ENTERED WITH R1=0 AND IS MEANT TO   *
*      LOCKUP THE SYSTEM SUCH THAT THE ONLY WAY OUT IS     *
*      VIA RESET.                                          *
*      ENTRY POINT IS 'CRASH$'                             *
*                                                          *
************************************************************
CRASH$ XOR  @D$XR1,R1         BUILD 'X  R1' IN R1
       B    R1                EXECUTE IT
       END
