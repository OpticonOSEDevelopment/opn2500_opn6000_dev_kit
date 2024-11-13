
#include <stdio.h>
#include "lib.h"

void main( void )
{
    for(;;)
    {
        printf("\nchange address\n%s\nto\n008098344e59", GetBltAddress());
        ResetKey();
        while( !kbhit() )
            Idle();
        SetBltAddress("008098344e59");

        printf("\nBlt Address:\n%s", GetBltAddress() );
        ResetKey();
        while( !kbhit() )
            Idle();
    }
}
