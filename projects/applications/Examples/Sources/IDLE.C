
// The following example demonstrates the use of Idle() and the power reduction.
// The example may reduce power consumption by about 2-4 times!

#include <stdio.h>
#include "lib.h"

void main( void )
{
    unsigned int x;
    for(;;)
    {
        printf("\nIdle loop");
        for( x=0; x < 200; x++)
            Idle();
        printf("\n normal loop");
        for( x=0; x < 10000; x++)
            continue;
    }
}
