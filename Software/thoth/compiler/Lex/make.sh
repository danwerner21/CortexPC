#!/bin/sh

gcc -o lex -g input.c token.c symtab.c string.c math.c lib.c
gcc -o plexil plexil.c

