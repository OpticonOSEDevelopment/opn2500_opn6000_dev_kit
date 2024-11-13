
// The code fragment below installs a callback function
// that is executed by the operating system every 20ms.
// The callback makes sure that the red LED is switched
// on for a short time every 2 seconds.
// (Press function key to uninstall)
#include <stdio.h>
#include "lib.h"

int Callback(void)
{
    static int counter=100;

    if (--counter == 0)
    {
        GoodReadLed(RED,10);
        counter = 100;
        return TRUE; // Return TRUE to indicate device needs to wake-up
    }
    return TRUE; // Return FALSE to indicate can stay asleep (return DISABLED if the device may power off)
}

void main( void )
{
    printf("\nTimer callback");

    InstallHeartbeatHandler(Callback);

    while(1)
    {
        if (getchar() == CLR_KEY)
            RemoveHeartbeatHandler(Callback);
    }
    Idle();
}
