#!/bin/sh

# assemble kernel files, place in link
cd kernel
echo link/mdex.rel=kernel/mdexlong.asm
../../emu/host 1/asm ../link/mdex.rel=mdexlong.asm

# assemble driver files, place in link
cd ../drivers
echo link/term.rel=drivers/term9902.asm
../../emu/host 1/asm ../link/term.rel=term9902.asm
echo link/print.rel=drivers/nullprnt.asm
../../emu/host 1/asm ../link/print.rel=nullprnt.asm
echo link/disk.rel=drivers/idedsk.asm
../../emu/host 1/asm ../link/disk.rel=idedsk.asm

# assemble config file
cd ../link
../../emu/host 1/asm config.rel=config.asm

# link the kernel, kernel image is mdex.sav
echo Linking kernel...
../../emu/host 1/link ../mdex.sav=@mdex.lnk

# rm *.rel


