
#include <stdio.h>
#include "lib.h"

void main( void )
{
    for(;;)
    {
        Sound(TSTANDARD, VHIGH, SLOW, SPAUSE, SMEDIUM, SPAUSE, SHIGH, SPAUSE, 0);

        while( IsBuzzerOn() )
            Idle();

        Sound(TSTANDARD, VHIGH, SHIGH, SPAUSE, SMEDIUM, SPAUSE, SLOW, SPAUSE,0);

        while( IsBuzzerOn() )
            Idle();
    }
}
