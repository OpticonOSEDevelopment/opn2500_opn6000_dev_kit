// This program demonstrates the saving and restoring of application settings using flash memory

#include <stdio.h>
#include <string.h>
#include "lib.h"

#define VERSION_CHECK        "DEMOv1.0"        // Verification string used by 'ApplicationSettingsMemorizing' to make sure that only
                                               // compatible application settings are restored from memory.
typedef struct
{
    int buzzer_frequency;
    int led_color;
} application_vars;

application_vars app;     // Structure in which all configurable application settings are placed

void main( void )
{
    char bcr_buf[42] = {0};
    struct barcode code = {0};
    int save;

    code.min   = 1;
    code.max   = 41;
    code.text  = bcr_buf;

    SystemSetting("8Z"); // Disable charge indicator (to avoid confusion about the LED color)

    //
    // The following function will try to restore application settings from flash memory
    // and will cause the configuration structure of this application to be stored
    // in flash memory when 'SystemSetting("Z2")' is called.
    //
    if(ApplicationSettingsMemorizing(ON, (void *)&app, sizeof(app), VERSION_CHECK) < 0)
    {
        app.buzzer_frequency = SHIGH;       // Reset all application settings
        app.led_color = GREEN;
        SystemSetting("Z2");                // Save settings in flash
    }

    ScannerPower(TRIGGER | SINGLE, 250);    // Trigger mode, 5 seconds read time, scanner off after 1 barcode
    for(;;)
    {
        if(ReadBarcode(&code) == OK)
        {
            save = TRUE;

            // The following lines compare if the barcode equals "HIGH", "MEDIUM", "LOW", "GREEN", "ORANGE" or "RED".
            // If the barcode equals any of these strings the buzzer/LED setting will be changed.
            if(strcmp(code.text, "HIGH") == 0)          {    app.buzzer_frequency = SHIGH;    }
            else if(strcmp(code.text, "MEDIUM") == 0)   {    app.buzzer_frequency = SMEDIUM;  }
            else if(strcmp(code.text, "LOW") == 0)      {    app.buzzer_frequency = SLOW;     }
            else if(strcmp(code.text, "GREEN") == 0)    {    app.led_color = GREEN;           }
            else if(strcmp(code.text, "ORANGE") == 0)   {    app.led_color = ORANGE;          }
            else if(strcmp(code.text, "RED") == 0)      {    app.led_color = RED;             }
            else
            {
                save = FALSE;
                printf("%*s\r", code.length, code.text);    // No configuration barcode, so output the barcode by USB or Cradle
            }

            if(save)
            {
                SystemSetting("Z2");        // Store settings in flash
            }

            Sound(TSTANDARD, VHIGH, app.buzzer_frequency, 0);
            GoodReadLed(app.led_color, 10);
        }

        Idle();        // Reduces power consumption
    }
}
