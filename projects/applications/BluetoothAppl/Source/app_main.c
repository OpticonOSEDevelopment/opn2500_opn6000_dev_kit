//------------------------------------------------------------------
//  Bluetooth application
//------------------------------------------------------------------
//  This Bluetooth application reads barcodes and transmit them by 
//  Bluetooth as either serial data by BLE or as Bluetooth keyboard data.
//
//  This application also supports the reading of the Univeral menu-book
//  making it possible to configure all sort of barcode and Bluetooth options.
//  Also OSE serial commands can be used to configure the various options.
//
//  Since all changed configurations are stored in flash, any changed
//  configurations are restored after a restart or when the battery was empty.
//
//  This application does also demonstrates the storing of barcodes
//  data in a file to memorize barcodes while not being connected.
//
//  See the application manual on more details on the support
//  features and on how the application works.
//
//------------------------------------------------------------------

// Includes
#include <stdlib.h>
#include <string.h>
#include <Ctype.h>
#include "lib.h"
#include "Debug.h"
#include "memorize.h"
#include "bt_main.h"
#include "BarcodeBuf.h"
#include "ff.h"
#include "BarcodeDBase.h"

// Return values of ProcessBarcode
#define NORMAL_BARCODE			0		// A normal barcode label was read
#define MENU_LABEL				1		// A normal menu label was read
#define DISCONNECT_LABEL		2		// A +-DISC-+ label was read
#define CONNECT_LABEL			3		// A +-CONN-+ label was read
#define CONNECTABLE_LABEL		4		// A +-DSCO-+ label was read
#define CONFIG_IPHONE_LABEL 	5		// A +-IOSON-+ or +-IOSOFF-+ lable was read
#define UNPAIR_LABEL			6		// A +-UNPR-+ label was read
#define MEMORIZE_CLEAR_LABEL	7		// A +-MCLR-+ label was read
#define MEMORIZE_START_LABEL	8 		// A +-MSTR-+ label was read
#define MEMORIZE_STOP_LABEL		9 		// A +-MSTP-+ label was read
#define MEMORIZE_XMIT_LABEL_CLR	10		// A +-MXMT-+ label was read
#define MEMORIZE_XMIT_LABEL		11		// A +-MDLD-+ label was read (SerialIO specific)
#define CONFIG_BT_LOCAL_NAME	12		// A +-BTLNS-+

// Defines used for the various timers used by this application
#define MAX_TIMERS				6
#define MAKE_CONNECTABLE		0		// Timer used for 'Trigger to make connectable' time
#define CONNECTABLE_TIME_OUT	1		// Timer used for time-out mechanism when connectable
#define DISCONNECT				2		// Timer used for 'Trigger to disconnect' time
#define TRANSMIT_ERROR_WAIT		3		// Timer used to wait a certain time after a failed transmission before trying to retransmit
#define BATT_LOW				4		// Timer used to signal when the battery is low
#define LOCATE_HOST_TIMER		5		// Timer used to detect locate host tripple trigger press with function key held


#define LOCATE_HOST_TIMER_VALUE			  25


#define TIMER_DISABLED	  ((unsigned int)-1)
#define TRIGGER_TO_CONNECT_TIME			  3			// Trigger to connect time: 3 seconds (using the trigger key)
#define TRIGGER_TO_DISCONNECT_TIME		  3			// Trigger to disconnect time: 3 seconds (using the clear key)
#define RECONNECT_WAIT_TIME			      5			// Time till between reconnects attempt: 5 seconds

#ifdef HAS_2D_ENGINE
#define IDLE_POWER_DOWN_TIME		   2*50			// Powerdown time when idle: 2 seconds
#define IDLE_POWER_OFF_TIME			   10*60*50		// Shutdown time when idle: 10 minutes
#else
#define IDLE_POWER_DOWN_TIME		   1*50			// Sleep time when idle: 1 second
#define IDLE_POWER_OFF_TIME			   2*50			// Powerdown time when idle: 2 seconds
#endif

#define XMIT_CONNECT_WAIT_TIME		   (3*50-1)		// After a reonnection, wait this long before trying a transmit: 3 seconds (-1 tick, so it's shorter than default trigger to disconnect)
#define XMIT_ERROR_WAIT_TIME		   5*50			// After a failed transmission with an active Bluetooh link, wait this long before trying a retransmit: 5 seconds
#define BATT_LOW_FLASH_TIME			   6*50

#define MAX_BATT_LOW_WARNINGS		   50			// Multiply by number of seconds of BATT_LOW_FLASH_TIME to find delay before shutting down

static volatile unsigned int timer[MAX_TIMERS];			// Array of various timers

static int com_opened;
static int last_key_pressed = TRIGGER_KEY; 	// Used to determine whether a barcode was read using the clear key or trigger key.

// When SystemSettingsMemorizing(ON) is called, this will result in 'Application_Default' being called, 
// but only if the menu-option default is known by the application, so i.e. "U2" won't trigger this function
// in this cause we have to perform a default in main() by checking this variable
static bool set_to_known_default = FALSE;

//
// States of the connection state machine of this application
//
#define STATE_DISCONNECTED		 0		// Idle state: No Bluetooth activity
#define STATE_CONNECTABLE		 1		// Connectable state: Waiting for a connection & discoverable
#define STATE_RECONNECTABLE		 2		// Connectable state: Waiting for a connection (no flashing)
#define STATE_CONNECTED          3		// Connected state (Bluetooth)
#define STATE_CONNECTED_USB      4		// Connected state (USB)
#define STATE_WAIT_FOR_RELEASE	 5		// Wait state: Waits until both keys are not pressed and then jumps back idle state

#define STATE_DISCONNECTED_USB	 6		// Only used for USB disconnect beep 
#define STATE_MANUAL_DISCONNECT	 7		// Only used for Manual disconnect beep 

#ifdef DEBUG
#define LOG_STATE
#endif

// Solely used to reduce differences between this app and SerialIO
#define OkBeep_App			OkBeep
#define MemorizeBeep_App	MemorizeBeep
#define ErrBeep_App			ErrBeep
#define Sound_App			Sound
//

#ifdef LOG_STATE
char* str_connection_state[] =
{
	"STATE_DISCONNECTED",
	"STATE_CONNECTABLE",
	"STATE_RECONNECTABLE",
	"STATE_CONNECTED",
	"STATE_CONNECTED_USB",
	"STATE_WAIT_FOR_RELEASE",
};

const char* str_barcode_scan[] = 
{
	"NORMAL_BARCODE",
	"MENU_LABEL",
	"DISCONNECT_LABEL",
	"CONNECT_LABEL",
	"CONNECTABLE_LABEL",
	"CONFIG_IPHONE_LABEL",
	"UNPAIR_LABEL",
	"MEMORIZE_CLEAR_LABEL",
	"MEMORIZE_START_LABEL",
	"MEMORIZE_STOP_LABEL" ,
	"MEMORIZE_XMIT_LABEL",	
};

const char *str_tstate[] =
{
	"TSTATE_NO_DATA",
	"TSTATE_MEMORIZING",
	"TSTATE_XMIT_BARCODE",	
	"TSTATE_XMIT_STORED_DATA",
	"TSTATE_ERROR_WAIT_STATE",
	"TSTATE_CONN_WAIT_STATE",
};  

static int old_connection_state = -1;
static int old_xmit_state = -1;
static int old_bc_scan_state = -1;
static int bc_scan_state = 0; 
#endif

//
// local functions
//
static void BluetoothStateMachine(bool barcode_read);
static void LocateHostStateMachine(void);
static void TransmitStateMachine(uint8_t new_barcode);
static void TriggerToConnectHandler(bool trigger_enabled, bool barcode_read);
static int ProcessBarcode(struct barcode *code);
static void ProcessMenuLabels(int res);
static int ShutdownCallback(void);
static void Application_Default(uint8_t ble_interface, uint8_t usb_mode);
static void Bluetooth_HID_Default(void);
static void Bluetooth_BLE_Default(void);
static void Usb_HID_Default(void);
static void Usb_VCP_Default(void);
static void Usb_CDC_Default(void);
static void Usb_OptiConnect_Default(void);
static int BltIsConnected_App(void);
static int UsbIsConnected_App(void);
static int IsConnected_App(void);
static int AutoDisconnectTimePassed(struct time *dt);
static void ResetAutoDisconnectTime(struct time *dt);
static void StopAutoDisconnectTime(void);
static int ProcessBarcodeInMemory(struct barcode *pCode);
#ifdef HAS_BATTERY
static void CheckForLowBattery(void);
static void ReportBatteryLevel(void);
#endif
static void ApplicationStoreSettings(void);
static void EnableBuzzer(void);
static void DisableBuzzer(void);
static void RS232_TriggerOn(void);
static void DeleteKeyEnabled(void);
static void DeleteKeyDisabled(void);

#ifndef HAS_2D_ENGINE
static void SetReadMode(int readmode, int readtime);
#endif
#ifdef LOG_STATE
void LogConnectionState(void);
#endif

// Global variables
bt_appl_vars app;			// Structure of configuration settings that is stored in non-volatile (flash) memory

// Local variables
static int connection_state = STATE_DISCONNECTED;
static int connection_lost = FALSE;
static int reset_read_mode;

static int xmit_state = TSTATE_NO_DATA;
static int xmit_state_clr = TRUE;		// Delete memorized barcode data after transmission or not

//
// The following table implements SystemSetting options that need to be added to the OS or overruled
//
// Note: The use of 'SystemSetting()' inside the functions in the table below is limited to a protect against recursion,
//       It is still possible to use SystemSetting() but only with a 1 menu option per function call.
//
const OPTION blt_menu_option_table[] =
{                                               // CLEAR mask                              SET mask (use 0xFF 0xFF in case of a function call)
    { "SU",(void*)Usb_HID_Default,						0xFF,                                   0xFF                        },
#ifndef HAS_2D_ENGINE
	{ "S0",(void*)&(app.read_mode),                     0xFF,                                   SCAN_SINGLE         		},
    { "S1",(void*)&(app.read_mode),                     0xFF,                                   SCAN_MULTIPLE   			},
    { "S2",(void*)&(app.read_mode),                     0xFF,                                   SCAN_CONTINUOUS				},
	{ "Y0",(void*)&(app.read_time),                     0xFF,                                   0							},
    { "Y1",(void*)&(app.read_time),                     0xFF,                                   10							},
    { "Y2",(void*)&(app.read_time),                     0xFF,                                   20							},
    { "Y3",(void*)&(app.read_time),                     0xFF,                                   30							},
    { "Y4",(void*)&(app.read_time),                     0xFF,                                   40							},
    { "Y5",(void*)&(app.read_time),                     0xFF,                                   50							},
    { "Y6",(void*)&(app.read_time),                     0xFF,                                   60							},
    { "Y7",(void*)&(app.read_time),                     0xFF,                                   70							},
    { "Y8",(void*)&(app.read_time),                     0xFF,                                   80							},
    { "Y9",(void*)&(app.read_time),                     0xFF,                                   90							},
    { "YM",(void*)&(app.read_time),                     0xFF,                                   READ_TIME_INDEFINITELY		},
#endif
    { "SO",(void*)Bluetooth_BLE_Default,				0xFF,                                   0xFF                        },
	{ "GC",(void*)&(app.buzzer_options),				0x00,                                   STARTUP_BUZZER              },
	{ "GD",(void*)&(app.buzzer_options),				STARTUP_BUZZER,                         0x00                        },
   	{ "VY",(void*)&(app.buzzer_options),      			0x00, 									BUZZER_BEFORE_TRANSMIT      },
    { "VZ",(void*)&(app.buzzer_options),      			BUZZER_BEFORE_TRANSMIT,        			0x00       					},
	{ "W0",(void*)DisableBuzzer,		         		0xFF,                                   0xFF                        },
	{ "W8",(void*)EnableBuzzer,							0xFF,                                   0xFF                        },
    { "C01",(void*)Usb_VCP_Default,						0xFF,                                   0xFF                        },
    { "BPC",(void*)Usb_CDC_Default,						0xFF,                                   0xFF                        },
	{ "BQO",(void*)Usb_OptiConnect_Default,				0xFF,                                   0xFF                        },
    { "BM0",(void*)&(app.memorizing),					MEMORIZE_MODE_MASK,						MEMORIZE_AFTER_CONNECT_LOSS },
    { "BM1",(void*)&(app.memorizing),					MEMORIZE_MODE_MASK,						MEMORIZE_WHEN_NOT_CONNECTED },
    { "BM2",(void*)&(app.memorizing),					MEMORIZE_MODE_MASK,						MEMORIZE_ALWAYS				},
	{ "C02",(void*)Bluetooth_HID_Default,				0xFF,                                   0xFF                        },
	{ "EBC",(void*)&(app.options),						BUTTON_TO_SEND_MASK,                    BUTTON_TO_SEND              },
	{ "EBLE",(void*)Bluetooth_BLE_Default,				0xFF,                                   0xFF                        },
    { "AD00",(void*)&(app.auto_disconnect_time),		0xFF,									0							},
    { "AD01",(void*)&(app.auto_disconnect_time),		0xFF,									10							},
    { "AD02",(void*)&(app.auto_disconnect_time),		0xFF,									20							},
    { "AD03",(void*)&(app.auto_disconnect_time),		0xFF,									30							},
    { "AD04",(void*)&(app.auto_disconnect_time),		0xFF,									40							},
    { "AD05",(void*)&(app.auto_disconnect_time),		0xFF,									50							},
    { "AD06",(void*)&(app.auto_disconnect_time),		0xFF,									60							},

	{ "BPK",(void*)&(app.options),						BUTTON_TO_SEND_MASK,			        BUTTON_TO_SEND_ADVANCED     },		//	Enable advanced send mode	(Press scan button 3 seconds to send, press delete button 6 seconds to delete)
    { "BPL",(void*)&(app.options),						BUTTON_TO_SEND_MASK,                    BUTTON_TO_SEND			    },		//	Disable advanced send mode	(auto clear after transmission)
	{ "BQB",(void*)&(app.options),						BUTTON_TO_SEND_MASK,                    SEND_AFTER_3SECONDS  		},
	{ "BQE",(void*)&(app.options),						BUTTON_TO_SEND_MASK,                    SEND_MANUALLY		  		},
	{ "BATT",(void*)ReportBatteryLevel,					0xFF,									0xFF						},
	{ "TSCD",(void*)&(app.blt_connect_options),			0x00,									DISABLE_TRIGGER_CONNECT		},
	{ "TSCE",(void*)&(app.blt_connect_options),			DISABLE_TRIGGER_CONNECT,				0x00                        },
	{ "DTMD",(void*)&(app.memorizing),					MEMORIZING_ENABLED,						0x00		                },
	{ "DTME",(void*)&(app.memorizing),					0x00,									MEMORIZING_ENABLED          },
	{ "ARCE",(void*)&(app.blt_connect_options),			0x00,									AUTO_CONNECT_AFTER_DECODE | DISABLE_TRIGGER_CONNECT },
	{ "ARCD",(void*)&(app.blt_connect_options),			AUTO_CONNECT_AFTER_DECODE,				0x00                        },
	{ "PC00",(void*)&(app.trigger_to_connect_time),		0xFF,                                   0x00                        },
	{ "PC01",(void*)&(app.trigger_to_connect_time),		0xFF,                                   0x01                        },
	{ "PC02",(void*)&(app.trigger_to_connect_time),		0xFF,                                   0x02                        },
	{ "PC03",(void*)&(app.trigger_to_connect_time),		0xFF,                                   0x03                        },
	{ "PC04",(void*)&(app.trigger_to_connect_time),		0xFF,                                   0x04                        },
	{ "PC05",(void*)&(app.trigger_to_connect_time),		0xFF,                                   0x05                        },
	{ "PC06",(void*)&(app.trigger_to_connect_time),		0xFF,                                   0x06                        },
	{ "PC07",(void*)&(app.trigger_to_connect_time),		0xFF,                                   0x07                        },
	{ "PC08",(void*)&(app.trigger_to_connect_time),		0xFF,                                   0x08                        },
	{ "PC09",(void*)&(app.trigger_to_connect_time),		0xFF,                                   0x09                        },
	{ "PD00",(void*)&(app.trigger_to_disconnect_time),  0xFF,                                   0x00                        },
	{ "PD01",(void*)&(app.trigger_to_disconnect_time),	0xFF,                                   0x01                        },
	{ "PD02",(void*)&(app.trigger_to_disconnect_time),	0xFF,                                   0x02                        },
	{ "PD03",(void*)&(app.trigger_to_disconnect_time),	0xFF,                                   0x03                        },
	{ "PD04",(void*)&(app.trigger_to_disconnect_time),	0xFF,                                   0x04                        },
	{ "PD05",(void*)&(app.trigger_to_disconnect_time),	0xFF,                                   0x05                        },
	{ "PD06",(void*)&(app.trigger_to_disconnect_time),	0xFF,                                   0x06                        },
	{ "PD07",(void*)&(app.trigger_to_disconnect_time),	0xFF,                                   0x07                        },
	{ "PD08",(void*)&(app.trigger_to_disconnect_time),	0xFF,                                   0x08                        },
	{ "PD09",(void*)&(app.trigger_to_disconnect_time),	0xFF,                                   0x09                        },
	{ "UNPR",(void*)BltUnpair,							0xFF,                                   0xFF                        },
	{ "BQN", (void*)&(app.options),						0x00,                                   ENABLE_KEYBOARD_TOGGLE|ENABLE_WAKEUP_KEY },
	{ "BQF", (void*)&(app.options),						ENABLE_KEYBOARD_TOGGLE|ENABLE_WAKEUP_KEY,0x00                       },
	{ "DELE",(void*)DeleteKeyEnabled,					0xFF,                                   0xFF                        },
	{ "DELD",(void*)DeleteKeyDisabled,					0xFF,                                   0xFF                        },

	// Copied from batch application
	{ "+QB",(void*)DeleteKeyDisabled,					0xFF,                                   0xFF                        },
	{ "+QC",(void*)DeleteKeyEnabled,					0xFF,                                   0xFF                        },

    { "Z2", ApplicationStoreSettings,                   0xFF,                                   0xFF                        },
    { "Z",  RS232_TriggerOn,							0xFF,									0xFF                         },
};

#define BLUETOOTH_MAX_OPTIONS sizeof(blt_menu_option_table)/sizeof(OPTION)

//------------------------------------------------------------------
//  GoodMenuBeep
//  ============================
//  Sounds an menu code read beep according to the current config
//------------------------------------------------------------------
void GoodMenuBeep(int code_id)
{
	Vibrate(TSTANDARD);

	if(code_id == MENU_CODE_C128 || code_id == MENU_CODE_QR || code_id == MENU_CODE_PDF || code_id == MENU_CODE_AZTEC)
	{
		Sound_App(TSTANDARD, VSYSTEM, SHIGH, SMEDIUM, SHIGH, SMEDIUM, SHIGH, SMEDIUM, SHIGH, 0);
		Delay(TSTANDARD * 8);
	}
	else
	{
		Sound_App(TSTANDARD, VSYSTEM, SHIGH, SMEDIUM, SHIGH, 0);
	}
}

//------------------------------------------------------------------
//  BadMenuBeep
//  ============================
//  Sounds an menu code bad beep according to the current config
//------------------------------------------------------------------
void BadMenuBeep(void)
{
	Vibrate(TLONG);

	Sound_App(TLONG, VSYSTEM, SLOW, 0);
}

//------------------------------------------------------------------
//  BltIsConnected_App
//  ============================
//  Returns the Bluetooth connection state of the application and
//  not the from the Bluetooth stack.
//------------------------------------------------------------------
int BltIsConnected_App(void)
{
	if(connection_state == STATE_CONNECTED)
		return TRUE;

	return FALSE;
}

//------------------------------------------------------------------
//  UsbIsConnected_App
//  ============================
//  Returns the USB connection state of the application and
//  not the from the Bluetooth stack.
//------------------------------------------------------------------
int UsbIsConnected_App(void)
{
	if(connection_state == STATE_CONNECTED_USB)
		return TRUE;

	return FALSE;
}

//------------------------------------------------------------------
//  IsConnected_App
//  ============================
//  Returns the Bluetooth or USB connection state of the application
//------------------------------------------------------------------
int IsConnected_App(void)
{
	if(connection_state == STATE_CONNECTED || connection_state == STATE_CONNECTED_USB)
		return TRUE;

	return FALSE;
}

void DeleteKeyEnabled(void)
{
	app.del_enable = 1;
	ClearKeyAsTrigger(app.del_enable);
}


void DeleteKeyDisabled(void)
{
	app.del_enable = 0;
	ClearKeyAsTrigger(app.del_enable);
}

//------------------------------------------------------------------
//  Application_Default
//  ============================
//  Reset all application specific settings to default
//------------------------------------------------------------------
void Application_Default(uint8_t ble_interface, uint8_t usb_mode)
{
	set_to_known_default = TRUE;		// When SystemSettingsMemorizing(ON) is called, this will result in 'Application_Default' being called, 
										// but only if the menu-option default is known by the application, so i.e. "U2" won't trigger this function
										// in this cause we have to perform a default in main()

	if(usb_mode == USB_MODE_NONE)
	{
		app.trigger_to_connect_time = TRIGGER_TO_CONNECT_TIME;			// Trigger to connect time (3 seconds)
		app.trigger_to_disconnect_time = TRIGGER_TO_DISCONNECT_TIME;	// Trigger to disconnect time (3 seconds)
		app.blt_connect_options = 0;								// Auto reconnect with trigger: Enabled
	}
	else
	{
		app.trigger_to_connect_time = 0;								// Trigger to connect time: Disabled
		app.trigger_to_disconnect_time = 0;								// Trigger to disconnect time: Disabled
		app.blt_connect_options = DISABLE_TRIGGER_CONNECT;				// Auto reconnect with trigger: Disabled
	}

	app.auto_disconnect_time = 0;									// Time that the device stays connected when idle (disabled)

	app.memorizing = MEMORIZING_ENABLED | MEMORIZE_WHEN_NOT_CONNECTED;	// Always Memorize when not connected
	
#ifndef HAS_2D_ENGINE
	app.read_mode = SCAN_SINGLE;                                    // Single read
    app.read_time = 0;                                              // 0 seconds
#endif
	app.options = ENABLE_KEYBOARD_TOGGLE | ENABLE_WAKEUP_KEY | BLE_CONNECT_BEEP;

	if(usb_mode == USB_MODE_HID || usb_mode == USB_MODE_VCP || usb_mode == USB_MODE_CDC || usb_mode == USB_MODE_CDC_OPC)
		app.options |= USB_CONNECT_BEEP;

	if(ble_interface == BLT_HID_DEVICE || usb_mode == USB_MODE_HID)
		app.options |= BUTTON_TO_SEND;
	else
		app.options |= SEND_AFTER_3SECONDS;
	
	app.manual_memorizing = STANDBY;
	app.del_enable = 1;
		
	app.buzzer_options = BUZZER_ENABLED;		// Buzzer enabled!
	
#ifdef HAS_2D_ENGINE
	SystemSetting( "S0" );			// set default read mode.
#endif
	SystemSetting("YC");			// Enable menu-labels (as data)
	SystemSetting("TT");			// Serial config enabled
	SystemSetting("BPV");			// Set interface independent default suffix: <CR> (VCP) / Enter key (HID)
	SystemSetting("7G");			// charging indicator enabled
	SystemSetting("-R");			// operate on battery low
	SystemSetting("W2");			// Two tone buzzer
	SystemSetting("W5");			// 200ms
	SystemSetting("W8");			// buzzer enabled.
#ifndef HAS_2D_ENGINE
	SystemSetting("BL+");			// C128 alternate decode alg.
#endif

	app.blt_interface = ble_interface;		// Select Bluetooth interface
	app.usb_mode = usb_mode;				// Set USB-mode
}

//------------------------------------------------------------------
//  Bluetooth_BLE_Default
//  ============================
//  Reset all Bluetooth settings to default
//------------------------------------------------------------------
void Bluetooth_BLE_Default(void)
{
	memset( &app, 0, sizeof(app) );
	SystemSetting("EBLE");			// Call Bluetooth BLE default function
	Application_Default(BLT_BLE_DEVICE, USB_MODE_NONE);
}

//------------------------------------------------------------------
//  Bluetooth_HID_Default
//  ============================
//  Reset all Bluetooth settings to default
//------------------------------------------------------------------
void Bluetooth_HID_Default(void)
{
	memset( &app, 0, sizeof(app) );
	SystemSetting("C02");			// Call Bluetooth HID default function

	Application_Default(BLT_HID_DEVICE, USB_MODE_NONE);		// Deselect USB-VCP/CDC and USB-HID transmission
	
	app.del_enable = 0;
}

//------------------------------------------------------------------
//  Usb_HID_Default
//  ============================
//  Reset all settings to default
//------------------------------------------------------------------
void Usb_HID_Default(void)
{
	memset( &app, 0, sizeof(app) );
	SystemSetting("SU");			// Call USB-HID default function

	Application_Default(BLT_HID_DEVICE, USB_MODE_HID);		// Enable USB-HID transmission
}


//------------------------------------------------------------------
//  Usb_VCP_Default
//  ============================
//  Reset all settings to default
//------------------------------------------------------------------
void Usb_VCP_Default(void)
{
	memset( &app, 0, sizeof(app) );
	SystemSetting("C01");			// Call USB-VCP default function

	Application_Default(BLT_BLE_DEVICE, USB_MODE_VCP);		// Enable USB-VCP transmission
}

//------------------------------------------------------------------
//  Usb_CDC_Default
//  ============================
//  Reset all settings to default
//------------------------------------------------------------------
void Usb_CDC_Default(void)
{
	memset( &app, 0, sizeof(app) );
	SystemSetting("BPC");			// Call USB-CDC default function
	
	Application_Default(BLT_BLE_DEVICE, USB_MODE_CDC);		// Enable USB-CDC transmission
}

//------------------------------------------------------------------
//  Usb_OptiConnect_Default
//  ============================
//  Reset all settings to default
//------------------------------------------------------------------
void Usb_OptiConnect_Default(void)
{
	memset( &app, 0, sizeof(app) );
	SystemSetting("BQO");			// Call USB-CDC default function
	
	Application_Default(BLT_BLE_DEVICE, USB_MODE_CDC_OPC);		// Enable USB-CDC transmission
}

void EnableBuzzer(void)
{
	app.buzzer_options |= BUZZER_ENABLED;
	SystemSetting("W8");
}

void DisableBuzzer(void)
{
	app.buzzer_options &= ~BUZZER_ENABLED;
	SystemSetting("W0");
}

//------------------------------------------------------------------
//  ReportBatteryLevel  ( "BATT" comand , BLE battery service is handled by the OS )
//  ============================
//  Put the battery percentage remaining over the open com port
//------------------------------------------------------------------
void ReportBatteryLevel(void)
{
	static char batt_level[6] = {'\0'};
	int percentage = GetBatteryLevel();
	sprintf(batt_level, "%d%%", percentage );
	PutString(batt_level);
}


void ApplicationStoreSettings(void)
{
    SystemSetting("Z2");
}

static void RS232_TriggerOn(void)
{
	last_key_pressed = TRIGGER_KEY;
    SystemSetting("Z");
}


//------------------------------------------------------------------
//  UpdateAutoPowerDown
//  ============================
//  Depending on the current state, this function changes the
//  AutoPowerDown and Auto-shutdown settings.
//
//  (Re)connecting:					 No sleep	No shutdown
//  Connectable:					 No sleep	No shutdown
//  Connected:						 Sleep		No shutdown
//	Not connected / no memorizng:	 Sleep		Shutdown
//	Not connected / memorizing ON:	 Sleep		Shutdown
//	Connection lost with memorizing: Sleep		No shutdown
//------------------------------------------------------------------
void UpdateAutoPowerDown(int state)
{
	switch(state)
	{
		case STATE_CONNECTABLE:
		case STATE_RECONNECTABLE:
			AutoPowerDown(ON, IDLE_POWER_DOWN_TIME);		// Allow sleep
			AutoPowerDown(APD_SHUTDOWN_OFF, 0);				// Disable power off to allow app timers to operate
			break;

		case STATE_CONNECTED:
		case STATE_CONNECTED_USB:
			AutoPowerDown(ON, IDLE_POWER_DOWN_TIME);				// Allow sleep
			AutoPowerDown(APD_SHUTDOWN_ON, IDLE_POWER_OFF_TIME);	// Turn off scan engine after a while			
			break;

		default:
			if(	app.manual_memorizing == STANDBY					// When connection was lost and MEMORIZE_AFTER_CONNECT_LOSS was set
				&& (app.memorizing & MEMORIZING_ENABLED)
				&& (app.memorizing & MEMORIZE_MODE_MASK) == MEMORIZE_AFTER_CONNECT_LOSS
				&& connection_lost )
			{
				AutoPowerDown(ON, IDLE_POWER_DOWN_TIME);			// Allow sleep
				AutoPowerDown(APD_SHUTDOWN_OFF, 0);					// Disable auto power off time (otherwise we can't blink slowly)
			}
			else
			{
				AutoPowerDown(ON, IDLE_POWER_DOWN_TIME);				// Allow sleep
				AutoPowerDown(APD_SHUTDOWN_ON, IDLE_POWER_OFF_TIME);	// Allow shutdown
			}
			break;
	}
}


//------------------------------------------------------------------
//  IsMemorizingEnabled
//  ============================
//  Returns TRUE if memorizing is (allowed to be) enabled
//------------------------------------------------------------------
int IsMemorizingEnabled(unsigned char lost_connection)
{
	if(app.manual_memorizing == ON)
		return TRUE;

	if(app.manual_memorizing == OFF)
		return FALSE;

	if( (app.memorizing & MEMORIZING_ENABLED) == 0 )
		return FALSE;

	switch(app.memorizing & MEMORIZE_MODE_MASK)
	{
		case MEMORIZE_ALWAYS:
			return TRUE;

		case MEMORIZE_WHEN_NOT_CONNECTED:
			if(lost_connection || !IsConnected_App())
				return TRUE;
			
			return FALSE;

		case MEMORIZE_AFTER_CONNECT_LOSS:
		default:
			return lost_connection;
	}
}

// If read modes are handled by the OS
#ifndef HAS_2D_ENGINE
//------------------------------------------------------------------
//  SetReadMode
//  ============================
//  Converts the configured read time/mode to a ScannerPower call
//------------------------------------------------------------------
void SetReadMode(int readmode, int readtime)
{
    switch (readmode)
    {
    case SCAN_MULTIPLE:
        if (readtime == 0)
            ScannerPower(SCAN_MULTIPLE | SCAN_TRIGGER, 10*50);        // Limit trigger mode to 10 seconds to avoid pressing the trigger for 20s, causing an unintended restart
        else if (readtime == READ_TIME_INDEFINITELY)
            ScannerPower(SCAN_MULTIPLE, -1);
        else
            ScannerPower(SCAN_MULTIPLE, readtime*5);
        break;

    case SCAN_CONTINUOUS:
        if (readtime == 0)
            ScannerPower(SCAN_CONTINUOUS | SCAN_TRIGGER, 10*50);      // Limit trigger mode to 10 seconds to avoid pressing the trigger for 20s, causing an unintended restart
        else if (readtime == READ_TIME_INDEFINITELY)
            ScannerPower(SCAN_CONTINUOUS, -1);
        else
            ScannerPower(SCAN_CONTINUOUS, readtime*5);
        break;

    case SCAN_SINGLE:
    default:
        if (readtime == 0)
            ScannerPower(SCAN_SINGLE | SCAN_TRIGGER, 10*50);          // Limit trigger mode to 10 seconds to avoid pressing the trigger for 20s, causing an unintended restart
        else if (readtime == READ_TIME_INDEFINITELY)
            ScannerPower(SCAN_SINGLE, -1);
        else
            ScannerPower(SCAN_SINGLE, readtime*5);
        break;
    }
}
#endif

//------------------------------------------------------------------
//  ErrIndicator(int color)
//  ============================
//  Sounds an error beep and shows a LED according to the current configuration
//------------------------------------------------------------------
void ErrorIndicator(int color)
{
	if(color == RED)
		ErrBeep_App();
	else
		WarningBeep();
		
	ShowGoodReadLed(color);
}

void ConnectionIndicator(int event_type)
{
	switch(event_type)
	{
		case STATE_CONNECTED:
			if(!(app.options & BLE_CONNECT_BEEP))
				return;
			
			Sound(TVLONG, VSYSTEM, SHIGH, 0);
			GoodReadLed(BLUE, TVLONG);
			break;

		case STATE_CONNECTED_USB:
			if(!(app.options & USB_CONNECT_BEEP))
				return;

			Sound(TVLONG, VSYSTEM, SHIGH, 0);
			GoodReadLed(GREEN, TVLONG);
			break;

		case STATE_DISCONNECTED:
			if(!(app.options & BLE_CONNECT_BEEP))
				return;

			Sound(TVLONG, VSYSTEM, SLOW, 0);
			GoodReadLed(BLUE_FLASH, 0);
			break;

		case STATE_DISCONNECTED_USB:
			if(!(app.options & USB_CONNECT_BEEP))
				return;

			Sound(TVLONG, VSYSTEM, SLOW, 0);
			GoodReadLed(BLUE_FLASH, 0);
			break;

		case STATE_MANUAL_DISCONNECT:
			Sound_App(TVLONG, VSYSTEM, SHIGH, SLOW, 0);
			GoodReadLed(BLUE_FLASH, 0);
			break;

		case STATE_RECONNECTABLE:
			GoodReadLed(BLUE_FLASH_SLOW, FOREVER);
			// Silent	
			break;

		case STATE_CONNECTABLE:
			Sound_App(TSTANDARD, VSYSTEM, SHIGH, SMEDIUM, 0);
			Delay(TSTANDARD * 3);		// Wait till buzzer Sound if finished before calling ComOpen, so it doesn't interfere with a disconnect beep if that feels
			GoodReadLed(BLUE_FLASH, FOREVER);
			break;
	}
}

//------------------------------------------------------------------
//  BluetoothTimers_Heartbeat
//  ============================
//  Heartbeat routine that is executed by the OS every 20ms, causing
//  the seperate Bluetooth timers to be decremented (if enabled)
//------------------------------------------------------------------
int BluetoothTimers_Heartbeat( void )
{
	int ret = DISABLED;

	for( int x = 0; x < MAX_TIMERS; x++ )
	{
		if( timer[x] != 0 && timer[x] != TIMER_DISABLED )
		{
			if(ret == DISABLED)
				ret = FALSE;

			if(--timer[x] == 0)
				ret = TRUE;
		}
	}

	return ret; // ret is used to indicate that the application needs to be serviced, because one or more timers ran out
}

//------------------------------------------------------------------
//------------------------------------------------------------------
//  AutoDisconnectTimePassed
//  ============================
//  Function that checks if the auto disconnect time has passed
//------------------------------------------------------------------
int AutoDisconnectTimePassed(struct time *dt)
{
	struct time now = {0};

	if(app.auto_disconnect_time == 0)		// If the auto disconnect time is disabled it can't pass either
		return FALSE;

	if(TriggerPressed())					// If trigger is pressed, the auto disconnect time can't pass because the scanner is clearly not idle
		return FALSE;

	GetTime(&now);							// Get current time, then compare the current time with the auto disconnect time

	if(dt->ti_hour + 12 < now.ti_hour)		// Check if the disconnect time is on the next day, causing the disconnect hour to be much lower
		return FALSE;
	
	if(now.ti_hour + 12 < dt->ti_hour)		// Check if the current day has passed, causing the current hour to be much lower
		now.ti_hour += 24;
		
	if(now.ti_hour < dt->ti_hour)			// Time not passed yet
		return FALSE;

	if(now.ti_hour == dt->ti_hour)
	{
		if(now.ti_min < dt->ti_min)
			return FALSE;					// Time not passed yet
	
		if(now.ti_min == dt->ti_min)
		{
			if(now.ti_sec < dt->ti_sec)
				return FALSE;				// Time not passed yet
		}
	}

	return TRUE;							// Time passed
}

//------------------------------------------------------------------
//  ResetAutoDisconnectTime
//  ============================
//  Function that disables the auto disconnect time out
//------------------------------------------------------------------
void ResetAutoDisconnectTime(struct time *dt)
{
	if(app.auto_disconnect_time == 0)
		return;

	GetTime( dt );					// Get current time

	dt->ti_min += (app.auto_disconnect_time % 60);		

	if(dt->ti_min >= 60)				// Calculate the time at which the auto-disconnect time passes
	{
		dt->ti_min -= 60;
		++dt->ti_hour;
	}

	dt->ti_hour += (app.auto_disconnect_time / 60);

	if(dt->ti_hour >= 24)
		dt->ti_hour -= 24;

	SetAlarm(dt, ON);				// Configure the Alarm (in case we've fallen asleep)
}

//------------------------------------------------------------------
//  StopAutoDisconnectTime
//  ============================
//  Function that halts the auto disconnect time out
//------------------------------------------------------------------
void StopAutoDisconnectTime(void)
{
	struct time now = {0};
	SetAlarm(&now, OFF);				// Disable the alarm
}

//  ProcessBarcode
//  =====================
//  Processes a barcode that has been read. This function
//  makes is possible to read set-up labels from the universal
//  book as well. (Changed settings will be lost after a restart)
//------------------------------------------------------------------
int ProcessBarcode(struct barcode *code)
{
	int result = 0;

	if( code->id == MENU_CODE || code->id == MENU_CODE_PDF || code->id == MENU_CODE_C128
		|| code->id == MENU_CODE_QR || code->id == MENU_CODE_AZTEC )	// If a menu label -> check for Bluetooth address, pin and other configuration labels
	{
		if(strncmp(code->text, "+-DISC-+", 8) == 0)
			return DISCONNECT_LABEL;

		if(strncmp(code->text, "+-CONN-+", 8) == 0)
			return CONNECT_LABEL;

		if(strncmp(code->text, "+-DSCO-+", 8) == 0 && !BltIsConnected_App())
			return CONNECTABLE_LABEL;
			
		if(strncmp(code->text, "+-UNPR-+", 8) == 0)
			return UNPAIR_LABEL;

		if(strncmp(code->text, "+-MCLR-+", 8) == 0)
			return MEMORIZE_CLEAR_LABEL;

		if(strncmp(code->text, "+-MSTR-+", 8) == 0)
			return MEMORIZE_START_LABEL;

		if(strncmp(code->text, "+-MSTP-+", 8) == 0)
			return MEMORIZE_STOP_LABEL;

		if(strncmp(code->text, "+-MXMT-+", 8) == 0)
			return MEMORIZE_XMIT_LABEL_CLR;

		if(strncmp(code->text, "+-MXTO-+", 8) == 0 || strncmp(code->text, "+-MDLD-+", 8) == 0)
			return MEMORIZE_XMIT_LABEL;

		if( (strncmp(code->text, "+-IOSON-+", 9) == 0)  || (strncmp(code->text, "+-IOSOFF-+", 10) == 0) )
			return CONFIG_IPHONE_LABEL;

		if( (strncmp(code->text, "+-BTLNS-+", 8)) == 0  || (strncmp(code->text, "+-BTLNA-+", 8) == 0) )
			return CONFIG_BT_LOCAL_NAME;
	}

	if(code->id == MENU_CODE)		// Check for standard Code-39 menu labels (from the OSE Universal menu book)
	{
		while(result != EXITING_MENU_MODE && result != ERROR)
		{
			if(result == 0 || ReadBarcode(code) == OK)
			{
				switch( (result=ExecuteMenuLabel(code)) )
				{
					case ENTERING_MENU_MODE:
						GoodMenuBeep(code->id);
						break;

					case EXITING_MENU_MODE:
						GoodMenuBeep(code->id);
						Delay(TSTANDARD * 4);		// Wait till buzzer Sound if finished before saving (saving flash settings turns off Sound timer temporarily)
						SystemSetting("Z2");		// Save changed settings in flash memory
						break;

					case INVALID_OPTION_READ:
						BadMenuBeep();
						break;

					case VALID_OPTION_READ:
						GoodMenuBeep(code->id);
						break;

					case LABEL_IGNORED:
						break;

					case ERROR:
						break;
				}
			}
		}

		return MENU_LABEL;
	}
	else if(code->id == MENU_CODE_PDF || code->id == MENU_CODE_C128 || code->id == MENU_CODE_QR || code->id == MENU_CODE_AZTEC)		// Check for Code-128 / PDF417 / QR / Aztec menu labels
	{
		switch( (result=ExecuteMenuLabel(code)) )
		{
			case INVALID_OPTION_READ:
				BadMenuBeep();
				break;

			case VALID_OPTION_READ:
				GoodMenuBeep(code->id);
				SystemSetting("Z2");		// Save changed settings in flash memory
				break;

			case LABEL_IGNORED:
			default:
				break;
		}

		return MENU_LABEL;
	}

	return NORMAL_BARCODE;
}

void ProcessMenuLabels(int res)
{
	int ble_port;
							
	switch(res)
	{
		case MENU_LABEL:						// If Menu label(s) read and executed -> reset trigger to connect/disconnect timers
			ClearKeyAsTrigger(app.del_enable); // Just to be sure, it may be changed
			timer[MAKE_CONNECTABLE] = TIMER_DISABLED;	// because they might time out when leaving menu mode with trigger pressed)
			timer[DISCONNECT] = TIMER_DISABLED;
			break;

		case MEMORIZE_XMIT_LABEL_CLR:
		case MEMORIZE_XMIT_LABEL:

			if( !BarcodesInMemory() )
			{
				GoodReadLed(ORANGE, 10);
				Sound_App(TSTANDARD, VSYSTEM, 800, SPAUSE, 800, 0);	//file empty beep
				break;
			}

			xmit_state = TSTATE_XMIT_STORED_DATA;
			xmit_state_clr = (res == MEMORIZE_XMIT_LABEL_CLR) ? TRUE : FALSE;
			
			if(IsConnected_App())
			{
				GoodReadLed( GREEN, 10 );
				break;
			}

			// Fall through !!!

		case CONNECTABLE_LABEL:
		case CONNECT_LABEL:
			if( IsConnected_App() )						// If already connected
			{
				Sound_App(TSTANDARD, VSYSTEM, SLOW, 0);
				break;
			}

			ConnectionIndicator(STATE_CONNECTABLE);
			UpdateAutoPowerDown(STATE_CONNECTABLE);		// Update AutoPowerDown, so the device doesn't fall asleep while connecting

			if( app.blt_interface & BLT_BLE_DEVICE )
				ble_port = (res == CONNECTABLE_LABEL) ? COM14 : COM15;
			else // if( app.blt_interface == BLT_HID_DEVICE )
				ble_port = (res == CONNECTABLE_LABEL) ? COM12 : COM13;

			if(ComOpen(ble_port) == OK)		// Make connectabe
			{
				com_opened = ble_port;
				connection_state = STATE_CONNECTABLE;
				timer[CONNECTABLE_TIME_OUT] = GetBltConnectableTime() * 60 * 50;
			}
			else
			{
				com_opened = -1;
				ComClose(COM14);
				ConnectionIndicator(STATE_DISCONNECTED);
				connection_state = STATE_WAIT_FOR_RELEASE;
			}
			break;

		case UNPAIR_LABEL:

			BltUnpair();	// This also closes the comport

			Sound_App(TSTANDARD, VSYSTEM, SHIGH, SMEDIUM, SHIGH,0);

			GoodReadLed( GREEN, 10 );
			Delay(75);

			ConnectionIndicator(STATE_CONNECTABLE);
			UpdateAutoPowerDown(STATE_CONNECTABLE);		// Update AutoPowerDown, so the device doesn't fall asleep while connecting

			ble_port = (app.blt_interface & BLT_BLE_DEVICE) ? COM14 : COM12;

			if(ComOpen(ble_port) == OK)		// Make connectabe
			{
				com_opened = ble_port;
				connection_state = STATE_CONNECTABLE;
				timer[CONNECTABLE_TIME_OUT] = GetBltConnectableTime() * 60 * 50;
			}
			else
			{
				com_opened = -1;
				ComClose(COM14);
				ConnectionIndicator(STATE_DISCONNECTED);
				connection_state = STATE_WAIT_FOR_RELEASE;
			}
			break;

		case DISCONNECT_LABEL:					// Reading the disconnect label ends the 'connection lost' state

			connection_lost = FALSE;		
			
			if(connection_state == STATE_DISCONNECTED)
			{
				Sound_App(TSTANDARD, VSYSTEM, SLOW, 0);
				GoodReadLed( ORANGE, 10 );
			}
			else
			{
				timer[DISCONNECT] = TIMER_DISABLED;

				ConnectionIndicator(STATE_MANUAL_DISCONNECT);

				ComClose(COM16);			// Any Bluetooth port will do (com_opened may return the USB port) 
				com_opened = -1;
			}

			connection_state = STATE_WAIT_FOR_RELEASE;
			break;

		case MEMORIZE_CLEAR_LABEL:
			if( BarcodesInMemory() )
			{
				Sound_App(TSTANDARD, VSYSTEM, 3000, 2500, 0);	
				GoodReadLed( GREEN, 10 );
			}
			else
			{
				Sound_App(TLONG, VSYSTEM, SLOW, 0);	
				GoodReadLed( ORANGE, 10 );
			}
			DeleteStorage();		// Always call it, just to be sure
			break;

		case MEMORIZE_START_LABEL:
		{
			unsigned char save_manual_memorizing = app.manual_memorizing;

			if(app.manual_memorizing == OFF)									// If memorizing was manually stopped, then end the manual overruling of the automatic memorizing options
				app.manual_memorizing = STANDBY;

			if(IsMemorizingEnabled(connection_lost) == FALSE || app.manual_memorizing != ON)		// If memorizing is now disabled, turn manual memorizing on
				app.manual_memorizing = ON;

			if(app.manual_memorizing != save_manual_memorizing)					// Store the changes in flash, because unlike the OPN2002, the OPN2005 really powers off
			{
				SystemSetting("Z2");
			}

			xmit_state = TSTATE_NO_DATA;
			Sound_App(TSTANDARD, VSYSTEM, 3000, 2500, 0);	
			GoodReadLed(GREEN, 10);
			break;
		}
		case MEMORIZE_STOP_LABEL:
		{
			unsigned char save_manual_memorizing = app.manual_memorizing;
			
			if(app.manual_memorizing == ON)				// If memorizing was manually started, then end the manual overruling of the automatic memorizing options
				app.manual_memorizing = STANDBY;

			if(IsMemorizingEnabled(connection_lost) == TRUE)			// If memorizing is currently enabled, turn manual memorizing off
				app.manual_memorizing = OFF;
			
			if(app.manual_memorizing != save_manual_memorizing)					// Store the changes in flash, because unlike the OPN2002, the OPN2005 really powers off
			{
				SystemSetting("Z2");
			}
			
			if(IsConnected_App() && BarcodesInMemory())
			{
				xmit_state = TSTATE_XMIT_STORED_DATA;
				xmit_state_clr = TRUE;
			}

			Sound_App(TSTANDARD, VSYSTEM, 3000, 2500, 0);	
			GoodReadLed(GREEN, 10);
			break;
		}
		
		case CONFIG_IPHONE_LABEL:			// +-IOSON-+  , +-IOSOFF-+
			GoodReadLed( GREEN, 10 );
			
			if( strncmp( &code.text[5], "ON" , 2 ) == 0 )
			{
				app.options |= ENABLE_KEYBOARD_TOGGLE | ENABLE_WAKEUP_KEY;
				GoodMenuBeep(MENU_CODE_C128);
			}
			else
			{
				app.options &= ~(ENABLE_KEYBOARD_TOGGLE | ENABLE_WAKEUP_KEY);
				Sound_App(TSTANDARD, VSYSTEM, SHIGH, SMEDIUM, SHIGH, SMEDIUM, SHIGH, SMEDIUM, 0);
			}

			Delay(TSTANDARD * 7);		// Wait till buzzer Sound if finished before saving (saving flash settings turns off Sound timer temporarily)
			SystemSetting("Z2");		// Save changed settings in flash memory
			break;
	}
}

//------------------------------------------------------------------------------
//	ShutdownCallback
//  =====================
// 	Closes the database file before shutting down
//------------------------------------------------------------------------------
int ShutdownCallback(void)
{
	CloseStorage();
	return OK;
}


//------------------------------------------------------------------------------
//	DoGoodRead
//  =====================
///------------------------------------------------------------------------------
void DoGoodRead( int quantity, uint8_t memorizing )
{
	if(quantity >= 0)
	{
		if(!memorizing)
		{
			if(BltIsConnected_App() || (UsbIsConnected_App() && app.usb_mode != USB_MODE_NONE))
			{
				ShowGoodReadLed(GOOD_READ_COLOR);
				OkBeep_App();
			}
			else
			{
				ErrorIndicator(ORANGE);
			}
		}
		else
		{
			ShowGoodReadLed(GOOD_READ_COLOR);
			MemorizeBeep_App();
		}
	}
	else
	{
		ShowGoodReadLed(ORANGE);
		RemoveBeep();
	}
}


//------------------------------------------------------------------
//  main
//  =====================
//  Main function of this application
//------------------------------------------------------------------
#ifdef ALLINONE_APP
void blt_main(void)
#else
void app_main(void)
#endif
{
	int res, ch;
	int x;

	// Initialize the barcode structure
	InitBarcodeBuf(0, MAX_BARCODE_LENGTH);

	reset_read_mode = FALSE;

	//
	// The following function implements SystemSetting menu options that aren't present in the OS or need to be overruled
	//
	AddMenuOptionTable(blt_menu_option_table, BLUETOOTH_MAX_OPTIONS);

	//
	// Initialize default read mode (Single read, 0 seconds). If the read mode is changed by the user, it is automatically overruled on calling of 'SystemSettingsMemorizing'
	//
	//ScannerPower(SCAN_SINGLE | SCAN_TRIGGER, 100); 


	//
	// Enable remembering of (decoder) settings in flash and load old settings if present
	//
	if(SystemSettingsMemorizing(ON) < 0 || set_to_known_default == FALSE)	// Enable remembering of (decoder) settings in flash and load old settings if present
	{
		Bluetooth_HID_Default();			// If no old settings were found -> Reset OS and application to default
		SystemSetting("Z2");				// Save changed settings in flash memory
	}

	//
	// Enable remembering of application settings in flash and load old settings if present and compatible
	//
	if( ApplicationSettingsMemorizing(ON, (void *)&app, sizeof(app), VERSION_CHECK) < 0 )
	{
        // If no valid application settings were found -> Application settings are 
		// already set to default by SystemSettingsMemorizing
		SystemSetting("Z2");				// Save changed settings in flash memory
	}

	ClearKeyAsTrigger(app.del_enable);


	//
	// Disable all timers
	//
	for(x=0; x<MAX_TIMERS; ++x)
		timer[x] = TIMER_DISABLED;

	//
	// Disable auto disconnect time
	//
	StopAutoDisconnectTime();

	//
	// Install our own heartbeat timer that beats every 20ms
	//
	InstallHeartbeatHandler( BluetoothTimers_Heartbeat );
	
#ifndef HAS_2D_ENGINE
	//
	// We're controlling the read mode in the app unfortunately; set the last settings from the app vars
	//
	SetReadMode(app.read_mode, app.read_time);
#endif

	if (IsColdBoot() && (app.buzzer_options & STARTUP_BUZZER))
    {
        Sound(TSTANDARD, VHIGH, SLOW, SMEDIUM, SHIGH, SLOW, SMEDIUM, SHIGH, 0);	// Start-up beep
    }

	if(app.usb_mode == 0 && !ClearKeyPressed()) // Don't become connectable with clear key pressed, because people want to make it manually reconnectable
	{
		connection_state = STATE_RECONNECTABLE;
		connection_lost = FALSE;

		if(app.blt_interface & BLT_BLE_DEVICE)
			com_opened = COM15;
		else if(app.blt_interface & BLT_HID_DEVICE)
			com_opened = COM13;

		UpdateAutoPowerDown(STATE_RECONNECTABLE);	// Don't power off while waiting for a connection
				
		if(ComOpen(com_opened) == OK)		// Make device connectable
		{				
			timer[CONNECTABLE_TIME_OUT] = GetBltConnectableTime() * 60 * 50;
			ConnectionIndicator(STATE_RECONNECTABLE);
		}
		else 								// If Bluetooth initialization failed -> abort
		{
			ComClose(com_opened);
			com_opened = -1;
			connection_state = STATE_WAIT_FOR_RELEASE;
		}
	}
	else
	{
		connection_state = STATE_DISCONNECTED;
		UpdateAutoPowerDown(STATE_DISCONNECTED);
	}

	InitStorage(QNT_OPT_ALLOW_DUPLICATES);	// Default (Allow duplicaties, don't allow negative quantities, don't transmit zero quantity)

	//
	// After a restart, reopen the barcode database if memorizing is enabled or there are barcodes left in memory
	//
	if( IsMemorizingEnabled(FALSE) || BarcodesInMemory() )
	{
		OpenStorage();
	}

    // Set shutdown callback to close the barcode database
	Application_ShutdownCallback(ShutdownCallback);

    com_opened = ComOpen(-1);

   // bool once = true;
	for(;;)
	{
		
#if NRF_CLI_ENABLED
        NRF_CLI_PROCESS();
#endif

        NRF_LOG_PROCESS();

		int barcode_read = FALSE;

		//
		// Barcode reading and handling
		//
		if(ReadBarcode(&code) == OK)				// If barcode read
		{
			code.quantity = (last_key_pressed == CLR_KEY) ? -1 : 1;

			res = ProcessBarcode(&code);			// Check if it's a special barcode that requires action
			
#ifdef LOG_STATE
			bc_scan_state = res;
#endif
			reset_read_mode = TRUE;
	
			if(res != NORMAL_BARCODE)
			{
				ProcessMenuLabels(res);
			}
			else
			{
				barcode_read = TRUE;
			}
		}

#ifdef LOG_STATE
		LogConnectionState();
#endif
		//
		// Transmission state machine
		//
		TransmitStateMachine(barcode_read);

		if ((ch = KeyOut()) > 0)
			last_key_pressed = ch;// Used to determine whether a barcode was read using the clear key or trigger key
		
#if !defined(HAS_2D_ENGINE) && !defined(HAS_NO_SCANNER)
		//
		// Legacy addition due to an old bug in the OPN2005. Most like not need anymore
		//
		if (TriggerPressed() && !IsScannerOff())
			SetReadMode(app.read_mode, app.read_time);
			
		if ( reset_read_mode && !TriggerPressed() )
        {
            reset_read_mode = FALSE;
            SetReadMode(app.read_mode, app.read_time);
        }
#endif

#ifdef HAS_BATTERY
		//
		// Check if battery drops below critical threshold
		//
		CheckForLowBattery();
#endif	
		//
		// Update Bluetooth connection state machine
		//
		BluetoothStateMachine(barcode_read);

		LocateHostStateMachine();

#ifdef LOG_STATE
		LogConnectionState();
#endif

        if(ComOpen(-1) != com_opened)
        {
            com_opened = ComOpen(-1);
        }

		if(com_opened == COM8 || com_opened == COM7)
			SetDTR(ON);	// Do this here and not only once in the beginning to work around an issue in OS

		if(com_opened == -1)		// Makes sure that barcodes can always can be transmitted by USB or Cradle if there's no Bluetooth connection
		{
			switch(app.usb_mode)
			{
				case USB_MODE_CDC:		com_opened = COM8; break;
				case USB_MODE_CDC_OPC:	com_opened = COM7; break;
				case USB_MODE_HID:		com_opened = COM10; break;
				case USB_MODE_VCP:	
				default:				com_opened = COM9; break;

			}

			ComOpen( com_opened );	
		}

		Idle();		// This reduces the power consumption
	}
}

#define LOCATE_HOST_STATE_IDLE			0
#define LOCATE_HOST_STATE_SEQ_STARTED	1
#define LOCATE_HOST_STATE_RELEASE_1		2
#define LOCATE_HOST_STATE_PRESS_2		3
#define LOCATE_HOST_STATE_RELEASE_2		4
#define LOCATE_HOST_STATE_PRESS_3		5
#define LOCATE_HOST_STATE_RELEASE_3		6
#define LOCATE_HOST_STATE_ACTIVE		7

void LocateHostStateMachine(void)
{
	static int locate_host_state = LOCATE_HOST_STATE_IDLE;

	static int TemperatureScale[100] = { 0x0000FF, 0x0200FC, 0x0500F9, 0x0700F7, 0x0A00F4, 0x0C00F2, 0x0F00EF, 0x1200EC, 0x1400EA, 0x1700E7, 0x1900E5, 0x1C00E2, 0x1E00E0, 0x2100DD, 0x2400DA, 0x2600D8, 0x2900D5, 0x2B00D3, 0x2E00D0, 0x3000CE, 0x3300CB, 0x3600C8, 0x3800C6, 0x3B00C3, 0x3D00C1, 0x4000BE, 0x4200BC, 0x4500B9, 0x4800B6, 0x4A00B4, 0x4D00B1, 0x4F00AF, 0x5200AC, 0x5500AA, 0x5700A7, 0x5A00A4, 0x5C00A2, 0x5F009F, 0x61009D, 0x64009A, 0x670097, 0x690095, 0x6C0092, 0x6E0090, 0x71008D, 0x73008B, 0x760088, 0x790085, 0x7B0083, 0x7E0080, 0x80007E, 0x83007B, 0x850079, 0x880076, 0x8B0073, 0x8D0071, 0x90006E, 0x92006C, 0x950069, 0x970067, 0x9A0064, 0x9D0061, 0x9F005F, 0xA2005C, 0xA4005A, 0xA70057, 0xAA0055, 0xAC0052, 0xAF004F, 0xB1004D, 0xB4004A, 0xB60048, 0xB90045, 0xBC0042, 0xBE0040, 0xC1003D, 0xC3003B, 0xC60038, 0xC80036, 0xCB0033, 0xCE0030, 0xD0002E, 0xD3002B, 0xD50029, 0xD80026, 0xDA0024, 0xDD0021, 0xE0001E, 0xE2001C, 0xE50019, 0xE70017, 0xEA0014, 0xEC0012, 0xEF000F, 0xF2000C, 0xF4000A, 0xF70007, 0xF90005, 0xFC0002, 0xFF0000};

	if(!BltIsConnected_App())
	{
		locate_host_state = LOCATE_HOST_STATE_IDLE;
		return;
	}

	switch(locate_host_state)
	{
		case LOCATE_HOST_STATE_IDLE:
			if(ClearKeyPressed() && TriggerPressed())
			{
				locate_host_state = LOCATE_HOST_STATE_SEQ_STARTED;
				timer[LOCATE_HOST_TIMER] = LOCATE_HOST_TIMER_VALUE;
				break;
			}
			timer[LOCATE_HOST_TIMER] = TIMER_DISABLED;
			break;

		case LOCATE_HOST_STATE_SEQ_STARTED:
		case LOCATE_HOST_STATE_RELEASE_1:
		case LOCATE_HOST_STATE_RELEASE_2:
		case LOCATE_HOST_STATE_RELEASE_3:
			if(!ClearKeyPressed() || timer[LOCATE_HOST_TIMER] == 0)
			{
				locate_host_state = LOCATE_HOST_STATE_IDLE;
				timer[LOCATE_HOST_TIMER] = TIMER_DISABLED;
			}
			else if(!TriggerPressed())
			{
				++locate_host_state;
				
				if(locate_host_state == LOCATE_HOST_STATE_ACTIVE)
				{
					Sound(TSHORT, VSYSTEM, SMEDIUM, SHIGH, SMEDIUM, SHIGH, 0);
					timer[LOCATE_HOST_TIMER] = 5;
				}
				else
				{
					timer[LOCATE_HOST_TIMER] = LOCATE_HOST_TIMER_VALUE;
				}

			}
			break;

		case LOCATE_HOST_STATE_PRESS_2:
		case LOCATE_HOST_STATE_PRESS_3:
			if(!ClearKeyPressed() || timer[LOCATE_HOST_TIMER] == 0)
			{
				locate_host_state = LOCATE_HOST_STATE_IDLE;
				timer[LOCATE_HOST_TIMER] = TIMER_DISABLED;
			}
			else if(TriggerPressed())
			{
				++locate_host_state;
				timer[LOCATE_HOST_TIMER] = LOCATE_HOST_TIMER_VALUE;
			}
			break;

		case LOCATE_HOST_STATE_ACTIVE:
			int rssi = BltGetRssi();

			if( (ClearKeyPressed() && TriggerPressed()))
			{
				Sound(TSHORT, VSYSTEM, SHIGH, SLOW, 0);
				locate_host_state = LOCATE_HOST_STATE_IDLE;
				timer[LOCATE_HOST_TIMER] = TIMER_DISABLED;
			}
			else if(timer[LOCATE_HOST_TIMER] == 0)
			{				
				if(rssi != -1)
				{
					int rssi_scale = (int)(2.5*(rssi+80));

					rssi_scale = MIN(99, rssi_scale);
					rssi_scale = MAX(0, rssi_scale);

					int color = TemperatureScale[rssi_scale];

					timer[LOCATE_HOST_TIMER] = 5;

					GoodReadLed(color, 100);
				}
			}
			break;
	}
}

void BluetoothStateMachine(bool barcode_read)
{
	static int pressed = FALSE; 
	static struct time disc_time = {0};
	int res;

	if(ClearKeyPressed() && TriggerPressed())			
	{
		ShowGoodReadLed(GOOD_READ_COLOR);
	}

	//
	// Main Bluetooth connection state machine
	//
	switch(connection_state)
	{
		case STATE_DISCONNECTED:			// Idle state (not connected)

			TriggerToConnectHandler( (app.blt_connect_options & DISABLE_TRIGGER_CONNECT) == 0, barcode_read	);	// Only react to triggger when trigger switch connect is enabled

			// If USB-VCP or USB-CDC mode is enabled and UsbIsConnected -> go to connected state
			if(app.usb_mode != USB_MODE_NONE && UsbIsConnected())				// If Connection established -> go to connected state
			{
				connection_state = STATE_CONNECTED_USB;
				connection_lost = FALSE;
				ResetAutoDisconnectTime(&disc_time);
				ConnectionIndicator(STATE_CONNECTED_USB);
				UpdateAutoPowerDown(STATE_CONNECTED);		// Don't shutdown when connected, otherwise the connection would be lost
			}
			break;			

		case STATE_CONNECTABLE:
		case STATE_RECONNECTABLE:

			if(!IsGoodReadActive())
			{
				GoodReadLed((connection_state == STATE_RECONNECTABLE) ? BLUE_FLASH_SLOW : BLUE_FLASH, FOREVER);
			}
			
			if(BltIsConnected() == TRUE)				// If Connection established -> go to connected state
			{
				com_opened = ComOpen(-1);
				connection_state = STATE_CONNECTED;
				pressed = FALSE; 	
				connection_lost = FALSE;					// no need to send a wake-up key when you've just connected (device is on already)
				ResetAutoDisconnectTime(&disc_time);
				ConnectionIndicator(STATE_CONNECTED);
				UpdateAutoPowerDown(STATE_CONNECTED);		// Don't shutdown when connected, otherwise the connection would be lost
				break;
			}

			if(ClearKeyPressed() && !TriggerPressed())					// Clear key can be used to abort the connection attempt
			{
				if(timer[DISCONNECT] == TIMER_DISABLED && app.trigger_to_disconnect_time != 0)
				{
					timer[DISCONNECT] = app.trigger_to_disconnect_time * 50;
				}
				else if(timer[DISCONNECT] == 0)
				{
					connection_lost = FALSE;	// Manual disconnect, so no 'connection lost'
					timer[CONNECTABLE_TIME_OUT] = 0;		// Simulates that the connection attempt timed out

					ConnectionIndicator(STATE_MANUAL_DISCONNECT);
				}
			}
			else
			{
				timer[DISCONNECT] = TIMER_DISABLED;
			}

			if(timer[CONNECTABLE_TIME_OUT] == 0 || BltIsConnected() == ERROR)			// If connection attempt has timed out -> abort
			{
				timer[DISCONNECT] = TIMER_DISABLED;
				timer[CONNECTABLE_TIME_OUT] = TIMER_DISABLED;
				
				if(connection_state == STATE_CONNECTABLE || BltIsConnected() == ERROR)
					ConnectionIndicator(STATE_DISCONNECTED);
				else 
					GoodReadLed(BLUE_FLASH, 0); // Stop flashing

				ComClose(COM16);					// Any Bluetooth port will do (com_opened may return the USB port) 
				com_opened = -1;
				connection_state = STATE_WAIT_FOR_RELEASE;
			}

			break;

		case STATE_CONNECTED:			// Connected state (Bluetooth)
			if((res=BltIsConnected()) != TRUE)		// If Bluetooth connection lost
			{
				timer[DISCONNECT] = TIMER_DISABLED;
				ConnectionIndicator(STATE_DISCONNECTED);
				StopAutoDisconnectTime();
				connection_lost = TRUE;				// Connection lost!!!! This enables memorizing is it configured to be enabled after connection lost.
				
				if(res == ERROR || ClearKeyPressed())			// If Bluetooth connection lost due to pair issue
				{
					ComClose(com_opened);
					com_opened = -1;
					connection_state = STATE_WAIT_FOR_RELEASE;
				}
				else 
				{
					connection_state = STATE_RECONNECTABLE;
					timer[CONNECTABLE_TIME_OUT] = GetBltAutoReconnectTime() * 60 * 50;
					
				}
				
				xmit_state = TSTATE_CONN_WAIT_STATE;
				UpdateAutoPowerDown(connection_state);		// Don't turn off while trying to reconnect or being connectable
				break;
			}

			// Reset auto disconnect time on every key press
			//
			if( ( TriggerPressed() || ClearKeyPressed() ) && ( pressed == FALSE ) )
			{
				ResetAutoDisconnectTime(&disc_time);
				//
				// Wake up iPhones/iPads on trigger press (if enabled)
				//
				if( app.blt_interface == BLT_HID_DEVICE )
				{
					if( (app.options & ENABLE_WAKEUP_KEY) && TriggerPressed() )
					{
						PutnString("", 0);
					}
					if( (app.options & ENABLE_KEYBOARD_TOGGLE) && ClearKeyPressed() )
					{
						PutCom( 0xA6 );  // toggle iPhone on screen keyboard.
					}
				}
			}
			
			if( pressed != TRIGGER_KEY && TriggerPressed() )
			{
				pressed = TRIGGER_KEY;
			}
			else 
			if ( pressed != CLR_KEY && ClearKeyPressed() )
			{
				pressed = CLR_KEY;
			}
			else if(!TriggerPressed() && !ClearKeyPressed())
			{
				pressed = FALSE;
			}

			//
			// Clear key can be used to disconnect
			// as well as using an auto disconnect time
			//
			int time_passed = FALSE;
			if((ClearKeyPressed() && !TriggerPressed()) || ( time_passed = AutoDisconnectTimePassed(&disc_time) )== TRUE)			// Clear key can be used to manually disconnect
			{
				if(time_passed == TRUE)
				{
					timer[DISCONNECT] = 0;		// Auto disconnect time passed -> Disconnect now!
				}
				else if(timer[DISCONNECT] == TIMER_DISABLED && app.trigger_to_disconnect_time != 0)
				{
					timer[DISCONNECT] = app.trigger_to_disconnect_time * 50;
				}
				
				if(timer[DISCONNECT] == 0)
				{
					timer[DISCONNECT] = TIMER_DISABLED;
					ConnectionIndicator(STATE_MANUAL_DISCONNECT);
					StopAutoDisconnectTime();
					ComClose(com_opened);
					com_opened = -1;
					connection_state = STATE_WAIT_FOR_RELEASE;
					connection_lost = FALSE;
				}
			}
			else
			{
				timer[DISCONNECT] = TIMER_DISABLED;
			}

			break;

		case STATE_CONNECTED_USB:			// Connected state (USB)
		{
			if(!UsbIsConnected())		// If USB connection closed
			{
				ConnectionIndicator(STATE_DISCONNECTED_USB);

				UpdateAutoPowerDown(STATE_DISCONNECTED);	
				connection_lost = TRUE;						// Connection lost!!!! This enables memorizing is it configured to be enabled after connection lost.
				connection_state = STATE_DISCONNECTED;
				xmit_state = TSTATE_CONN_WAIT_STATE;
			}

			TriggerToConnectHandler(FALSE, FALSE);
			break;
		}

		case STATE_WAIT_FOR_RELEASE:		// Waits here until all keys are released before returning to idle state
		default:
			if(!TriggerPressed() && !ClearKeyPressed())
			{
				connection_state = STATE_DISCONNECTED;
				UpdateAutoPowerDown(STATE_DISCONNECTED);			// Update AutoPowerDown, so the device can fall asleep and power down again
				StopAutoDisconnectTime();
			}
			break;
	}

	//
	// Check if data is received, if so, reset auto disconnect time
	//
	if(GetCom(0) >= 0 && BltIsConnected_App())
	{
		ResetAutoDisconnectTime(&disc_time);
	}
}

void TransmitStateMachine(uint8_t new_barcode)
{
	static uint8_t process_barcode_data = FALSE;

	int res;

	process_barcode_data |= new_barcode;
	
	switch(xmit_state)
	{
		case TSTATE_NO_DATA:				// Idle state: No data to be transmitted

			if(process_barcode_data)
			{
				if( BarcodesInMemory() || IsMemorizingEnabled(connection_lost))
				{
					process_barcode_data = FALSE;

					ProcessBarcodeInMemory(&code);
				}
			}

			if(BarcodesInMemory())
			{
				if(!IsMemorizingEnabled( (IsConnected_App() != TRUE ) ))		// Transmit barcodes only when memorizing is disabling due the fact that the connection is restored (not when using manual memorizing)
				{
					xmit_state = TSTATE_ERROR_WAIT_STATE;		// Wait 1s before trying to transmit memorized data
					timer[TRANSMIT_ERROR_WAIT] = TIMER_DISABLED;
				}
			}
			else if(process_barcode_data)
			{
				process_barcode_data = FALSE;
				xmit_state = TSTATE_XMIT_BARCODE;
			}
			break;

		case TSTATE_XMIT_BARCODE:			// Xmit barcode: Transmit a new barcode immediatly
		{
			xmit_state = TSTATE_NO_DATA;
			process_barcode_data = FALSE;

			if(code.quantity >= 0 || IsOptiConnectPort(com_opened))
			{
				if( app.buzzer_options & BUZZER_BEFORE_TRANSMIT ) 
				{
					DoGoodRead(code.quantity, FALSE);
				}

				res = PutBarcode(com_opened, &code);
	
				switch(res)
				{
					case OK:
						if( ( app.buzzer_options & BUZZER_BEFORE_TRANSMIT ) == 0 ) 
						{
							DoGoodRead(code.quantity, FALSE);
						}

					case DC1:						// Transmission without good read or error buzzer (possible if ACK/NAK handshaking is enabled)
						break;

					case DC2:
						ErrorIndicator(RED);		// NAK (without retransmission) or no response (but connection is still OK)
						break;

					default:
						if(connection_state == STATE_CONNECTED || connection_state == STATE_CONNECTED_USB)
							connection_lost = TRUE;

						if( IsMemorizingEnabled(connection_lost) )
						{
							ProcessBarcodeInMemory(&code);
						}
						else
						{
							ErrorIndicator(RED);
						}
						break;
				}
			}
			else	// Can't delete barcodes when not connected to OptiConnect and not memorizing
			{
				ErrorIndicator(RED);
			}
			break;
		}
		case TSTATE_XMIT_STORED_DATA:		// Xmit stored data: transmit memorized data

			if(!IsConnected_App())
			{
				xmit_state = TSTATE_CONN_WAIT_STATE;		// Wait for a connection to be re-established
				timer[TRANSMIT_ERROR_WAIT] = XMIT_ERROR_WAIT_TIME;
				break;
			}

			if(BarcodesInMemory())
			{
				if( ReadFirstBarcodeFromMemory(&code) == OK )
				{
					do
					{
						res = PutBarcode(com_opened, &code);
					
						switch(res)
						{
							case OK:
							case DC1:
								if (xmit_state_clr)
									DeleteCurrentBarcodeFromMemory();
								
								connection_lost = FALSE;
								break;

							default:
								GoodReadLed(ORANGE, 10);

								if (app.buzzer_options & BUZZER_ENABLED)
									Sound_App(TSTANDARD, VSYSTEM, SLOW, 0);

								connection_lost = TRUE;
								break;
						}

						//pop out if the connection was lost during transmit
						if (connection_lost)
							break;

					} while (ReadNextBarcodeFromMemory(&code) == OK);
				}

				//pop out of here if the connection was lost during transmit
				if (connection_lost)
				{
					if(xmit_state_clr)
					{
						xmit_state = TSTATE_ERROR_WAIT_STATE;		// Wait a certain time before trying to retransmit
						timer[TRANSMIT_ERROR_WAIT] = XMIT_ERROR_WAIT_TIME;
					}
					else
					{
						xmit_state = TSTATE_NO_DATA;
					}
					break;
				}

				OkBeep_App();		// RDW 2016-02016: Added OK beep after successful transmission

				//delete the barcode memory if +-MXMT-+
				//DeleteBarcodeFromMemory should have already taken care of this, but it won't hurt to do it again 
				if (xmit_state_clr)
					DeleteStorage();
			}
			else
			{
				GoodReadLed(RED_FLASH, 100);

				//file empty beep
				if (app.buzzer_options & BUZZER_ENABLED)
					Sound_App(TLONG, VSYSTEM, 800, SPAUSE, 800, 0);
			}

			xmit_state = TSTATE_NO_DATA;
			break;

		case TSTATE_ERROR_WAIT_STATE:		// If a transmission failed, but IsConnected_App() -> Wait a certain time before trying to retransmit
			
			if(IsConnected_App())
			{
				bool xmit = false;

				switch(app.options & BUTTON_TO_SEND_MASK)
				{
					case BUTTON_TO_SEND:
						xmit = TriggerPressed();
						xmit_state_clr = true;
						break;

					case SEND_AFTER_3SECONDS:
						if(timer[TRANSMIT_ERROR_WAIT] == TIMER_DISABLED)
						{
							timer[TRANSMIT_ERROR_WAIT] = XMIT_CONNECT_WAIT_TIME;
							break;
						}
						xmit = (timer[TRANSMIT_ERROR_WAIT] == 0);
						xmit_state_clr = true;
						break;

					case BUTTON_TO_SEND_ADVANCED:
						if(process_barcode_data)	// In advanced send mode, we try to sent new barcode data prior to sending the memorized data
						{
							xmit_state = TSTATE_XMIT_BARCODE;
						}
						else if(!TriggerPressed() && !ClearKeyPressed())
						{
							if(timer[TRANSMIT_ERROR_WAIT] != TIMER_DISABLED)
							{
								timer[TRANSMIT_ERROR_WAIT] = TIMER_DISABLED;
								GoodReadLed( ORANGE_FLASH, 0 );
							}
						}
						else
						{
							if(timer[TRANSMIT_ERROR_WAIT] == TIMER_DISABLED)
							{
								timer[TRANSMIT_ERROR_WAIT] = XMIT_CONNECT_WAIT_TIME;
								GoodReadLed( TriggerPressed() ? GREEN_FLASH : ORANGE_FLASH, XMIT_CONNECT_WAIT_TIME );
							}
							else  if (timer[TRANSMIT_ERROR_WAIT] == 0)
							{
								if(TriggerPressed())
								{
									xmit_state_clr = false;
									xmit = true;
									break;
								}
								// If clear key pressed and timer passed
								if( BarcodesInMemory() )
								{
									Sound(TSTANDARD, VSYSTEM, SMEDIUM, SLOW, SMEDIUM, NULL);
									GoodReadLed( ORANGE, 10 );
									DeleteStorage();		// Always call it, just to be sure
									xmit_state = TSTATE_NO_DATA;

									if(timer[DISCONNECT] != TIMER_DISABLED && timer[DISCONNECT] == 0 && app.trigger_to_disconnect_time != 0)
										timer[DISCONNECT] = app.trigger_to_disconnect_time * 50;
								}
							}
						}
						break;

						case SEND_MANUALLY:
						default:
							// don't transmit memorized data
							if(process_barcode_data)	// In advanced send mode, we try to sent new barcode data prior to sending the memorized data
							{
								xmit_state = TSTATE_XMIT_BARCODE;
							}
							break;
				}

				if(xmit)
				{
					Sound(TSTANDARD, VSYSTEM, SHIGH, NULL);
					/*if( app.blt_interface == BLT_HID_DEVICE && (app.options & ENABLE_WAKEUP_KEY) )
					{
						PutnString("", 0);							// Force a wake-up
						Delay(25);
					}*/
					xmit_state = TSTATE_XMIT_STORED_DATA;
				}
				
				if(xmit_state != TSTATE_ERROR_WAIT_STATE)	// If state changed -> don't fall through
					break;
			}
			else
			{
				xmit_state = TSTATE_CONN_WAIT_STATE;
				timer[TRANSMIT_ERROR_WAIT] = TIMER_DISABLED;
			}
			/* no break */
			// Fall Through

		case TSTATE_CONN_WAIT_STATE:		// If a transmission failed, because BltIsConnected()==FALSE -> Wait for the connection to be re-established

			if(!BarcodesInMemory())
			{
				xmit_state = TSTATE_NO_DATA;
				break;
			}

			if(xmit_state == TSTATE_CONN_WAIT_STATE && IsConnected_App())
			{
				if(IsMemorizingEnabled(FALSE) == FALSE)			// Only start transmission if the reconnection disables automatic memorizing
				{
					xmit_state = TSTATE_ERROR_WAIT_STATE;		// Wait a certain time before trying to transmit memorized data
					xmit_state_clr = TRUE;
					timer[TRANSMIT_ERROR_WAIT] = XMIT_ERROR_WAIT_TIME;
				}
				else
				{
					xmit_state = TSTATE_NO_DATA;
				}
				break;
			}

			if(process_barcode_data)
			{
				process_barcode_data = FALSE;

				if( BarcodesInMemory() || IsMemorizingEnabled(connection_lost) )
				{
					ProcessBarcodeInMemory(&code);
				}
				else
				{
					xmit_state = TSTATE_XMIT_BARCODE;
				}
			}
			break;
	}
}

int ProcessBarcodeInMemory(struct barcode *pCode)
{
	if( UpdateBarcodeInMemory(pCode) == OK )
	{
		DoGoodRead(pCode->quantity, TRUE);
		return OK;
	}
	else
	{
		ErrorIndicator(RED);
		return ERROR;
	}
}

#ifdef HAS_BATTERY
void CheckForLowBattery(void)
{
	static int max_batt_low_warnings = MAX_BATT_LOW_WARNINGS;

	//if the battery is low, warn the user
	if (IsBatteryLow() && !IsCharging())
	{
		if (timer[BATT_LOW] == TIMER_DISABLED)
		{
			AutoPowerDown(OFF, 0);						//seems funny, but won't see the flashes if we sleep
			timer[BATT_LOW] = BATT_LOW_FLASH_TIME;
		}
		else if (timer[BATT_LOW] == 0)
		{
			ShowGoodReadLed(ORANGE);
			timer[BATT_LOW] = TIMER_DISABLED;
			max_batt_low_warnings--;

			if (max_batt_low_warnings == 0)
			{
				if (connection_state != STATE_DISCONNECTED)
				{
					timer[DISCONNECT] = TIMER_DISABLED;
					ConnectionIndicator(STATE_DISCONNECTED);
					StopAutoDisconnectTime();
					ComClose(com_opened);
					com_opened = -1;
				}
				
				AutoPowerDown(APD_SHUTDOWN_NOW, 0);
			}
		}
	}
	else
	{
		timer[BATT_LOW] = TIMER_DISABLED;
		max_batt_low_warnings = MAX_BATT_LOW_WARNINGS;
	}
}
#endif

void TriggerToConnectHandler(bool trigger_enabled, bool barcode_read)
{
	if( (ClearKeyPressed() && !TriggerPressed()) || (trigger_enabled && TriggerPressed() && !ClearKeyPressed()) || ((app.blt_connect_options & AUTO_CONNECT_AFTER_DECODE) && barcode_read) )		// Check trigger key for 'Trigger to connect' (as master)
	{
		if(ClearKeyPressed() && timer[MAKE_CONNECTABLE] == TIMER_DISABLED && app.trigger_to_connect_time != 0)
		{
			timer[MAKE_CONNECTABLE] = app.trigger_to_connect_time * 50;
		}
		else if((TriggerPressed() && timer[MAKE_CONNECTABLE] == TIMER_DISABLED) || timer[MAKE_CONNECTABLE] == 0)		// If 'Trigger to make connectable' time passed
		{
			timer[MAKE_CONNECTABLE] = TIMER_DISABLED;

			connection_state = ClearKeyPressed() ? STATE_CONNECTABLE : STATE_RECONNECTABLE;

			if(app.blt_interface & BLT_BLE_DEVICE)
				com_opened = (connection_state == STATE_CONNECTABLE) ? COM14 : COM15;
			else if(app.blt_interface & BLT_HID_DEVICE)
				com_opened = (connection_state == STATE_CONNECTABLE) ? COM12 : COM13;

			ConnectionIndicator(connection_state);
			UpdateAutoPowerDown(connection_state);	// Don't power off while waiting for a connection
			
			if(ComOpen(com_opened) == OK)		// Make device connectable
			{				
				timer[CONNECTABLE_TIME_OUT] = GetBltConnectableTime() * 60 * 50;
			}
			else 								// If Bluetooth initialization failed -> abort
			{
				if(connection_state == STATE_CONNECTABLE)
					ConnectionIndicator(STATE_DISCONNECTED);

				ComClose(com_opened);
				com_opened = -1;
				connection_state = STATE_WAIT_FOR_RELEASE;
			}
		}
	}
	else
	{
		timer[MAKE_CONNECTABLE] = TIMER_DISABLED;
	}
}

#ifdef LOG_STATE
void LogConnectionState(void)
{
	if( old_connection_state != connection_state )
	{
		DEBUGPutString( "%s", str_connection_state[ connection_state ] );
		old_connection_state = connection_state;
	}
	
	if(old_bc_scan_state != bc_scan_state)
	{ 
		DEBUGPutString( "%s", str_barcode_scan[ bc_scan_state ] );
		old_bc_scan_state = bc_scan_state;
	}

	if(old_xmit_state != xmit_state)
	{
		//DEBUGPutString( "%s\r", str_tstate[ xmit_state ] );
		old_xmit_state = xmit_state;
	}
}
#endif
