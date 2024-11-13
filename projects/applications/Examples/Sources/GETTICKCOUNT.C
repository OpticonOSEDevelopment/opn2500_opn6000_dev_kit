
#include <stdio.h>
#include "lib.h"

#define HIGH_DELAY      ( 13 * 5 )       //Note that these are prime numbers
#define MEDIUM_DELAY    ( 23 * 5 )
#define LOW_DELAY       ( 37 * 5 )

void main( void )
{
    unsigned long current_time;
    unsigned long next_high_time;
    unsigned long next_medium_time;
    unsigned long next_low_time;

    current_time = GetTickCount();

    next_high_time = current_time + HIGH_DELAY;
    next_medium_time = current_time + MEDIUM_DELAY;
    next_low_time = current_time + LOW_DELAY;

    printf("\n3 independent\ntimers using\nGetTickCount");

    for(;;)
    {
        //Detect timer reset after 655535 ticks
        if(current_time > GetTickCount())
        {
            next_high_time = HIGH_DELAY;
            next_medium_time = MEDIUM_DELAY;
            next_low_time = LOW_DELAY;
        }

        current_time = GetTickCount();

        if(current_time>=next_low_time)
        {
            Sound( TSTANDARD, VSTANDARD, SLOW, 0);
            GoodReadLed(RED,10);
            next_low_time += LOW_DELAY;
        }

        if(current_time>=next_medium_time)
        {
            Sound( TSTANDARD, VSTANDARD, SMEDIUM, 0);
            GoodReadLed(ORANGE,10);
            next_medium_time += MEDIUM_DELAY;
        }

        if(current_time>=next_high_time)
        {
            Sound( TSTANDARD, VSTANDARD, SHIGH, 0);
            GoodReadLed(GREEN,10);
            next_high_time += HIGH_DELAY;
        }

        Idle();
    }
}

