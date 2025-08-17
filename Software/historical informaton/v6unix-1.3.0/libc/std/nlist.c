#include <stdlib.h>
#include <string.h>
#include <a.out.h>

#define SPACE 100

struct exec hdr;

nlist(filename, nl)
	char *filename;
	struct nlist *nl;
{
	register struct nlist *p, *q;
	struct nlist space[SPACE];
	int f, count, n, m, k = 0;
	unsigned long offs;

	if(nl==0) return;
	for(p = nl; p->n_name[0]; p++) {
		p->n_type = 0;
		p->n_value = 0;
		k++;
	}
	if((f = open(filename, 0))<0) return;

	if(read(f, &hdr, sizeof(hdr))!=sizeof(hdr))
		goto err;
	count = hdr.a_syms/12;
	offs = N_SYMOFF(hdr);
	lseek(f, offs, 0);

	while(count && k) {
		m = SPACE;
		if(count < SPACE)
			m = count;
		count -= m;
		n = m * sizeof(struct nlist);
		if(read(f, space, n)!=n)
			goto err;
		for(q = space; m--; q++) {  
			for(p = nl; p->n_name[0]; p++) {
				if(p->n_type) continue;
				if(strncmp(p->n_name, q->n_name, 8)==0) {
					p->n_type = q->n_type;
					p->n_value = q->n_value;
					k--;
					break;
				}
			}
		}
	}

err:
	close(f);
}

