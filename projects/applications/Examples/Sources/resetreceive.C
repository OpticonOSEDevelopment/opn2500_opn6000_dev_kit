
#include <stdio.h>
#include "lib.h"

void main( void )
{
    int ch;

    ComOpen( COM9 );    // Open the USB-VCP port

    for(;;)
    {
        ResetReceive();    // Clear receive buffer

        while(GetCom( 0 ) != CR)   // Wait for enter
        {
            Idle();    // Very important to lower the power consumption
        }
        PutCom( '!' );
        
        Delay(2 * 50);    // Ignore all data for 2 seconds
    }
}
    