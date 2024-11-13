
#include <stdio.h>
#include "lib.h"

void main( void )
{
    for(;;)
    {
        Sound(TSTANDARD, VHIGH, SHIGH, 0);
        DelayMs( 500 );    // sounds a beep every 500 milliseconds 
    }
}
