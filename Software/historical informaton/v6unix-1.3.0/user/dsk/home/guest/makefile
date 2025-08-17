all: hello romans

clean:
	-rm -f hello romans romans.c

hello: hello.c
	cc -o hello hello.c

romans: romans.c
	cc -o romans romans.c -L/lib -lln

romans.c: romans.l
	lex -t romans.l >romans.c
