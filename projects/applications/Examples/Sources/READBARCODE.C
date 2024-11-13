

// This program reads and displays a barcode. After each successful barcode reading,
// it gives a good read signal on the LED and a buzzer signal.

#include <stdio.h>
#include "lib.h"

void main( void )
{
    char bcr_buf[42] = {0};
    struct barcode code  = {0};

    code.min   = 3;
    code.max   = 41;
    code.text  = bcr_buf;

    ScannerPower(SINGLE|TRIGGER, 100);        // Single read, trigger mode, 2 sec

    for(;;)
    {
        if(ReadBarcode(&code) == OK)
        {
             GoodReadLed(GREEN, 10);
             Sound( TSTANDARD, VHIGH, SMEDIUM, SHIGH, 0);
             printf("%*s\r", code.length, code.text);
        }

        Idle();
    }
}
