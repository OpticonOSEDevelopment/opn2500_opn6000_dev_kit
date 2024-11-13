
// The following example shows how to implement 'continuous read' mode.

#include <stdio.h>
#include "lib.h"

void main( void )
{
    char bcr_buf[42];
    struct barcode code;

    code.min = 1;
    code.max = 41;
    code.text = bcr_buf;

    for(;;)
    {
        if( !IsScannerOff())
        {
            ResetKey();
            if( ReadBarcode( &code ) == OK )
            {
                GoodReadLed( GREEN, 10);
                Sound( TSTANDARD, VHIGH, SMEDIUM, SHIGH, 0);
                ScannerPower( ON, 250);
                printf("%*s\r", code.length, code.text);
            }
        }
        else
            ScannerPower( ON, 250);
    }
}
