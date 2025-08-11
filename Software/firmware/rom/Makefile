all:  	rom.bin bootsel.s19 mdexboot.s19 unixboot.S19 monitor.s19


monitor.s19: monitor.asm
	asm990  -l monitor.lst -o monitor.obj monitor.asm
	lnk990 -a -o monitor.out monitor.obj
	srec_cat  monitor.out -binary -exclude 0x0000 0x0012 -offset -0x0012 -o monitor.s19 -Data_Only

bootsel.s19: bootsel.asm
	asm990  -l bootsel.lst -o bootsel.obj bootsel.asm
	lnk990 -a -o bootsel.out bootsel.obj
	srec_cat  bootsel.out -binary -exclude 0x0000 0x0012 -offset 0x14EE -o bootsel.s19 -Data_Only

mdexboot.s19: mdexboot.asm
	asm990  -l mdexboot.lst -o mdexboot.obj mdexboot.asm
	lnk990 -a -o mdexboot.out mdexboot.obj
	srec_cat  mdexboot.out -binary -exclude 0x0000 0x0012 -offset 0x77EE -o mdexboot.s19 -Data_Only

unixboot.S19: unixboot.asm
	asm990  -l unixboot.lst -o unixboot.obj unixboot.asm
	lnk990 -a -o unixboot.out unixboot.obj
	srec_cat  unixboot.out -binary -exclude 0x0000 0x0012 -offset 0x7AEE -o unixboot.s19 -Data_Only


rom.bin: bootsel.s19 mdexboot.s19 unixboot.S19
	srec_cat  bootsel.s19 mdexboot.s19 unixboot.S19 -o rom.bin -binary


pretty:
	cp *.asm ./backup
	pretty6502 -p4 -a0 -du -mu -c50  ./backup/bootsel.asm bootsel.asm
	pretty6502 -p4 -a0 -du -mu -c50  ./backup/mdexboot.asm mdexboot.asm
	pretty6502 -p4 -a0 -du -mu -c50  ./backup/unixboot.asm unixboot.asm
	pretty6502 -p4 -a0 -du -mu -c50  ./backup/monitor.asm monitor.asm

clean:
	@rm -f *.obj
	@rm -f *.map
	@rm -f *.lst
	@rm -f *.bin
	@rm -f *.out
	@rm -f *.hex
	@rm -f *.s19
	@rm -f *.bak
