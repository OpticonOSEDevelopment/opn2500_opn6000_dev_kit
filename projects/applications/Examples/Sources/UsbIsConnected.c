
#include <stdio.h>
#include "lib.h"

void main( void )
{
    int ch, open, connected;

    ComOpen( COM9 );    // Enable the COM port for USB-VCP

    Sound(TSTANDARD, VHIGH, SMEDIUM, SHIGH, 0);
    open = TRUE;
    connected = FALSE;

    for(;;)
    {
        if(connected != UsbIsConnected())
        {   // Sound a high beep when the port is opened by host
            connected = UsbIsConnected();
            Sound(TVLONG, VHIGH, connected ? SHIGH : SLOW, 0);
        }
        
        if((ch=getchar()) == TRIGGER_KEY)
        {
            if(!open)   // Re-open the COM port for USB-VCP
            {
                Sound(TSTANDARD, VHIGH, SMEDIUM, SHIGH, 0);
                ComOpen( COM9 );
                open = TRUE;
            }
            else
            {
                PutCom2(COM9, 'X');  // If connected -> Type 'X' on trigger press
            }
        }
        else if(ch == CLR_KEY && open)    // Close the COM port using function key
        {
            ComClose(COM9);
            Sound(TLONG, VHIGH, SERROR, 0);
            connected = open = FALSE;
        }
        Idle();    // Very important to lower the power consumption
    }
}
