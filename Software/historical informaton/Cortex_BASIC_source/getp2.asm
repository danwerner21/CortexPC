       TITL 'GET POWER ROUTINE - CORTEX BASIC REV. 1.1'
       IDT  'GETP2'
*
*       GETP2                 ;GET POWER OF 2
*
       DXOP FMUL,4            ;MULTIPLY FPAC
*
       REF  CVCH              ;VARIABLE HOLDER
       DEF  GETP2             ;ENTRY POINT
*
*       GETP2 WILL RETURN A 3 WORD FLOATING
*       POINT POWER OF 2 AS SPECIFIED BY R2.
*
* CALLING SEQUENCE:
*
*       BL @GETP2
*
*       IN R2    = 1,2 OR 3
*       OUT CVCH = #
*
*
GETP2  LI   R2,GETPC-2
       SRA  R2,1              ;/2
       A    R3,R2             ;INDEX
       SLA  R2,1              ;MAKE WORD INDEX
       MOV  *R2,@CVCH         ;MOVE INTO CONSTANT
       FMUL @CVCH             ;MULTIPLY FPAC
       RT
*
GETPC  DATA >4120,>4140,>4180 POWERS OF 2
       END
