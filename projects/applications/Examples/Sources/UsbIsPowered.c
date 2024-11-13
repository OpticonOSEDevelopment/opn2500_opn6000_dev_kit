
#include <stdio.h>
#include "lib.h"

void main( void )
{
    SystemSetting("8Z");    // Disable charge indicator

    for(;;)
    {
        if(UsbIsConnected())
            GoodReadLed(GREEN, 50);
        else if(UsbIsPowered())
            GoodReadLed(ORANGE, 50);
        else
            GoodReadLed(RED, 50);
        Delay(50);
    }
}
