# TMS9995 CPU Card

*** THIS IS UNTESTED DO NOT BUILD ***

This is an ATX format implimentation of Stuart Conner's Mini-Cortex system.  The Mini-Cortex system is a further development of his TMS 9995 breadboard project to produce a system similar to a Powertran Cortex, but using more modern components. The system is based around a TMS 9995 running at 3 MHz, with 32K byte EEPROM, and will use 512K bytes of RAM  accessed through a memory mapper. An on-board serial port and IDE interface exists on the card.  There is also provisions for a TMS9918 clone, a keyboard and some expansion slots -- these are not supported by the firmware at this time however.

The main EEPROM image provides a boot menu which enables the user to select between the EVMBUG system monitor from TI's TMS 9995 Evaluation Module, a port of the Powertran Cortex Power BASIC made for a TM 990 computer, the Marinchip Disk Executive (MDEX) operating system, and an implementation of V6 Unix including a C compiler.

[Much more information can be found at Stuart's site](https://www.stuartconner.me.uk/mini_cortex/mini_cortex.htm)

[Source code for the Cortex Version of Unix V6 and a ton more TI990 software (including cross development tools) can be found here on Dave Pitts' TI-990 web page.](https://www.cozx.com/dpitts/ti990.html)

## Memory Map
The memory map is shown in the table below.

```
Memory Address	Mapped To
>0000 - >7FFF	ROM when enabled, otherwise RAM
>8000 - >EFFF	RAM
>F000 - >F0FB	TMS 9995 internal RAM
>F0FC - >FDFF	RAM
>FE00 - >FE03	CF card ATA registers 
>FE10 - >FE13	SOUND
>FE20 - >FE23	Video
>FE30 - >FE33	Keyboard
>FE40 - >FE4F	Memory mapper registers 0-15 
>FE80 - >FEBF	Offboard IO (ports $80-$FF)
>FFF0 - >FFF9	RAM
>FFFA - >FFFF	TMS 9995 internal RAM

CRU Address	Mapped To
>0000 - >003F	TMS 9902 registers
>0040 - >007F	Control signal latch (further details here)
 	(Plus processor internal CRU bits)
    
(Note that in TI Speak ">" means Hexidecimal, so for the rest of us >FFFA is the same as $FFFA or 0xFFFA or FFFAH)
```
[for more information see Stuart's site](https://www.stuartconner.me.uk/mini_cortex/mini_cortex.htm)

## Creating CF card Image for MDEX and Unix

The CF card has to be newly formatted with an MBR (master boot record), a single partition formatted as FAT32, and a cluster size of 4096 bytes - the CF card needs to be at least 256 MB to support this. Not all brands/sizes of CF card seem to work - you may have more success with an older, low capacity card. A SanDisk 2.0GB card with a "SDCFB" marking on the back works well.

It can be problematic formatting a new card using Windows 10 as it does not always seem to write an MBR to the card. I use a formatting utility called Rufus [details and download here](https://rufus.ie/en/) with Boot selection==Non bootable, Partition Scheme==MBR, File System=FAT32, and Cluster Size= 4096 Bytes. Once the card is correctly formatted, use Windows to write the four files from the software folder ZIP filedirectly to the card.
[for more information see Stuart's site](https://www.stuartconner.me.uk/mini_cortex/mini_cortex.htm)

## Jumper Settings
```

```
## Powering up
The serial port on the TMS9995 board is configured for 9600 Baud, 7 data bits, even parity, one stop bit, no flow control.

The EVMBUG monitor and Cortex BASIC software applications, and the boot loaders for MDEX and Unix, are stored on a single 32K byte EEPROM. The required application is selected from a boot menu. To display the boot menu, connect the board to a configured serial port on a PC, apply power then press any key.

You should then see:
```
TMS 9995 BREADBOARD SYSTEM
BY STUART CONNER
PRESS 1 FOR EVMBUG MONITOR
PRESS 2 FOR CORTEX BASIC
PRESS 3 FOR MDEX
PRESS 4 FOR UNIX
```
To select a software application from the menu, press the corresponding numeric key.

## Useful Links
MDEX documentation is available on the site http://www.powertrancortex.com/documentation.html. The documentation and MDEX disks were recovered and made available by Dave Hunter, the owner of the www.powertrancortex.com site. Porting of the MDEX files to the Mini-Cortex was by Paul Ruizendaal. On the Mini-Cortex, the files are stored in two emulated disks on the CF card.

The first version of Unix ported was LSX Unix (https://www.mailcom.com/lsx/), which was designed to run with 40K byte RAM and two 256K byte floppies. LSX Unix was adapted from Unix V6 by Dr. Heinz Lycklama (https://www.60bits.net/msu/mycomp/terak/termubel.htm). This port has been further ported to the TI 990 minicomputer by Dave Pitts, who has also made improvements to the code build system.

[Dave Pitts' TI-990 web page.](https://www.cozx.com/dpitts/ti990.html)
[EVMBUG documentation](https://www.stuartconner.me.uk/tibug_evmbug/tibug_evmbug.htm#evmbug)


## Bugs/To Do

BUILD ONE AND TEST IT :)

## Troubleshooting

The first test image tests just the processor (TMS9995_test_1_eprom_image), the EPROM and the EPROM chip select logic. Program and fit the EPROM, power on, press the reset switch.  The CPU should execute a loop from ROM., then use the logic probe to check for a series of pulses on pin 20 of the EPROM every 2 seconds or so. 

If the first test passes, repeat with the second test image (TMS9995_test_2_eprom_image), which also tests the RAM and the RAM chip select logic. With the second test, a series of pulses should be seen on pin 20 of the EPROM every 6 seconds or so.

The third test image has two versions.   The first version configures the serial port to 9600 Baud, 7 bits/character, even parity, 2 stops bits, no flow control, and then loops continually sending the ASCII characters >21 ("!") to >7E .  This version tests most of the system including ROM and RAM.   The second version is a ROM only version. It also configures the serial port to 9600 Baud, 7 bits/character, even parity, 2 stops bits, no flow control, and then loops continually sending the ASCII characters >21 ("!") to >7E , but does not use any external RAM.



## Patches
 
## BOM
Reference|Value|Qty|Description
---------|-----|---|-----------
C1,C2,C3,C4,C5,C6,C7,C8,C9,C10,C11,C12,C13,C14,C15,C16,C17,C19,C21,C23,C25,C27,C29|0.1u|23|
C18,C20,C22,C24|10u|4|
C26|22u|1|
C28|47uF|1|
C30|1uF|1|Polarized capacitor
C31|10uF|1|Polarized capacitor
C32|0.1 uF|1|Unpolarized capacitor
C33|0.01 uF|1|Unpolarized capacitor
C34|100µF|1|
C35|0.1µF|1|
C40,C41|15pf|2|
D1|PWR|1|LED
D2|IDLE|1|LED
D3|MAP|1|LED
D5|1N5817|1|20V 1A Schottky Barrier Rectifier Diode, DO-41
D6|IDE ACT|1|LED
D8|Tx|1|LED
D9|Rx|1|LED
D10,D11|1N4148|2|100V 0.15A standard switching diode, DO-35
J1|F18A TANG NANO Connector|1|Generic connector, double row, 02x10, odd/even pin numbering scheme (row 1 odd numbers, row 2 even numbers), script generated (kicad-library-utils/schlib/autogen/connector/)
J2,J8,J11,J13,J15,J17|IRQ Selection|6|Generic connector, double row, 02x06, odd/even pin numbering scheme (row 1 odd numbers, row 2 even numbers), script generated (kicad-library-utils/schlib/autogen/connector/)
J3,J10,J12,J14,J16,J18|Bus_I_not_SA_8bit|6|8-bit ISA-PC bus connector
J4|Enable Serial Int|1|Generic connector, single row, 01x02, script generated
J5|TTL SERIAL 0|1|
J6|TTL SER 0 PWR|1|
J7|USER OUT|1|Generic connector, single row, 01x07, script generated
J9|AUDIO OUT|1|coaxial connector (BNC, SMA, SMB, SMC, Cinch/RCA, LEMO, ...)
J19|Parallel Keyboard IN|1|Generic connector, single row, 01x11, script generated
JP1|JUMPER|1|Generic connector, single row, 01x02, script generated (kicad-library-utils/schlib/autogen/connector/)
JP4|EXT IDE LED|1|
JP5|IDE PWR|1|
P1|14.31818 MHz CLK|1|
P4|8 BIT IDE (CF)|1|
P5|CONN_10X2|1|Generic connector, double row, 02x10, odd/even pin numbering scheme (row 1 odd numbers, row 2 even numbers), script generated (kicad-library-utils/schlib/autogen/connector/)
P6|POWER_SW|1|Generic connector, single row, 01x02, script generated (kicad-library-utils/schlib/autogen/connector/)
R1,R3,R6,R7,R11,R12,R13,R17|470|8|-- mixed values --
R2,R4,R8,R10,R14|10K|5|
R5|1000|1|Resistor
R9|10Ω|1|
R16,R18|10k|2|Resistor
SW1|RESET|1|Push button switch, generic, two pins
SW2|POWER|1|Push button switch, generic, two pins
U1|74LS14|1|
U2|74LS07|1|
U3|TMS9995-DIP_1|1|
U4|74LS04|1|
U5,U27|74LS74|2|-- mixed values --
U6|74LS112|1|
U7|SN76489|1|
U8|DS1233|1|
U9|SRAM_512K|1|512K x 8 Low Power CMOS RAM, DIP-32
U10|TMS9918 EMU|1|
U11,U30|GAL22V10|2|
U12,U13|74LS32|2|Quad 2-input OR
U14,U28,U29,U31|74LS244|4|Octal Buffer and Line Driver With 3-State Output, active-low enables, non-inverting outputs
U15|74LS138|1|
U17|74LS688|1|
U18|27C256|1|
U21|TMS9902A|1|
U22|74LS259|1|
U23|74LS612-DIP|1|
U25|74LS245|1|Octal BUS Transceivers, 3-State outputs
U26|74HCT14|1|Hex inverter schmitt trigger
Y1|12MHz|1|
