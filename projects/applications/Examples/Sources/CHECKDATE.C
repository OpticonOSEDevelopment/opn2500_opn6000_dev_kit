
#include <stdio.h>
#include "lib.h"

void main( void )
{
    struct date d;

    d.da_year = 2002;
    d.da_mon = 2;
    for(;;)
    {
        d.da_day = 29; // only leap years have 29 days in February
        printf("\rDate: %02d-%02d-%04d",d.da_day,d.da_mon,d.da_year);
        if( CheckDate( &d ) == OK )
            printf("\nDate ok!!\n");
        else
            printf("\nIllegal Date!!");
        while( !kbhit() )
            Idle();
        ResetKey();

        d.da_day = 28;
        printf("\rDate: %02d-%02d-%04d",d.da_day,d.da_mon,d.da_year);
        if( CheckDate( &d ) == OK )
            printf("\nDate ok!!\n");
        else
            printf("\nIllegal Date!!");
        while( !kbhit() )
            Idle();
        ResetKey();
    }
}