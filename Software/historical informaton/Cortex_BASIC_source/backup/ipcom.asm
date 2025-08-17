       TITL 'I/O UTILITIES - CORTEX BASIC REV. 1.1'
       IDT  'IPCOM'
*
* ROUTINE LIST:
*
*       OUTL1           ;LIST LINE
*       INPEND          ;END INPUT
*       PRTEND          ;END PRINT
*       UCCNT           ;UPDATE CCNT
*
* EXTERNAL ROUTINES:
*
       REF  TYPBE$
       REF  TYPC$             ;OUTPUT CRLF
       REF  NLIN              ;EXIT TO MULTIPLEXOR
       REF  IOB               ;I/O BUFFER
*
* EXTERNAL DATA:
*
       REF  CCNT              ;COLUMN COUNTER
       REF  FFLG              ;FORMATTING FLAG
       PAGE
* ABSTRACT:
*
*       OUTL1 - UPDATES CCNT AND PRINTS BUFFER.
*
*       PRTEND - UPDATES CCNT, PRINTS BUFFER, AND
*               AND ENDS WITH A CRLF IF R15=0
*               BEFORE EXITING TO NLIN.
*
*       INPEND - PRINTS A CRLF IF R15=0 AND
*               EXITS TO NLIN.
*
*       UCCNT - UPDATES CCNT SUCH THAT IF REFLECTS
*               WHAT HAS BEEN PRINTED PLUS WHAT
*               IS IN THE BUFFER AND TERMINATES
*               BUFFER WITH NULL.
*
* CALLING SEQUENCE:
*
*       BL @OUTL1
*
*       IN  R7 = BUFFER PTR
*       OUT R7 = BUFFER ADR
*
*
*       B @PRTEND
*
*       IN  R7 = BUFFER PTR
*          R15 = CRLF FLAG
*       EXITS TO NLIN
*
*
*       B @INPEND
*
*       IN R15 = CRLF FLAG
*       EXITS TO NLIN
*
*
*       BL @UCCNT
*
*       IN R7 = BUFFER PTR
*
* EXCEPTIONS AND CONDITIONS:  (NONE)
*
       DEF  OUTL1,PRTEND,INPEND,UCCNT
       PAGE
*
OUTL1  MOV  R11,R13           ;SAVE RETURN
       BL   @PBIN             ;UPDATE CCNT & OUT BUFFER
       MOV  @IOB,R7
       B    *R13              ;RETURN
*
*END PRINT LINE
*
PRTEND BL   @PBIN             ;UPDATE CCNT & OUT BUFFER
*
*END INPUT LINE
*
INPEND CLR  @FFLG             ;CLEAR FORMATTING FLAG
       MOV  R15,R15           ;NEED CRLF?
       JNE  INPE1             ;N
       DATA TYPC$             ;Y, OUT CRLF
INPE1  B    @NLIN             ;CONTINUE
*
*  PRINT BUFFER IF NEEDED
*
PBIN   C    R7,@IOB           BUFFER EMPTY
       JEQ  PBIN1             Y, EXIT
       DATA TYPBE$            N, PRINT BUFFER
PBIN1  EQU  $
*
*UPDATE CCNT
*
UCCNT  MOV  @CCNT,R0          ;GET COUNT
       A    R7,R0             ;ADD CURRENT PTR
       S    @IOB,R0           ;SUBTRACT IOB
       ANDI R0,>7F            ;MOD 128
       MOV  R0,@CCNT          ;SAVE RESULT
       SB   *R7,*R7           ;TERMINATE STRING
       RT
       END
