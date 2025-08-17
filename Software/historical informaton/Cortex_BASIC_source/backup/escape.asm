       TITL 'ESCAPE/NOESC STATEMENTS - CORTEX BASIC REV. 1.1'
       IDT  'ESCAPE'
*
*
*
       DEF  ESCY
       DEF  NOEY
       REF  ESCFLG
       REF  NLIN0
*
*ESCAPE ENABLE COMMAND
*
ESCY   CLR  @ESCFLG           ;RESET ESCAPE FLAG
       JMP  ESCR              ;RETURN
*
*
*ESCAPE DISABLE COMMAND
*
NOEY   SETO @ESCFLG           ;SET ESCAPE FLAG
ESCR   B    @NLIN0
       END
