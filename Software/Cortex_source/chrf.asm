       TITL 'CHARACTER FUNCTIONS - CORTEX BASIC REV. 1.1'
       IDT  'CHRF'
*
*       POSF            ;STRING SEARCH FUNCTION
*       MCHF            ;STRING MATCH FUNCTION
*       LENF            ;STRING LENGTH FUNCTION
*
       REF  EVSFR$            EXIT ENTRY TO EVALUATOR
       REF  FPAC2             FLOATING POINT ACCUMULATOR
       DEF  POSF              ENTRY PT. FOR 'POS' FUNCTION
       DEF  MCHF              ENTRY PT. FOR 'MCH' FUNCTION
       DEF  LENF              ENTRY PT. FOR 'LEN' FUNCTION
*
*      POS:-   SEARCH FOR THE FIRST STRING IN THE SECOND
*              STRING AND RETURN THE STARTING POSITION
*              OF THE FIRST MATCH.
*              IF NO MATCH IS FOUND A '0' IS RETURNED.
*
*      MCH:-   MATCH STRING 1 INTO STRING 2 AND RETURN
*              THE NUMBER OF CHARACTERS TO WHICH THEY
*              AGREE. IF NO MATCH IS FOUND A '0' IS RETURNED
*
*      LEN:-   RETURN THE LENGTH OF THE STRING UPTO THE 1ST
*              NULL BYTE.
       PAGE
* CALLING SEQUENCE:
*
*       B @POSF
*
*       EXIT TO EVSFR$
*
POSF   INC  @FPAC2            ;COUNT
       MOVB *R1+,R0           ;GET FIRST BYTE
       JEQ  POSF2             ;NOT FOUND, RETURN 0
       CB   *R2,R0            ;SAME?
       JNE  POSF              ;N, CONTINUE LOOKING
       MOV  R2,R3             ;Y, LOOK FURTHER
       INC  R3
       MOV  R1,R4
*
POSF1  MOVB *R3+,R0           ;MATCH?
       JEQ  POSF3             ;Y, RETURN FPAC
       CB   R0,*R4+           ;SAME?
       JEQ  POSF1             ;Y, CONTINUE
       JMP  POSF              ;N, START AGAIN
*
POSF2  CLR  @FPAC2
* EXIT TO EVSFR WITH R2 RELOADED WITH 'FPAC'
POSF3  B    @EVSFR$
*
*
* CALLING SEQUENCE:
*
*       B @MCHF
*
*       EXIT TO EVSFR$
*
MCHF   CB   *R2,*R1+          SAME?
       JNE  POSF3             N
       MOVB *R2,*R2+          Y - BUT WERE THEY NULL?
       JEQ  POSF3             Y - THEN DON'T COUNT
       INC  @FPAC2            N - NOT NULL; THEN COUNT
       JMP  MCHF
*
*
* CALLING SEQUENCE:
*
*       B @LENF
*
*       EXIT TO EVSFR$
*
LENF   MOVB *R2+,R1
       JEQ  POSF3             ;DONE
       INC  @FPAC2            ;COUNT
       JMP  LENF
       END
