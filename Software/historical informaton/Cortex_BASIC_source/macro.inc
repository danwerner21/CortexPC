*
*  DEFINE A MACRO TO GO THE 'READ Gs' ETC. INSTRUCTIONS
*  WHERE THE RESULTING OPCODE WILL BE OF THE FORM
*
*        0 0 0 0   1 1 1 X   X X X X   X X X X
*                       |  N    | Ts |   Rs    |
*
*    WNBL      N=0
*    RHXW      N=1
*    WHXW      N=2
*    EKO       N=3
*    WRIT      N=4
*    READ      N=5
*    MSG       N=6
*    BKPT      N=7
*
*
*     NOTE : IF THE OPERAND IS INDEXED THEN THE CORRECT
*            CODE IS GENERATED BUT SDSMAC WILL ALSO ERROR
*            IT.
*
Z$$Z$$ $MACRO P1,O
*     DEFINE THE REQUIRED DATA PASSED THE BASE OPCODE
*     AND THE OPERAND.
       $VAR T
       $ASG P1 TO T
*   IS IT INDEXED
   $IF  P1.A&$PNDX
       $ASG O.V+>20 TO O.V
       $ASG :O.V:'+R':P1.V:',':P1.S: TO T.S
       DATA :T.S:              INDEX
       $EXIT
   $ENDIF
*   NO , IS IT SYMBOLIC ?
   $IF  P1.A&$PSYM
       $ASG O.V+>20 TO O.V
       $ASG :O.V:',':P1.S: TO T.S
       DATA :T.S:              SYMBOLIC
       $EXIT
   $ENDIF
*   NO , IS IT INDIRECT
   $IF  P1.A&$PIND
       $ASG O.V+>10 TO O.V
       $ASG :O.V:'+R':P1.V: TO T.S
       DATA :T.S:             INDIRECT
       $EXIT
   $ENDIF
*   NO , IS IT INDIRECT AUTOINCREMENT
   $IF P1.A&$PATO
       $ASG O.V+>30 TO O.V
       $ASG :O.V:'+R':P1.V: TO T.S
       DATA :T.S:             AUTOINC
       $EXIT
   $ENDIF   
*   NO IT MUST BE REGISTER THEN
       $ASG :O.V:'+R':P1.V: TO T.S
       DATA :T.S:             REGISTER
   $END            END OF INTERNAL MACRO 'Z$$Z$$'
*
*    DEFINE UTILITY MACRO'S
*
WNBL   $MACRO P2
       Z$$Z$$ :P2.S:,>0E00        WRITE HEX NIBBLE
       $END
RHXW   $MACRO P2
       Z$$Z$$ :P2.S:,>0E40        READ HEX WORD
       $END
WHXW   $MACRO P2
       Z$$Z$$ :P2.S:,>0E80        WRITE HEX WORD
       $END
EKO    $MACRO P2
       Z$$Z$$ :P2.S:,>0EC0        ECHO CHARACTER  
       $END
WRIT   $MACRO P2
       Z$$Z$$ :P2.S:,>0F00        WRITE CHARACTER
       $END
READ   $MACRO P2
       Z$$Z$$ :P2.S:,>0F40        READ CHARACTER
       $END
MSG    $MACRO P2
       Z$$Z$$ :P2.S:,>0F80        OUT MESSAGE
       $END
BKPT   $MACRO P2
       Z$$Z$$ :P2.S:,>0FC0        BREAKPOINT      
       $END
*
*    END OF MACRO DEFINITIONS
*