
#include <stdio.h>
#include "lib.h"

void main( void )
{
    for(;;)
    {
        Sound(TCLICK, VHIGH, SERROR, SPAUSE, SLOW, SPAUSE, SMEDIUM, SPAUSE, SHIGH,0);
        Delay(200);   // Wait 4 sec
        Sound(TSTANDARD, VMEDIUM, SERROR, SPAUSE, SLOW, SPAUSE, SMEDIUM, SPAUSE, SHIGH,0);
        Delay(200);
        Sound(TLONG, VLOW, SERROR, SPAUSE, SLOW, SPAUSE, SMEDIUM, SPAUSE, SHIGH,0);
        Delay(200);
        Sound(TVLONG, VSYSTEM, SERROR, SPAUSE, SLOW, SPAUSE, SMEDIUM, SPAUSE, SHIGH, 0);
        Delay(200);
    }
}
