
int
Length(char *s)
{
    int i = 0;

    while(s[i] != 0 ) ++i;
    return i;
}

int
Equal(char *s1, char *s2)
{
    int i = 0, ch;
    while( (ch = s1[i]) != 0 && ch == s2[i] ) ++i;
    return s1[i] == s2[i];
}

char *
Copy(char *src, char *dst)
{
    int i;
    for( i = 0; src[i] != 0; ++i ) dst[i] = src[i];
    dst[i] = 0;
    return( dst+i );
}
