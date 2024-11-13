
#include <stdio.h>
#include <stdlib.h>
#include "lib.h"

void main( void )
{
    static char data[20+1];
    FILE *fp;

    for(;;)
    {
        printf("\n1: mem = %lu", CoreLeft());
        if( NULL != ( fp = fopen("TEST.DAT", "wb")))
        {
            fwrite( data, 1, 20, fp);
            fclose(fp);
        }

        while( getchar() == EOF )
            Idle();

        printf("\n2: mem = %lu", CoreLeft());
        format();
        while( getchar() == EOF )
            Idle();

        printf("\n3: mem = %lu", CoreLeft());
        while( getchar() == EOF )
            Idle();
    }
}
