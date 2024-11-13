//------------------------------------------------------------------
//  Main header file of Bluetooth Demo application for the OPN2002
//------------------------------------------------------------------

#include "boards.h"

//
// States of the transmission state machine of this application
//
#define TSTATE_NO_DATA			 0		// Idle state: No data to be transmitted
#define TSTATE_MEMORIZING		 1		// Memorizing: Store all barcodes in memory
#define TSTATE_XMIT_BARCODE		 2		// Xmit barcode: Transmit a new barcode immediatly
#define TSTATE_XMIT_STORED_DATA	 3		// Xmit stored data: transmit memorized data
#define TSTATE_ERROR_WAIT_STATE  4		// If a transmission failed, but BltIsConnected()==TRUE -> Wait a certain time before trying to retransmit
#define TSTATE_CONN_WAIT_STATE   5		// If a transmission failed, because BltIsConnected()==FALSE -> Wait for the connection to be re-established

/*
 *--------------------------------------
 *  USB / Bluetooth HID Keyboard defines
 *--------------------------------------
 */
#define KB_BS_KEY               0xB0
#define KB_TAB_KEY              0xB1
#define KB_ENT_KEY              0xB2
#define KB_ESC_KEY              0xB3
#define KB_DOWN_KEY             0xB4
#define KB_UP_KEY               0xF3
#define KB_RIGHT_KEY            0xB6
#define KB_LEFT_KEY             0xB7
#define KB_DELETE_KEY           0xB8
#define KB_INSERT_KEY           0xB9
#define KB_HOME_KEY             0xBA
#define KB_END_KEY              0xBB
#define KB_PAGE_UP_KEY          0xBC
#define KB_PAGE_DOWN_KEY        0xFB
#define KB_CAPSLOCK_KEY         0xBE

#define NUMPAD_0_KEY            0xBF
#define NUMPAD_1_KEY            0xC0
#define NUMPAD_2_KEY            0xC1
#define NUMPAD_3_KEY            0xC2
#define NUMPAD_4_KEY            0xC3
#define NUMPAD_5_KEY            0xC4
#define NUMPAD_6_KEY            0xC5
#define NUMPAD_7_KEY            0xC6
#define NUMPAD_8_KEY            0xC7
#define NUMPAD_9_KEY            0xC8

#define KB_F1_KEY               0xF0
#define KB_F2_KEY               0xE2
#define KB_F3_KEY               0xC9
#define KB_F4_KEY               0xCA
#define KB_F5_KEY               0xCB
#define KB_F6_KEY               0xCC
#define KB_F7_KEY               0xCD
#define KB_F8_KEY               0xCE
#define KB_F9_KEY               0xF2
#define KB_F10_KEY              0xD0
#define KB_F11_KEY              0xF1
#define KB_F12_KEY              0xD2

#define NUMLOCK_KEY             0xD3
#define NUMPAD_SLASH_KEY        0xD4
#define NUMPAD_STAR_KEY         0xD5
#define NUMPAD_MINUS_KEY        0xA9
#define NUMPAD_PLUS_KEY         0xD7
#define NUMPAD_ENTER_KEY        0xD8

#define LEFT_SHIFT_KEY          0xD9
#define RIGHT_SHIFT_KEY         0xDA
#define LEFT_CONTROL_KEY        0xDB
#define RIGHT_CONTROL_KEY       0xDC
#define LEFT_ALT_KEY            0x9F
#define RIGHT_ALT_KEY           0xDE
#define LEFT_GUI_KEY            0xDF
#define WINDOWS_KEY             LEFT_GUI_KEY
#define RIGHT_GUI_KEY           0xFC
#define MENU_KEY                0xE8

#define NUMPAD_POINT_KEY        0xE3
#define PRINT_SCREEN_KEY        0xE4
#define SCROLL_LOCK_KEY         0xE5
#define PAUSE_KEY               0xFE
#define EURO_SIGN               0xE7

#define TOGGLE_KEYBOARD         0xA6        // Toggle iPhone/iPad onscreen keyboard

// Defines for app.blt_interface
#define BLT_HID_DEVICE					0x01
#define BLT_BLE_DEVICE					0x02
#define BLT_SPP_DEVICE					0x06		// Deliberately added the BLT_BLE_DEVICE flag

// Defines for app.blt_connect_options
#define DISABLE_TRIGGER_CONNECT			0x01		// If set, the scanner will only configure the Bluetooth address when reading a Bluetooth address label (and won't connect)
#define AUTO_CONNECT_AFTER_DECODE		0x02

// Defines for app.memorizing
#define MEMORIZING_ENABLED				0x01
#define MEMORIZE_AFTER_CONNECT_LOSS		0x00
#define MEMORIZE_WHEN_NOT_CONNECTED		0x02
#define MEMORIZE_ALWAYS					0x04
#define MEMORIZE_MODE_MASK				0x06

// Defines for app.read_time
#define READ_TIME_INDEFINITELY			0xFE		// Read time indefinitely

// Defines for app.buzzer_options
#define BUZZER_ENABLED					0x01
#define STARTUP_BUZZER					0x02
#define BUZZER_BEFORE_TRANSMIT			0x40

// Defines for app.usb_mode 
#define USB_MODE_NONE					0x00
#define USB_MODE_HID					0x01
#define USB_MODE_VCP					0x02
#define USB_MODE_CDC					0x03
#define USB_MODE_MSD					0x04
#define USB_MODE_CDC_OPC				0x05

// Defines for app.options 
#define ENABLE_KEYBOARD_TOGGLE			0x01
#define ENABLE_WAKEUP_KEY				0x02
#define USE_SERIAL_FOR_BT_NAME			0x08
#define SEND_MANUALLY					0x00
#define SEND_AFTER_3SECONDS				0x04
#define BUTTON_TO_SEND					0x10
#define BUTTON_TO_SEND_ADVANCED			0x14
#define BUTTON_TO_SEND_MASK				0x14
#define USB_CONNECT_BEEP				0x40
#define BLE_CONNECT_BEEP				0x80

//
// Note: Only change this version if the application settings of the previous version are no longer compatible
//
// Verification string used by 'ApplicationSettingsMemorizing' to check if any possible application settings found in flash memory 
// are compatible with this application. If this version check fails, then all application settings will be set back to default.
// Only check this string if the application settings 
#define	VERSION_CHECK					"BLT_DEMOv1.3"		
															
//
// Defines a structure of configuration settings that is stored in non-volatile (flash) memory
// to make it possible that certain configurations aren't lost after a restart
//
typedef struct
{
	unsigned char blt_interface;							// Indicates the currently selected interface (HID or VCP)
	unsigned char trigger_to_connect_time;					// Trigger to connect time (stored in seconds, 0 = disabled)
	unsigned char trigger_to_disconnect_time;				// Trigger to disconnect time (stored in seconds, 0 = disabled)
	unsigned char blt_connect_options;						// Bluetooth connect options (Trigger to connect; Auto connect)
	unsigned char auto_disconnect_time;						// Time that the device stays connected when idle (stored in minutes, 0=disabled)
	unsigned char memorizing;								// Memorizing options
	unsigned char read_mode;                                // Read mode
    unsigned char read_time;                                // Read time (stored in 0,1s steps)
	unsigned char goodread_duration;						// Good read led duration (stored in 20ms steps)
	unsigned char buzzer_options;							// Buzzer tone and options
	unsigned char options;									// Additional options, for now only used for iOS 'enable keyboeard toggle flag'
	unsigned char usb_mode;									// Determines the USB behaviour
	unsigned char manual_memorizing;						// Stores the state of manual memorizing
	unsigned char del_enable;
} bt_appl_vars;

