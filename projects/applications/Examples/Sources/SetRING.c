
// The following example demonstrates the use SetRING() on USB-CDC ports

#include <stdio.h>
#include "lib.h"

void main( void )
{
    int key = -1;

    ComOpen( COM8 );     // USB-CDC COM-port

    for(;;)
    {
        printf("\nPress any key");

        ResetKey();

        key = WaitKey();

        SetRING((key == TRIGGER_KEY) ? ON : OFF);

        Idle();
    }
}
