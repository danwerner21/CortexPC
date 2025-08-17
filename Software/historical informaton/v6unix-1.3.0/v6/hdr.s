// Assemble together with one of the .cfg files to select a model

// Define netmain wait channel

	.globl		_netmain
	_netmain	= 0x0080

// Machine parameters for each model
//

.if TI990
	EIS		= 0		// CPU does not have extended inst.
.endif

.if TI990_10A + TI990_12
	EIS		= 1		// CPU has extended instruction set
.endif

.if CORTEX
	EIS		= 1		// CPU has extended instruction set
	mapper		= 0x0040
	HZ		= 60

	mapon		= 1		// Enable mapper CRU bit
.endif
.if 1-CORTEX
	printcru	= 0x0060	// EIA board printer base address
	mapper		= 0x1fa0	// Memory mapper
	panelcru	= 0x1fe0	// panel leds CRU base address
	errcru		= 0x1fc0	// Error CRU
	HZ		= 120

	mapon		= 3		// Enable mapper CRU bit
	mapclr		= 4		// Clear mapper error CRU bit
	tltimout	= 15		// TILINE timout CRU bit
.endif

.if TTYEIA
	tty0addr	= 0x0000	// EIA board TTY0 base address
.endif

.if TTY9902
.if TI990_10A
	tty0addr	= 0x1700	// 9902 TTY0 base address
.endif
.if CORTEX
	tty0addr	= 0x0000	// 9902 TTY0 base address
.endif
.endif
	tty1addr	= 0xFFFF	// Null device address.
	tty2addr	= 0xFFFF	// Null device address.
	tty3addr	= 0xFFFF	// Null device address.

.if VDT911
	v9110addr	= 0x0100	// 911 VDT board TTY1 base address
	v9111addr	= 0x0120	// 911 VDT board TTY2 base address
	v9112addr	= 0xFFFF	// Null device address.
	v9113addr	= 0xFFFF	// Null device address.
.endif

.if TTY403
	c4030addr	= 0xF980	// CI403 board TTY[2-5] base address
	c4031addr	= 0xFFFF	// Null device address.
	c4032addr	= 0xFFFF	// Null device address.
	c4033addr	= 0xFFFF	// Null device address.
.endif

// CRU bits for the 9902 ACA chip + its external bits
//
.if TTY9902
	// 9902 ACA output bits
	ldir		= 13	// select rate control word
	rtson		= 16	// set RTS on/off
	rienb		= 18	// read interrupt enable
	xienb		= 19	// xmit interrupt enable
	timenb		= 20	// timer interrupt enable
	dscenb		= 21	// DSC state change interrupr enable
	reset		= 31	// full reset

	// 9902 ACA input bits
	rbint		= 16	// read interrupt status
	xbint		= 17	// xmit interrupt status
	rbrl		= 21	// read buffer loaded
	xbre		= 22	// xmit buffer empty

.if TI990_10A
	// TI990/10a external bits
	acadtr		= 32	// set DTR on/off
	acaien		= 38	// interrupt enable
	acaenb		= 39	// switch ACA on/off 
	// settings for 9600-8N1 (9902 is clocked at 2.5 MHz)
	acarat		= 0x002b
	acacnf		= 0x8300
.endif
.if CORTEX
	// settings for 2400-8N1 (9902 is clocked at 2.5 MHz)
	acarat		= 0x00d0
	acacnf		= 0x8300
.endif
.endif

// CRU bits for the TTYEIA interface board
//
	// EIA board I/O bits
	ttyxmit		= 8 	// transmitting
	ttydtr		= 9	// set DTR on/off
	ttyrts		= 10	// set RTS on/off
	ttywrq		= 11	// xmit interrupt status + reset
	ttyrrq		= 12	// read interrupt status + reset
	ttynsf		= 13	// reset new status flag + interrupt
	ttyeint		= 14	// Enable interrupts
