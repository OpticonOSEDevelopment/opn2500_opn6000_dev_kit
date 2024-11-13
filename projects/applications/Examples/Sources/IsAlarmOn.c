
// The following example demonstrates the use of SetAlarm() function.
#include <stdio.h>
#include "lib.h"

void main( void )
{
    struct time at = {0};
    bool alarmOn = false;

    AutoPowerDown(ON, 2*50); // 2 seconds

    AutoPowerDown(APD_SHUTDOWN_ON, 60*50);  // Alarm will boot device from off state
    // AutoPowerDown(APD_SHUTDOWN_OFF, 0);  // Alarm will wake-up device from sleep

    Sound(TSTANDARD, VHIGH, SLOW, SMEDIUM, SHIGH, SLOW, SMEDIUM, SHIGH, 0); // 6 tones: Start-up beep

    for(;;)
    {
        if(getchar() != EOF)
        {
            GoodReadLed(GREEN_FLASH, 0);
            GetTime(&at);
            at.ti_min = (at.ti_min+2) % 60; // Set alarm 2 minutes in the future
            at.ti_hour += (at.ti_min < 2) ? 1 : 0;
            SetAlarm(&at, ON);
            alarmOn = true;
            Sound(TSTANDARD, VHIGH, SMEDIUM, SHIGH, 0); // 2 tones: alarm set
            printf("\rALARM SET\r%02d:%02d\rwaiting...", at.ti_hour, at.ti_min);
        }

        // Check to see if terminal is woken by the Alarm function
        if(alarmOn != IsAlarmOn())
        {
            alarmOn = false;
            Sound( TSTANDARD, VHIGH, SLOW, SMEDIUM, SHIGH, 0);  // 3 tones: wake from alarm beep
            GoodReadLed(GREEN_FLASH, 30*50);
        }

        Idle();
    }
}
