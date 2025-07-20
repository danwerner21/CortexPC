       TITL 'RANDOM STATEMENT - CORTEX BASIC REV. 1.1'
       IDT  'RANDOM'
*
*
*
       REF  NLIN              ;EXIT TO MULTIPLEXOR
       DXOP EVFIX,11          ;EVALUATE AND FIX
       REF  RANDS             ;RANDOM NUMBER SEED
*
       DEF  RANY
* ABSTRACT:
*
*       THE RANDOM COMMAND WILL SET THE RANDOM
*       SEED.
*
* CALLING SEQUENCE:
*
*       B @RANY
*
*       EXIT TO NLIN
*
* EXCEPTIONS AND CONDITIONS:  (NONE)
*
*
RANY   EVFIX @RANDS           ;GET RANDOM SEED
       B    @NLIN
       END
