unsigned long
inet_addr (ip)
char *ip;
{
   unsigned long a;
   int digs[4];
   int i;

   a = 0;
   digs[0] = 0;
   digs[1] = 0;
   digs[2] = 0;
   digs[3] = 0;
   i = 0;
   for (; *ip; ip++)
   {
      if (*ip >= '0' && *ip <= '9')
         digs[i] = (digs[i] * 10) + (*ip - '0');
      else if (*ip == '.')
         i++;
      else
         break;
   }
   a = digs[0];
   a = (a << 8) | digs[1];
   a = (a << 8) | digs[2];
   a = (a << 8) | digs[3];
   return a;
}
