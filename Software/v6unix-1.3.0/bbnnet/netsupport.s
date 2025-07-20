// Define netmain wait channel

	.globl		_netmain
	_netmain	= 0x0080

// sum = cksum(struct mbuf *m, int len)

	.globl	_cksum
	.text
	
_cksum:
	mov	r11,r0
	bl	@csv
	clr	r4		// mlen  = 0
	clr	r8		// sum   = 0
	clr	r7		// swaps = 0;

dombuf:
	mov	@18(bp),r2	// mlen = m->m_len
	movb	@5(r2),r2
	jeq	nxtmbuf		// if (mlen==0) skip
	srl	r2,8
	mov	r2,r4

	mov	@18(bp),r2	// w = m + m->m_offs
	movb	@4(r2),r2
	srl	r2,8
	mov	r2,r5
	a	@18(bp),r5
	c	@20(bp),r4	// if (len < mlen) mlen = len;
	bjhe	1f
	mov	@20(bp),r4

	// figure out how many whole words left in this mbuf
1:	clr	r6		// convert mlen to word count
	mov	r4,r1
	srl	r1,1
	jnc	1f
	seto	r6		// set r6 if extra trailing byte
1:	mov	r1,r1
	jeq	3f		// if no words left, skip

	// process whole words
2:	a	(r5)+,r8	// sum += [words] (one's complement plus)
	jnc	1f
	inc	r8
1:	dec	r1
	jne	2b

	// process trailing byte, if any
3:	mov	r6,r6		// test trailing byte flag
	jeq	2f
	clr	r0		// tmp = *w++ << 8
	movb	(r5)+,r0
	a	r0,r8		// sum += tmp (one's complement plus)
	jnc	1f
	inc	r8	
1:	swpb	r8		// reverse sum bytes
	inc	r7		// keep track of number of swaps

2:	s	r4,@20(bp)	// len -= mlen
	jeq	done

nxtmbuf:
	mov	@18(bp),r2	// m = m->m_next
	mov	(r2),@18(bp)
	jne	dombuf		// while (m)

done:
	srl	r7,1		// if odd number of bytes, reverse swap
	jnc	1f
	swpb	r8
1:	mov	r8,r2
	inv	r2
	b	@cret

