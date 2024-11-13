
#include <stdio.h>
#include "lib.h"

void main( void )
{
    char buffer[] = "This is a string";

    ComOpen( COM9 );    // Open the COM port for USB-VCP

    PutString2(COM9, buffer);

    for(;;)
    {
        Idle();    // Very important to lower the power consumption
    }
}
