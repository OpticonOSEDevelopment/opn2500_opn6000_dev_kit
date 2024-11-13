
#include <stdio.h>
#include "lib.h"

void main( void )
{
    int ch;

    ComOpen( COM9 );    // Open the USB-VCP port

    for(;;)
    {
        if( (ch = GetCom( 0 )) != -1)
        {
            PutCom( ch );
        }

        Idle();    // Very important to lower the power consumption
    }
}
