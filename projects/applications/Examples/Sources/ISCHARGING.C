
#include <stdio.h>
#include "lib.h"

void main( void )
{
    int previous_state, current_state;

    previous_state = -1;    // Not equal to 0, 1, etc. 

    for(;;)
    {
        current_state = IsCharging();
        if( current_state != previous_state)
        {
            previous_state = current_state;
            switch( current_state )
            {
                case 0:
                    printf("Not charging\n");
                    break;
                case 1:
                    printf("Charging\n");
                    break;
                case 2:
                    printf("Completed\n");
                    break;
                case 3:
                    printf("Charging error\n");
                    break;
            }
        }
        Idle();
    }
}
