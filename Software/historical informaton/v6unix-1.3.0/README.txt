V6 Unix 1.3.0
Date: 2020/10/01

I. Introduction

The V6 Unix distribution supports the TI-990 line of minicomputers. The models
supported are 990/10, 990/10A and 990/12 with a minimum of 512K of memory.
Also, supported is the Powertran Cortex using the TI TMS9995 chip with extended
memory.

The devices support in the version:

   DISK      - Any 10MB or larger TILINE disk. Address >F800 interrupt 13.
   TAPE      - TILINE attached tape drive. Address >F880 interrupt 9.
   Printer   - Serial printer on CRU >60 interrupt 14.
   Console   - Serial terminals:
      TTYEIA: TTY/EIA Card:
         tty0 - CRU >0 interrupt 6
      TTY9902: TMS9902 console port:
         tty0 - 990/10A CRU >1700 interrupt 8, Cortex CRU >0 interrupt 4.
   Terminals   - Serial terminals:
      VDT911: 911 VDT, runs as a "glass" tty:
         tty1 - port 0, CRU >100 interrupt 10
         tty2 - port 1, CRU >120 interrupt 8
      TTY403: CI 403 serial multiplexor, Address >F980 interrupt 11:
         tty3 - port 0.
         tty4 - port 1.
         tty5 - port 2.
         tty6 - port 3.


II. Build the System

To build the V6 Unix you need to download and extract the source tarball:

   $ tar xzf v6unix-1.3.0-source.tar.gz
   $ cd v6unix

Then run make:

   $ make [MODEL=model] [DISK=disk] [CONSOLE=console] [MAXSIZE=maxsize]

Where:

   model   - TI990, TI990_10A, TI990_12 or TI_CTX, Default = TI990.
             TI990     - Standard TI-990.
             TI990_10A - /10A support, console port and Extended instructions.
             TI990_12  - /12 support, Extended instructions.
	     TI_CTX    - Powertran Cortex using a TMS9995 chip.

   disk    - DS10, DS25, DS50, DS80, DS200, DS300 or larger disk,
	     Default = DS50.

   console - TTYEIA or TTY9902, Default = TTYEIA.
             TTYEIA  - TTYEIA cards.
             TTY9902 - TMS9902 console port. Only on TI990_10A and TI_CTX.

   maxsize - Size of / filesystem in bytes. Default = 33024000 (64500 blocks).
             DS10 = 3643000 (7115 blocks)
	     DS25 = 19301000 (37697 blocks)

The swap area, 1024 blocks, limits the size of the / file system to 33024000
bytes or 64500 blocks. V6 Unix limits the size of filesystems to a mximum of
65535 blocks. On smaller devices the root partition on the primary device will
be the size of the device minus the swap area.


III. Run the System

To run the system as shown here you will need to download, build and install 
the sim990 simulator. The Powertran Cortex is not supported by sim990.

1. To run under sim990, with the default build, place the following into a
   configuration file (v6unix.cfg):

   a ce >0 6 TTY
   a pe >60 14 print.out
   a v0 >100 10 2011
   a v1 >120  8 2012
   a a30 >F980 11 LOOP
   a a31/t >F980 11 2013
   a a32/t >F980 11 2014
   a m0 >F880 9 NULL
   a d0 >F800 13 v6unix.dsk
   b d0

   The terminal tty0 is the console and the additional terminals tty1, tty2,
   tty4 and tty5 are accessible using telnet on ports 2011, 2012, 2013 and 2014.

   The terminals tty1 and tty2 are 911 VDTs. The terminals tty4 and tty5 are 
   CI403 connected serial terminals.

   The terminals have the login process started on them based upon the
   /etc/ttys file (man 5 ttys). As a default only the console (/dev/tty0) has
   a login process.

   tty3 is allocated to the network process as SLIP device sl0. The
   configuration file, above, is set to loopback. For a real serial connection
   you can configure the device for example:

   a a30/r >F980 11 /dev/tty1

2. Start sim990

   $ sim990 -msc 12 1m v6unix.cfg
   TI 990 simulator 3.5.6: /12 CPU 1024K

   Mem................1024KB
   Initialize Date/Time
      Year: 2020
     Month: 8
       Day: 6
      Hour: 8
    Minute: 49
   Thu Aug 06 08:49:00 2020
   Booting V6 Unix
   Network process running.

   login: 

3. Login

   Two accounts have been provided for user:

   Username: root	Password: root
   Username: guest	Password: guest 

   For example:

   login: guest
   Password: 
   ***********************************
   *   Running Unix V6 on a TI-990   *
   ***********************************

   $ ls
   hello.c        makefile       romans.l       
   $ make
   cc -o hello hello.c
   lex -t romans.l >romans.c
   cc -o romans romans.c -L/lib -lln
   $ hello
   Hello World!
   $ romans
   MMXIV IX
   2014 + 9 = 2023
   $ exit

   login:


IV. Disk Partitions

This implementation supports a maximum of 16 partitions of a maximum of 65535
blocks per partition. When the system is booted it probes the disk geometry and 
creates a table in the kernel of active drives and paritions. Then /dev entries
are created for each active disk and partition. The boot disk partition, if on
unit 0, is /dev/dsk0. Additional, as needed, partitions on the drive are
/dev/dsk0p1 through /dev/dsk0p15. On smaller disks you may only have the root
partition. This works out to a maximum supported disk size of 512MB.

For example on the default DS50 disk there are only two partitions:

   /dev/dsk0   - Root/boot partition (64500 blocks, 1024 blocks swap).
   /dev/dsk0p1 - 11836 blocks.

For a DS200 the partitions are:

   /dev/dsk0   - Root/boot partition (64500 blocks, 1024 blocks swap).
   /dev/dsk0p1 - 65535 blocks.
   /dev/dsk0p2 - 65535 blocks.
   /dev/dsk0p3 - 65535 blocks.
   /dev/dsk0p4 - 31368 blocks.

For a DS10 only the root partition, less the swap area.

   /dev/dsk0   - Root/boot partition (7115 blocks, 1024 blocks swap).

The mkfs program, /etc/mkfs, will probe the size of the partition, as default,
and create the filesystem to the maximum size for that partition. For example
on the default DS50 disk:

   # /etc/mkfs /dev/dsk0p1
   # /etc/mount /dev/dsk0p1 /mnt
   # df
   Filesystem    512B-blocks       Used  Available Use% Mount
   /dev/dsk0           64500       4107      60393   6% /
   /dev/dsk0p1         11836        222      11614   1% /mnt

