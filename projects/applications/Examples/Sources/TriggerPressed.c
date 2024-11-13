
// This example will sound the buzzer when the trigger key is being pressed

#include <stdio.h>
#include "lib.h"

void main( void )
{    
    for(;;)
    {
        printf("\nPress any key!");
        if( TriggerPressed() )
            Sound( TSTANDARD, VHIGH, SMEDIUM, 0);
        if( DownPressed() )
            Sound( TSTANDARD, VHIGH, SLOW, 0);
        Idle();
    }
}
