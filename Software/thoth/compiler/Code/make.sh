#!/bin/sh

cat B/table.mac | tool/tabgen >tables.c
gcc -o Code -g main.c code.c load.c defer.c expr.c symbol.c \
    twit.c modify.c regs.c arith.c tables.c select.c

