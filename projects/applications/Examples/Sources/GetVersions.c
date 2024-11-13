#include <stdio.h>
#include "lib.h"

void main( void )
{
    int key;

    ResetKey();
    printf("\nPress trigger key");
    for(;;)
    {
        if( (key = getchar()) != EOF)
        {
            ResetKey();
            switch( key )
            {
                case TRIGGER_KEY:
                    Sound( TSTANDARD, VHIGH, SHIGH, 0);
                    printf("\nBootloader = %s", GetBootVersion());
                    printf("\nO/S = %s", GetOsVersion());
                    printf("\nAppl. = %s", GetApplVersion());
                    break;
                default:
                    Sound( TCLICK, VHIGH, SLOW, 0);
                    break;
            }
        }
        Idle();
    }
}