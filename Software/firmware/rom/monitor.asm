        AORG >0000

******************************************************************************
* Interrupt vectors.                                                         *
******************************************************************************

        DATA >EC00,A0226                          WP/PC level 0 interrupt (reset).
        DATA >F0D6,>F0F6                          WP/PC level 1 interrupt.
        DATA >F0CA,>F0EA                          WP/PC level 2 interrupt.
        DATA >F0BE,>F0DE                          WP/PC level 3 interrupt.
        DATA >F0B2,>F0D2                          WP/PC level 4 interrupt.

*Interrupts 5-15 not used. Use vector space for text strings.

A0014   BYTE >0D,>0A
        TEXT 'MON? '
        BYTE >00

A001C   TEXT '    '
A0020   TEXT ' '
A0021   TEXT ' '
        BYTE >00

A0023   BYTE >0D,>0A
        TEXT 'BP'
        BYTE >00

A0028   TEXT 'IDT='
        BYTE >00

A002D   BYTE >0D,>0A
        TEXT 'READY Y/N '
        BYTE >00

A003A   TEXT 'W'
        BYTE >00
A003C   TEXT 'P'
        BYTE >00
A003E   TEXT 'S'
        BYTE >00

******************************************************************************
* XOP vectors.                                                               *
******************************************************************************

        DATA >F0AC,>F0BE                          XOP 0.
        DATA >F09E,>F0B0                          XOP 1.
        DATA >F090,>F0A2                          XOP 2.
        DATA >F082,>F094                          XOP 3.
        DATA >F074,>F086                          XOP 4.
        DATA >F066,>F078                          XOP 5.
        DATA >F058,>F06A                          XOP 6.
        DATA >F04A,>F05C                          XOP 7.
        DATA >EC24,>03F8                          XOP 8.
        DATA >EC24,>0396                          XOP 9.
        DATA >EC24,>0402                          XOP 10.
        DATA >EC0A,A0326                          XOP 11 - read character and echo to terminal.
        DATA >EC16,A02EE                          XOP 12 - write character.
        DATA >EC16,A02E2                          XOP 13 - read character.
        DATA >EC24,A032C                          XOP 14 - write message.
        DATA >EC00,>0442                          XOP 15.

        DATA >0460
        DATA >0142

*Baud rate table.

A0084   DATA >0009,>0034                          19200 BAUD (changed from 1a to 34)
        DATA >0012,>0034                          9600 BAUD
        DATA >0023,>0034                          4800 BAUD (changed from 68 to 34)
        DATA >0046,>0034                          2400 BAUD (changed from d0 to 34)
A0094   DATA >008D,>0034                          1200 BAUD (changed from 1a1 to 34)
        DATA >0119,>0034                          600 BAUD (I assume)
        DATA >02A4,>0034                          300 BAUD  (changed from 4d0 to 34)
        DATA >7FFF,>0034                          110 BAUD (changed from 638 to 34)

A00A4   BYTE >0D,>0A                              Error message.
        TEXT 'ERROR '
A00AC   BYTE >00

A00AD   BYTE >0D,>0A                              Logon message.
        TEXT 'EVMBUG  R1.0'
        BYTE >00

A00BC   DATA >0D0A
        DATA >5200
        DATA >0D0A
        DATA >4831
        DATA >2B48
        DATA >323D
        DATA >0020
        DATA >4831
        DATA >2D48
A00CE   TEXT '2'
A00CF   TEXT '='
        BYTE >00
        TEXT ' '
        DATA >4552
        DATA >524F
A00D6   DATA >5200
A00D8   DATA >0D0A
        DATA >434F
        DATA >4D3F
A00DE   DATA >2000
        DATA >3A0D
        BYTE >00

A00E3   BYTE >0D,>0A
        TEXT 'TERMINAL MODE'
A00F2   BYTE >0D,>0A
        BYTE >00

A00F5   BYTE >0D
        DATA >0A43
        DATA >4D44
        DATA >2045
        DATA >5252
        DATA >000D
        DATA >0A50
        DATA >4152
        DATA >4D20
        DATA >4552
        DATA >5200
        DATA >0D0A
        DATA >434B
        DATA >534D
        DATA >2045
        DATA >5252
        DATA >000D
        DATA >0A54
        DATA >4147
        DATA >2045
        DATA >5252
        DATA >0055
        DATA >504C
        DATA >4420
        DATA >4552
        DATA >5200
A0128   DATA >460D
        DATA >000A
        DATA >7F3A
        DATA >0D0A
        DATA >7F13
        DATA >0D14
A0134   DATA >7F00
A0136   DATA >120A
        DATA >7F00

*Initialisation values for TMS9902 control registers.

A013A   BYTE >62                                  (0110 0010) 7 bits/character, even parity, 2 stop bits.

A013B   BYTE >42                                  (0100 0010) 7 bits/character, no parity, 2 stop bits.


A013C   DATA >1100                                <DC1> character written to RS-232 port as part of XCL command.
A013E   DATA >0000                                1st TMS9902 CRU base address.
A0140   DATA >0400                                2nd TMS9902 CRU base address.

*Branch taken to here after printing logon message.

A0142   LWPI >EC00                                Initialise workspace.
*

        CLR R1                                    Address of WP vector for level 0 interrupt.
        LI R2,>FFFC                               Address of NMI WP vector.
*
        MOV *R1+,*R2+                             Copy level 0 interrupt WP vector to NMI WP vector.
        LI R1,A042E                               Address to put in PC vector for NMI (address of load for single step).
*
        MOV R1,*R2+                               Store NMI PC vector.

        LI R9,>0142                               Re-entry address for monitor. Used by some commands to return to command scanner.
*

        CLR R1                                    Delay loop (leftover from TIBUG for microterminal?).
A015A   DECT R1
        JNE A015A

        MOV @>013E,R12                            CRU address of 1st TMS9902 serial port.
*
        MOV R12,@>EC2E                            Store CRU address.
*
        TB 21                                     Receive buffer register full?
        JNE A016C                                 No, jump.

*Input command.
*Command is one to three uppercase letters followed by a <CR>, <space> or <comma> character.

A016A   XOP R5,13                                 Read character into R5.
A016C   XOP @A0014,14                             Print "MON? ".
*
        CLR R0
        LI R2,>0A05                               Opcode for SLA R5,0.
*
        LI R3,>0004                               Max number of characters that can be entered + 1.
*
        CLR R4                                    Keystrokes assembled into R4.
        LI R8,>0001
*
A0180   XOP R5,11                                 Read character into R5 and echo to terminal.
        CI R5,>0D00                               <CR> character?
*
A0186   JNE A0190                                 No, jump.
A0188   LI R5,>0A00                               Yes, load a <LF> character.
*
A018C   XOP R5,12                                 Write character to terminal.
        JMP A01C2                                 Find command in command table.
A0190   CI R5,>2000                               <Space> character?
*
A0194   JEQ A01C2                                 Yes, find command in command table.
A0196   CI R5,>2C00                               No, <Comma> character?
*
A019A   JEQ A01C2                                 Yes, find command in command table.
A019C   CI R5,>4100                               'A' character?
*
A01A0   JLT A021A                                 Jump if ASCII character < 'A' and display error number 4.

        CI R5,>5A00                               'Z' character?
*
        JGT A021A                                 Jump if character > 'Z' and display error number 4.
        DEC R3                                    Decrement number of characters entered.
        JEQ A021A                                 Jump if entered 3 characters and display error number 4.
        SWPB R5                                   Swap character in R5 into LS byte.
        ANDI R5,>001F                             Mask the least 5 significant bits.
*
        CI R3,>0003                               This the first character entered?
*
        JEQ A01BA                                 Yes, skip next instruction.
        X R2                                      Initially SLA R5,0.
A01BA   A R5,R4                                   Add result to R4.
        AI R2,>0050                               Add 5 to the shift count in R2.
*
        JMP A0180                                 Get next character.

* So following the routine above, the ASCII codes of the characters entered are packed in R4
* as follows:

* XXXX XXXX XXXX XXXX
*              ^ ^^^^ = 1st character entered.
*        ^^ ^^^       = 2nd character entered.
*  ^^^ ^^             = 3rd character entered.

A01C2   LI R11,A027A                              Address of start of command table.
*
        JMP A01CC                                 Skip following increment.

A01C8   AI R11,>0004
*

A01CC   MOV *R11+,R10                             Get command value from table.
        JEQ A021A                                 Jump if end of table reached.
        C R10,R4                                  Command value = command entered?
        JNE A01C8                                 No, loop and check next command in table.
        MOV *R11+,R6                              Yes, get number of operands for this command.
        MOV *R11,R11                              Get entry point address for this command.
        LI R7,>EC00                               Address to store operands.
*
        CI R5,>0A00                               Previously a <CR> character, and we loaded a <LF>?
*
        JEQ A0200                                 Yes, jump.
A01E2   SRL R6,1                                  Command has at least one operand?
        JNC A0202                                 No, execute command.
        XOP R4,9                                  Input 4 digit hex number to R4.
        DATA A01F8                                Returns to this address if input is termination character only.
        DATA A020E                                Returns to this address if invalid entry - print error number 2.
        MOV R4,*R7+                               Store operand and increment operand pointer.
A01EE   INC R3
        CI R5,>0D00
*
        JEQ A0202                                 Yes, execute command.
        JMP A01E2                                 Loop and check for another operand.

A01F8   INCT R7
        CI R5,>0D00
*
        JNE A01EE
A0200   CLR R3
A0202   B *R11                                    Execute command.

A0204   CLR R0                                    Error number 0.
        JMP A021E                                 Print error message.

A0208   LI R0,>0001                               Error number 1.
*
        JMP A021E                                 Print error message.

A020E   LI R0,>0002                               Error number 2.
*
        JMP A021E                                 Print error message.

A0214   LI R0,>0003                               Error number 3.
*
        JMP A021E                                 Print error message.

A021A   LI R0,>0004                               End of command table reached. Error number 4.
*
A021E   XOP @A00A4,14                             Print error message ...
*
        XOP R0,8                                  ... followed by error number.
        JMP A0142

******************************************************************************
* Reset entry point.                                                         *
******************************************************************************

A0226   LI R12,>EC44
*
        CLR *R12+                                 Clear ASR flag (>EC44).
        SETO *R12+                                Reset dump flag (>EC46).
        CLR *R12+                                 Clear step flag (>EC48).
        CLR *R12                                  Clear halt flag (>EC50).

*Initialise TMS9902.

        MOV @A013E,R12                            CRU address of TMS9902 serial port.
*
        MOV R12,@>EC2E                            Store CRU address.
*
        SBO 31                                    Reset TMS9902.
        CLR R3                                    Reset loop count.
        LDCR @A013A,8                             Initialise TMS9902 control register.
*
        SBZ 13                                    Do not initalise interval register.
A0244   TB 15                                     RS-232 space?
        JEQ A0244                                 No, jump back.
A0248   INC R3                                    Yes, time the start bit.
        TB 15                                     RS-232 mark?
        JNE A0248                                 No, jump back.

*Table search for baud rate.

A024E   LI R7,A0084                               Set pointer to baud rate table.
*
A0252   C R3,*R7+                                 Matching baud rate?
        JLT A025A                                 Yes, jump.
        INCT R7                                   No, update pointer to check next baud rate.
        JMP A0252

A025A   LDCR *R7,12                               Initialise TMS9902 receive/transmit baud rate.
        MOV *R7,R7
        CI R7,>01A1                               1200 baud?
*
        JLT A0274                                 If faster baud rate, jump.
        JNE A026A                                 If 300 or 110 baud, jump.
        SETO @>EC44                               Set ASR flag.
*

A026A   XOP R5,13                                 Read character into R5.
        ****************  OLD XOP @A00AD,14                             Print logon message.
        B @>1500                                  Branch to menu (DDW)
*
        B @A0142                                  Branch.
*

A0274   DATA >05A0
        DATA >EC44
        DATA >10F8

*Command table.
*Second word is a bitmap of the number of operands for the command.
*Example: >0003 = binary 0011 = two operands.

A027A   DATA >01A9                                Command IM (Inspect/Change Memory - as TIBUG 'M' command with start address only).
A027C   DATA >0001                                One operand.
A027E   DATA A036C                                Address.

A0280   DATA >01A4                                Command DM (Dump Memory - as TIBUG 'M' command with start and stop addresses).
A0282   DATA >0003                                Two operands.
A0284   DATA A0334                                Address.

A0286   DATA >4AE9                                Command IWR (Inspect/Change Workspace Register - as TIBUG 'W' command).
A0288   DATA >0001                                One operand.
A028A   DATA A04AC                                Address.

A028C   DATA >0305                                Command EX (Execute - as TIBUG 'E' command).
A028E   DATA >0000                                No operands.
A0290   DATA A042C                                Address.

A0292   DATA >0B05                                Command EXB (Execute With Breakpoint - as TIBUG 'B' command).
A0294   DATA >0001                                One operand.
A0296   DATA A0436                                Address.

A0298   DATA >0273                                Command SS (Single Step - as TIBUG 'S' command).
A029A   DATA >0000                                No operand.
A029C   DATA A0426                                Address.

A029E   DATA >0DAC                                Command LMC (Tag Format Loader - as TIBUG 'L' command).
A02A0   DATA >0001                                One operand.
A02A2   DATA A064A                                Address.

A02A4   DATA >0DA4                                Command DMC (Dump RAM Image - as TIBUG 'D' command).
A02A6   DATA >0007                                Three operands.
A02A8   DATA A0552                                Address.

A02AA   DATA >0069                                Command IC (Inspect/Change CRU - as TIBUG 'C' command).
A02AC   DATA >0003                                Two operands.
A02AE   DATA A0460                                Address.

A02B0   DATA >0249                                Command IR (Inspect/Change WP, PC and ST Registers - as TIBUG 'R' command).
A02B2   DATA >0000                                No operands.
A02B4   DATA A051A                                Address.

A02B6   DATA >0086                                Command FD (Find - as TIBUG 'F' command).
A02B8   DATA >0007                                Three operands.
A02BA   DATA A0770                                Address.

A02BC   DATA >60A8                                Command HEX (Hex Arithmetic - as TIBUG 'H' command).
A02BE   DATA >0003                                Two operands.
A02C0   DATA A07A0                                Address

A02C2   DATA >19D4                                Command TNF (Toggle ASR Flag - as TIBUG 'T' command).
A02C4   DATA >0000                                No operands.
A02C6   DATA A07B4                                Address.

A02C8   DATA >1438                                Command XAE (Line-By-Line Assembler, retain symbol table).
A02CA   DATA >0001                                One operand.
A02CC   DATA A07C2                                Address.

A02CE   DATA >0038                                Command XA (Line-By-Line Assembler, clear symbol table).
A02D0   DATA >0001                                One operand.
A02D2   DATA A07BA                                Address.

A02D4   DATA >3078                                Command XCL (Terminal Mode)
A02D6   DATA >0000                                No operands.
A02D8   DATA A0E1A                                Address.

A02DA   DATA >0658                                Command XRA (Disassemble - Reverse Assemble?).
A02DC   DATA >0003                                Two operands.
A02DE   DATA >1166                                Address.

A02E0   DATA >0000                                End of command table.

******************************************************************************
* XOP 13. Read character.                                                    *
******************************************************************************

A02E2   TB 21                                     Receive buffer register full?
        JNE A02E2                                 No, loop round.
        CLR *R11
        STCR *R11,8
        SBZ 18                                    Disable receive buffer interrupts.
        RTWP                                      Return from XOP.

******************************************************************************
* XOP 12. Write character.                                                   *
******************************************************************************

A02EE   LI R10,>186A                              Carriage return, ASR delay.
*
        SBO 16                                    Set RTS on.
A02F4   TB 22                                     Transmit buffer register empty?
        JNE A02F4                                 No, wait until it is.
        LDCR *R11,8                               Send character to TMS9902.
        MOVB *R11,R11
********* REPLACED FOR ROM MATCH        CB R11,@A00F2                             Carriage return?
        RTWP
        DATA >1000
*
        JNE A0318                                 No, jump.
        MOV @>EC44,R11                            Yes, ASR flag set?
*
        JGT A0324                                 Yes, exit.
        SLA R10,3                                 Adjust ASR delay?
A030A   TB 22                                     Wait for transmission to end.
        JNE A030A
        TB 23
        JNE A030A
A0312   DEC R10                                   Decrement ASR delay loop.
        JNE A0312
        RTWP                                      Return from XOP.

A0318   MOV @>EC46,R11                            Dump flag set?
*
        JEQ A0324                                 Yes, ignore ASR flag and exit.
        MOV @>EC44,R11                            No, ASR flag set?
*
        JLT A030A                                 Yes, wait 3 nulls.
A0324   RTWP                                      Return from XOP.

******************************************************************************
* XOP 11. Reach character and echo to terminal.                              *
******************************************************************************

A0326   XOP *R11,13                               Read character.
        XOP *R11,12                               Write character.
A032A   RTWP                                      Return from XOP.

******************************************************************************
* XOP 14. Write message.                                                     *
******************************************************************************

A032C   MOVB *R11+,R12                            Get character from buffer.
        JEQ A032A                                 If end of message, exit.
        XOP R12,12                                Write character.
        JMP A032C                                 Loop for next character.

******************************************************************************
* DM (Dump Memory) command.                                                  *
* Two operands.                                                              *
******************************************************************************

A0334   SZC R8, R0
        SZC R8, R1
A0338   LI R3, >0008
        XOP @A00F2, R14
        XOP R0, R10
A0342   XOP @A00CF, R14
A0346   XOP *R0, R10
        TB 21
        JNE A0350
        XOP R0, R13
A034E   B *R9
A0350   C R0, R1
        JEQ A034E
        INCT R0
        DEC R3
        CI R3, >0004
        JNE A0362
        XOP @A0020, R14
A0362   MOV R3, R3
        JEQ A0338
        XOP @A0020, R14
        JMP A0346


******************************************************************************
* IM (Inspect Memory) command.                                               *
* One operand.                                                               *
******************************************************************************
A036C   SZC R8,R0
        JMP A0378
A0370   DECT R0
        SLA R5,2
        JNE A036C
        C *R0+,*R0+
A0378   XOP @A00F2,R14

        XOP R0,R10
        XOP @A00CF, R14

        XOP *R0, R10
        XOP @A0020, R14

        XOP R4,R9
        DATA A0390
        LI R14,>C404

A0390   SLA R5,2
        JLT A0370
        B *R9
        CLR R9
        CLR R12
A039A   XOP R10,R11
        CI R10,>3000

        JL A03C4
        CI R10,>3900
        JLE A03B8
        CI R10,>4100
        JL A03C4
        CI R10,>4600
        JH A03C4
        AI R10,>0900

A03B8   SLA R10, 4
        SRL R10, 12
        SLA R12, 4
        A R10, R12
        INC R9
        JMP A039A
A03C4   CI R10,>2000

        JEQ A03E0
        CI R10, >2D00

        JEQ A03E0
        CI R10, >0D00

        JEQ A03E0
        CI R10, >2C00

        JNE A03F4
        LI R10, >2000

A03E0   MOV R9, R9
        JEQ >03EC
        MOV R12, *R11+
        MOV R10, *R11
        C *R14+, *R14+
        RTWP
A03EC   INCT R11
        MOV R10, *R11
A03F0   MOV *R14, R14
        RTWP
A03F4   INCT R14
        JMP A03F0
        MOV *R11, R12
        SLA R12, 12
        LI R9, >0001

        JMP A0408
        MOV *R11, R12
        LI R9, >0004

A0408   MOV R12, R10
        SRL R10, 12
        SLA R10, 8
        CI R10, >0900
        JLE >0418
        AI R10, >0700
        AI R10, >3000
        XOP R10, R12
        SRC R12, 12
        DEC R9
        JNE A0408
        RTWP

******************************************************************************
* SS (Single Step) command.                                                  *
* No operands.                                                               *
******************************************************************************

A0426   LI R7,>9900                               DATA >0207  Set trace flag.
        DATA >03E0                                Opcode for LREX instruction (assembler doesn't recognise it.
*                        Initiate load interrupt.

******************************************************************************
* (embedded with SS code - EX (Execute) command.                             *
* No operands.                                                               *
******************************************************************************

A042C   RTWP

*
* Load (NMI) entry point.
*
A042E   CI R7,>9900                               Trace flag set?
        JEQ A044A                                 Yes, jump to print user WP, PC and ST.
        JMP A045C                                 No, return to monitor.

******************************************************************************
* EXB (Execute With Breakpoint) command.                                     *
* One operand.                                                               *
******************************************************************************


A0436   SZC R8, R0
        MOV *R0, R6
        MOV @A0440, *R0
        RTWP
A0440   XOP R0, R15
        DECT R14
        MOV R6, *R0
        XOP @A0023, R14
A044A   CLR R7
        LI R10, >FFFA
A0450   XOP @>001E, R14
        XOP @>EC20(R10), R10
        INCT R10
        JNE A0450
A045C   B @>0142                                  Return to monitor.

******************************************************************************
* IC (Inspect/Change CRU) command.                                           *
* Two operands.                                                              *
******************************************************************************

A0460   MOV R0, R12
        CLR R7
        ANDI R1, >000F
        JEQ A0470
        CI R1, >0009
        JL A0472
A0470   INC R7
A0472   SLA R1, 6
A0474   LI R8, >3406
        SOC R1, R8
        X R8
        XOP @A00F2, R14
        XOP R12, R10
        XOP @A00CF, R14
        MOV R7, R7
        JNE A048C
        SRL R6, 8
A048C   XOP R6, R10
        XOP @A0020, R14
        XOP R4, R9
        X @A020E(R6)
        MOV R4, R6
        MOV R7, R7
        JNE A04A0
        SLA R6, 8
A04A0   ANDI R8, >F3FF
        X R8
        SRL R5, 12
        JNE A0474
        B *R9

******************************************************************************
* IWR (Inspect/Change Workspace Register) command.                           *
* One operand.                                                               *
******************************************************************************

A04AC   MOV R13,R7
        MOV R3,R3
        JEQ A04F0
        ANDI R0, >000F
        MOV R0, R6
        SLA R0, 1
        A R0, R7
A04BC   XOP @A00BC, R14
        XOP R6, R8
        XOP @A00CF, R14
        XOP *R7, R10
        XOP @A0020, R14
        XOP R4, R9
        CLR *R4
        LI R14, >C5C4
        SLA R5, 2
        JGT A0550
        SLA R5, 1
        JEQ A04E4
        DEC R6
        JLT A0550
        DECT R7
        JMP A04BC
A04E4   CI R6, >000F
        JEQ A0550
        INC R6
        INCT R7
        JMP A04BC
A04F0   CLR R6
        MOV R13, R7
A04F4   XOP @A00BC, R14
A04F8   XOP R6, R8
        XOP @A00CF, R14
        XOP *R7, R10
        INC R6
        INCT R7
        CI R6, >0008
        JEQ A04F4
        CI R6, >0010
        JEQ A0550
        XOP @A0020, R14
        XOP @A00D6, R14
        JMP A04F8

******************************************************************************
* IR (Set WP, PC and ST Registers) command.                                  *
* No operands.                                                               *
******************************************************************************

A051A   LI R6,A003A                               Pointer to 'W' text.
        LI R7,3                                   Set loop count.
        LI R8,>EC1A                               Address of R13 in workspace.
A0526   XOP @A00F2,14                             Print <CR><LF>.
        XOP *R6,14                                Print 'W'.
        XOP @A00CE+1,14                           Print '='.
        MOV *R8,R4                                Get current value in R13.
        XOP *R8,10                                Print value.
        XOP @A0020,14                             Print two spaces.
        XOP R4,9                                  Get new value into R4.
        DATA A053E                                Returns to this address if null entry.
        DATA A020E                                Returns to this address if invalid entry - print error number 2.
A053E   MOV R4,*R8                                Move value into R13.
        SLA R5,2
        JGT A0550
        SLA R5,1
        JNE A0526
        INCT R6                                   Increment text pointer to point to next register name.
        INCT R8                                   Point to R14 (then R15 on next loop) in workspace.
        DEC R7                                    Decrement loop counter.
        JNE A0526                                 Loop round for next register.
A0550   B *R9                                     Return to command scanner.

******************************************************************************
* DMC (Dump RAM Image) command.                                              *
* Three operands.                                                            *
******************************************************************************
A0552   SZC R8, R0
        SZC R8, R1
        SZC R8, R2
        C R0, R1
        JLE A0560
        B @A0214
A0560   CLR R4
        CLR R3
        XOP @A0028, R14
A0568   XOP R4, R13
        CI R4, A0D00
        JNE A0576
        LI R4, >2000
        JMP A0578
A0576   XOP R4, R12
A0578   MOVB R4, @>EC0C(R3)
        INC R3
        CI R3, >0008
        JEQ A058C
        CI R4, >2000
        JNE A0568
        JMP A0578
A058C   XOP @A002D, R14
        XOP R4, R13
        CI R4,>5900
        JNE A061A
        CLR @>EC46
        XOP @>0136, R14
        CLR R10
        CLR R5
        BL @A061C
        LDCR R0, R0
        XOP @>EC0C, R14
        LI R3, >0008
A05B2   MOVB @>EC0B(R3), R4
        SRL R4, 8
        A R4, R5
        DEC R3
        JNE A05B2
        MOV R2, R10
        BL @>061C
        LDCR R0, R4
A05C6   MOV R0, R10
        BL @>061C
        MPY R0, R4
A05CE   MOV *R0, R10
        BL @A061C
        SZC R0, R8
        C R0, R1
        JEQ A05E2
        INCT R0
        CI R3, >003C
        JL A05CE
A05E2   AI R5, >0037
        MOV R5, R10
        NEG R10
        BL @A061C
        STCR R0, R12
        CLR R5
        XOP @A0128, R14
        C R0, R1
        JEQ A0602
        CLR R3
        XOP @>0137, R14
        JMP A05C6
A0602   XOP @>012B, R14
        LI R3, >003C
A060A   XOP @A0134, R14
        DEC R3
        JNE A060A
        SETO @>EC46
        XOP @A00F2, R14
A061A   JMP A06B0
A061C   MOV *R11+, R4
        XOP R4, R12
        SRL R4, 8
        A R4, R5
        XOP R10, R10
        AI R3, >0005
        LI R4, >0004
A062E   SRC R10, 4
        MOV R10, R12
        SRL R12, 12
        A R12, R5
        AI R5, >0030
        CI R12, >000A
        JL A0644
        AI R5, >0007
A0644   DEC R4
        JNE A062E
        RT


******************************************************************************
* LMC (Tag Format Loader) command.                                           *
* One operand.                                                               *
******************************************************************************
A064A   XOP @A002D, R14
        XOP R4, R13
        CI R4, >5900
        JNE A06B0
        LI R6, >1100
        XOP R6, R12
A065C   CLR R7
A065E   CLR R8
        BL @A0728
        JMP A067C
A0666   MOVB @>070E(R10), R8
        JEQ A06C6
        BL @A0722
        JMP A068E
        LI R5, >0008
        SRA R8, 7
A0678   B @A0678(R8)
A067C   CI R6, >0047
        JLT A068E
        CI R6, >004A
        JGT A06B4
        AI R6, >FFC9
        JMP A0666
A068E   CI R6, >003A
        JNE A06B4
        CLR R10
A0696   SETO R5
        LI R12, >0080
A069C   TB 15
        JNE A0696
        DEC R5
        JNE A069C
        MOV R10, R10
        JNE A06BA
        XOP @>00F2, R14
        XOP @>EC02, R14
A06B0   B @A0142
A06B4   CLR R0
A06B6   SETO R10
        JMP A0696
A06BA   MOV R0, R0
        JEQ A06C2
        B @A0208
A06C2   B @A0204
A06C6   XOP R6, R13
        CB R6, @A00F2
        JNE A06C6
        JMP A065C
        A R0, R10
        MOV R10, R9
        JMP A065E
        A R0, R10
        MOV R10, *R9+
        JMP A065E
        A R10, R7
        JEQ A065E
        LI R0, >0001
        JMP A06B6
        LI R10, >EC02
        JMP A06F2
        DECT R5
        LI R10, >EC22
A06F2   XOP R6, R13
        MOVB R6, *R10+
        SRL R6, 8
        A R6, R7
        DEC R5
        JNE A06F2
        JMP A065E
        A R0, R10
        MOV R10, R14
        JMP A065E
        ANDI R10, >FFFE
        MOV R10, R0
        JMP A065E
        STCR R5, R13
        SZC *R10+, *R0
        MPY *R10+, R8
        MPY *R2+, R8
        SOCB @>2C30(R13), R8
        XOP R7, R13
        SBZ 0
        MPY *R10+, R8
        MPY *R3+, R15
A0722   LI R5, >FFFC
        JMP A072A
A0728   SETO R5
A072A   CLR R10
A072C   XOP R6, R13
        CI R6, >2000
        JLT A072C
        CI R6, >5F00
        JGT A072C

*If character in R6 is a hex digit, add it to R10 and return to address *R11+2.
*If character in R6 is NOT a hex digit, return to address *R11.

A073A   SRL R6,8                                  Shift character in MSB of R6 into LSB.
        CI R8,>3200                               (R8 given value in >EC52)
*
        JEQ A0744
        A R6,R7                                   Add ASCII of character read to R7.
A0744   CI R6,>0030                               Is character lower in ASCII than '0'?
*
        JLT A076E                                 Yes, return.
        CI R6,>0039                               Is character lower in ASCII than '9'?
*
        JLE A0760                                 Yes, jump (character was numeric).

        CI R6,>0041                               Is character lower in ASCII than 'A'? (so it is ;:<=>?@)
*
        JLT A076E                                 Yes, return.
        CI R6,>0046                               Is character higher in ASCII than 'F'?
*
        JGT A076E                                 Yes, return.
        AI R6,>0009                               Shift ASCII 'A' to 'F' so we can convert to numeric.
*
A0760   ANDI R6,>000F                             Convert ASCII value to numeric.
*
        SLA R10,4                                 Shift current value across to next digit, then add new digit???
        A R6,R10                                  Add number to R10.
        INC R5
        JNE A072C
        INCT R11                                  Bump return address.

A076E   B *R11                                    Return.

******************************************************************************
* FD (Find) command.                                                         *
* Three operands.                                                            *
******************************************************************************
A0770   LI R3, >8402
        LI R4, >05C0
        SLA R5, 2
        JLT A0782
        SZC R8, R0
        SZC R8, R1
        JMP A0790
A0782   AI R3, >1000
        AI R4, >FFC0
        SLA R2, 8
        JMP A0790
A078E   X R4
A0790   X R3
        JNE A079A
        XOP @A00F2, R14
        XOP R0, R10
A079A   C R0, R1
        JNE A078E
        B *R9

******************************************************************************
* Hex (Hexadecimal Arithmetic) command.                                      *
* Two operands.                                                              *
******************************************************************************
A07A0   XOP @>00C0, R14
        MOV R0, R4
        A R1, R4
        XOP R4, R10
        XOP @>00C9, R14
        S R1, R0
        XOP R0, R10
        B *R9

******************************************************************************
* TNF (Toggle ASR Flag) command.                                             *
* No operands.                                                               *
******************************************************************************
A07B4   INV @>EC44
A07B8   B *R9

******************************************************************************
* XA (Line-By-Line Assembler) command, clear symbol table.                   *
* One operand.                                                               *
******************************************************************************

A07BA   CLR @>EC4E
        CLR @>EC4C

******************************************************************************
* XAE (Line-By-Line Assembler) command, retain symbol table.                 *
* One operand.                                                               *
******************************************************************************
A07C2   MOV R0,R9
A07C4   XOP @A00F2,14
A07C8   LI R10,A0850
        LI R0,>EC52
        LI R8, >0006
A07D4   CLR *R0+
        DEC R8
        JGT A07D4
        XOP R9, R10
        XOP @A001C, R14
        BL *R10
        CI R4, A0020
        JEQ A0814
        CI R4, >002A
        JNE A07F8
A07EE   BL *R10
        CI R4, >000D
        JNE A07EE
        JMP A07C4
A07F8   BL @A0BEE
        MOV R7, @>EC52
        MOV R9, @>EC54
        MOV R7, R4
        BL @A0C64
        JEQ A084E
        CB R7, @>0BF3
        JEQ A0818
        JMP A081C
A0814   XOP @A0021, R14
A0818   XOP @A0021, R14
A081C   LI R7, >0CE3
        CLR R5
        CLR R6
A0824   BL *R10
        BL @A0C34
        JNE A08B2
        SLA R4, 11
A082E   INC R7
        MOVB *R7, R0
        JLT A0838
        JEQ A091A
        INCT R6
A0838   SLA R0, 1
        SRL R0, 14
        C R5, R0
        JLT A082E
        JGT A091A
        MOVB *R7, R0
        SLA R0, 3
        CB R0, R4
        JNE A082E
        INC R5
        JMP A0824
A084E   JMP A091A
A0850   XOP R4, R13
        CI R4, >1B00
        JEQ A07C4
        CI R4, >2000
        JL A0860
        XOP R4, R12
A0860   SRL R4, 8
        MOV R4, @>EC60
        RT
A0868   BL *R10
        CI R4, >0027
        JNE A091A
        MOV R9, R7
A0872   SB *R7, *R7
        INC R8
        BL *R10
        CI R4, >0027
        JEQ A092E
        SWPB R4
        MOVB R4, *R7+
        JMP A0872
A0884   BL @A0B14
        MOV R6, @>EC54
        JMP A0956
        SETO R14
        MOV R6, R0
        BL @A0B14
        MOV R9, @>EC54
        CI R0, >0014
        JNE A08AA
        A R9, R6
        JEQ A08AA
        INC R6
        C R6, R9
        JL A091A
A08AA   MOV R6, R9
        ANDI R9, >FFFE
        JMP A0956
A08B2   MOV R5, R5
        JEQ A0814
        MOVB *R7, R0
        JLT A091A
        SETO R14
        CI R6, >0032
        JEQ A0884
        CI R6, >009A
        JEQ A0868
        MOV @A0D76(R6), R0
        MOV R0, R1
        ANDI R1, >FFF0
        JEQ A08D8
        MOV R1, *R9
        INCT R8
A08D8   MOV R0, R1
        ANDI R1, >000F
        MOVB @A0CD6(R1), R0
        SWPB R0
        ORI R0, >FFE0
        MOV R0, R1
        SRL R1, 2
        ANDI R1, >0006
        MOV @A0CC6(R1), R1
        JEQ A0904
        CI R4, >0020
        JNE A091A
        CLR R14
        LI R15, A0904
        BL *R1
A0904   MOV R0, R1
        SLA R1, 13
        SRL R1, 12
        MOV @>0CC6(R1), R1
        JEQ A0922
        CLR R0
        CLR R14
        LI R15, A0922
        BL *R1
A091A   XOP @>00D1, R14
        B @A07C4
A0922   CI R4, >000D
        JEQ A0936
        CI R4, >0020
        JNE A091A
A092E   BL *R10
        CI R4, >000D
        JNE A092E
A0936   CI R0, >0030
        JEQ A09F4
        XOP @>00F2, R12
A0940   XOP R9, R10
        MOV R9, R2
        BL @A0C86
        LI R4, >2052
        MOV R3, R3
        JEQ A0952
        SWPB R4
A0952   XOP R4, R12
        XOP *R9+, R10
A0956   XOP @A00F2, R14
        DECT R8
        JGT A0940
        LI R0, >EC56
        BL @A0C9A
        BL @A0C9A
        MOV @>EC52, R4
        JEQ A09D2
        AI R4, >8000
        BL @A0C64
        JNE A098A
A097A   BL @A09D6
A097E   MOV *R0, R14
        MOV R3, *R0
        BL @A09E6
        MOV R14, R0
        JNE A097E
A098A   AI R4, >8080
        BL @A0C64
        JNE A09C8
        BL @A09D6
A0998   CLR R14
A099A   INC R0
        MOVB *R0, R14
        MOV R3, R2
        S R0, R2
        DEC R2
        SLA R2, 7
        JNO A09B6
        DEC R0
        XOP R0, R10
        XOP @>00D1, R14
        XOP @A00F2, R14
        JMP A09BE
A09B6   MOVB R2, *R0
        DEC R0
        BL @A09E6
A09BE   SRA R14, 7
        INCT R14
        JEQ A09C8
        A R14, R0
        JMP A0998
A09C8   LI R0, >EC52
        CLR R4
        BL @A0C9C
A09D2   B @A07C8
A09D6   MOV *R2, R0
        CLR @>FFFE(R2)
        DEC @>EC4C
        MOV @>EC54, R3
        RT
A09E6   XOP R0, R10
        XOP @>0A05, R12
        XOP *R0, R10
        XOP @A00F2, R14
        RT
A09F4   XOP @>001F, R14
        XOP @>EC4C, R10
        B @A0142
        BL *R10
        CI R4, >002A
        JEQ A0A3C
        CI R4, >0040
        JNE A0A52
        BL @A0B14
        MOV R8, R2
        A R9, R2
        MOV R6, *R2
        INCT R8
        LI R6, >0020
        CI R4, >0028
        JNE A0A34
        BL @A0AD0
        ORI R6, >0020
        CI R4, >0029
        JNE A0AC4
        BL *R10
A0A34   MOV R0, R0
        JNE A0A3A
        SLA R6, 6
A0A3A   JMP A0ABC
A0A3C   BL @A0AD0
        ORI R6, >0010
        CI R4, >002B
        JNE A0A50
        BL *R10
        ORI R6, >0030
A0A50   JMP A0A34
A0A52   LI R14, A0A34
        MOV R14, @>EC5E
        B @A0AD6
        BL @A0AD0
        SLA R6, 4
        JMP A0ABC
        MOV R6, R0
        CI R0, >0030
        JNE A0A76
        CI R4, >000D
        JEQ A0A98
        SETO R14
A0A76   BL @A0B14
        CI R0, >0030
        JEQ A0A96
        MOV R9, R2
        A R8, R2
        MOV R6, *R2
        INCT R8
        CI R0, >0026
        JNE A0A98
        SETO R14
        CI R4, >002C
        JEQ A0A76
A0A96   MOV R6, R14
A0A98   B *R15
        BL @A0AD0
        JMP A0A34
        BL @A0B14
        MOV R9, R2
        INCT R2
        S R2, R6
        SRA R6, 1
A0AAC   CI R6, >007F
        JGT A0AC0
        CI R6, >FF80
        JLT A0AC0
        ANDI R6, >00FF
A0ABC   SOC R6, *R9
        B *R15
A0AC0   XOP @>00D6, R14
A0AC4   B @A091A
        SETO R14
        BL @A0B14
        JMP A0AAC
A0AD0   MOV R11, @>EC5E
        BL *R10
A0AD6   LI R12, A0B04
        CI R4, >0052
        JEQ A0AF8
        CI R4, >003A
        JLT A0AFA
        CI R4, >003E
        JEQ A0AFE
        LI R14, >FFFE
        LI R13, A0B04
        B @A0B18
A0AF8   BL *R10
A0AFA   B @A0C2A
A0AFE   BL *R10
        B @A0C0C
A0B04   MOV R5, R5
A0B06   JLT A0AC4
        CI R6, >0010
        JHE A0AC4
        MOV @>EC5E, R11
        RT
A0B14   MOV R11, R13
        BL *R10
A0B18   CLR @>EC50
        CI R4, >0027
        JEQ A0B30
A0B22   CI R4, >002D
        JNE A0B48
        INV R13
A0B2A   INCT R14
        BL *R10
        JMP A0B52
A0B30   CLR R6
        CLR R14
A0B34   BL *R10
        CI R4, >0027
        JEQ A0B44
        SWPB R6
        MOVB R6, R4
        MOV R4, R6
        JMP A0B34
A0B44   BL *R10
        JMP A0BCE
A0B48   CI R4, >002B
        JEQ A0B2A
        MOV R14, R14
        JGT A0BE8
A0B52   CI R4, >0024
        JNE A0B5E
        MOV R9, R6
A0B5A   BL *R10
        JMP A0BCC
A0B5E   CI R4, >003E
        JNE A0B6C
        BL *R10
        BL @A0C0A
        JMP A0BCC
A0B6C   BL @A0C34
        JLT A0AC4
        JEQ A0BBE
        BL @A0C28
        JMP A0BCC
A0B7A   MOV R14, R14
        JNE A0AC4
        MOV *R9, R1
        CLR R2
        BL @A0C86
        MOV R9, *R3
        ANDI R1, >F000
        CI R1, >1000
        JNE A0BB4
        ORI R4, >0080
        MOV R9, R6
A0B98   DECT R3
        MOV R4, *R3
        C @>EC56, @>EC5A
        JNE A0BAA
        MOV @>EC58, R6
        JMP A0BCE
A0BAA   BL @>0C64
        JNE A0BB2
        MOV *R2, R6
A0BB2   JMP A0BCE
A0BB4   A R8, *R3
        ORI R4, >8000
        CLR R6
        JMP A0B98
A0BBE   BL @A0BEE
        MOV R7, R4
        BL @A0C64
        JNE A0B7A
        MOV *R2, R6
A0BCC   INCT R14
A0BCE   MOV @>EC60, R4
        MOV R13, R13
        JGT A0BDA
        NEG R6
        INV R13
A0BDA   MOV R5, R5
        JLT A0B06
        MOV R14, R14
        JEQ A0BEC
        A R6, @>EC50
        JMP A0B22
A0BE8   MOV @>EC50, R6
A0BEC   B *R13
A0BEE   MOV R11, R12
        LI R7, >0031
A0BF4   BL @A0C34
        JLT A0C32
        JEQ A0C02
        SLA R7, 8
        JNO A0C32
        JMP A0C04
A0C02   SLA R7, 8
A0C04   A R4, R7
        BL *R10
        JMP A0BF4
A0C0A   MOV R11, R12
A0C0C   LI R2, >0010
A0C10   CLR R6
        SETO R5
A0C14   BL @A0C34
        JLT A0C32
        C R3, R2
        JHE A0C30
        MOV R6, R5
        MPY R2, R5
        A R3, R6
        BL *R10
        JMP A0C14
A0C28   MOV R11, R12
A0C2A   LI R2, >000A
        JMP A0C10
A0C30   SETO R5
A0C32   B *R12
A0C34   SETO R1
        MOV R4, R3
        CI R4, >0024
        JEQ A0C4A
        AI R3, >FFD0
        JNC A0C60
        CI R3, >0009
        JGT A0C4E
A0C4A   NEG R1
        RT
A0C4E   AI R3, >FFF9
        CI R3, >000A
        JL A0C60
        CI R3, >0023
        JH A0C60
        CLR R1
A0C60   MOV R1, R1
        RT
A0C64   SETO R3
        MOV @>EC4E, R1
        JEQ A0C82
        SLA R1, 2
        LI R2, >EC62
        A R2, R1
        CLR R3
A0C76   INCT R2
        C R4, *R2+
        JEQ A0C82
        C R2, R1
        JL A0C76
        INC R3
A0C82   MOV R3, R3
        RT
A0C86   LI R3, >EC58
        C R2, *R3
        JEQ A0C98
        LI R3, >EC5C
        C R2, *R3
        JEQ A0C98
        CLR R3
A0C98   RT
A0C9A   MOV *R0, R4
A0C9C   MOV R11, R12
A0C9E   BL @A0C64
        JEQ A0CBE
        MOV R4, R4
        JEQ A0CB0
        CLR R4
        INC @>EC4C
        JMP A0C9E
A0CB0   INC @>EC4E
        MOV @>EC4E, R2
        SLA R2, 2
        AI R2, >EC62
A0CBE   DECT R2
        MOV *R0+, *R2+
        MOV *R0+, *R2
        B *R12
; THIS COULDE BE DATA?
A0CC6   DATA >0000
        DATA >0A00
        DATA >0A9A
        DATA >0A66
        DATA >0A5E
        DATA >0AA0
        DATA >0AC8
        DATA >088E
A0CD6   DATA >0905
        DATA >0A0A
        DATA >1408
        DATA >0013
        DATA >0A06
        DATA >0310
        DATA >0307
        DATA >0122
        DATA >5329
        DATA >AEC4
        DATA >69AF
        DATA >D267
        DATA >022C
        DATA >D770
        DATA >B353
        DATA >0322
        DATA >29AB
        DATA >CF6E
        DATA >66AC
        DATA >52AF
        DATA >43BA
A0D00   DATA >4384
        DATA >A1D4
        DATA >61A5
        DATA >4374
        DATA >A956
        DATA >7385
        DATA >AE44
        DATA >B155
        DATA >89A4
        DATA >CC65
        DATA >AE43
        DATA >7456
        DATA >8AA5
        DATA >51A7
        DATA >5428
        DATA >452C
        DATA >4554
        DATA >AD50
        DATA >AE43
        DATA >454F
        DATA >AF43
        DATA >508C
        DATA >A4C3
        DATA >7229
        DATA >CD69
        DATA >B2C5
        DATA >78B3
        DATA >54B7
        DATA >5069
        DATA >8DAF
        DATA >5662
        DATA >B059
        DATA >738E
        DATA >A547
        DATA >AF50
        DATA >8FB2
        DATA >4992
        DATA >B3C5
        DATA >74B4
        DATA >D770
        DATA >1322
        DATA >4F5A
        DATA >A5D4
        DATA >6FAC
        DATA >41AF
        DATA >4362
        DATA >B241
        DATA >434C
        DATA >B4C3
        DATA >72D3
        DATA >74D7
        DATA >70B7
        DATA >D062
        DATA >BA43
        DATA >6294
        DATA >22A5
        DATA >D874
        DATA >18AF
        DATA >5052
A0D76   DATA >0000
        DATA >A000
        DATA >B000
        DATA >0745
        DATA >0227
        DATA >0247
        DATA >000D
        DATA >0445
        DATA >0685
        DATA >0405
        DATA >000D
        DATA >8000
        DATA >9000
        DATA >0287
        DATA >03A6
        DATA >03C6
        DATA >04C5
        DATA >2002
        DATA >2402
        DATA >000A
        DATA >0605
        DATA >0645
        DATA >3C08
        DATA >0185
        DATA >000C
        DATA >0006
        DATA >0346
        DATA >0585
        DATA >05C5
        DATA >0545
        DATA >1301
        DATA >1501
        DATA >1B01
        DATA >1401
        DATA >1A01
        DATA >1201
        DATA >1101
        DATA >1001
        DATA >1701
        DATA >1601
        DATA >1901
        DATA >1801
        DATA >1C01
        DATA >3003
        DATA >0207
        DATA >030A
        DATA >03E6
        DATA >008B
        DATA >009B
        DATA >02EA
        DATA >C000
        DATA >D000
        DATA >3808
        DATA >01C5
        DATA >0505
        DATA >1006
        DATA >0267
        DATA >0366
        DATA >0386
        DATA >6000
        DATA >7000
        DATA >1D09
        DATA >1E09
        DATA >0705
        DATA >0A04
        DATA >E000
        DATA >F000
        DATA >0804
        DATA >0B04
        DATA >0904
        DATA >3403
        DATA >02CB
        DATA >02AB
        DATA >06C5
        DATA >4000
        DATA >5000
        DATA >1F09
        DATA >0006
        DATA >0485
        DATA >2C08
        DATA >2802
        DATA >0000

******************************************************************************
* XCL (Terminal Mode???????) command.                                        *
* No operands.                                                               *
******************************************************************************

A0E1A   LWPI >EC00                                Initialise workspace pointer.

        LI R3,>EC4C

        LI R5,>ED00                               Some sort of buffer from >ED00 to >EFFE????????????

        LI R6,>EFFE

        MOV R5,*R3+
        MOV R6,*R3+
        MOV R5,*R3+
        CLR R5
        CLR *R3
        LI R9,>EC2E                               TMS9902 serial port previously stored at >EC2E.

        MOV @>EC44,@>EC54                         Save state of ASR flag for 1st RS-232 port.
        LI R12,>0400                              Address 2nd RS-232 port.

        SBO 31                                    Reset TMS9902.
        LDCR @A013B,8                             Initialise TMS9902 control register.

        SBZ 13                                    Do not initalise interval register.
        LDCR @A0094+2,12                          Set to 1200 Baud.

        SBO 16                                    Set /RTS active.
        LI R12,>0000                              Address 1st RS-232 port.

        SBO 14                                    Set to load control register.
        LDCR @A013B,8                             Set 1st RS-232 port to 1200 Baud also.

        CLR R1
        CLR R2
        MOV @A013E,*R9                            1st TMS9902 CRU base address.

        XOP @A00E3,14                             Print "TERMINAL MODE" to 1st RS-232 port.

        INCT @>EC44                               ASR flag.

A0E6A   LI R12,>0000                              Address 1st RS-232 port.

        TB 21                                     Character received on 1st RS-232 port?
        JEQ A0EDA                                 Yes, jump.
        LI R12,>0400                              No, address 2nd RS-232 port.

        TB 21                                     Character received on 2nd RS-232 port?
        JEQ A0E80                                 Yes, jump.
        MOV R1,R1
        JNE A0ED6
        JMP A0E6A                                 Loop round and check for character on 1st RS-232 port again.

*Character received on 2nd RS-232 port.

A0E80   MOV @A0140,*R9                            2nd TMS9902 CRU base address.

        MOV R2,R2                                 (R2 set to FFFF if character received on 2nd RS-232 port is not <DLE> or <DC2>)
        JNE A0ED2
        XOP R10,13                                Read character into R10.
        CI R10,>0000                              Is character a null?

        JEQ A0E6A                                 Yes, ignore and check for character on 1st RS-232 port again.
        CI R10,>7F00                              No, is character <DEL>?

        JEQ A0E6A                                 Yes, ignore and check for character on 1st RS-232 port again.
        CI R10,>1000                              No, is character <DLE>?

        JNE A0EB6                                 No, jump.
A0E9C   XOP R10,13                                Yes, wait for next character and read into R10.
        CI R10,>0000                              Is next character a null?

        JEQ A0E9C                                 Yes, ignore and check for character again.
        CI R10,>3700
*                                                   No, is character a '7'?
        JEQ A0EC6                                 Yes, jump.
        CI R10,>3C00                              No, is character a '<'?

        JNE A0E6A                                 No, ignore and check for character on 1st RS-232 port again.
        XOP @A013C,12                             Write character

        JMP A0E6A                                 Check for character on 1st RS-232 port again.

*                                    (Character received on 2nd RS-232 port is not <DLE>)

A0EB6   CI R10,>1200                              Is character <DC2>?

        JNE A0EC0                                 No, jump.
A0EBC   B @>1070                                  Yes, branch.

A0EC0   CI R10,>1100
*                                                   Is character <DC1>?
        JNE A0ECA                                 No, jump.

A0EC6   B @A0FA0                                  Yes, branch.

A0ECA   MOV @A013E,*R9                            1st TMS9902 CRU base address.

        XOP R10,12                                Write character from 2nd RS-232 port to 1st port.
        JMP A0E6A                                 Check for character on 1st RS-232 port again.

A0ED2   B @>107C                                  Branch.


A0ED6   B @A0FC8                                  Branch.


*Character received on 1st RS-232 port.

A0EDA   MOV @A013E,*R9                            1st TMS9902 CRU base address.

        XOP R10,13                                Read character into R10.
        CI R10,>1A00                              Is character <SUB>?

        JNE A0EF0                                 No, jump.
        MOV @>EC54,@>EC44                         Yes, restore state of ASR flag for 1st RS-232 port.
        B @A0142                                  Return to monitor.

*                                    (Character received on 1st RS-232 port is not <SUB>)

A0EF0   CI R10,>0300                              Is character <ETX>?

        JEQ A0F16                                 Yes, jump.
        CI R10,>1200                              No, is character <DC2>?

        JEQ A0EBC                                 Yes, jump.
        CI R10,>1400                              No, is character <DC4>?

        JNE A0F06                                 No, jump.
        B @A10FE                                  Yes, jump.

A0F06   MOV R1,R1
        JNE A0ED6
        MOV R2,R2
        JNE A0ED2
        MOV @A0140,*R9                            2nd TMS9902 CRU base address.

        XOP R10,12                                Write character to 2nd RS-232 port.
        JMP A0E6A                                 Loop round and check for character on ports again.

*                                    (Character received on 1st RS-232 port is <ETX>)
A0F16   MOV @A013E, *R9
        MOV @>EC54, @>EC44
        XOP @A00D8, R14
        XOP R10, R11
        XOP @A00DE, R14
        BL @A0F42
        SZCB R0, *R4
        DATA >0F7E
        SZC R0, *R0
        DATA >0F94
        SZCB R0, *R0
        DATA >0F5A
        SZCB R0, R4
        DATA >0E5A
        DATA >0000
A0F40   INCT R11
A0F42   MOV *R11, R0
        JEQ A0F4E
        C *R11+, R10
        JNE A0F40
        MOV *R11, R11
        RT
A0F4E   XOP @A00F5, R14
        JMP A0F16
A0F54   XOP @>00FF, R14
        JMP A0F16
        XOP R10, R9
        DATA >0F16
        DATA >0F54
        SLA R10, 2
        DECT R10
        JLE A0F54
        CI R10, >0023
        JHE A0F54
        LI R12, >0400
        SBO 11
        SBO 12
        LDCR @A0084(R10), R12
        SBZ 11
        SBZ 12
        JMP A0F16
        XOP R10, R9
        DATA >0F88
        DATA >0F54
        MOV R10, @>EC50
        XOP R10, R9
        DATA >0F16
        DATA >0F54
        MOV R10, @>EC4E
        JMP A0F16
        XOP R10, R9
        DATA >0F16
        DATA >0F54
        MOV R10, @>EC4C
        JMP A0F16
A0FA0   SETO R1
        MOV @>EC52, R7
        JLT A1002
        JGT A1010
        MOV @>EC50, R7
        C R7, @>EC4E
        JH A1022
        MOV @A0140, *R9
        CLR R5
        CLR R3
        XOP @>0137, R14
        MOV R7, R10
        BL @A1042
        MPY R0, R4
A0FC8   MOV *R7, R10
        BL @A1042
        SZC R0, R8
        C R7, @>EC4E
        JL A0FDC
        SETO @>EC52
        JMP A0FE4
A0FDC   INCT R7
        CI R3, A003C
        JLT A101E
A0FE4   AI R5, >0037
        MOV R5, R10
        NEG R10
        BL @A1042
        STCR R0, R12
        MOV R7, @>EC50
        XOP @A0128, R14
A0FFA   MOV @>EC54, R5
        JLT A101C
        JMP A0FA0
A1002   XOP @>0137, R14
        XOP @>00E0, R14
        INCT @>EC52
        JMP A0FFA
A1010   XOP @>0137, R14
        XOP @>0131, R14
A1018   CLR @>EC52
A101C   CLR R1
A101E   B @>0E6A
A1022   XOP @>0137, R14
        XOP @>0131, R14
        MOV @>013E, *R9
        MOV @>EC54, @>EC44
        XOP @>011F, R14
        INCT @>EC44
        SETO @>EC4C
        JMP A1018
A1042   MOV *R11+, R0
        XOP R0, R12
        SRL R0, 8
        A R0, R5
        XOP R10, R10
        AI R3, >0005
        LI R0, >0004
A1054   SRC R10, 4
        MOV R10, R6
        SRL R6, 12
        A R6, R5
        AI R5, >0030
        CI R6, >000A
        JL A106A
        AI R5, >0007
A106A   DEC R0
        JNE A1054
        RT
*                         Character received on 2nd RS-232 port is not <DLE> or <DC2>.

A1070   SETO R2
        MOV @A0140,*R9                            2nd TMS9902 CRU base address.
        MOV @>EC4C,R0
        CLR R7

A107C   XOP R6,13                                 Read character into R6.
        CI R6,>1400                               Is character <DC4>?
        JEQ A10FE                                 Yes, jump.
        MOV @>EC52,R8
        JNE A1138
        CI R6,>2000                               Is character a control character?
        JLT A107C                                 Yes, loop round and read another character.
A1090   CI R6,>5F00                               Is character higher in ASCII than "_"?
        JGT A107C                                 Yes, loop round and read another character.
*                                    (So character is between >20 and >5F)
        SETO R5
        CLR R10
        BL @A073A                                 Check if character in R6 is a hex digit. Add it to R10 if so.
        JMP A10B6                                 (Returns here if character in R6 is not a hex digit)

A10A0   MOVB @>1152(R10), R8                      (RETURNS HERE IF CHARACTER IN R6 IS A HEX DIGIT)
        JEQ >110A
        BL @>0722
        JMP A10C8

        LI R5, >0008
        SRA R8, 7
A10B2   B @A10B2(R8)
A10B6   CI R6,>0047                               R6 has been reversed?
        JLT A10C8
        CI R6, >004A
        JGT A1106
        AI R6, >FFC9
        JMP A10A0
A10C8   CI R6, >003A
        JNE A1106
        JMP A114C
A10D0   LI R12, >0400
A10D4   CLR R5
A10D6   TB 15
        JNE A10D4
        DEC R5
        JNE A10D6
        MOV @>013E, *R9
        MOV @>EC54, @>EC44
        SETO @>EC4C
        MOV R0, R0
        JEQ A10F6
        XOP @>010A, R14
        JMP A10FA
A10F6   XOP @>0115, R14
A10FA   INCT @>EC44
A10FE   CLR R2
        CLR @>EC52
        JMP A1148
A1106   CLR R0
        JMP A10D0
A110A   XOP R6, R13
        CB R6, @>00F2
        JNE A110A
        CLR R7
        JMP A1148
        A R10, R7
        JEQ A1148
        SETO R0
        JMP A10D0
        A R0, R10
        MOV R10, R3
        JMP A1148
        A R0, R10
        MOV R10, *R3+
        JMP A1148
        DECT R5
A112C   XOP R6, R13
        SRL R6, 8
        JEQ A112C
        A R6, R7
        DEC R5
        JNE A112C
A1138   JMP A1148
        A R0, R10
        MOV R10, @>EC1C
        JMP A1148
        ANDI R10, >FFFE
        MOV R10, R0
A1148   B @A0E6A
A114C   SETO @>EC52
        JMP A1148
        DIV R5, R5
        SZC *R12+, *R0
        DIV *R12+, R0
        DIV *R2+, R0
        SOC *R7+, *R0
        STCR *R10+, R8
        MPY R8, R5
        XOR R0, R8
        DIV *R12+, R0
        DIV R11, R6
        MOV R1, R5
A1168   LI R8, A12D8
        LI R9, A12BA
        LI R7, A12A6
        XOP @>00F2, R14
        LI R6, >202C
        MOV *R0, R1
        XOP R0, R10
        XOP R6, R12
        XOP R1, R10
        XOP R6, R12
        CLR R3
        MOV *R0, R1
        ANDI R1, >FFF0
A118E   MOV @A13B4(R3), R10
        MOV R10, R2
        JEQ A11E8
        ANDI R10, >FFF0
        C R1, R10
        JHE A11A2
        DECT R3
        JMP A118E
A11A2   MOV *R0, R1
A11A4   SLA R3, 1
        AI R3, A14D6
        S R10, R1
        ANDI R2, >000F
        MOVB @A11BA(R2), R2
        SRL R2, 7
        B @A11BA(R2)
A11BA   DEC R12
        DEC *R15
        COC @>3337(R11), R12
        DEC *R15+
        SZC R5, R5
        BL *R7
        BL *R8
A11CA   XOP R6, R12
        SRL R1, 6
A11CE   BL *R8
        JMP A124C
        MOV R1, R1
        JNE A11DC
        LI R3, >14DA
        JMP A1224
A11DC   SWPB R1
        SRA R1, 7
        INCT R1
        A R0, R1
        BL *R7
        JMP A11F0
A11E8   MOV *R0, R1
        LI R3, A14DE
        BL *R7
A11F0   XOP @A14F5, R12
        XOP R1, R10
        JMP A124C
A11F8   BL *R7
        BL *R8
        MOV R9, R8
        JMP A11CA
        MOVB R1, R1
        JNE A11E8
        SRC R1, 4
        MOVB R1, R2
        SLA R1, 6
        SRL R2, 12
        A R2, R1
        JMP A11F8
A1210   C *R0, @>1320
        JNE A121C
        LI R3, >14EE
        JMP A1224
A121C   BL *R7
        JMP A11CE
        MOV R1, R1
        JNE A11E8
A1224   BL *R7
        JMP A124C
        SLA R1, 12
        JOC >11E8
        SRL R1, 12
        BL *R7
        BL *R8
        XOP R6, R12
A1234   MOV *R0+, R1
        JMP A11F0
        BL *R7
        JMP A126E
        MOV R1, R1
        JNE A11E8
        BL *R7
        JMP A1234
        SLA R1, 12
        JOC A11E8
        SRL R1, 12
        JMP A1210
A124C   MOV R5, R5
        JNE A1256
A1250   MOV R0, *R13
        B @>0142
A1256   MOV @>013E, R12
        TB 21
        JNE A1266
        XOP R2, R13
        CI R2, >2F00
        JEQ A1250
A1266   C R0, R5
        JH A1250
        B @A1168
A126E   LI R3, >2D31
        SWPB R1
        SRA R1, 8
        JEQ A12A2
        JGT A127E
        XOP R3, R12
        NEG R1
A127E   CI R1, >0064
        JLT A128C
        SLA R3, 8
        XOP R3, R12
        AI R1, >FF9C
A128C   LI R4, >000A
        MOV R1, R2
        CLR R1
        DIV R4, R1
        SLA R3, 8
        JEQ A129E
        MOV R1, R1
        JEQ A12A0
A129E   BL *R9
A12A0   MOV R2, R1
A12A2   BL *R9
        JMP A124C
A12A6   LI R2, >0004
A12AA   XOP *R3, R12
        INC R3
        DEC R2
        JNE A12AA
        XOP R6, R12
        SWPB R6
        INCT R0
        RT
A12BA   MOV R1, R3
        SLA R3, 12
        SRL R3, 4
        CI R3, >0900
        JLE A12CC
        SWPB R3
        AI R3, >0126
A12CC   AI R3, >3000
A12D0   XOP R3, R12
        SLA R3, 8
        JNE A12D0
A12D6   RT
A12D8   LI R4, >2A52
        MOV R1, R2
        SLA R2, 10
        SRL R2, 14
        JNE A12EA
A12E4   SWPB R4
        XOP R4, R12
        JMP A12BA
A12EA   DEC R2
        JNE A12F2
        XOP R4, R12
        JMP A12E4
A12F2   DEC R2
        JNE A1316
        XOP @>14F4, R14
        XOP *R0+, R10
        MOV R1, R2
        SLA R2, 12
        JEQ A12D6
        LI R2, >2829
        XOP R2, R12
        SWPB R4
        XOP R4, R12
A130C   MOV R11, R12
        BL *R9
        SWPB R2
        XOP R2, R12
        B *R12
A1316   XOP @A14F7, R14
        LI R2, >002B
        JMP A130C
        RT

A1322   DATA >0000,>008B,>009B,>0185,>01C5,>0207,>0227
A1330   DATA >0247,>0267,>0287,>02AB,>02CB,>02EA,>030A,>0346
A1340   DATA >0366,>0386,>03A6,>03C6,>03E6,>0405,>0445,>0485 ;.f...........E..
A1350   DATA >04C5,>0505,>0545,>0585,>05C5,>0605,>0645,>0685 ;.....E.......E..
A1360   DATA >06C5,>0705,>0745,>0804,>0904,>0A04,>0B04,>1001 ;.....E..........
A1370   DATA >1101,>1201,>1301,>1401,>1501,>1601,>1701,>1801 ;................
A1380   DATA >1901,>1A01,>1B01,>1C01,>1D09,>1E09,>1F09,>2002 ;.............. .
A1390   DATA >2402,>2802,>2C03,>3003,>3403,>3808,>3C08,>4000 ;$.(.,.0.4.8.<.@.
A13A0   DATA >5000,>6000,>7000,>8000,>9000,>A000,>B000,>C000 ;P.`.p...........
A13B0   DATA >D000,>E000                          ;....
A13B4   DATA >F000,>4C53,>5420,>4C57,>5020,>4449  ;..LST LWP DI
A13C0   DATA >5653,>4D50,>5953,>4C49,>2020,>4149,>2020,>414E ;VSMPYSLI  AI  AN
A13D0   DATA >4449,>4F52,>4920,>4349,>2020,>5354,>5750,>5354 ;DIORI CI  STWPST
A13E0   DATA >5354,>4C57,>5049,>4C49,>4D49,>4944,>4C45,>5253 ;STLWPILIMIIDLERS
A13F0   DATA >4554,>5254,>5750,>434B,>4F4E,>434B,>4F46,>4C52 ;ETRTWPCKONCKOFLR
A1400   DATA >4558,>424C,>5750,>4220,>2020,>5820,>2020,>434C ;EXBLWPB   X   CL
A1410   DATA >5220,>4E45,>4720,>494E,>5620,>494E,>4320,>494E ;R NEG INV INC IN
A1420   DATA >4354,>4445,>4320,>4445,>4354,>424C,>2020,>5357 ;CTDEC DECTBL  SW
A1430   DATA >5042,>5345,>544F,>4142,>5320,>5352,>4120,>5352 ;PBSETOABS SRA SR
A1440   DATA >4C20,>534C,>4120,>5352,>4320,>4A4D,>5020,>4A4C ;L SLA SRC JMP JL
A1450   DATA >5420,>4A4C,>4520,>4A45,>5120,>4A48,>4520,>4A47 ;T JLE JEQ JHE JG
A1460   DATA >5420,>4A4E,>4520,>4A4E,>4320,>4A4F,>4320,>4A4E ;T JNE JNC JOC JN
A1470   DATA >4F20,>4A4C,>2020,>4A48,>2020,>4A4F,>5020,>5342 ;O JL  JH  JOP SB
A1480   DATA >4F20,>5342,>5A20,>5442,>2020,>434F,>4320,>435A ;O SBZ TB  COC CZ
A1490   DATA >4320,>584F,>5220,>584F,>5020,>4C44,>4352,>5354 ;C XOR XOP LDCRST
A14A0   DATA >4352,>4D50,>5920,>4449,>5620,>535A,>4320,>535A ;CRMPY DIV SZC SZ
A14B0   DATA >4342,>5320,>2020,>5342,>2020,>4320,>2020,>4342 ;CBS   SB  C   CB
A14C0   DATA >2020,>4120,>2020,>4142,>2020,>4D4F,>5620,>4D4F ;A   AB  MOV MO
A14D0   DATA >5642,>534F,>4320                    ;VBSOC
A14D6   DATA >534F,>4342,>4E4F,>5020              ;SOCBNOP
A14DE   DATA >4441                                ;DA
A14E0   DATA >5441,>5445,>5854,>414F,>5247,>454E,>4420,>5254 ;TATEXTAORGEND RT
A14F0   DATA >2020,>0000
A14F4   BYTE >40
A14F5   BYTE >3E
A14F6   BYTE >00
A14F7   BYTE >2A
A14F8   DATA >5200,>FFFF,>FFFF,>FFFF

        END
