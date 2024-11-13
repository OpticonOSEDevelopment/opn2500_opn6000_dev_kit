
// GetDate() / GetTime() example

#include <stdio.h>
#include "lib.h"

void main( void )
{
    struct date d;
    struct time t;

    for(;;)
    {
        GetDate( &d );
        GetTime( &t );
        printf("\n%02d:%02d:%02d", t.ti_hour, t.ti_min, t.ti_sec);
        printf(" %02d/%02d/%04d", d.da_day, d.da_mon, d.da_year);
        Delay( 50 );
    }
}
