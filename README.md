# TMS9995 CPU Card

*** THIS IS UNTESTED DO NOT BUILD ***

This is an ATX format implimentation of Stuart Conner's Mini-Cortex system.  The Mini-Cortex system is a further development of his TMS 9995 breadboard project to produce a system similar to a Powertran Cortex, but using more modern components. The system is based around a TMS 9995 running at 3 MHz, with 32K byte EEPROM, and will use 512K bytes of RAM from a Duodyne ROMRAM card accessed through a memory mapper. An on-board serial port and IDE interface exists on the card.  There is also provisions for a TMS9918 clone, a keyboard and some expansion slots -- these are not supported by the firmware at this time however.

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

Note that the TMS9995 uses the RAM3 chip on the ROMRAM card, therefore it must be populated for the TMS9995 board to function.

## Patches
 
## BOM
