
#include <stdio.h>
#include "lib.h"

void main( void )
{
    int ch;

    ComOpen( COM9 );    // Open the USB-VCP port

    for(;;)
    {
        if( (ch = GetCom2(COM9, 0)) != -1)
        {
            PutCom2(COM9, ch);
        }

        Idle();    // Very important to lower the power consumption
    }
}
