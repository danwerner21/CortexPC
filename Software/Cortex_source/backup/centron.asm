       TITL 'CENTRONICS PRINTER DRIVER - CORTEX BASIC REV. 1.1'
       IDT  'CENTRON'
       COPY 'IODEFS.INC'  
*
       DEF  CENTRO
* 
       REF  WP9928
*
*         R9   = OUTPUT BUFFER START
*         R10  = OUTPUT BUFFER END
*         R11  = POINTER TO LOCAL STORAGE
*         R13-15 RETURN CONTEXT
*
CENTRO DATA WP9928,$+2
       LI   R12,2*PPRINT      POINT TO THE PARALLEL PRINTER
*
DONE   C    R9,R10            DONE?
       JH   EXIT
*
OUTCHR TB   PPBUSY-PPRINT     BUSY?
       JEQ  OUTCHR            Y, LOOP
*
       LDCR *R9+,8            OUTPUT DATA 
       SBO  PPDS-PPRINT       DATA STROBE =1
       SBZ  PPDS-PPRINT       DATA STROBE =0
       JMP  DONE
*
EXIT   RTWP
       END
      