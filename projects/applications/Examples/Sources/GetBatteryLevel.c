
#include <stdio.h>
#include <stdlib.h>
#include "lib.h"

void main( void )
{
    printf("\nBattery level:");
    for(;;)
    {
        printf("%d %%", GetBatteryLevel() );
        Sound( TCLICK, VMEDIUM, SHIGH, 0);
        GoodReadLed( GREEN, TCLICK);
        Delay(5*50);
    }
}
