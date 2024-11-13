
#include <stdio.h>
#include "lib.h"

void main( void )
{
    SetTerminalId( 1234 );

    for(;;)
    {
        ResetKey();

        while(!kbhit())
            Idle();

        printf("\nTerminal ID: %d", GetTerminalId());
        Sound(TSTANDARD, VHIGH, SHIGH, 0);
    }
}
