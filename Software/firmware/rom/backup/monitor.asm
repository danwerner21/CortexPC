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
A0020   TEXT '  '
A0021   BYTE >00

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

A0084   DATA >0009,>001A                          19200 BAUD
        DATA >0012,>0034                          9600 BAUD
        DATA >0023,>0068                          4800 BAUD
        DATA >0046,>00D0                          2400 BAUD
A0094   DATA >008D,>01A1                          1200 BAUD
        DATA >0119,>0341                          600 BAUD (I assume)
        DATA >02A4,>04D0                          300 BAUD
        DATA >7FFF,>0638                          110 BAUD

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
A00CE   TEXT '2='
A00CF   BYTE >00
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
        XOP @A00AD,14                             Print logon message.
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
        CB R11,@A00F2                             Carriage return?
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
        A R5, >0030
        C R12, >000A
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
A07C6   LI R10,A0850
A07C8   LI R0,>EC52
A07D0   LI R8, >0006
A07D4   CLR *R0+
A07D6   DEC R8
A07D8   JGT A07D4
A07DA   XOP R9, R10
A07DC   XOP @A001C, R14
A07E0   BL *R10
A07E2   CI R4, A0020
A07E6   JEQ A0814
A07E8   CI R4, >002A
A07EC   JNE A07F8
A07EE   BL *R10
A07F0   CI R4, >000D
A07F4   JNE A07EE
A07F6   JMP A07C4
A07F8   BL @A0BEE
A07FC   MOV R7, @>EC52
A0800   MOV R9, @>EC54
A0804   MOV R7, R4
A0806   BL @A0C64
A080A   JEQ A084E
A080C   CB R7, @>0BF3
A0810   JEQ A0818
A0812   JMP A081C
A0814   XOP @A0021, R14
A0818   XOP @A0021, R14
A081C   LI R7, >0CE3
A0820   CLR R5
A0822   CLR R6
A0824   BL *R10
A0826   BL @A0C34
A082A   JNE A08B2
A082C   SLA R4, 11
A082E   INC R7
A0830   MOVB *R7, R0
A0832   JLT A0838
A0834   JEQ A091A
A0836   INCT R6
A0838   SLA R0, 1
A083A   SRL R0, 14
A083C   C R5, R0
A083E   JLT A082E
A0840   JGT A091A
A0842   MOVB *R7, R0
A0844   SLA R0, 3
A0846   CB R0, R4
A0848   JNE A082E
A084A   INC R5
A084C   JMP A0824
A084E   JMP A091A
A0850   XOP R4, R13
A0852   CI R4, >1B00
A0856   JEQ A07C4
A0858   CI R4, >2000
A085C   JL A0860
A085E   XOP R4, R12
A0860   SRL R4, 8
A0862   MOV R4, @>EC60
A0866   RT
A0868   BL *R10
A086A   CI R4, >0027
A086E   JNE A091A
A0870   MOV R9, R7
A0872   SB *R7, *R7
A0874   INC R8
A0876   BL *R10
A0878   CI R4, >0027
A087C   JEQ A092E
A087E   SWPB R4
A0880   MOVB R4, *R7+
A0882   JMP A0872
A0884   BL @A0B14
A0888   MOV R6, @>EC54
A088C   JMP A0956
A088E   SETO R14
A0890   MOV R6, R0
A0892   BL @A0B14
A0896   MOV R9, @>EC54
A089A   CI R0, >0014
A089E   JNE A08AA
A08A0   A R9, R6
A08A2   JEQ A08AA
A08A4   INC R6
A08A6   C R6, R9
A08A8   JL A091A
A08AA   MOV R6, R9
A08AC   ANDI R9, >FFFE
A08B0   JMP A0956
A08B2   MOV R5, R5
A08B4   JEQ A0814
A08B6   MOVB *R7, R0
A08B8   JLT A091A
A08BA   SETO R14
A08BC   CI R6, >0032
A08C0   JEQ A0884
A08C2   CI R6, >009A
A08C6   JEQ A0868
A08C8   MOV @A0D76(R6), R0
A08CC   MOV R0, R1
A08CE   ANDI R1, >FFF0
A08D2   JEQ A08D8
A08D4   MOV R1, *R9
A08D6   INCT R8
A08D8   MOV R0, R1
A08DA   ANDI R1, >000F
A08DE   MOVB @A0CD6(R1), R0
A08E2   SWPB R0
A08E4   ORI R0, >FFE0
A08E8   MOV R0, R1
A08EA   SRL R1, 2
A08EC   ANDI R1, >0006
A08F0   MOV @A0CC6(R1), R1
A08F4   JEQ A0904
A08F6   CI R4, >0020
A08FA   JNE A091A
A08FC   CLR R14
A08FE   LI R15, A0904
A0902   BL *R1
A0904   MOV R0, R1
A0906   SLA R1, 13
A0908   SRL R1, 12
A090A   MOV @>0CC6(R1), R1
A090E   JEQ A0922
A0910   CLR R0
A0912   CLR R14
A0914   LI R15, A0922
A0918   BL *R1
A091A   XOP @>00D1, R14
A091E   B @A07C4
A0922   CI R4, >000D
A0926   JEQ A0936
A0928   CI R4, >0020
A092C   JNE A091A
A092E   BL *R10
A0930   CI R4, >000D
A0934   JNE A092E
A0936   CI R0, >0030
A093A   JEQ A09F4
A093C   XOP @>00F2, R12
A0940   XOP R9, R10
A0942   MOV R9, R2
A0944   BL @A0C86
A0948   LI R4, >2052
A094C   MOV R3, R3
A094E   JEQ A0952
A0950   SWPB R4
A0952   XOP R4, R12
A0954   XOP *R9+, R10
A0956   XOP @A00F2, R14
A095A   DECT R8
A095C   JGT A0940
A095E   LI R0, >EC56
A0962   BL @A0C9A
A0966   BL @A0C9A
A096A   MOV @>EC52, R4
A096E   JEQ A09D2
A0970   AI R4, >8000
A0974   BL @A0C64
A0978   JNE A098A
A097A   BL @A09D6
A097E   MOV *R0, R14
A0980   MOV R3, *R0
A0982   BL @A09E6
A0986   MOV R14, R0
A0988   JNE A097E
A098A   AI R4, >8080
A098E   BL @A0C64
A0992   JNE A09C8
A0994   BL @A09D6
A0998   CLR R14
A099A   INC R0
A099C   MOVB *R0, R14
A099E   MOV R3, R2
A09A0   S R0, R2
A09A2   DEC R2
A09A4   SLA R2, 7
A09A6   JNO A09B6
A09A8   DEC R0
A09AA   XOP R0, R10
A09AC   XOP @>00D1, R14
A09B0   XOP @A00F2, R14
A09B4   JMP A09BE
A09B6   MOVB R2, *R0
A09B8   DEC R0
A09BA   BL @A09E6
A09BE   SRA R14, 7
A09C0   INCT R14
A09C2   JEQ A09C8
A09C4   A R14, R0
A09C6   JMP A0998
A09C8   LI R0, >EC52
A09CC   CLR R4
A09CE   BL @A0C9C
A09D2   B @A07C8
A09D6   MOV *R2, R0
A09D8   CLR @>FFFE(R2)
A09DC   DEC @>EC4C
A09E0   MOV @>EC54, R3
A09E4   RT
A09E6   XOP R0, R10
A09E8   XOP @>0A05, R12
A09EC   XOP *R0, R10
A09EE   XOP @A00F2, R14
A09F2   RT
A09F4   XOP @>001F, R14
A09F8   XOP @>EC4C, R10
A09FC   B @A0142
A0A00   BL *R10
A0A02   CI R4, >002A
A0A06   JEQ A0A3C
A0A08   CI R4, >0040
A0A0C   JNE A0A52
A0A0E   BL @A0B14
A0A12   MOV R8, R2
A0A14   A R9, R2
A0A16   MOV R6, *R2
A0A18   INCT R8
A0A1A   LI R6, >0020
A0A1E   CI R4, >0028
A0A22   JNE A0A34
A0A24   BL @A0AD0
A0A28   ORI R6, >0020
A0A2C   CI R4, >0029
A0A30   JNE A0AC4
A0A32   BL *R10
A0A34   MOV R0, R0
A0A36   JNE A0A3A
A0A38   SLA R6, 6
A0A3A   JMP A0ABC
A0A3C   BL @A0AD0
A0A40   ORI R6, >0010
A0A44   CI R4, >002B
A0A48   JNE A0A50
A0A4A   BL *R10
A0A4C   ORI R6, >0030
A0A50   JMP A0A34
A0A52   LI R14, A0A34
A0A56   MOV R14, @>EC5E
A0A5A   B @A0AD6
A0A5E   BL @A0AD0
A0A62   SLA R6, 4
A0A64   JMP A0ABC
A0A66   MOV R6, R0
A0A68   CI R0, >0030
A0A6C   JNE A0A76
A0A6E   CI R4, >000D
A0A72   JEQ A0A98
A0A74   SETO R14
A0A76   BL @A0B14
A0A7A   CI R0, >0030
A0A7E   JEQ A0A96
A0A80   MOV R9, R2
A0A82   A R8, R2
A0A84   MOV R6, *R2
A0A86   INCT R8
A0A88   CI R0, >0026
A0A8C   JNE A0A98
A0A8E   SETO R14
A0A90   CI R4, >002C
A0A94   JEQ A0A76
A0A96   MOV R6, R14
A0A98   B *R15
A0A9A   BL @A0AD0
A0A9E   JMP A0A34
A0AA0   BL @A0B14
A0AA4   MOV R9, R2
A0AA6   INCT R2
A0AA8   S R2, R6
A0AAA   SRA R6, 1
A0AAC   CI R6, >007F
A0AB0   JGT A0AC0
A0AB2   CI R6, >FF80
A0AB6   JLT A0AC0
A0AB8   ANDI R6, >00FF
A0ABC   SOC R6, *R9
A0ABE   B *R15
A0AC0   XOP @>00D6, R14
A0AC4   B @A091A
A0AC8   SETO R14
A0ACA   BL @A0B14
A0ACE   JMP A0AAC
A0AD0   MOV R11, @>EC5E
A0AD4   BL *R10
A0AD6   LI R12, A0B04
A0ADA   CI R4, >0052
A0ADE   JEQ A0AF8
A0AE0   CI R4, >003A
A0AE4   JLT A0AFA
A0AE6   CI R4, >003E
A0AEA   JEQ A0AFE
A0AEC   LI R14, >FFFE
A0AF0   LI R13, A0B04
A0AF4   B @A0B18
A0AF8   BL *R10
A0AFA   B @A0C2A
A0AFE   BL *R10
A0B00   B @A0C0C
A0B04   MOV R5, R5
A0B06   JLT A0AC4
A0B08   CI R6, >0010
A0B0C   JHE A0AC4
A0B0E   MOV @>EC5E, R11
A0B12   RT
A0B14   MOV R11, R13
A0B16   BL *R10
A0B18   CLR @>EC50
A0B1C   CI R4, >0027
A0B20   JEQ A0B30
A0B22   CI R4, >002D
A0B26   JNE A0B48
A0B28   INV R13
A0B2A   INCT R14
A0B2C   BL *R10
A0B2E   JMP A0B52
A0B30   CLR R6
A0B32   CLR R14
A0B34   BL *R10
A0B36   CI R4, >0027
A0B3A   JEQ A0B44
A0B3C   SWPB R6
A0B3E   MOVB R6, R4
A0B40   MOV R4, R6
A0B42   JMP A0B34
A0B44   BL *R10
A0B46   JMP A0BCE
A0B48   CI R4, >002B
A0B4C   JEQ A0B2A
A0B4E   MOV R14, R14
A0B50   JGT A0BE8
A0B52   CI R4, >0024
A0B56   JNE A0B5E
A0B58   MOV R9, R6
A0B5A   BL *R10
A0B5C   JMP A0BCC
A0B5E   CI R4, >003E
A0B62   JNE A0B6C
A0B64   BL *R10
A0B66   BL @A0C0A
A0B6A   JMP A0BCC
A0B6C   BL @A0C34
A0B70   JLT A0AC4
A0B72   JEQ A0BBE
A0B74   BL @A0C28
A0B78   JMP A0BCC
A0B7A   MOV R14, R14
A0B7C   JNE A0AC4
A0B7E   MOV *R9, R1
A0B80   CLR R2
A0B82   BL @A0C86
A0B86   MOV R9, *R3
A0B88   ANDI R1, >F000
A0B8C   CI R1, >1000
A0B90   JNE A0BB4
A0B92   ORI R4, >0080
A0B96   MOV R9, R6
A0B98   DECT R3
A0B9A   MOV R4, *R3
A0B9C   C @>EC56, @>EC5A
A0BA2   JNE A0BAA
A0BA4   MOV @>EC58, R6
A0BA8   JMP A0BCE
A0BAA   BL @>0C64
A0BAE   JNE A0BB2
A0BB0   MOV *R2, R6
A0BB2   JMP A0BCE
A0BB4   A R8, *R3
A0BB6   ORI R4, >8000
A0BBA   CLR R6
A0BBC   JMP A0B98
A0BBE   BL @A0BEE
A0BC2   MOV R7, R4
A0BC4   BL @A0C64
A0BC8   JNE A0B7A
A0BCA   MOV *R2, R6
A0BCC   INCT R14
A0BCE   MOV @>EC60, R4
A0BD2   MOV R13, R13
A0BD4   JGT A0BDA
A0BD6   NEG R6
A0BD8   INV R13
A0BDA   MOV R5, R5
A0BDC   JLT A0B06
A0BDE   MOV R14, R14
A0BE0   JEQ A0BEC
A0BE2   A R6, @>EC50
A0BE6   JMP A0B22
A0BE8   MOV @>EC50, R6
A0BEC   B *R13
A0BEE   MOV R11, R12
A0BF0   LI R7, >0031
A0BF4   BL @A0C34
A0BF8   JLT A0C32
A0BFA   JEQ A0C02
A0BFC   SLA R7, 8
A0BFE   JNO A0C32
A0C00   JMP A0C04
A0C02   SLA R7, 8
A0C04   A R4, R7
A0C06   BL *R10
A0C08   JMP A0BF4
A0C0A   MOV R11, R12
A0C0C   LI R2, >0010
A0C10   CLR R6
A0C12   SETO R5
A0C14   BL @A0C34
A0C18   JLT A0C32
A0C1A   C R3, R2
A0C1C   JHE A0C30
A0C1E   MOV R6, R5
A0C20   MPY R2, R5
A0C22   A R3, R6
A0C24   BL *R10
A0C26   JMP A0C14
A0C28   MOV R11, R12
A0C2A   LI R2, >000A
A0C2E   JMP A0C10
A0C30   SETO R5
A0C32   B *R12
A0C34   SETO R1
A0C36   MOV R4, R3
A0C38   CI R4, >0024
A0C3C   JEQ A0C4A
A0C3E   AI R3, >FFD0
A0C42   JNC A0C60
A0C44   CI R3, >0009
A0C48   JGT A0C4E
A0C4A   NEG R1
A0C4C   RT
A0C4E   AI R3, >FFF9
A0C52   CI R3, >000A
A0C56   JL A0C60
A0C58   CI R3, >0023
A0C5C   JH A0C60
A0C5E   CLR R1
A0C60   MOV R1, R1
A0C62   RT
A0C64   SETO R3
A0C66   MOV @>EC4E, R1
A0C6A   JEQ A0C82
A0C6C   SLA R1, 2
A0C6E   LI R2, >EC62
A0C72   A R2, R1
A0C74   CLR R3
A0C76   INCT R2
A0C78   C R4, *R2+
A0C7A   JEQ A0C82
A0C7C   C R2, R1
A0C7E   JL A0C76
A0C80   INC R3
A0C82   MOV R3, R3
A0C84   RT
A0C86   LI R3, >EC58
A0C8A   C R2, *R3
A0C8C   JEQ A0C98
A0C8E   LI R3, >EC5C
A0C92   C R2, *R3
A0C94   JEQ A0C98
A0C96   CLR R3
A0C98   RT
A0C9A   MOV *R0, R4
A0C9C   MOV R11, R12
A0C9E   BL @A0C64
A0CA2   JEQ A0CBE
A0CA4   MOV R4, R4
A0CA6   JEQ A0CB0
A0CA8   CLR R4
A0CAA   INC @>EC4C
A0CAE   JMP A0C9E
A0CB0   INC @>EC4E
A0CB4   MOV @>EC4E, R2
A0CB8   SLA R2, 2
A0CBA   AI R2, >EC62
A0CBE   DECT R2
A0CC0   MOV *R0+, *R2+
A0CC2   MOV *R0+, *R2
A0CC4   B *R12
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
A0F1A   MOV @>EC54, @>EC44
A0F20   XOP @A00D8, R14
A0F24   XOP R10, R11
A0F26   XOP @A00DE, R14
A0F2A   BL @A0F42
A0F2E   SZCB R0, *R4
A0F30   DATA >0F7E
A0F32   SZC R0, *R0
A0F34   DATA >0F94
A0F36   SZCB R0, *R0
A0F38   DATA >0F5A
A0F3A   SZCB R0, R4
A0F3C   DATA >0E5A
A0F3E   DATA >0000
A0F40   INCT R11
A0F42   MOV *R11, R0
A0F44   JEQ A0F4E
A0F46   C *R11+, R10
A0F48   JNE A0F40
A0F4A   MOV *R11, R11
A0F4C   RT
A0F4E   XOP @A00F5, R14
A0F52   JMP A0F16
A0F54   XOP @>00FF, R14
A0F58   JMP A0F16
A0F5A   XOP R10, R9
A0F5C   DATA >0F16
A0F5E   DATA >0F54
A0F60   SLA R10, 2
A0F62   DECT R10
A0F64   JLE A0F54
A0F66   CI R10, >0023
A0F6A   JHE A0F54
A0F6C   LI R12, >0400
A0F70   SBO 11
A0F72   SBO 12
A0F74   LDCR @A0084(R10), R12
A0F78   SBZ 11
A0F7A   SBZ 12
A0F7C   JMP A0F16
A0F7E   XOP R10, R9
A0F80   DATA >0F88
A0F82   DATA >0F54
A0F84   MOV R10, @>EC50
A0F88   XOP R10, R9
A0F8A   DATA >0F16
A0F8C   DATA >0F54
A0F8E   MOV R10, @>EC4E
A0F92   JMP A0F16
A0F94   XOP R10, R9
A0F96   DATA >0F16
A0F98   DATA >0F54
A0F9A   MOV R10, @>EC4C
A0F9E   JMP A0F16
A0FA0   SETO R1
A0FA2   MOV @>EC52, R7
A0FA6   JLT A1002
A0FA8   JGT A1010
A0FAA   MOV @>EC50, R7
A0FAE   C R7, @>EC4E
A0FB2   JH A1022
A0FB4   MOV @A0140, *R9
A0FB8   CLR R5
A0FBA   CLR R3
A0FBC   XOP @>0137, R14
A0FC0   MOV R7, R10
A0FC2   BL @A1042
A0FC6   MPY R0, R4
A0FC8   MOV *R7, R10
A0FCA   BL @A1042
A0FCE   SZC R0, R8
A0FD0   C R7, @>EC4E
A0FD4   JL A0FDC
A0FD6   SETO @>EC52
A0FDA   JMP A0FE4
A0FDC   INCT R7
A0FDE   CI R3, A003C
A0FE2   JLT A101E
A0FE4   AI R5, >0037
A0FE8   MOV R5, R10
A0FEA   NEG R10
A0FEC   BL @A1042
A0FF0   STCR R0, R12
A0FF2   MOV R7, @>EC50
A0FF6   XOP @A0128, R14
A0FFA   MOV @>EC54, R5
A0FFE   JLT A101C

*****************************
* 2ND EPROM *****************
*****************************

A1000   DATA >10CF
A1002   DATA >2FA0
        DATA >0137
        DATA >2FA0
        DATA >00E0
        DATA >05E0
        DATA >EC52
        DATA >10F5
A1010   DATA >2FA0
        DATA >0137
        DATA >2FA0
        DATA >0131
        DATA >04E0
        DATA >EC52
A101C   DATA >04C1
A101E   DATA >0460
A1020   DATA >0E6A
A1022   DATA >2FA0
        DATA >0137
        DATA >2FA0
        DATA >0131
        DATA >C660
        DATA >013E
        DATA >C820
A1030   DATA >EC54
        DATA >EC44
        DATA >2FA0
        DATA >011F
        DATA >05E0
        DATA >EC44
        DATA >0720
        DATA >EC4C
A1040   DATA >10EB
A1042   DATA >C03B
        DATA >2F00
        DATA >0980
        DATA >A140
        DATA >2E8A
        DATA >0223
        DATA >0005
A1050   DATA >0200
        DATA >0004
        DATA >0B4A
        DATA >C18A
        DATA >09C6
        DATA >A146
        DATA >0225
        DATA >0030
A1060   DATA >0286
        DATA >000A
        DATA >1A02
        DATA >0225
        DATA >0007
        DATA >0600
        DATA >16F3
        DATA >045B
1000    10CF                                      jmp  >0fa0
1002    2FA0                                      xop  @>0137, r14
1004    0137
1006    2FA0                                      xop  @>00e0, r14
1008    00E0
100a    05E0                                      inct @>ec52
100c    EC52
100e    10F5                                      jmp  >0ffa
1010    2FA0                                      xop  @>0137, r14
1012    0137
1014    2FA0                                      xop  @>0131, r14
1016    0131
1018    04E0                                      clr  @>ec52
101a    EC52
101c    04C1                                      clr  r1
101e    0460                                      b    @>0e6a
1020    0E6A
1022    2FA0                                      xop  @>0137, r14
1024    0137
1026    2FA0                                      xop  @>0131, r14
1028    0131
102a    C660                                      mov  @>013e, *r9
102c    013E
102e    C820                                      mov  @>ec54, @>ec44
1030    EC54
1032    EC44
1034    2FA0                                      xop  @>011f, r14
1036    011F
1038    05E0                                      inct @>ec44
103a    EC44
103c    0720                                      seto @>ec4c
103e    EC4C
1040    10EB                                      jmp  >1018
1042    C03B                                      mov  *r11+, r0
1044    2F00                                      xop  r0, r12
1046    0980                                      srl  r0, 8
1048    A140                                      a    r0, r5
104a    2E8A                                      xop  r10, r10
104c    0223                                      ai   r3, >0005
104e    0005
1050    0200                                      li   r0, >0004
1052    0004
1054    0B4A                                      src  r10, 4
1056    C18A                                      mov  r10, r6
1058    09C6                                      srl  r6, 12
105a    A146                                      a    r6, r5
105c    0225                                      ai   r5, >0030
105e    0030
1060    0286                                      ci   r6, >000a
1062    000A
1064    1A02                                      jl   >106a
1066    0225                                      ai   r5, >0007
1068    0007
106a    0600                                      dec  r0
106c    16F3                                      jne  >1054
106e    045B                                      rt

*Character received on 2nd RS-232 port is not <DLE> or <DC2>.

A1070   SETO R2                                   DATA >0702
        MOV @A0140,*R9                            DATA >C660  2nd TMS9902 CRU base address.
*                        DATA >0140
        MOV @>EC4C,R0                             DATA >C020
*                        DATA >EC4C
        CLR R7                                    DATA >04C7

A107C   XOP R6,13                                 DATA >2F46  Read character into R6.
        CI R6,>1400                               DATA >0286  Is character <DC4>?
*                        DATA >1400
        JEQ A10FE                                 DATA >133D  Yes, jump.
        MOV @>EC52,R8                             DATA >C220
*                        DATA >EC52
        JNE A1148                                 DATA >165F
        CI R6,>2000                               DATA >0286  Is character a control character?
*                        DATA >2000
        JLT A107C                                 DATA >11F6  Yes, loop round and read another character.
A1090   CI R6,>5F00                               DATA >0286  Is character higher in ASCII than "_"?
*                        DATA >5F00
        JGT A107C                                 DATA >15F3  Yes, loop round and read another character.
*                                    (So character is between >20 and >5F)
        SETO R5                                   DATA >0705
        CLR R10                                   DATA >04CA
        BL @A073A                                 DATA >06A0  Check if character in R6 is a hex digit. Add it to R10 if so.
*                        DATA >073A
        JMP A10B6                                 DATA >100B  (Returns here if character in R6 is not a hex digit)

A10A0   DATA >D22A                                (Returns here if character in R6 is a hex digit)
        DATA >1152
        DATA >1332
        DATA >06A0
        DATA >0722
        DATA >100E
        DATA >0205
        DATA >0008
A10B0   DATA >0878
        DATA >0468
        DATA >10B2
10a0    D22A                                      movb @>1152(r10), r8
10a2    1152
10a4    1332                                      jeq  >110a
10a6    06A0                                      bl   @>0722
10a8    0722
10aa    100E                                      jmp  >10c8
10ac    0205                                      li   r5, >0008
10ae    0008
10b0    0878                                      sra  r8, 7
10b2    0468                                      b    @>10b2(r8)
10b4    10B2


A10B6   CI R6,>0047                               DATA >0286  R6 has been reversed?
*                        DATA >0047
        DATA >1106
        DATA >0286
        DATA >004A
A10C0   DATA >1522
        DATA >0226
        DATA >FFC9
        DATA >10EC
        DATA >0286
        DATA >003A
        DATA >161C
        DATA >103E
A10D0   DATA >020C
        DATA >0400
        DATA >04C5
        DATA >1F0F
        DATA >16FD
        DATA >0605
        DATA >16FC
        DATA >C660
A10E0   DATA >013E
        DATA >C820
        DATA >EC54
        DATA >EC44
        DATA >0720
        DATA >EC4C
        DATA >C000
        DATA >1303
A10F0   DATA >2FA0
        DATA >010A
        DATA >1002
        DATA >2FA0
        DATA >0115
        DATA >05E0
        DATA >EC44

A10FE   DATA >04C2
A1100   DATA >04E0
        DATA >EC52
        DATA >1021
        DATA >04C0
        DATA >10E3
        DATA >2F46
        DATA >9806
        DATA >00F2
A1110   DATA >16FC
        DATA >04C7
        DATA >1019
        DATA >A1CA
        DATA >1317
        DATA >0700
        DATA >10D9
        DATA >A280
A1120   DATA >C0CA
        DATA >1012
        DATA >A280
        DATA >CCCA
        DATA >100F
        DATA >0645
        DATA >2F46
        DATA >0986
A1130   DATA >13FD
        DATA >A1C6
        DATA >0605
        DATA >16FA

A1148   DATA >1007
        DATA >A280
        DATA >C80A
        DATA >EC1C
A1140   DATA >1003
        DATA >024A
        DATA >FFFE
        DATA >C00A
        DATA >0460
        DATA >0E6A
        DATA >0720
        DATA >EC52
A1150   DATA >10FB
        DATA >3D45
        DATA >443C
        DATA >3C3C
        DATA >3C32
        DATA >E437
        DATA >363A
        DATA >3948
A1160   DATA >2A00
        DATA >3C3C
        DATA >3D8B
        DATA >C141
        DATA >0208
        DATA >12D8
        DATA >0209
        DATA >12BA
A1170   DATA >0207
        DATA >12A6
        DATA >2FA0
        DATA >00F2
        DATA >0206
        DATA >202C
        DATA >C050
        DATA >2E80
A1180   DATA >2F06
        DATA >2E81
        DATA >2F06
        DATA >04C3
        DATA >C050
        DATA >0241
        DATA >FFF0
        DATA >C2A3
A1190   DATA >13B4
        DATA >C08A
        DATA >1329
        DATA >024A
        DATA >FFF0
        DATA >8281
        DATA >1402
        DATA >0643
A11A0   DATA >10F6
        DATA >C050
        DATA >0A13
        DATA >0223
        DATA >14D6
        DATA >604A
        DATA >0242
        DATA >000F
A11B0   DATA >D0A2
        DATA >11BA
        DATA >0972
        DATA >0462
        DATA >11BA
        DATA >060C
        DATA >061F
        DATA >232B
A11C0   DATA >3337
        DATA >063F
        DATA >4145
        DATA >0697
        DATA >0698
        DATA >2F06
        DATA >0961
        DATA >0698
A11D0   DATA >103D
        DATA >C041
        DATA >1603
        DATA >0203
        DATA >14DA
        DATA >1024
        DATA >06C1
        DATA >0871
A11E0   DATA >05C1
        DATA >A040
        DATA >0697
        DATA >1004
        DATA >C050
        DATA >0203
        DATA >14DE
        DATA >0697
A11F0   DATA >2F20
        DATA >14F5
        DATA >2E81
        DATA >102A
        DATA >0697
        DATA >0698
        DATA >C209
        DATA >10E5
A1200   DATA >D041
        DATA >16F2
        DATA >0B41
        DATA >D081
        DATA >0A61
        DATA >09C2
        DATA >A042
        DATA >10F4
A1210   DATA >8810
        DATA >1320
        DATA >1603
        DATA >0203
        DATA >14EE
        DATA >1004
        DATA >0697
        DATA >10D7
A1220   DATA >C041
        DATA >16E2
        DATA >0697
        DATA >1012
        DATA >0AC1
        DATA >18DE
        DATA >09C1
        DATA >0697
A1230   DATA >0698
        DATA >2F06
        DATA >C070
        DATA >10DC
        DATA >0697
        DATA >1019
        DATA >C041
        DATA >16D4
A1240   DATA >0697
        DATA >10F8
        DATA >0AC1
        DATA >18D0
        DATA >09C1
        DATA >10E2
        DATA >C145
        DATA >1603
A1250   DATA >C740
        DATA >0460
        DATA >0142
        DATA >C320
        DATA >013E
        DATA >1F15
        DATA >1604
        DATA >2F42
A1260   DATA >0282
        DATA >2F00
        DATA >13F5
        DATA >8140
        DATA >1BF3
        DATA >0460
        DATA >1168
        DATA >0203
A1270   DATA >2D31
        DATA >06C1
        DATA >0881
        DATA >1315
        DATA >1502
        DATA >2F03
        DATA >0501
        DATA >0281
A1280   DATA >0064
        DATA >1104
        DATA >0A83
        DATA >2F03
        DATA >0221
        DATA >FF9C
        DATA >0204
        DATA >000A
A1290   DATA >C081
        DATA >04C1
        DATA >3C44
        DATA >0A83
        DATA >1302
        DATA >C041
        DATA >1301
        DATA >0699
A12A0   DATA >C042
        DATA >0699
        DATA >10D3
        DATA >0202
        DATA >0004
        DATA >2F13
        DATA >0583
        DATA >0602
A12B0   DATA >16FC
        DATA >2F06
        DATA >06C6
        DATA >05C0
        DATA >045B
        DATA >C0C1
        DATA >0AC3
        DATA >0943
A12C0   DATA >0283
        DATA >0900
        DATA >1203
        DATA >06C3
        DATA >0223
        DATA >0126
        DATA >0223
        DATA >3000
A12D0   DATA >2F03
        DATA >0A83
        DATA >16FD
        DATA >045B
        DATA >0204
        DATA >2A52
        DATA >C081
        DATA >0AA2
A12E0   DATA >09E2
        DATA >1603
        DATA >06C4
        DATA >2F04
        DATA >10E8
        DATA >0602
        DATA >1602
        DATA >2F04
A12F0   DATA >10F9
        DATA >0602
        DATA >1610
        DATA >2FA0
        DATA >14F4
        DATA >2EB0
        DATA >C081
        DATA >0AC2
A1300   DATA >13EA
        DATA >0202
        DATA >2829
        DATA >2F02
        DATA >06C4
        DATA >2F04
        DATA >C30B
        DATA >0699
A1310   DATA >06C2
        DATA >2F02
        DATA >045C
        DATA >2FA0
        DATA >14F7
        DATA >0202
        DATA >002B
        DATA >10F6
A1320   DATA >045B
        DATA >0000
        DATA >008B
        DATA >009B
        DATA >0185
        DATA >01C5
        DATA >0207
        DATA >0227
A1330   DATA >0247
        DATA >0267
        DATA >0287
        DATA >02AB
        DATA >02CB
        DATA >02EA
        DATA >030A
        DATA >0346
A1340   DATA >0366
        DATA >0386
        DATA >03A6
        DATA >03C6
        DATA >03E6
        DATA >0405
        DATA >0445
        DATA >0485
A1350   DATA >04C5
        DATA >0505
        DATA >0545
        DATA >0585
        DATA >05C5
        DATA >0605
        DATA >0645
        DATA >0685
A1360   DATA >06C5
        DATA >0705
        DATA >0745
        DATA >0804
        DATA >0904
        DATA >0A04
        DATA >0B04
        DATA >1001
A1370   DATA >1101
        DATA >1201
        DATA >1301
        DATA >1401
        DATA >1501
        DATA >1601
        DATA >1701
        DATA >1801
A1380   DATA >1901
        DATA >1A01
        DATA >1B01
        DATA >1C01
        DATA >1D09
        DATA >1E09
        DATA >1F09
        DATA >2002
A1390   DATA >2402
        DATA >2802
        DATA >2C03
        DATA >3003
        DATA >3403
        DATA >3808
        DATA >3C08
        DATA >4000
A13A0   DATA >5000
        DATA >6000
        DATA >7000
        DATA >8000
        DATA >9000
        DATA >A000
        DATA >B000
        DATA >C000
A13B0   DATA >D000
        DATA >E000
        DATA >F000
        DATA >4C53
        DATA >5420
        DATA >4C57
        DATA >5020
        DATA >4449
A13C0   DATA >5653
        DATA >4D50
        DATA >5953
        DATA >4C49
        DATA >2020
        DATA >4149
        DATA >2020
        DATA >414E
A13D0   DATA >4449
        DATA >4F52
        DATA >4920
        DATA >4349
        DATA >2020
        DATA >5354
        DATA >5750
        DATA >5354
A13E0   DATA >5354
        DATA >4C57
        DATA >5049
        DATA >4C49
        DATA >4D49
        DATA >4944
        DATA >4C45
        DATA >5253
A13F0   DATA >4554
        DATA >5254
        DATA >5750
        DATA >434B
        DATA >4F4E
        DATA >434B
        DATA >4F46
        DATA >4C52
A1400   DATA >4558
        DATA >424C
        DATA >5750
        DATA >4220
        DATA >2020
        DATA >5820
        DATA >2020
        DATA >434C
A1410   DATA >5220
        DATA >4E45
        DATA >4720
        DATA >494E
        DATA >5620
        DATA >494E
        DATA >4320
        DATA >494E
A1420   DATA >4354
        DATA >4445
        DATA >4320
        DATA >4445
        DATA >4354
        DATA >424C
        DATA >2020
        DATA >5357
A1430   DATA >5042
        DATA >5345
        DATA >544F
        DATA >4142
        DATA >5320
        DATA >5352
        DATA >4120
        DATA >5352
A1440   DATA >4C20
        DATA >534C
        DATA >4120
        DATA >5352
        DATA >4320
        DATA >4A4D
        DATA >5020
        DATA >4A4C
A1450   DATA >5420
        DATA >4A4C
        DATA >4520
        DATA >4A45
        DATA >5120
        DATA >4A48
        DATA >4520
        DATA >4A47
A1460   DATA >5420
        DATA >4A4E
        DATA >4520
        DATA >4A4E
        DATA >4320
        DATA >4A4F
        DATA >4320
        DATA >4A4E
A1470   DATA >4F20
        DATA >4A4C
        DATA >2020
        DATA >4A48
        DATA >2020
        DATA >4A4F
        DATA >5020
        DATA >5342
A1480   DATA >4F20
        DATA >5342
        DATA >5A20
        DATA >5442
        DATA >2020
        DATA >434F
        DATA >4320
        DATA >435A
A1490   DATA >4320
        DATA >584F
        DATA >5220
        DATA >584F
        DATA >5020
        DATA >4C44
        DATA >4352
        DATA >5354
A14A0   DATA >4352
        DATA >4D50
        DATA >5920
        DATA >4449
        DATA >5620
        DATA >535A
        DATA >4320
        DATA >535A
A14B0   DATA >4342
        DATA >5320
        DATA >2020
        DATA >5342
        DATA >2020
        DATA >4320
        DATA >2020
        DATA >4342
A14C0   DATA >2020
        DATA >4120
        DATA >2020
        DATA >4142
        DATA >2020
        DATA >4D4F
        DATA >5620
        DATA >4D4F
A14D0   DATA >5642
        DATA >534F
        DATA >4320
        DATA >534F
        DATA >4342
        DATA >4E4F
        DATA >5020
        DATA >4441
A14E0   DATA >5441
        DATA >5445
        DATA >5854
        DATA >414F
        DATA >5247
        DATA >454E
        DATA >4420
        DATA >5254
A14F0   DATA >2020
        DATA >0000
        DATA >403E
        DATA >002A
        DATA >5200
        DATA >FFFF
        DATA >FFFF
        DATA >FFFF
10ba    1106                                      jlt  >10c8
10bc    0286                                      ci   r6, >004a
10be    004A
10c0    1522                                      jgt  >1106
10c2    0226                                      ai   r6, >ffc9
10c4    FFC9
10c6    10EC                                      jmp  >10a0
10c8    0286                                      ci   r6, >003a
10ca    003A
10cc    161C                                      jne  >1106
10ce    103E                                      jmp  >114c
10d0    020C                                      li   r12, >0400
10d2    0400
10d4    04C5                                      clr  r5
10d6    1F0F                                      tb   15
10d8    16FD                                      jne  >10d4
10da    0605                                      dec  r5
10dc    16FC                                      jne  >10d6
10de    C660                                      mov  @>013e, *r9
10e0    013E
10e2    C820                                      mov  @>ec54, @>ec44
10e4    EC54
10e6    EC44
10e8    0720                                      seto @>ec4c
10ea    EC4C
10ec    C000                                      mov  r0, r0
10ee    1303                                      jeq  >10f6
10f0    2FA0                                      xop  @>010a, r14
10f2    010A
10f4    1002                                      jmp  >10fa
10f6    2FA0                                      xop  @>0115, r14
10f8    0115
10fa    05E0                                      inct @>ec44
10fc    EC44
10fe    04C2                                      clr  r2
1100    04E0                                      clr  @>ec52
1102    EC52
1104    1021                                      jmp  >1148
1106    04C0                                      clr  r0
1108    10E3                                      jmp  >10d0
110a    2F46                                      xop  r6, r13
110c    9806                                      cb   r6, @>00f2
110e    00F2
1110    16FC                                      jne  >110a
1112    04C7                                      clr  r7
1114    1019                                      jmp  >1148
1116    A1CA                                      a    r10, r7
1118    1317                                      jeq  >1148
111a    0700                                      seto r0
111c    10D9                                      jmp  >10d0
111e    A280                                      a    r0, r10
1120    C0CA                                      mov  r10, r3
1122    1012                                      jmp  >1148
1124    A280                                      a    r0, r10
1126    CCCA                                      mov  r10, *r3+
1128    100F                                      jmp  >1148
112a    0645                                      dect r5
112c    2F46                                      xop  r6, r13
112e    0986                                      srl  r6, 8
1130    13FD                                      jeq  >112c
1132    A1C6                                      a    r6, r7
1134    0605                                      dec  r5
1136    16FA                                      jne  >112c
1138    1007                                      jmp  >1148
113a    A280                                      a    r0, r10
113c    C80A                                      mov  r10, @>ec1c
113e    EC1C
1140    1003                                      jmp  >1148
1142    024A                                      andi r10, >fffe
1144    FFFE
1146    C00A                                      mov  r10, r0
1148    0460                                      b    @>0e6a
114a    0E6A
114c    0720                                      seto @>ec52
114e    EC52
1150    10FB                                      jmp  >1148
1152    3D45                                      div  r5, r5
1154    443C                                      szc  *r12+, *r0
1156    3C3C                                      div  *r12+, r0
1158    3C32                                      div  *r2+, r0
115a    E437                                      soc  *r7+, *r0
115c    363A                                      stcr *r10+, r8
115e    3948                                      mpy  r8, r5
1160    2A00                                      xor  r0, r8
1162    3C3C                                      div  *r12+, r0
1164    3D8B                                      div  r11, r6
1166    C141                                      mov  r1, r5
1168    0208                                      li   r8, >12d8
116a    12D8
116c    0209                                      li   r9, >12ba
116e    12BA
1170    0207                                      li   r7, >12a6
1172    12A6
1174    2FA0                                      xop  @>00f2, r14
1176    00F2
1178    0206                                      li   r6, >202c
117a    202C
117c    C050                                      mov  *r0, r1
117e    2E80                                      xop  r0, r10
1180    2F06                                      xop  r6, r12
1182    2E81                                      xop  r1, r10
1184    2F06                                      xop  r6, r12
1186    04C3                                      clr  r3
1188    C050                                      mov  *r0, r1
118a    0241                                      andi r1, >fff0
118c    FFF0
118e    C2A3                                      mov  @>13b4(r3), r10
1190    13B4
1192    C08A                                      mov  r10, r2
1194    1329                                      jeq  >11e8
1196    024A                                      andi r10, >fff0
1198    FFF0
119a    8281                                      c    r1, r10
119c    1402                                      jhe  >11a2
119e    0643                                      dect r3
11a0    10F6                                      jmp  >118e
11a2    C050                                      mov  *r0, r1
11a4    0A13                                      sla  r3, 1
11a6    0223                                      ai   r3, >14d6
11a8    14D6
11aa    604A                                      s    r10, r1
11ac    0242                                      andi r2, >000f
11ae    000F
11b0    D0A2                                      movb @>11ba(r2), r2
11b2    11BA
11b4    0972                                      srl  r2, 7
11b6    0462                                      b    @>11ba(r2)
11b8    11BA
11ba    060C                                      dec  r12
11bc    061F                                      dec  *r15
11be    232B                                      coc  @>3337(r11), r12
11c0    3337
11c2    063F                                      dec  *r15+
11c4    4145                                      szc  r5, r5
11c6    0697                                      bl   *r7
11c8    0698                                      bl   *r8
11ca    2F06                                      xop  r6, r12
11cc    0961                                      srl  r1, 6
11ce    0698                                      bl   *r8
11d0    103D                                      jmp  >124c
11d2    C041                                      mov  r1, r1
11d4    1603                                      jne  >11dc
11d6    0203                                      li   r3, >14da
11d8    14DA
11da    1024                                      jmp  >1224
11dc    06C1                                      swpb r1
11de    0871                                      sra  r1, 7
11e0    05C1                                      inct r1
11e2    A040                                      a    r0, r1
11e4    0697                                      bl   *r7
11e6    1004                                      jmp  >11f0
11e8    C050                                      mov  *r0, r1
11ea    0203                                      li   r3, >14de
11ec    14DE
11ee    0697                                      bl   *r7
11f0    2F20                                      xop  @>14f5, r12
11f2    14F5
11f4    2E81                                      xop  r1, r10
11f6    102A                                      jmp  >124c
11f8    0697                                      bl   *r7
11fa    0698                                      bl   *r8
11fc    C209                                      mov  r9, r8
11fe    10E5                                      jmp  >11ca
1200    D041                                      movb r1, r1
1202    16F2                                      jne  >11e8
1204    0B41                                      src  r1, 4
1206    D081                                      movb r1, r2
1208    0A61                                      sla  r1, 6
120a    09C2                                      srl  r2, 12
120c    A042                                      a    r2, r1
120e    10F4                                      jmp  >11f8
1210    8810                                      c    *r0, @>1320
1212    1320
1214    1603                                      jne  >121c
1216    0203                                      li   r3, >14ee
1218    14EE
121a    1004                                      jmp  >1224
121c    0697                                      bl   *r7
121e    10D7                                      jmp  >11ce
1220    C041                                      mov  r1, r1
1222    16E2                                      jne  >11e8
1224    0697                                      bl   *r7
1226    1012                                      jmp  >124c
1228    0AC1                                      sla  r1, 12
122a    18DE                                      joc  >11e8
122c    09C1                                      srl  r1, 12
122e    0697                                      bl   *r7
1230    0698                                      bl   *r8
1232    2F06                                      xop  r6, r12
1234    C070                                      mov  *r0+, r1
1236    10DC                                      jmp  >11f0
1238    0697                                      bl   *r7
123a    1019                                      jmp  >126e
123c    C041                                      mov  r1, r1
123e    16D4                                      jne  >11e8
1240    0697                                      bl   *r7
1242    10F8                                      jmp  >1234
1244    0AC1                                      sla  r1, 12
1246    18D0                                      joc  >11e8
1248    09C1                                      srl  r1, 12
124a    10E2                                      jmp  >1210
124c    C145                                      mov  r5, r5
124e    1603                                      jne  >1256
1250    C740                                      mov  r0, *r13
1252    0460                                      b    @>0142
1254    0142
1256    C320                                      mov  @>013e, r12
1258    013E
125a    1F15                                      tb   21
125c    1604                                      jne  >1266
125e    2F42                                      xop  r2, r13
1260    0282                                      ci   r2, >2f00
1262    2F00
1264    13F5                                      jeq  >1250
1266    8140                                      c    r0, r5
1268    1BF3                                      jh   >1250
126a    0460                                      b    @>1168
126c    1168
126e    0203                                      li   r3, >2d31
1270    2D31
1272    06C1                                      swpb r1
1274    0881                                      sra  r1, 8
1276    1315                                      jeq  >12a2
1278    1502                                      jgt  >127e
127a    2F03                                      xop  r3, r12
127c    0501                                      neg  r1
127e    0281                                      ci   r1, >0064
1280    0064
1282    1104                                      jlt  >128c
1284    0A83                                      sla  r3, 8
1286    2F03                                      xop  r3, r12
1288    0221                                      ai   r1, >ff9c
128a    FF9C
128c    0204                                      li   r4, >000a
128e    000A
1290    C081                                      mov  r1, r2
1292    04C1                                      clr  r1
1294    3C44                                      div  r4, r1
1296    0A83                                      sla  r3, 8
1298    1302                                      jeq  >129e
129a    C041                                      mov  r1, r1
129c    1301                                      jeq  >12a0
129e    0699                                      bl   *r9
12a0    C042                                      mov  r2, r1
12a2    0699                                      bl   *r9
12a4    10D3                                      jmp  >124c
12a6    0202                                      li   r2, >0004
12a8    0004
12aa    2F13                                      xop  *r3, r12
12ac    0583                                      inc  r3
12ae    0602                                      dec  r2
12b0    16FC                                      jne  >12aa
12b2    2F06                                      xop  r6, r12
12b4    06C6                                      swpb r6
12b6    05C0                                      inct r0
12b8    045B                                      rt
12ba    C0C1                                      mov  r1, r3
12bc    0AC3                                      sla  r3, 12
12be    0943                                      srl  r3, 4
12c0    0283                                      ci   r3, >0900
12c2    0900
12c4    1203                                      jle  >12cc
12c6    06C3                                      swpb r3
12c8    0223                                      ai   r3, >0126
12ca    0126
12cc    0223                                      ai   r3, >3000
12ce    3000
12d0    2F03                                      xop  r3, r12
12d2    0A83                                      sla  r3, 8
12d4    16FD                                      jne  >12d0
12d6    045B                                      rt
12d8    0204                                      li   r4, >2a52
12da    2A52
12dc    C081                                      mov  r1, r2
12de    0AA2                                      sla  r2, 10
12e0    09E2                                      srl  r2, 14
12e2    1603                                      jne  >12ea
12e4    06C4                                      swpb r4
12e6    2F04                                      xop  r4, r12
12e8    10E8                                      jmp  >12ba
12ea    0602                                      dec  r2
12ec    1602                                      jne  >12f2
12ee    2F04                                      xop  r4, r12
12f0    10F9                                      jmp  >12e4
12f2    0602                                      dec  r2
12f4    1610                                      jne  >1316
12f6    2FA0                                      xop  @>14f4, r14
12f8    14F4
12fa    2EB0                                      xop  *r0+, r10
12fc    C081                                      mov  r1, r2
12fe    0AC2                                      sla  r2, 12
1300    13EA                                      jeq  >12d6
1302    0202                                      li   r2, >2829
1304    2829
1306    2F02                                      xop  r2, r12
1308    06C4                                      swpb r4
130a    2F04                                      xop  r4, r12
130c    C30B                                      mov  r11, r12
130e    0699                                      bl   *r9
1310    06C2                                      swpb r2
1312    2F02                                      xop  r2, r12
1314    045C                                      b    *r12
1316    2FA0                                      xop  @>14f7, r14
1318    14F7
131a    0202                                      li   r2, >002b
131c    002B
131e    10F6                                      jmp  >130c
1320    045B                                      rt
1322    0000                                      data >0000
1324    008B                                      data >008b
1326    009B                                      data >009b
1328    0185                                      data >0185
132a    01C5                                      data >01c5
132c    0207                                      li   r7, >0227
132e    0227
1330    0247                                      andi r7, >0267
1332    0267
1334    0287                                      ci   r7, >02ab
1336    02AB
1338    02CB                                      stst r11
133a    02EA                                      data >02ea
133c    030A                                      data >030a
133e    0346                                      data >0346
1340    0366                                      data >0366
1342    0386                                      data >0386
1344    03A6                                      data >03a6
1346    03C6                                      data >03c6
1348    03E6                                      data >03e6
134a    0405                                      blwp r5
134c    0445                                      b    r5
134e    0485                                      x    r5
>1350   04C5 0505 0545 0585 05C5 0605 0645 0685   .....E.......E..
>1360   06C5 0705 0745 0804 0904 0A04 0B04 1001   .....E..........
>1370   1101 1201 1301 1401 1501 1601 1701 1801   ................
>1380   1901 1A01 1B01 1C01 1D09 1E09 1F09 2002   .............. .
>1390   2402 2802 2C03 3003 3403 3808 3C08 4000   $.(.,.0.4.8.<.@.
>13A0   5000 6000 7000 8000 9000 A000 B000 C000   P.`.p...........
>13B0   D000 E000 F000 4C53 5420 4C57 5020 4449   ......LST LWP DI
>13C0   5653 4D50 5953 4C49 2020 4149 2020 414E   VSMPYSLI  AI  AN
>13D0   4449 4F52 4920 4349 2020 5354 5750 5354   DIORI CI  STWPST
>13E0   5354 4C57 5049 4C49 4D49 4944 4C45 5253   STLWPILIMIIDLERS
>13F0   4554 5254 5750 434B 4F4E 434B 4F46 4C52   ETRTWPCKONCKOFLR
>1400   4558 424C 5750 4220 2020 5820 2020 434C   EXBLWPB   X   CL
>1410   5220 4E45 4720 494E 5620 494E 4320 494E   R NEG INV INC IN
>1420   4354 4445 4320 4445 4354 424C 2020 5357   CTDEC DECTBL  SW
>1430   5042 5345 544F 4142 5320 5352 4120 5352   PBSETOABS SRA SR
>1440   4C20 534C 4120 5352 4320 4A4D 5020 4A4C   L SLA SRC JMP JL
>1450   5420 4A4C 4520 4A45 5120 4A48 4520 4A47   T JLE JEQ JHE JG
>1460   5420 4A4E 4520 4A4E 4320 4A4F 4320 4A4E   T JNE JNC JOC JN
>1470   4F20 4A4C 2020 4A48 2020 4A4F 5020 5342   O JL  JH  JOP SB
>1480   4F20 5342 5A20 5442 2020 434F 4320 435A   O SBZ TB  COC CZ
>1490   4320 584F 5220 584F 5020 4C44 4352 5354   C XOR XOP LDCRST
>14A0   4352 4D50 5920 4449 5620 535A 4320 535A   CRMPY DIV SZC SZ
>14B0   4342 5320 2020 5342 2020 4320 2020 4342   CBS   SB  C   CB
>14C0   2020 4120 2020 4142 2020 4D4F 5620 4D4F   A   AB  MOV MO
>14D0   5642 534F 4320 534F 4342 4E4F 5020 4441   VBSOC SOCBNOP DA
>14E0   5441 5445 5854 414F 5247 454E 4420 5254   TATEXTAORGEND RT
>14F0   2020 0000 403E 002A 5200 FFFF FFFF FFFF   ..@>.*R.......
1350    04C5                                      clr  r5
1352    0505                                      neg  r5
1354    0545                                      inv  r5
1356    0585                                      inc  r5
1358    05C5                                      inct r5
135a    0605                                      dec  r5
135c    0645                                      dect r5
135e    0685                                      bl   r5
1360    06C5                                      swpb r5
1362    0705                                      seto r5
1364    0745                                      abs  r5
1366    0804                                      sra  r4, 0
1368    0904                                      srl  r4, 0
136a    0A04                                      sla  r4, 0
136c    0B04                                      src  r4, 0
136e    1001                                      jmp  >1372
1370    1101                                      jlt  >1374
1372    1201                                      jle  >1376
1374    1301                                      jeq  >1378
1376    1401                                      jhe  >137a
1378    1501                                      jgt  >137c
137a    1601                                      jne  >137e
137c    1701                                      jnc  >1380
137e    1801                                      joc  >1382
1380    1901                                      jno  >1384
1382    1A01                                      jl   >1386
1384    1B01                                      jh   >1388
1386    1C01                                      jop  >138a
1388    1D09                                      sbo  9
138a    1E09                                      sbz  9
138c    1F09                                      tb   9
138e    2002                                      coc  r2, r0
1390    2402                                      czc  r2, r0
1392    2802                                      xor  r2, r0
1394    2C03                                      xop  r3, r0
1396    3003                                      ldcr r3, r0
1398    3403                                      stcr r3, r0
139a    3808                                      mpy  r8, r0
139c    3C08                                      div  r8, r0
139e    4000                                      szc  r0, r0
13a0    5000                                      szcb r0, r0
13a2    6000                                      s    r0, r0
13a4    7000                                      sb   r0, r0
13a6    8000                                      c    r0, r0
13a8    9000                                      cb   r0, r0
13aa    A000                                      a    r0, r0
13ac    B000                                      ab   r0, r0
13ae    C000                                      mov  r0, r0
13b0    D000                                      movb r0, r0
13b2    E000                                      soc  r0, r0
13b4    F000                                      socb r0, r0
13b6    4C53                                      szc  *r3, *r1+
13b8    5420                                      szcb @>4c57, *r0
13ba    4C57
13bc    5020                                      szcb @>4449, r0
13be    4449
13c0    5653                                      szcb *r3, *r9
13c2    4D50                                      szc  *r0, *r5+
13c4    5953                                      szcb *r3, @>4c49(r5)
13c6    4C49
13c8    2020                                      coc  @>4149, r0
13ca    4149
13cc    2020                                      coc  @>414e, r0
13ce    414E
13d0    4449                                      szc  r9, *r1
13d2    4F52                                      szc  *r2, *r13+
13d4    4920                                      szc  @>4349, @>2020(r4)
13d6    4349
13d8    2020
13da    5354                                      szcb *r4, r13
13dc    5750                                      szcb *r0, *r13
13de    5354                                      szcb *r4, r13
13e0    5354                                      szcb *r4, r13
13e2    4C57                                      szc  *r7, *r1+
13e4    5049                                      szcb r9, r1
13e6    4C49                                      szc  r9, *r1+
13e8    4D49                                      szc  r9, *r5+
13ea    4944                                      szc  r4, @>4c45(r5)
13ec    4C45
13ee    5253                                      szcb *r3, r9
13f0    4554                                      szc  *r4, *r5
13f2    5254                                      szcb *r4, r9
13f4    5750                                      szcb *r0, *r13
13f6    434B                                      szc  r11, r13
13f8    4F4E                                      szc  r14, *r13+
13fa    434B                                      szc  r11, r13
13fc    4F46                                      szc  r6, *r13+
13fe    4C52                                      szc  *r2, *r1+
1400    4558                                      szc  *r8, *r5
1402    424C                                      szc  r12, r9
1404    5750                                      szcb *r0, *r13
1406    4220                                      szc  @>2020, r8
1408    2020
140a    5820                                      szcb @>2020, @>434c
140c    2020
140e    434C
1410    5220                                      szcb @>4e45, r8
1412    4E45
1414    4720                                      szc  @>494e, *r12
1416    494E
1418    5620                                      szcb @>494e, *r8
141a    494E
141c    4320                                      szc  @>494e, r12
141e    494E
1420    4354                                      szc  *r4, r13
1422    4445                                      szc  r5, *r1
1424    4320                                      szc  @>4445, r12
1426    4445
1428    4354                                      szc  *r4, r13
142a    424C                                      szc  r12, r9
142c    2020                                      coc  @>5357, r0
142e    5357
1430    5042                                      szcb r2, r1
1432    5345                                      szcb r5, r13
1434    544F                                      szcb r15, *r1
1436    4142                                      szc  r2, r5
1438    5320                                      szcb @>5352, r12
143a    5352
143c    4120                                      szc  @>5352, r4
143e    5352
1440    4C20                                      szc  @>534c, *r0+
1442    534C
1444    4120                                      szc  @>5352, r4
1446    5352
1448    4320                                      szc  @>4a4d, r12
144a    4A4D
144c    5020                                      szcb @>4a4c, r0
144e    4A4C
1450    5420                                      szcb @>4a4c, *r0
1452    4A4C
1454    4520                                      szc  @>4a45, *r4
1456    4A45
1458    5120                                      szcb @>4a48, r4
145a    4A48
145c    4520                                      szc  @>4a47, *r4
145e    4A47
1460    5420                                      szcb @>4a4e, *r0
1462    4A4E
1464    4520                                      szc  @>4a4e, *r4
1466    4A4E
1468    4320                                      szc  @>4a4f, r12
146a    4A4F
146c    4320                                      szc  @>4a4e, r12
146e    4A4E
1470    4F20                                      szc  @>4a4c, *r12+
1472    4A4C
1474    2020                                      coc  @>4a48, r0
1476    4A48
1478    2020                                      coc  @>4a4f, r0
147a    4A4F
147c    5020                                      szcb @>5342, r0
147e    5342
1480    4F20                                      szc  @>5342, *r12+
1482    5342
1484    5A20                                      szcb @>5442, @>2020(r8)
1486    5442
1488    2020
148a    434F                                      szc  r15, r13
148c    4320                                      szc  @>435a, r12
148e    435A
1490    4320                                      szc  @>584f, r12
1492    584F
1494    5220                                      szcb @>584f, r8
1496    584F
1498    5020                                      szcb @>4c44, r0
149a    4C44
149c    4352                                      szc  *r2, r13
149e    5354                                      szcb *r4, r13
14a0    4352                                      szc  *r2, r13
14a2    4D50                                      szc  *r0, *r5+
14a4    5920                                      szcb @>4449, @>5620(r4)
14a6    4449
14a8    5620
14aa    535A                                      szcb *r10, r13
14ac    4320                                      szc  @>535a, r12
14ae    535A
14b0    4342                                      szc  r2, r13
14b2    5320                                      szcb @>2020, r12
14b4    2020
14b6    5342                                      szcb r2, r13
14b8    2020                                      coc  @>4320, r0
14ba    4320
14bc    2020                                      coc  @>4342, r0
14be    4342
14c0    2020                                      coc  @>4120, r0
14c2    4120
14c4    2020                                      coc  @>4142, r0
14c6    4142
14c8    2020                                      coc  @>4d4f, r0
14ca    4D4F
14cc    5620                                      szcb @>4d4f, *r8
14ce    4D4F
14d0    5642                                      szcb r2, *r9
14d2    534F                                      szcb r15, r13
14d4    4320                                      szc  @>534f, r12
14d6    534F
14d8    4342                                      szc  r2, r13
14da    4E4F                                      szc  r15, *r9+
14dc    5020                                      szcb @>4441, r0
14de    4441
14e0    5441                                      szcb r1, *r1
14e2    5445                                      szcb r5, *r1
14e4    5854                                      szcb *r4, @>414f(r1)
14e6    414F
14e8    5247                                      szcb r7, r9
14ea    454E                                      szc  r14, *r5
14ec    4420                                      szc  @>5254, *r0
14ee    5254
14f0    2020                                      coc  @>0000, r0
14f2    0000
14f4    403E                                      szc  *r14+, r0
14f6    002A                                      data >002a
14f8    5200                                      szcb r0, r8
14fa    FFFF                                      socb *r15+, *r15+
14fc    FFFF                                      socb *r15+, *r15+
14fe    FFFF                                      socb *r15+, *r15+



        END
