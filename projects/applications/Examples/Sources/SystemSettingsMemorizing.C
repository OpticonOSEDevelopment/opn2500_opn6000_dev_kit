

// This program executes menu labels from the Universal Menu Book and saves changed settings in flash

#include <stdio.h>
#include "lib.h"

void main( void )
{
    char bcr_buf[42] = {0};
    struct barcode code  = {0};
    int result;

    code.min   = 1;
    code.max   = 41;
    code.text  = bcr_buf;

    //
    // The following function will try to restore system settings from flash memory and will cause
    // system settings to be stored in flash memory when 'SystemSetting("Z2")' is called.
    //
    if(SystemSettingsMemorizing(ON) < 0)
    {
        SystemSetting("U2");               // If no old settings were found -> Reset to default
    }

    SystemSetting("YC");                   // Make sure menu labels are always enabled after a restart

    ScannerPower(TRIGGER | SINGLE, 250);   // Trigger mode, 5 seconds read time, scanner off after 1 barcode

    for(;;)
    {
        if(ReadBarcode(&code) == OK)
        {
            result = 0;

            if(code.id == MENU_CODE)
            {
                ScannerPower(ON, -1);        // Scanner on indefinitely

                while(result != EXITING_MENU_MODE && result != ERROR)
                {
                    if(result == 0 || ReadBarcode(&code) == OK)
                    {
                       switch( (result=ExecuteMenuLabel(&code)) )
                       {
                           case ENTERING_MENU_MODE:
                               Sound(TSTANDARD, VHIGH, SHIGH, SMEDIUM, SHIGH, 0);
                               break;

                           case EXITING_MENU_MODE:
                               ScannerPower(OFF, 0);
                               Sound(TSTANDARD,VHIGH, SHIGH, SMEDIUM, SHIGH, 0);
                               Delay(TSTANDARD * 4);        // Wait till buzzer sound ends before saving (saving flash settings temp. turns of sound timer)
                               SystemSetting("Z2");         // Save changed settings in flash memory

                               while(TriggerPressed())      // Wait for release before resetting read mode
                                   Idle();

                               ScannerPower(TRIGGER | SINGLE, 250);  // Reset read mode, because it's turned off after reading menu labels
                               break;

                           case INVALID_OPTION_READ:
                               Sound(TLONG, VHIGH, SLOW, 0);
                               break;

                           case VALID_OPTION_READ:
                               Sound(TSTANDARD, VHIGH, SHIGH, SMEDIUM, SHIGH,0);
                               break;

                           case LABEL_IGNORED:
                               break;

                           case ERROR:
                               ScannerPower(OFF, 0);

                               while(TriggerPressed())      // Wait for release before resetting read mode
                                   Idle();

                               ScannerPower(TRIGGER | SINGLE, 250);  // Reset read mode, because it's turned off after reading menu labels
                               break;

                        }
                    }
                }
            }
        }

        Idle();        // Reduces power consumption
    }
}
