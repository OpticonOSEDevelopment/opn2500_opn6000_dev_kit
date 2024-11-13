
#include <stdio.h>
#include "lib.h"

void main( void )
{
    struct time t;

    t.ti_hour = 12;
    t.ti_min = 12;
    for(;;)
    {
        t.ti_sec = 62;
        printf("\nTime: %02d:%02d:%02d",t.ti_hour,t.ti_min,t.ti_sec);
        if( CheckTime( &t ) == OK )
            printf("\nTime ok!!");
        else
            printf("\nIllegal time!!");
        while( !kbhit() )
            Idle();
        ResetKey();

        t.ti_sec = 12;
        printf("\nTime: %02d:%02d:%02d",t.ti_hour,t.ti_min,t.ti_sec);
        if( CheckTime( &t ) == OK )
            printf("\nTime ok!!");
        else
            printf("\nIllegal time!!");
        while( !kbhit() )
            Idle();
        ResetKey();
    }
}
