#!/bin/sh

./eh globals.eh;  ../Ale/Ale Thoth -u eh.out
./eh alloc.eh;    ../Ale/Ale Thoth -u eh.out
./eh dispatch.eh; ../Ale/Ale Thoth -u eh.out
./eh create.eh;   ../Ale/Ale Thoth -u eh.out
./eh sendrcv.eh;  ../Ale/Ale Thoth -u eh.out
./eh time.eh;     ../Ale/Ale Thoth -u eh.out
./eh fred.eh;     ../Ale/Ale Thoth -u eh.out
./eh file.eh;     ../Ale/Ale Thoth -u eh.out
./eh devices.eh;  ../Ale/Ale Thoth -u eh.out
./eh tty.eh;      ../Ale/Ale Thoth -u eh.out
./eh rtc.eh;      ../Ale/Ale Thoth -u eh.out
../Tla/Tla disc.as; ../Ale/Ale Thoth -u eh.out
./eh init.eh;

rm strings manifests lexil externals functions cleanil data.out

../uld/uld $1 eh.out Thoth
