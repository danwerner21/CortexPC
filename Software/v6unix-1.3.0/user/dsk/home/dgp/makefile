all: hello crash test1 goodbye ttape romans

clean:
	-rm -f ttape hello crash test1 goodbye
	-rm -f romans romans.c

goodbye: goodbye.c
	cc -o goodbye goodbye.c

hello: hello.c
	cc -o hello hello.c

test1: test1.c
	cc -o test1 test1.c

crash: crash.s
	cc -o crash crash.s

ttape: ttape.c
	cc -o ttape ttape.c

romans: romans.c
	cc -o romans romans.c -L/lib -lln

romans.c: romans.l
	lex -t romans.l >romans.c
