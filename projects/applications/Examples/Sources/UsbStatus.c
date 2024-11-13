
#include <stdio.h>
#include "lib.h"

void main( void )
{
    int previous_status, current_status;

    SystemSetting("8Z");    // Disable charge indicator

    previous_status = -1;

    for(;;)
    {
        current_status = UsbStatus();

        if(current_status != previous_status)
        {
            previous_status = current_status;

            switch( current_status )
            {
                case USB_NOT_CONNECTED:
                    GoodReadLed(ORANGE, -1);
                    break;
                case USB_OK:
                    GoodReadLed(GREEN, -1);
                    GoodReadLed(RED, 0);
                    break;
                default:
                    GoodReadLed(RED, -1);
                    GoodReadLed(GREEN, 0);
                    break;
            }
        }
        Idle();
    }
}
