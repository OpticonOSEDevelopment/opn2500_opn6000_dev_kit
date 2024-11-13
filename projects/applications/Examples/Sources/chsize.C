
#include <stdio.h>
#include <stdlib.h>
#include "lib.h"

void main( void )
{
    long size;
    FILE *fp;
    static char data[20+1];
    int recsize;

    for( recsize=0; recsize < 20; recsize++)    // prepare some data
        data[recsize] = 'a' + recsize;      // quit when recsize == 20 

    data[recsize] = '\0';
    printf("\nPress a key");

    for(;;)
    {
        ResetKey();
        while( !kbhit())
            Idle();

        // Open for appending or else create (binary) file
        if( (fp = fopen("TEST.DAT","a+b")) == NULL)
        {
            printf("\nError opening\nTEST.DAT");
            continue;
        }

        chsize(*fp, (long) recsize);

        if( fwrite( data, 1, recsize, fp) != recsize)
        {
            fclose( fp );
            printf("\nError writing\nTEST.DAT");
            continue;
        }

        fclose(fp);

        size = fsize("TEST.DAT");
        if( size > -1)
            printf("\nSize = \n%ld bytes", size);
        else
            printf("\nTEST.DAT cannot\nbe found!");
    }
}
