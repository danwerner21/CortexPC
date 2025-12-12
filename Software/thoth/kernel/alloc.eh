
.Init_core(initial_addr,len)

\ setup initial free chain for dynamic memory allocation

{
    extrn .Rover,.Memsize;
    auto l;
    
\ if not much to start, try to add more
\   if (len < 30) initial_addr = .Add_mem(len = 1024);
    if ((initial_addr & 1) != 1)    \ make last bit one;
    {
        ++initial_addr;
        --len;
    }
    .Zero( initial_addr, len );
    initial_addr[0] = 1;    \ indicate low core is allocated
    if ((len & 1) == 1) --len;
    len -= 2;           \ reserve dummy descriptors at ends
    initial_addr[1] = initial_addr[len] = len;
    initial_addr[len + 1] = -len + 1;    \ wrap around pointer
    .Rover = initial_addr + 1;
    .Memsize  = len - 2;
}


.Alloc_vec(size)

\ get a vector of size+1 words

{
    extrn .Rover,.Mark;
    auto alloc, l;
    
    if (size < 0)
    {
        \.Print_error("bad memory request");
        \.Panic();
        ; \printf("bad");
    }
    ++size;         \ getting words 0 to size - total of size+1
    if( size & 1) ++size;   \ make request even
    size += 2;      \ allow for two descriptor words
    disable;
    .Mark = .Rover;
    while ((size > (l = .Rover[0])) ||  \ block is too small
            (l & 1))                    \ or in use
    {
        l &= ~1;                        \ set last bit to 0
        .Rover += l;
        if (.Rover == .Mark)
        {
            printf("out of memory*n");
            .Panic();
            \.Grow_mem(size);
            \.Mark = .Rover;
        }
        enable;
        disable;
    }
    
\ we now have a free block which is large enough

    alloc = .Rover + 1;     \ save addr to return
    if (l - size > 10)  \ not worth splitting if not
    {
        .Rover[0] = .Rover[size - 1] =
                    size + 1;           \ indicate allocated
\ set .Rover to next block and change end pointers for this block
        .Rover[0] = (.Rover += size)[l - 1] = (l -= size);
    }
    else
    {
        .Rover[0] += 1;
        .Rover[l - 1] += 1;
        .Rover += l;
    }
    .Mark = .Rover;
    enable;
    return(alloc);
}

.Free( vec )

\ free a block which has previously been gotten via .Alloc_vec
\ or .Alloc_str

{
    extrn .Rover,.Mark;
    auto l,t;
    --vec;
    l = vec[0] - 1;
    if (vec[l-1] != vec[0] || (vec[0] & 1) != 1)
    {
        \Print_error("attempt to release bad record");
        \.Panic();
        ; \ printf("bad");
    }
    disable;
    if ( ( (t = *(vec-1))&1 ) == 0 )
    {
        l += t;
        vec -= t;
    }
    if (((t = vec[l]) & 1) == 0) l += t;
    vec[0] = vec[l-1] = l;
    
\ in case we have coalesced, set .Rover to beginning of this block
\ and set .Mark to .Rover
    .Mark = .Rover = vec;
    enable;
}


