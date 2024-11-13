
#include <stdio.h>
#include "lib.h"

void main( void )
{
    int c;

    ComOpen( COM9 );    // Open the COM port for USB-VCP

    for(;;)
    {
        if( (c = GetCom(0)) >= 0)
        {
            PutShort( (unsigned short)(c * 16) );
        }
        Idle();    // Very important to lower the power consumption
    }
}
