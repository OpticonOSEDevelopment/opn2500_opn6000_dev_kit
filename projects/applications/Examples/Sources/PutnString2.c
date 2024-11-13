
#include <stdio.h>
#include "lib.h"

void main( void )
{
    char buffer[] = "This is a string";

    ComOpen( COM9 );    // Open the COM port for USB-VCP

    for(;;)
    {
		PutnString2(COM9, buffer, 4);
        Delay(50);
    }
}
