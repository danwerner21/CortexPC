char dot[] = ".";
char dotdot[] = "..";
char root[] = "/";
char name[512];
int rino = 1, file, off = -1;
struct statb {int devn, inum, i[18];} x;
struct entry { int jnum; char name[16];} y;

main() {
	int n;

	for (;;) {
		stat(dot, &x);
		if(x.inum == rino) {
			write(1,root,1);
			prname();
		}
		if((file = open(dotdot,0)) < 0) prname();
		do {
			if ((n = read(file,&y,16)) < 16) prname();
		} while (y.jnum != x.inum);
		close(file);
		cat();
		chdir(dotdot);
	}
}

prname() {
	if(off<0)off=0;
	name[off] = '\n';
	write(1,name,off+1);
	exit();
}

cat() {
	int i, j;

	i = -1;
	while(y.name[++i] != 0);
	if((off+i+2) > 511) prname();
	for(j=off+1; j>=0; --j) name[j+i+1] = name[j];
	off=i+off+1;
	name[i] = root[0];
	for(--i; i>=0; --i) name[i] = y.name[i];
}
