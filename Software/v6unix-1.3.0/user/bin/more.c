/*
** more.c - General purpose tty output filter and file perusal program
*/

#include <stdio.h>
#undef	putchar                 /* force use of function rather than macro */
#include <sys/types.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sgtty.h>
#include <setjmp.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define Fopen(s,m)	(Currline = 0,file_pos=0,fopen(s,m))
#define Ftell(f)	file_pos
#define Fseek(f,off)	(file_pos=off,fseek(f,off,0))
#define Getc(f)		(++file_pos, getc(f))
#define Ungetc(c,f)	(--file_pos, ungetc(c,f))

#define MBIT	CBREAK

#define LINSIZ	256
#define RUBOUT	'\177'
#define ESC	'\033'

static struct sgttyb otty, savetty;
static long file_pos, file_size;
static int fnum, no_intty, no_tty, slow_tty;
static int dlines;
static int nscroll = 11;        /* Number of lines scrolled by 'd' */
static int fold_opt = 1;        /* Fold long lines */
static int stop_opt = 1;        /* Stop after form feeds */
static int ssp_opt = 0;         /* Suppress white space */
static int ul_opt = 1;          /* Underline as best we can */
static int promptlen;
static int Currline;            /* Line we are currently at */
static int startup = 1;
static int firstf = 1;
static int notell = 1;
static int docrterase = 0;
static int bad_so;              /* True if overwriting does not turn off standout */
static int inwait, Pause, errors;
static int within;              /* true if we are within a file,
                                   false if we are between files */
static int dumb, noscroll, hardtabs;
static char **fnames;           /* The list of file names */
static int nfiles;              /* Number of files left to process */
static char ch;
static jmp_buf restore;
static char Line[LINSIZ];       /* Line buffer */
static int Lpp = 24;            /* lines per page */
static char *eraseln;           /* erase line */
static char *chUL;              /* underline character */
static char *chBS;              /* backspace character */
static int Mcol = 80;           /* number of columns */
static int Wrap = 1;            /* set if automargins */
static int ulglitch;            /* terminal has underline mode glitch */
static int pstate = 0;          /* current UL state */

struct
{
   long chrctr, line;
} context, screen_start;

static int lastcmd, lastarg, lastp;
static int lastcolon;

extern int ungetc ();

void
argscan (s)
     char *s;
{
   for (dlines = 0; *s != '\0'; s++)
   {
      switch (*s)
      {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
         dlines = dlines * 10 + *s - '0';
         break;
      case 'l':
         stop_opt = 0;
         break;
      case 'f':
         fold_opt = 0;
         break;
      case 'p':
         noscroll++;
         break;
      case 's':
         ssp_opt = 1;
         break;
      case 'u':
         ul_opt = 0;
         break;
      }
   }
}

/*
** Print an integer as a string of decimal digits,
** returning the length of the print representation.
*/

int
printd (n)
     register int n;
{
   register int a, nchars;

   if (a = n / 10)
      nchars = 1 + printd (a);
   else
      nchars = 1;
   putchar (n % 10 + '0');
   return (nchars);
}

/*
**  Print string and return number of characters
*/

int
pr (s1)
     char *s1;
{
   register char *s;
   register char c;

   for (s = s1; c = *s++;)
      putchar (c);
   return (s - s1 - 1);
}

/* Simplified printf function */

int
lprintf (fmt, args)
     register char *fmt;
     int args;
{
   register int *argp;
   register char ch;
   register int ccount;

   ccount = 0;
   argp = &args;
   while (*fmt)
   {
      while ((ch = *fmt++) != '%')
      {
         if (ch == '\0')
            return (ccount);
         ccount++;
         putchar (ch);
      }
      switch (*fmt++)
      {
      case 'd':
         ccount += printd (*argp);
         break;
      case 's':
         ccount += pr ((char *) *argp);
         break;
      case '%':
         ccount++;
         argp--;
         putchar ('%');
         break;
      case '0':
         return (ccount);
      default:
         break;
      }
      ++argp;
   }
   return (ccount);

}

/*
** Check whether the file named by fs is an ASCII file which the user may
** access.  If it is, return the opened file. Otherwise return NULL.
*/

FILE *
checkf (fs, clearfirst)
     register char *fs;
     int *clearfirst;
{
   struct stat stbuf;
   register FILE *f;
   char c;

   if (stat (fs, &stbuf) == -1)
   {
      fflush (stdout);
      perror (fs);
      return (NULL);
   }
   if ((stbuf.st_mode & S_IFMT) == S_IFDIR)
   {
      lprintf ("\n*** %s: directory ***\n\n", fs);
      return (NULL);
   }
   if ((f = Fopen (fs, "r")) == NULL)
   {
      fflush (stdout);
      perror (fs);
      return (NULL);
   }
   c = Getc (f);

   if (c == '\f')
      *clearfirst = 1;
   else
   {
      *clearfirst = 0;
      Ungetc (c, f);
   }
   if ((file_size = stbuf.st_size) == 0)
      file_size = 0x7fffffffL;
   return (f);
}

/*
** Get a logical line
*/

int
getline (f, length)
     register FILE *f;
     int *length;
{
   register int c;
   register char *p;
   register int column;
   static int colflg;

   p = Line;
   column = 0;
   c = Getc (f);
   if (colflg && c == '\n')
   {
      Currline++;
      c = Getc (f);
   }
   while (p < &Line[LINSIZ - 1])
   {
      if (c == EOF)
      {
         if (p > Line)
         {
            *p = '\0';
            *length = p - Line;
            return (column);
         }
         *length = p - Line;
         return (EOF);
      }
      if (c == '\n')
      {
         Currline++;
         break;
      }
      *p++ = c;
      if (c == '\t')
         if (hardtabs && column < promptlen)
         {
            if (eraseln && !dumb)
            {
               column = 1 + (column | 7);
               promptlen = 0;
            }
            else
            {
               for (--p; column & 7 && p < &Line[LINSIZ - 1]; column++)
               {
                  *p++ = ' ';
               }
               if (column >= promptlen)
                  promptlen = 0;
            }
         }
         else
            column = 1 + (column | 7);
      else if (c == '\b' && column > 0)
         column--;
      else if (c == '\r')
         column = 0;
      else if (c == '\f' && stop_opt)
      {
         p[-1] = '^';
         *p++ = 'L';
         column += 2;
         Pause++;
      }
      else if (c == EOF)
      {
         *length = p - Line;
         return (column);
      }
      else if (c >= ' ' && c != RUBOUT)
         column++;
      if (column >= Mcol && fold_opt)
         break;
      c = Getc (f);
   }
   if (column >= Mcol && Mcol > 0)
   {
      if (!Wrap)
      {
         *p++ = '\n';
      }
   }
   colflg = column == Mcol && fold_opt;
   *length = p - Line;
   *p = 0;
   return (column);
}

/* Print a buffer of n characters */

void
prbuf (s, n)
     register char *s;
     register int n;
{
   register char c;             /* next output character */
   register int state;          /* next output char's UL state */

#define wouldul(s,n)	((n) >= 2 && (((s)[0] == '_' && (s)[1] == '\b') || ((s)[1] == '\b' && (s)[2] == '_')))

   while (--n >= 0)
      if (!ul_opt)
         putchar (*s++);
      else
      {
         if (*s == ' ' && pstate == 0 && ulglitch && wouldul (s + 1, n - 1))
         {
            s++;
            continue;
         }
         if (state = wouldul (s, n))
         {
            c = (*s == '_') ? s[2] : *s;
            n -= 2;
            s += 3;
         }
         else
            c = *s++;
         if (state != pstate)
         {
            if (c == ' ' && state == 0 && ulglitch && wouldul (s, n - 1))
               state = 1;
         }
         if (c != ' ' || pstate == 0 || state != 0 || ulglitch == 0)
            putchar (c);
         if (state && *chUL)
         {
            pr (chBS);
            pr (chUL);
         }
         pstate = state;
      }
}

/*
** Erase the rest of the prompt, assuming we are starting at column col.
*/

void
erase (col)
     register int col;
{
   putchar ('\n');
   promptlen = 0;
}

void
prompt (filename)
     char *filename;
{
   promptlen = 8;
   pr ("--More--");
   if (filename != NULL)
   {
      promptlen += lprintf ("(Next file: %s)", filename);
   }
   else if (!no_intty)
   {
      promptlen += lprintf ("(%d%%)", (int) ((file_pos * 100) / file_size));
   }
   else
   {
      promptlen += pr ("[Press space to continue, 'q' to quit.]");
   }
   fflush (stdout);
   inwait++;
}

int
readch ()
{
   char ch;

   if (read (2, &ch, 1) <= 0)
   {
      if (errno != EINTR)
         exit (0);
      else
         ch = otty.sg_kill;
   }
   return (ch);
}

/*
** Read a decimal number from the terminal. Set cmd to the non-digit which
** terminates the number.
*/

int
number (cmd)
     char *cmd;
{
   register int i, ch;

   i = 0;
   ch = otty.sg_kill;
   for (;;)
   {
      ch = readch ();
      if (ch >= '0' && ch <= '9')
         i = i * 10 + ch - '0';
      else if (ch == otty.sg_kill)
         i = 0;
      else
      {
         *cmd = ch;
         break;
      }
   }
   return (i);
}

/*
** Skip nskip files in the file list (from the command line). Nskip may be
** negative.
*/

void
skipf (nskip)
     register int nskip;
{
   if (nskip == 0)
      return;
   if (nskip > 0)
   {
      if (fnum + nskip > nfiles - 1)
         nskip = nfiles - fnum - 1;
   }
   else if (within)
      ++fnum;
   fnum += nskip;
   if (fnum < 0)
      fnum = 0;
   pr ("\n...Skipping ");
   pr ("\n");
   pr ("...Skipping ");
   pr (nskip > 0 ? "to file " : "back to file ");
   pr (fnames[fnum]);
   pr ("\n");
   pr ("\n");
   --fnum;
}

/*
** Clean up terminal state and exit. Also come here if interrupt signal received
*/

void
reset_tty ()
{
   if (pstate)
   {
      fflush (stdout);
      pstate = 0;
   }
   otty.sg_flags |= ECHO;
   otty.sg_flags &= ~MBIT;
   stty (fileno (stderr), &savetty);
}

void
end_it ()
{

   reset_tty ();
   putchar ('\n');
   fflush (stdout);
   write (2, "\n", 1);
   exit (0);
}

/*
 * Execute a colon-prefixed command.
 * Returns <0 if not a command that should cause
 * more of the file to be printed.
 */

int
colon (filename, cmd, nlines)
     char *filename;
     int cmd;
     int nlines;
{
   if (cmd == 0)
      ch = readch ();
   else
      ch = cmd;
   lastcolon = ch;
   switch (ch)
   {
   case 'f':
      if (!no_intty)
         promptlen = lprintf ("\"%s\" line %d", fnames[fnum], Currline);
      else
         promptlen = lprintf ("[Not a file] line %d", Currline);
      fflush (stdout);
      return (-1);
   case 'n':
      if (nlines == 0)
      {
         if (fnum >= nfiles - 1)
            end_it ();
         nlines++;
      }
      putchar ('\r');
      erase (0);
      skipf (nlines);
      return (0);
   case 'p':
      putchar ('\r');
      erase (0);
      if (nlines == 0)
         nlines++;
      skipf (-nlines);
      return (0);
   case 'q':
   case 'Q':
      end_it ();
   default:
      return (-1);
   }
}

/*
** Skip n lines in the file f
*/

void
skiplns (n, f)
     register int n;
     register FILE *f;
{
   register char c;

   while (n > 0)
   {
      while ((c = Getc (f)) != '\n')
         if (c == EOF)
            return;
      n--;
      Currline++;
   }
}

void
copy_file (f)
     register FILE *f;
{
   register int c;

   while ((c = getc (f)) != EOF)
      putchar (c);
}

void
error (mess)
     char *mess;
{
   promptlen += strlen (mess);
   pr (mess);
   fflush (stdout);
   errors++;
   longjmp (restore, 1);
}

/*
** Read a command and do it. A command consists of an optional integer
** argument followed by the command character.  Return the number of lines
** to display in the next screenful.  If there is nothing more to display
** in the current file, zero is returned.
*/

int
command (filename, f)
     char *filename;
     register FILE *f;
{
   register int nlines;
   register int retval;
   register char c;
   FILE *helpf;
   char colonch;
   int done;
   char comchar;

#define ret(val) retval=val;done++;break

   done = 0;
   retval = 0;
   if (!errors)
      prompt (filename);
   else
      errors = 0;
   if (MBIT == RAW && slow_tty)
   {
      otty.sg_flags |= MBIT;
      stty (fileno (stderr), &otty);
   }
   for (;;)
   {
      nlines = number (&comchar);
      lastp = colonch = 0;
      if (comchar == '.')
      {                         /* Repeat last command */
         lastp++;
         comchar = lastcmd;
         nlines = lastarg;
         if (lastcmd == ':')
            colonch = lastcolon;
      }
      lastcmd = comchar;
      lastarg = nlines;
      if (comchar == otty.sg_erase)
      {
         prompt (filename);
         continue;
      }
      switch (comchar)
      {
      case ':':
         retval = colon (filename, colonch, nlines);
         if (retval >= 0)
            done++;
         break;
      case 'b':
         {
            register int initline;

            if (nlines == 0)
               nlines++;

            putchar ('\r');
            erase (0);
            lprintf ("\n");
            lprintf ("...back %d page", nlines);
            if (nlines > 1)
               pr ("s\n");
            else
               pr ("\n");

            pr ("\n");

            initline = Currline - dlines * (nlines + 1);
            if (!noscroll)
               --initline;
            if (initline < 0)
               initline = 0;
            Fseek (f, 0L);
            Currline = 0;       /* skiplns() will make Currline correct */
            skiplns (initline, f);
            if (!noscroll)
            {
               ret (dlines + 1);
            }
            else
            {
               ret (dlines);
            }
         }
      case ' ':
      case 'z':
         if (nlines == 0)
            nlines = dlines;
         else if (comchar == 'z')
            dlines = nlines;
         ret (nlines);
      case 'd':
         if (nlines != 0)
            nscroll = nlines;
         ret (nscroll);
      case 'q':
      case 'Q':
         end_it ();
      case 's':
      case 'f':
         if (nlines == 0)
            nlines++;
         if (comchar == 'f')
            nlines *= dlines;
         putchar ('\r');
         erase (0);
         lprintf ("\n");
         lprintf ("...skipping %d line", nlines);
         if (nlines > 1)
            pr ("s\n");
         else
            pr ("\n");

         pr ("\n");

         while (nlines > 0)
         {
            while ((c = Getc (f)) != '\n')
               if (c == EOF)
               {
                  retval = 0;
                  done++;
                  goto endsw;
               }
            Currline++;
            nlines--;
         }
         ret (dlines);
      case '\n':
         if (nlines != 0)
            dlines = nlines;
         else
            nlines = 1;
         ret (nlines);
      case '\f':
         if (!no_intty)
         {
            Fseek (f, screen_start.chrctr);
            Currline = screen_start.line;
            ret (dlines);
         }
      case '\'':
         if (!no_intty)
         {
            pr ("\n***Back***\n\n");
            Fseek (f, context.chrctr);
            Currline = context.line;
            ret (dlines);
         }
      case '=':
         promptlen = printd (Currline);
         fflush (stdout);
         break;
      case 'n':
         lastp++;
      case '?':
      case 'h':
         if ((helpf = fopen ("/usr/share/more.help", "r")) == NULL)
            error ("Can't open help file");
         copy_file (helpf);
         fclose (helpf);
         prompt (filename);
         break;
      default:
         promptlen = pr ("[Press 'h' for instructions.]");
         fflush (stdout);
         break;
      }
      if (done)
         break;
   }
   putchar ('\r');
 endsw:
   inwait = 0;
   notell++;
   if (MBIT == RAW && slow_tty)
   {
      otty.sg_flags &= ~MBIT;
      stty (fileno (stderr), &otty);
   }
   return (retval);
}

/*
** Print out the contents of the file f, one screenful at a time.
*/

#define STOP -10

void
screen (f, num_lines)
     register FILE *f;
     register int num_lines;
{
   register int c;
   register int nchars;
   int length;                  /* length of current line */
   static int prev_len = 1;     /* length of previous line */

   for (;;)
   {
      while (num_lines > 0 && !Pause)
      {
         if ((nchars = getline (f, &length)) == EOF)
         {
            return;
         }
         if (ssp_opt && length == 0 && prev_len == 0)
            continue;
         prev_len = length;
         prbuf (Line, length);
         if (nchars < promptlen)
            erase (nchars);     /* erase () sets promptlen to 0 */
         else
            promptlen = 0;
         if (nchars < Mcol || !fold_opt)
            prbuf ("\n", 1);    /* will turn off UL if necessary */
         if (nchars == STOP)
            break;
         num_lines--;
      }
      fflush (stdout);
      if ((c = Getc (f)) == EOF)
      {
         return;
      }

      Ungetc (c, f);
      setjmp (restore);
      Pause = 0;
      startup = 0;
      if ((num_lines = command (NULL, f)) == 0)
         return;
      if (promptlen > 0)
         erase (0);
      screen_start.line = Currline;
      screen_start.chrctr = Ftell (f);
   }
}

/*
** Come here if a quit signal is received
*/

void
onquit ()
{
   signal (SIGQUIT, SIG_IGN);
   if (!inwait)
   {
      putchar ('\n');
      if (!startup)
      {
         signal (SIGQUIT, onquit);
         longjmp (restore, 1);
      }
      else
         Pause++;
   }
   else if (notell)
   {
      write (2, "[Use q or Q to quit]", 20);
      promptlen += 20;
      notell = 0;
   }
   signal (SIGQUIT, onquit);
}

/* Put the print representation of an integer into a string */

static char *sptr;

void
Sprintf (n)
     register int n;
{
   register int a;

   if (a = n / 10)
      Sprintf (a);
   *sptr++ = n % 10 + '0';
}

void
scanstr (n, str)
     int n;
     char *str;
{
   sptr = str;
   Sprintf (n);
   *sptr = '\0';
}

/* See whether the last component of the path name "path" is equal to the
** string "string"
*/

int
tailequ (path, string)
     char *path;
     register char *string;
{
   register char *tail;

   tail = path + strlen (path);
   while (tail >= path)
      if (*(--tail) == '/')
         break;
   ++tail;
   while (*tail++ == *string++)
      if (*tail == '\0')
         return (1);
   return (0);
}

void
set_tty ()
{
   otty.sg_flags |= MBIT;
   otty.sg_flags &= ~ECHO;
   stty (fileno (stderr), &otty);
}

/*----------------------------- Terminal I/O -------------------------------*/

void
initterm ()
{

   Lpp = 24;
   Mcol = 80;
   no_intty = gtty (fileno (stdin), &otty);
   gtty (fileno (stderr), &otty);
   savetty = otty;
   hardtabs = !(otty.sg_flags & XTABS);
   if (!no_tty)
   {
      otty.sg_flags &= ~ECHO;
      if (MBIT == CBREAK || !slow_tty)
         otty.sg_flags |= MBIT;
   }
}

static char BS = '\b';
static char *BSB = "\b \b";
static char CARAT = '^';

#define ERASEONECHAR \
    if (docrterase) \
	write (2, BSB, sizeof(BSB)); \
    else \
	write (2, &BS, sizeof(BS));

void
show (ch)
     register char ch;
{
   char cbuf;

   if ((ch < ' ' && ch != '\n' && ch != ESC) || ch == RUBOUT)
   {
      ch += ch == RUBOUT ? -0100 : 0100;
      write (2, &CARAT, 1);
      promptlen++;
   }
   cbuf = ch;
   write (2, &cbuf, 1);
   promptlen++;
}

void
ttyin (buf, nmax, pchar)
     char buf[];
     register int nmax;
     char pchar;
{
   register char *sptr;
   register char ch;
   register int slash = 0;
   int maxlen;
   char cbuf;

   sptr = buf;
   maxlen = 0;
   while (sptr - buf < nmax)
   {
      if (promptlen > maxlen)
         maxlen = promptlen;
      ch = readch ();
      if (ch == '\\')
      {
         slash++;
      }
      else if ((ch == otty.sg_erase) && !slash)
      {
         if (sptr > buf)
         {
            --promptlen;
            ERASEONECHAR-- sptr;
            if ((*sptr < ' ' && *sptr != '\n') || *sptr == RUBOUT)
            {
               --promptlen;
            ERASEONECHAR}
            continue;
         }
         else
         {
            if (!eraseln)
               promptlen = maxlen;
            longjmp (restore, 1);
         }
      }
      else if ((ch == otty.sg_kill) && !slash)
      {
         show (ch);
         putchar ('\n');
         putchar (pchar);
         sptr = buf;
         fflush (stdout);
         continue;
      }
      if (slash && (ch == otty.sg_kill || ch == otty.sg_erase))
      {
         ERASEONECHAR-- sptr;
      }
      if (ch != '\\')
         slash = 0;
      *sptr++ = ch;
      if ((ch < ' ' && ch != '\n' && ch != ESC) || ch == RUBOUT)
      {
         ch += ch == RUBOUT ? -0100 : 0100;
         write (2, &CARAT, 1);
         promptlen++;
      }
      cbuf = ch;
      if (ch != '\n' && ch != ESC)
      {
         write (2, &cbuf, 1);
         promptlen++;
      }
      else
         break;
   }
   *--sptr = '\0';
   if (!eraseln)
      promptlen = maxlen;
   if (sptr - buf >= nmax - 1)
      error ("Line too long");
}

int
expand (outbuf, inbuf)
     char *outbuf;
     char *inbuf;
{
   register char *instr;
   register char *outstr;
   register char ch;
   char temp[200];
   int changed = 0;

   instr = inbuf;
   outstr = temp;
   while ((ch = *instr++) != '\0')
      switch (ch)
      {
      case '%':
         if (!no_intty)
         {
            strcpy (outstr, fnames[fnum]);
            outstr += strlen (fnames[fnum]);
            changed++;
         }
         else
            *outstr++ = ch;
         break;
      case '\\':
         if (*instr == '%' || *instr == '!')
         {
            *outstr++ = *instr++;
            break;
         }
      default:
         *outstr++ = ch;
      }
   *outstr++ = '\0';
   strcpy (outbuf, temp);
   return (changed);
}

void
rdline (f)
     register FILE *f;
{
   register char c;
   register char *p;

   p = Line;
   while ((c = Getc (f)) != '\n' && c != EOF && p - Line < LINSIZ - 1)
      *p++ = c;
   if (c == '\n')
      Currline++;
   *p = '\0';
}

int
main (argc, argv)
     int argc;
     char *argv[];
{
   register FILE *f;
   register char *s;
   register char ch;
   register int left;
   int prnames = 0;
   int initopt = 0;
   int clearit = 0;
   int initline;
   FILE *checkf ();

   nfiles = argc;
   fnames = argv;
   initterm ();
   nscroll = Lpp / 2 - 1;
   if (nscroll <= 0)
      nscroll = 1;
   while (--nfiles > 0)
   {
      if ((ch = (*++fnames)[0]) == '-')
      {
         argscan (*fnames + 1);
      }
      else if (ch == '+')
      {
         s = *fnames;
         initopt++;
         for (initline = 0; *s != '\0'; s++)
            if (isdigit ((int) *s))
               initline = initline * 10 + *s - '0';
         --initline;
      }
      else
         break;
   }
   if (dlines == 0)
      dlines = Lpp - (noscroll ? 1 : 2);
   left = dlines;
   if (nfiles > 1)
      prnames++;
   if (!no_intty && nfiles == 0)
   {
      fputs ("Usage: ", stderr);
      fputs (argv[0], stderr);
      fputs (" [-flpsu] [+linenum] name1 name2 ...\n", stderr);
      exit (1);
   }
   else
      f = stdin;
   if (!no_tty)
   {
      signal (SIGQUIT, onquit);
      signal (SIGINT, end_it);
      stty (fileno (stderr), &otty);
   }
   if (no_intty)
   {
      if (no_tty)
         copy_file (stdin);
      else
      {
         skiplns (initline, stdin);
         screen (stdin, left);
      }
      no_intty = 0;
      prnames++;
      firstf = 0;
   }

   while (fnum < nfiles)
   {
      if ((f = checkf (fnames[fnum], &clearit)) != NULL)
      {
         context.line = context.chrctr = 0;
         Currline = 0;
         if (firstf)
            setjmp (restore);
         if (firstf)
         {
            firstf = 0;
            if (initopt)
               skiplns (initline, f);
         }
         else if (fnum < nfiles && !no_tty)
         {
            setjmp (restore);
            left = command (fnames[fnum], f);
         }
         if (left != 0)
         {
            if ((noscroll || clearit) && (file_size != 0x7fffffffL))
               if (prnames)
               {
                  if (bad_so)
                     erase (0);
                  pr ("::::::::::::::");
                  if (promptlen > 14)
                     erase (14);
                  lprintf ("\n");
                  lprintf ("%s\n", fnames[fnum]);
                  lprintf ("::::::::::::::\n", fnames[fnum]);
                  if (left > Lpp - 4)
                     left = Lpp - 4;
               }
            if (no_tty)
               copy_file (f);
            else
            {
               within++;
               screen (f, left);
               within = 0;
            }
         }
         setjmp (restore);
         fflush (stdout);
         fclose (f);
         screen_start.line = screen_start.chrctr = 0L;
         context.line = context.chrctr = 0L;
      }
      fnum++;
      firstf = 0;
   }
   reset_tty ();
   exit (0);
}
