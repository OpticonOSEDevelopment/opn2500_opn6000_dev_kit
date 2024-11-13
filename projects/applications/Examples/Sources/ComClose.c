
#include <stdio.h>
#include "lib.h"

void main( void )
{
    int ch, open, connected;

    ComOpen( COM9 );    // Open the COM port for USB-VCP

    Sound(TSTANDARD, VHIGH, SMEDIUM, SHIGH, 0);
    open = TRUE;
    connected = FALSE;

    for(;;)
    {
        if(connected != UsbIsConnected())
        {
            Sound(TLONG, VHIGH, SHIGH, 0);
            connected = UsbIsConnected();
        }
        
        if((ch=getchar()) == TRIGGER_KEY && !open)
        {
            Sound(TSTANDARD, VHIGH, SMEDIUM, SHIGH, 0);
            ComOpen( COM9 );
            open = TRUE;
        }
        else if(ch == CLR_KEY && open)
        {
            ComClose(COM9);
            Sound(TLONG, VHIGH, SERROR, 0);
            connected = open = FALSE;
        }
        
        if( (ch = GetCom( 0 )) != -1)    // Echo
            PutCom( ch );
        
        Idle();    // Very important to lower the power consumption
    }
}
