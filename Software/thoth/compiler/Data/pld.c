
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

int IFile;

int
Get(void)
{
    char ch;
    
    if( read(IFile, &ch, 1) <= 0 ) {
        exit(1);
    }
    return (ch & 0xff);
}

unsigned char buf[40];

void
print_load(void)
{
    int check, i, cmd, len;

    while( 1 )
    {
        buf[0] = cmd = Get();
        buf[1] = len = Get();
        for( i = 0; i < len; ++i)
            buf[2+i] = Get();
        buf[len+2] = Get();
        
        check = 0;
        for( i = 0; i < len + 3; ++i ) {
            check ^= buf[i];
        }
        if( check != 0 ) {
            fprintf(stderr, "Bad checksum %x\n", check);
            //exit(1);
        }
        buf[len+2] = 0;
        
        switch( cmd ) {

        case 'M':  /* Start module */
            printf("M %d %d %s\n", buf[2], buf[3], &buf[4]);
            break;

        case 'G':  /* Define symbol */
        case 'T':
            printf("%c %d %04x %s\n", buf[0], buf[2], (buf[3]<<8)+buf[4], &buf[5]);
            break;
        
        case 'g':  /* Reference symbol */
        case 't':
            printf("%c %d %s\n", buf[0], buf[2], &buf[3]);
            break;
        
        case 'L':  /* Data */
            len = buf[1] + 2;
            printf("L   %04X %d ", (buf[2]<<8)+buf[3], buf[4]);
            for( i = 5; i < len; i += 2) printf("%04x ", (buf[i]<<8)+buf[i+1]);
            printf("\n");
            break;

        case 'O':  /* Relocation */
            len = buf[1] + 2;
            printf("O ");
            for( i = 2; i < len; i += 2) printf("(%d %d %d) ", (buf[i]>>3), buf[i]&0x7, buf[i+1]);
            printf("\n");
            break;

        case 'A':  /* Align base */
            printf("A %d %d\n", buf[2], buf[3]);
            break;

        case 'I':  /* Increment base */
            printf("I %d %04X\n", buf[2], (buf[3]<<8)+buf[4]);
            break;
            
        case 'E':  /* End module */
            printf("E %d %d %d %d\n\n", buf[2], buf[3], buf[4], buf[5] );
            break;
                
        default: {
            printf("unrecognised load statement: '%c'\n", cmd);
            }
        }
    }
}

int
main()
{
/*	if( (IFile = open("functions", O_RDONLY)) < 0 ) {
        printf("cannot open 'eh.out' file\n");
        exit(1);
    }
*/
    print_load();
}

