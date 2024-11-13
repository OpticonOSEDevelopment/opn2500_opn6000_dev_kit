
#include <stdio.h>
#include "lib.h"

void main( void )
{
    int color = GREEN;

    for(;;)
    {
        printf("\nPress any key");

        ResetKey();
         
        while(!kbhit())
            Idle();

        GoodReadLed(color, 50);        // ON, 1 second

        color = (color == GREEN) ? RED : GREEN;
    }
}
