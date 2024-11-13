
#include <stdio.h>
#include "lib.h"

void main(void)
{
    for(;;)
    {
        ResetKey();

        while(!kbhit())
            Idle();

        printf("\nSerial: %06ld", GetSerialNumber());
        Sound(TSTANDARD, VHIGH, SHIGH, 0);
    }
}
