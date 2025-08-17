* Breadboard ROM boot menu
* Based on Code by Stuart Conner
*
* D. Werner 8/16/2025
*
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
        CI R1,'2'*256                             ; MDEX option selected?
        JNE TAG4F                                 ; no, loop round and wait for another character.
        B @>7800                                  ; branch to MDEX boot loader.

TAG4F
        CI R1,'3'*256                             ; UNIX option selected?
        JNE TAG1B                                 ; no, loop round and wait for another character.
        B @>7b00                                  ; branch to UNIX boot loader.

* logon message
LOGON
        DATA >0D0A
        DATA >0D0A
        TEXT '  ___         _           ___  ___'
        DATA >0D0A
        TEXT ' / __|___ _ _| |_ _____ _| _ \/ __| '
        DATA >0D0A
        TEXT '| (__/ _ \ '
        BYTE >27
        TEXT '_|  _/ -_) \ /  _/ (__'
        DATA >0D0A
        TEXT ' \___\___/_|  \__\___/_\_\_|  \___| '
        DATA >0D0A
        DATA >0D0A
        TEXT 'TMS9995 COMPUTER SYSTEM '
        DATA >0D0A
        TEXT 'BIOS V1.0 '
        DATA >0D0A
        DATA >0D0A
        TEXT 'PRESS 1 FOR EVMBUG MONITOR'
        DATA >0D0A
        TEXT 'PRESS 2 FOR MDEX'
        DATA >0D0A
        TEXT 'PRESS 3 FOR UNIX'
        DATA >0D0A,00

        END
