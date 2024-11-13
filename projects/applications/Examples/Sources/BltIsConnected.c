
#include <stdio.h>
#include "lib.h"

void main( void )
{
    int ch, open, connected;

    ComOpen( COM12 );    // Enable the COM port for BLE HID (discoverable)

    Sound(TSTANDARD, VHIGH, SMEDIUM, SHIGH, 0);
    open = TRUE;
    connected = FALSE;

    for(;;)
    {   // Sound a high beep when the port is opened by host
        if(connected != BltIsConnected())
        {   // Sound a high beep when connected to host
            connected = BltIsConnected();
            Sound(TVLONG, VHIGH, connected ? SHIGH : SLOW, 0);
        }
        
        if((ch=getchar()) == TRIGGER_KEY)
        {
            if(!open)  // Re-Enable the COM port for BLE HID (discoverable)
            {
                Sound(TSTANDARD, VHIGH, SMEDIUM, SHIGH, 0);
                ComOpen( COM12 );
                open = TRUE;
            }
            else
            {
                PutCom2(COM12, 'X');   // If connected -> Type 'X' on trigger press
            }
        }
        else if(ch == CLR_KEY && open)   // Close the COM port using function key
        {
            ComClose(COM12);
            Sound(TLONG, VHIGH, SERROR, 0);
            connected = open = FALSE;
        }
        Idle();    // Very important to lower the power consumption
    }
}
