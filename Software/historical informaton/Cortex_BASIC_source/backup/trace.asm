       TITL 'TRACE ROUTINES - CORTEX BASIC REV. 1.1'
       IDT  'TRACE'
*
*
*
       REF  TRAFLG,NLIN0
       DEF  TONY,TOFY
*
*  TRACE ON (TON)
TONY   SETO @TRAFLG           ;SET TRACE FLAG
TONY1  B    @NLIN0            ;EXIT TO NLIN
*
*  TRACE OFF (TOF)
*
TOFY   CLR  @TRAFLG           ;RESET TRACE FLAG
       JMP  TONY1             ;EXIT
       END
