
#include <stdio.h>
#include "lib.h"

void main( void )
{
    SystemSetting("WK");    // Key click on

    for(;;)
    {
        printf("\nHello world");

        ResetKey();
        while( !kbhit())
            Idle();
    }
}
