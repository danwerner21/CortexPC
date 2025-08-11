* Breadboard ROM boot menu
*

        AORG >1500

BMENU
        XOP @LOGON,14                             ; print boot logon message and options.

TAG1B
        XOP R1,13                                 ; read character into msb of r1.
        CI R1,'1'*256                             ; EVMBUG option selected?
        JNE TAG2F                                 ; no, test BASIC option
        XOP @>00ad,14                             ; yes, print evmbug logon message.
        B @>0142                                  ; branch to evmbug.

TAG2F
        CI R1,'2'*256                             ; BASIC option selected?
        JNE TAG3F                                 ; no, test MDEX option
        LI R1,>1600                               ; yes, copy basic from EPROM to RAM.
        LI R2,>8000
        LI R3,>5eba
TAG9B
        MOV *R1+,*R2+
        DECT R3
        JNE TAG9B
        B @>8036                                  ; branch to BASIC.

TAG3F
        CI R1,'3'*256                             ; MDEX option selected?
        JNE TAG4F                                 ; no, loop round and wait for another character.
        B @>7800                                  ; branch to MDEX boot loader.

TAG4F
        CI R1,'4'*256                             ; UNIX option selected?
        JNE TAG1B                                 ; no, loop round and wait for another character.
        B @>7b00                                  ; branch to UNIX boot loader.

* logon message
LOGON
        DATA >0D0A
        TEXT 'TMS 9995 BREADBOARD SYSTEM'
        DATA >0D0A
        TEXT 'BY STUART CONNER'
        DATA >0D0A
        TEXT 'PRESS 1 FOR EVMBUG MONITOR'
        DATA >0D0A
        TEXT 'PRESS 2 FOR CORTEX BASIC'
        DATA >0D0A
        TEXT 'PRESS 3 FOR MDEX'
        DATA >0D0A
        TEXT 'PRESS 4 FOR UNIX'
        DATA >0D0A,00

        END
