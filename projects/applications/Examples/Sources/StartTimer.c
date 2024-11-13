
#include <stdio.h>
#include "lib.h"

void main( void )
{
    for(;;)
    {
        printf("\nTick off 5 sec");
        StartTimer( 5 * 50 );
        while( !EndTimer())
        {
            printf("\nTime left %d s", TimerValue()/50);
            Sound( TSTANDARD, VSTANDARD, SMEDIUM, 0);
            Delay( 50 );
        }

        printf("\nDone");
        ResetKey();
        while( !kbhit())
            Idle();
    }
}
