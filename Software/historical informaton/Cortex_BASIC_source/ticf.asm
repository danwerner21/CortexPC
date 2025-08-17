       TITL 'TIC FUNCTION - CORTEX BASIC REV. 1.1'
       IDT  'TICF'
*
*
*
       REF  FNRM              ;FIXES ARGUMENT
*
       DXOP LOADF,0           ;LOAD FPAC
       DXOP STORE,1           ;STORE FPAC
       DXOP FADD,2            ;ADD TO FPAC
       DXOP FSUB,3            ;SUBTRACT FROM FPAC
       DXOP FMUL,4            ;MULTIPLY FPAC
       DXOP FDIV,5            ;DIVIDE FPAC
       DXOP SCALE,6           ;SCALE FPAC
       DXOP NORMAL,7          ;NORMALIZE FPAC
       DXOP CLEAR,8           ;CLEAR FPAC
       DXOP NEGATE,9          ;NEGATE FPAC
       DXOP FLOATF,10         ;FLOAT FPAC
       DXOP EVFIX,11          ;EVALUATE AND FIX
       DXOP OUTFP,12          ;OUT FLOATING POINT #
       DXOP OUTINT,13         ;OUT INTEGER
*
       REF  FPWP              ;FLOATING POINT WORKSPACE POINTER
       REF  TEMP              ;3 WRD TEMPORARY STORAGE
       REF  CLKT01
       REF  CLKT02
       REF  EVSFR$            ;EXIT ADDRESS
       DEF  GTICZ,TICF
*
* ABSTRACT:
*
*       GET AND FLOAT THE NUMBER OF ELAPSED
*       TICS OF THE CLOCK IN FPAC.
*
* CALLING SEQUENCE:
*
*       BLWP @GTICZ
*
*       OUT FPAC = CLOCK TICS
*
       PAGE
*
GTICZ  DATA FPWP,GTIC         ;ACCESS VECTOR
*
GTIC   LIMI 0                 ;DISABLE INTERRUPTS
       MOV  @CLKT01,R1
       MOV  @CLKT02,R2
       LIMI 15                ;ENABLE INTERRUPTS
       LI   R0,>4A00          ;GET EXPONENT
       B    @FNRM             ;NORMALIZE
*
* ABSTRACT:
*
*       GET DELTA TICS.  (SUBTRACT FROM THE
*       FUNCTION ARGUMENT THE NUMBER OF ELAPSED
*       TICS OF THE CLOCK.)
*
* CALLING SEQUENCE:
*
*       B @TICF
*
*       IN (R2) = ARGUMENT
*
*       OUT (R2) = ARG - TICS
*
*       NORMAL EXIT TO EVSFR
*
TICF   LOADF *R2              ;LOAD FPAC
       FLOATF 0               ;FLOAT IF NECESSARY
       STORE @TEMP            ;STORE IN TEMP
       BLWP @GTICZ            ;GET TICS IN FPAC
       FSUB @TEMP             ;FPAC=FPAC-TEMP
       B    @EVSFR$           ;RETURN
       END
