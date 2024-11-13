
#include <stdio.h>
#include "lib.h"

void main( void )
{
    int x;

    for(;;)
    {
        for( x=0; x < 10; x++)
        {
            GoodReadLed( RED, 25);
            Delay( 50 );
            GoodReadLed( GREEN, 25);
            Delay( 50 );
            GoodReadLed( BLUE, 25);
            Delay( 50 );
        }
        GoodReadLed( RED | LED_FLASH, FOREVER);
        Delay( 200 );
        GoodReadLed( RED, 0);
        GoodReadLed( GREEN | LED_FLASH, FOREVER);
        Delay( 200 );
        GoodReadLed( GREEN, 0);
        GoodReadLed( BLUE | LED_FLASH, FOREVER);
        Delay( 200 );
        GoodReadLed( BLUE, 0);
    }
}
