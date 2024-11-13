// This program executes menu labels from the Universal Menu Book and saves changed settings in flash

#include <stdio.h>
#include "lib.h"

void main( void )
{
    char bcr_buf[42] = {0};
    struct barcode code  = {.min = 1, .max = 41};
    int result;

    code.text  = bcr_buf;

	autopowerdown(ON, 5*50);				// 5 Seconds until power down
	
	autopowerdown(APD_SHUTDOWN_ON, 120*50);	// 120 Seconds until shut down

    //
    // The following function will try to restore system settings from flash memory and will cause
    // system settings to be stored in flash memory when 'systemsetting("Z2")' is called.
    //
    if(SystemSettingsMemorizing(ON) < 0)
    {
        systemsetting("U2");               // If no old settings were found -> Reset to default
    }

    systemsetting("YC");                   // Make sure menu labels are always enabled after a restart

    //scannerpower(TRIGGER | SINGLE, 250); // Disabling this allows the OS to take care of the configured read mode (default: SINGLE, 2 seconds)

    for(;;)
    {
        if(readbarcode(&code) == OK)
        {
            result = 0;

			if (code.id == MENU_CODE)	// Check for standard Code-39 menu labels (from the OSE Universal menu book)
			{
				while (result != EXITING_MENU_MODE && result != ERROR)
				{
					if (result == 0 || readbarcode(&code) == OK)
					{
						switch ((result = ExecuteMenuLabel(&code)))
						{
							case ENTERING_MENU_MODE:
								sound(TSTANDARD, VHIGH, SHIGH, SMEDIUM, SHIGH, 0);
								break;

							case EXITING_MENU_MODE:
								sound(TSTANDARD, VHIGH, SHIGH, SMEDIUM, SHIGH, 0);
								delay(TSTANDARD * 5);	// Wait till buzzer sound if finished before saving (saving flash settings turns off sound timer temporarily)
								systemsetting("Z2");		// Save changed settings in flash memory and apply possibly changed interface
								break;

							case INVALID_OPTION_READ:
								sound(TLONG, VHIGH, SLOW, 0);
								break;

							case VALID_OPTION_READ:
								sound(TSTANDARD, VHIGH, SHIGH, SMEDIUM, SHIGH, 0);
								break;

							case LABEL_IGNORED:
								break;

							case ERROR:
								break;
						}
					}
				}
			}
			else if (code.id == MENU_CODE_C128)	// Check for Code-128 menu labels
			{
				switch ((result = ExecuteMenuLabel(&code)))
				{
					case INVALID_OPTION_READ:
						sound(TLONG, VHIGH, SLOW, 0);
						break;

					case VALID_OPTION_READ:
						sound(TSTANDARD, VHIGH, SMEDIUM, SHIGH, SMEDIUM, SHIGH, 0);
						delay(TSTANDARD * 5);	// Wait till buzzer sound if finished before saving (saving flash settings turns off sound timer temporarily)
						systemsetting("Z2");	// Save changed settings in flash memory and apply possibly changed interface
						break;

					case LABEL_IGNORED:
					default:
						break;
				}
			}
			else
			{
				goodreadled(GREEN, 10);
				sound( TSTANDARD, VHIGH, SMEDIUM, SHIGH, 0);
				printf("%*s\r", code.length, code.text);
			}
        }

        idle();
    }
}
