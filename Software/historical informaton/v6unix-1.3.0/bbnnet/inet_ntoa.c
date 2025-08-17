#include <stdio.h>

static char *
pdec (n, s)
   int n;
   char *s;
{
   int i, j;
   char t[6];

   j = 0;
   if (n == 0)
      t[j++] = '0';
   else while (n) {
      i = n % 10;
      n = n / 10;
      t[j++] = i + '0';
   }
   for (; j; j--) {
      *s++ = t[j-1];
   }
   *s = 0;
   return s;
}

char *
inet_ntoa (a)
unsigned long a;
{
   static char ip[16];
   char *s;
   int o[4], i;

   for (i = 0; i < 4; i++)
   {
      o[i] = a & 0xFF;
      a = a >> 8;
   }
   s = pdec (o[3], ip);
   *s++ = '.';
   s = pdec (o[2], s);
   *s++ = '.';
   s = pdec (o[1], s);
   *s++ = '.';
   s = pdec (o[0], s);
   *s++ = 0;
   return ip;
}
