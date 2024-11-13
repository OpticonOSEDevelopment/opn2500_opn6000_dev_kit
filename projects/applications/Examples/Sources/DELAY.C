
#include <stdio.h>
#include "lib.h"

void main( void )
{
    for(;;)
    {
        printf("\nHello world.");
        Sound(TSTANDARD, VHIGH, SHIGH, 0);
        Delay( 100 );
    }
}
