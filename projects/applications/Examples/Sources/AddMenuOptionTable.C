// This program executes menu labels and demonstrates the adding of menu options to the OS
#include <stdio.h>
#include "lib.h"

unsigned char buzzer_enabled;

// Customized default
void Custom_Default(void)
{
    SystemSetting("U2");            // Inherit OS menu option "U2"
    SystemSetting("A0");            // Enable all decoders
    buzzer_enabled = TRUE;
    SystemSetting("YC");            // Enable menu labels
}

//
// The following table implements SystemSetting options that need to be added to the OS or overruled
//
// Note: The use of 'SystemSetting()' inside the functions in the table below is limited to protect against recursion.
//       It is still possible to use SystemSetting() but only with 1 menu option per function call.
//
const OPTION menu_option_table[] =
{                                                // CLEAR mask                              SET mask (use 0xFF 0xFF in case of a function call)
    { "U2",(void*)Custom_Default,                   0xFF,                                   0xFF                        },
    { "W0",(void*)&(buzzer_enabled),                0x01,                                   0x00                        },
    { "W8",(void*)&(buzzer_enabled),                0x00,                                   0x01                        },
};

#define MAX_OPTIONS sizeof(menu_option_table)/sizeof(OPTION)


void main( void )
{
    char bcr_buf[42] = {0};
    struct barcode code  = {0};
    int result;

    //
    // The following function implements systemsettings / menu options that aren't present in the OS or need to be overruled
    //
    AddMenuOptionTable(menu_option_table, MAX_OPTIONS);

    SystemSetting("U2");        // Reset to customized default

    code.min   = 1;
    code.max   = 41;
    code.text  = bcr_buf;

    ScannerPower(TRIGGER | SINGLE, 250);    // Trigger mode, 5 seconds read time, scanner off after 1 barcode

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

                               while(TriggerPressed())               // Wait for release before resetting read mode
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

                               while(TriggerPressed())               // Wait for release before resetting read mode
                                   Idle();

                               ScannerPower(TRIGGER | SINGLE, 250);  // Reset read mode, because it's turned off after reading menu labels
                               break;
                        }
                    }
                }
            }
            else
            {
                if(buzzer_enabled)        // Read menu option "W0" and "W8" to enable/disabled the buzzer
                    Sound(TSTANDARD, VHIGH, SMEDIUM, SHIGH, 0);

                GoodReadLed(GREEN, 10);
            }
        }

        Idle();        // Reduces power consumption
    }
}
