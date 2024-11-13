
// SetDateTime() / GetDateTime() example

#include <stdio.h>
#include "lib.h"

void main( void )
{
    struct date d = { .da_day = 3, .da_mon = 11, .da_year = 2028 };
    struct time t = { .ti_hour = 8, .ti_min = 9, .ti_sec = 42 };

    SetDateTime( &d,  &t );

    for(;;)
    {
        GetDateTime( &d, &t );
        printf("\n%02d:%02d:%02d", t.ti_hour, t.ti_min, t.ti_sec);
        printf(" %02d/%02d/%04d", d.da_day, d.da_mon, d.da_year);
        Delay( 50 );
    }
}
