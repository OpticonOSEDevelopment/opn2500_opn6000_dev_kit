
#include <stdio.h>
#include "lib.h"

void main( void )
{
    for(;;)
    {
        printf("\nLocal name: %s", GetBltLocalName());
        ResetKey();
        while( !kbhit() )
            Idle();
        SetBltLocalName(MODEL_NAME"_EXAMPLE");
    }
}
