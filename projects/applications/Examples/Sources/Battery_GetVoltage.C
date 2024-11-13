
#include <stdio.h>
#include <stdlib.h>
#include "lib.h"

void main( void )
{
    unsigned int voltage;

    printf("\nBattery voltage:");

    for(;;)
    {
        voltage = GetBatteryVoltage();
        printf("\r%dmV", voltage);
        Sound(TCLICK, VMEDIUM, SHIGH, 0);
        Delay(5*50);
    }
}
