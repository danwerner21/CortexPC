#!/bin/sh

rm -r ../libb
mkdir ../libb
../Tla/Tla ehrt.as
../Ale/Ale ../libb -a eh.out
./eh print.eh
../Ale/Ale ../libb -a eh.out
rm eh.out
echo "Library ../libb is:"
ls -la ../libb

