#!/bin/sh

./yacc eh_gram.y
gcc -o Parse -g y.tab.c input.c expr.c ref.c
gcc -o pil pil.c
