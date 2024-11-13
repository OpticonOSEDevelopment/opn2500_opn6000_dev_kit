
#include <stdio.h>
#include <stdlib.h>
#include "lib.h"

void main( void )
{
    unsigned char *block;

    for(;;)
    {
    	while( getchar() == EOF )
            Idle();

        printf("\n1: mem = %lu", Heap_CoreLeft());

        while( getchar() == EOF )
            Idle();

        block = (unsigned char *) malloc(5000L);

        printf("\n2: mem = %lu", Heap_CoreLeft());

        while( getchar() == EOF )
		    Idle();

        free(block);

        printf("\n3: mem = %lu", Heap_CoreLeft());
    }
}
