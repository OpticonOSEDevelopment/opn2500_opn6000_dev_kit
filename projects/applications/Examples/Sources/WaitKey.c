
#include <stdio.h>
#include "lib.h"

void main( void )
{
    int key = -1;

    for(;;)
    {
         printf("\nPress any key");

         ResetKey();

         key = WaitKey();

         GoodReadLed((key == TRIGGER_KEY) ? GREEN : RED, 50);        // ON, 1 second
    }
}
