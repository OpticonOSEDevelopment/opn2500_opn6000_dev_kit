
#include <stdio.h>
#include "lib.h"

void main( void )
{
    for(;;)
    {
        printf("\nBlt address is: %s", GetBltAddress());
        ResetKey();
        while( !kbhit() )
            Idle();
    }
}
