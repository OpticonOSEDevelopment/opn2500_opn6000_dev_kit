
#include <stdio.h>
#include "lib.h"

void main( void )
{
    AutoPowerDown(ON, 30*50);     // Auto power down time of 30s

    for(;;)
    {
        printf("\nWaiting for 5 sec");
        StartTimer( 250+1 );
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
