
#include <stdio.h>
#include "lib.h"

void main(void)
{
    AutoPowerDown(ON, 1*50); // 1 second
    AutoPowerDown(APD_SHUTDOWN_ON, 20*50); // 20 seconds

    if (IsColdBoot())
        Sound(TSTANDARD, VHIGH, SLOW, SMEDIUM, SHIGH, SLOW, SMEDIUM, SHIGH, 0); // 6 tones: Start-up beep
    else
        Sound(TSTANDARD, VHIGH, SHIGH, SMEDIUM, SHIGH, 0); // 3 tones: was powered off beep
        
    PoweredDown();   // Reset the sleep flag on start-up

    for(;;)
    {
        if(getchar() == TRIGGER_KEY)
        {
            if(PoweredDown())
                Sound(TSTANDARD, VHIGH, SMEDIUM, SHIGH, 0); // 2 tones: Device has slept
        }

        Idle(); // Important to lower power consumption while awake
    }
}
