
#include <stdio.h>
#include "lib.h"

void main( void )
{
    for(;;)
    {
        printf("Local address:\r\n%s\r\n", GetBltLocalAddress());
        ResetKey();
        while( !kbhit() )
            Idle();
    }
}
