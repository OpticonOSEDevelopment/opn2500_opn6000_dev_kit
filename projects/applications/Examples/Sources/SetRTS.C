
// The following example demonstrates the use SetRTS() on USB-VCP ports

#include <stdio.h>
#include "lib.h"

void main( void )
{
    int key = -1;

    ComOpen( COM9 );     // USB-VCP COM-port

    for(;;)
    {
        printf("\nPress any key");

        ResetKey();

        key = WaitKey();

        SetRTS((key == TRIGGER_KEY) ? ON : OFF);

        Idle();
    }
}
