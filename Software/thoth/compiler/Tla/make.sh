#!/bin/sh

gcc -g -o tla input.c symtab.c ../Lex/lib.c token.c parse.c expr.c load.c ti990.c

