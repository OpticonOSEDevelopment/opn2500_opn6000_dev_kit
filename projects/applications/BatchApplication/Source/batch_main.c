//------------------------------------------------------------------
//  Batch mode application
//------------------------------------------------------------------
//  This application allows the reading of barcodes, store them and
//  transmit them by OseComm or copy them as file in USB-MSD mode
//
//  This application also support the reading of the
//  universal menu-book, so all sorts of barcode options can easily be tested.
//
//  Since all changed configurations are stored in flash, any changed
//  configurations are restored after a restart or when the battery was empty.
//
//  See the application manual on more details on the support
//  features and on how the application works.
//------------------------------------------------------------------
//  Version history:
//
//  FFN41010 (OPN2500)  Ported from the OPN2006 / PX20
//  FFM41010 (OPN6000)  Minimal required OS version: FBxV0128
//                      Release date: September 1, 2024
//                      Author: Ronny de Winter
//                      Company: Opticon Sensors Europe BV
//
//------------------------------------------------------------------
// Includes
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <Ctype.h>
#include "lib.h"
#include "ff.h"
#include "Debug.h"
#include "batch_main.h"
#include "BarcodeDBase.h"
#include "memorize.h"
#include "BarcodeBuf.h"

static batch_appl_vars app; 			// Structure containing all application settings that will be stored in flash memory.
static uint8_t app_settings_restored = FALSE;

#include "menu_options.inc"		// Contains table with all application specific menu options

// barcode record structure
typedef struct
{
	char *barcode;
	int barcode_len;
	char quantity[SZ_QUANTITY + 1];
	char serial[SZ_SERIAL + 1];
	char time[SZ_TIME + 1];
	char date[SZ_DATE + 1];
} db_record;

// Globals
//char g_Record[MAX_RECORD_SIZE];

// Locals

static FIL SDBExportFile;
static SDBOutVal dbExportStorage = { NULL };

// Timer related stuff
#define TIMER_DISABLED	  ((unsigned int)-1)
#define MAX_TIMERS					2
#define FORMAT_TIMER				0			// Timer tracking clear key being held to erase stored data
#define RESET_TIMER					1			// Timer tracking

#ifdef HAS_2D_ENGINE
#define IDLE_POWER_DOWN_TIME		   2*50			// Powerdown time when idle: 2 seconds
#define IDLE_POWER_OFF_TIME			   10*60*50		// Shutdown time when idle: 10 minutes
#else
#define IDLE_POWER_DOWN_TIME		   2*50			// Sleep time when idle: 2 second
#define IDLE_POWER_OFF_TIME			   4*50			// Powerdown time when idle: 4 seconds
#endif

#define CLR_KEY_FORMAT_TIME			12*50		// Time clear key must be held to erase stored data
#define RESET_TIME					12*50		// Time both keys must be held to perform a manual reset

static volatile unsigned int timer[MAX_TIMERS];		// Array of various timers
static int Aiming_Enabled = FALSE;
static int reset_read_mode;

// When SystemSettingsMemorizing(ON) is called, this will result in 'Application_Default' being called, 
// but only if the menu-option default is known by the application, so i.e. "U2" won't trigger this function
// in this cause we have to perform a default in main() by checking this variable
static bool set_to_known_default = FALSE;


//
// States of the main state machine of this application
//
#define STATE_IDLE					0			// Idle state
#define STATE_USB_CONNECTED			2			// Connected to USB
#define STATE_WAIT_FOR_RELEASE		3			// Wait until both keys are not pressed and then jump back idle state
//
// Public & local functions
//
static void Application_Default(int comport);
//static int GetBarcodeFieldNr(void);
static int GetQuantityFieldNr(void);
static int ShutdownCallback(void);
static int ExportDatabase(void);
static int ProcessBarcode(struct barcode *code);
//static int ImportDatabase(void);
static int FieldSize(int field_type);
#ifndef HAS_2D_ENGINE
static void SetReadMode(int readmode, int readtime);
#endif


//
// Note: Only change this version if the application settings of the previous version are no longer compatible
//
#define	VERSION_CHECK				"BATCHDEMO_V13"	// Verification string used by 'LoadApplicationSettings' to check if any possible
													// application settings found in flash memory are compatible with this application.
													// If this version check fails all application settings will be set back to default
			
static int last_key_pressed = -1; 	// Used to determine whether a barcode was read using the clear key or trigger key.

//------------------------------------------------------------------
//  Application_Default
//  ============================
//  Reset all application specific settings to the specified default
//------------------------------------------------------------------
void Application_Default(int comport)
{
	set_to_known_default = TRUE;		// When SystemSettingsMemorizing(ON) is called, this will result in 'Application_Default' being called, 
										// but only if the menu-option of the default is known by the application, so i.e. "U2" won't trigger this function
										// in this cause we have to perform a default in main()

	switch(comport)
	{
		case COM11:
			SystemSetting("C04");			// Call USB-MSD default function
			app.default_interface = COM11;
			break;

		default:
			SystemSetting("OPND");			// Call OseComm default function
			app.default_interface = COM9;
			break;
	}

	SystemSetting("YC");											// Enable menu-labels
	SystemSetting("7G");											// Enable charge indicator
	SystemSetting("S0");											// Single read
	SystemSetting("Y0");											// Trigger mode
	SystemSetting("TS");											// Serial config disabled
	SystemSetting("BPV");											// Set interface independent default suffix: <CR> (VCP) / Enter key (HID)
	
	app.special_options = OSECOMM_COMMUNICATE;						// Use OseComm by default

#ifndef HAS_2D_ENGINE
	app.read_mode = SCAN_SINGLE;											// Single read
    app.read_time = 0;												// 0 seconds (trigger)
#endif

	app.func_key_del = FALSE;										// Disable ability to format flash disk by holding function key for CLR_KEY_FORMAT_TIME

	app.output.field_separator = ',';								// Output field separator
	app.output.date_time_format = TIME_FORMAT_HH_MM_SS;				// Output time format hh:mm:ss
	app.output.date_time_format |= DATE_FORMAT_SLASH_DD_MM_YYYY;	// Output data dd/mm/yyyy
	app.output.barcode_size = 0x00;									// Output barcode field size (variable length fields)
	app.output.quantity_size = 0x00;								// Output quantity field size (0=empty field)
	app.output.quantity_options = QUANTITY_DELETE_ZERO_QNTY;		// Don't allow negative quantity, delete key enabled, delete record is quantity reaches zero

	app.output.sequence[0] = SEQUENCE_BARCODE;						// Output sequence: Barcode, Serial number, Time, Date
	//app.output.sequence[1] = SEQUENCE_QUANTITY;
	app.output.sequence[1] = SEQUENCE_SERIAL_NUMBER;
	app.output.sequence[2] = SEQUENCE_TIME;
	app.output.sequence[3] = SEQUENCE_DATE;

	ReinitBarcodeStorage();	// Configure storage with application specific database options
}

#ifndef HAS_2D_ENGINE
//------------------------------------------------------------------
//  SetReadMode
//  ============================
//  Converts the configured read time/mode to a scannerpower call
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

//------------------------------------------------------------------------------
// Name:        ClearBarcodes
// Description: Erases all barcodes in memory
// Args:
// Returns:
//------------------------------------------------------------------------------
void ClearBarcodes(bool reopen)
{
	DeleteStorage();

	if(reopen)
		OpenStorage(); // Batch_OpenStorage();
}

//------------------------------------------------------------------
//  OnCommInfo
//  ============================
//  Callback function that gets the status, error and progress
//  information from 'OseComm' during the up- and dowloading of files.
//  Visual process information is shown by this callback function.
//------------------------------------------------------------------
void OnCommInfo(int status, int errorsuccess, int progress, const char *info)
{
	switch (status)
	{
		case STAT_LIST_FILES:
			break;
			
		case STAT_XMIT_FILE_TO_PC:
			if (errorsuccess == SUCC_COMPLETE)
			{
				Sound(TSTANDARD, VHIGH, SLOW, SMEDIUM, SHIGH, SLOW, SMEDIUM, SHIGH, 0);		// Ready beep
				ClearBarcodes(false);
			}
			break;
			
		case STAT_RECV_FILE_FROM_PC:
		case STAT_GET_TIME_DATE:
		case STAT_SET_TIME_DATE:
		case STAT_GET_OS_VERSION:
		case STAT_GET_APPL_VERSION:
			break;
	}
}

//------------------------------------------------------------------
//  Communicate
//  ============================
//
//------------------------------------------------------------------
void Communicate(void)
{
	if (app.special_options & OSECOMM_COMMUNICATE)
		OseComm(GetSerialNumber(), CONNECTION_ABORT, GetApplVersion(), OnCommInfo);
}

//------------------------------------------------------------------
//  Timers_Heartbeat
//  ============================
//  Heartbeat routine that is executed by the OS every 20ms, causing
//  the seperate timers to be decremented (if enabled)
//------------------------------------------------------------------
int Timers_Heartbeat( void )
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
//  ExportBarcodeData
//  ============================
//  Saves the barcode data into the database file
//------------------------------------------------------------------
int ExportBarcodeData(db_record *db_rec)
{
	char *ptr;
	int i;
	//int barcode_field;
	int field_size;
	int result = OK;
	
	if(CoreLeft() < 10000)	// During export we keep a safe margin to prevent a full disk
		return ERROR_FILESYSTEM;
	
	// Start building the output record
	ptr = g_Record;

	// Add remaining fields as configured
	for (i = 0; i < SEQUENCE_MAX; ++i)
	{
		field_size = FieldSize(app.output.sequence[i]);
		
		if ((app.output.sequence[i] == SEQUENCE_BARCODE) && (app.output.barcode_size == 0x00))
		{
			field_size = db_rec->barcode_len;
		}
		else if ((app.output.sequence[i] == SEQUENCE_QUANTITY) && (app.output.quantity_size == 0x00))
		{
			field_size = strlen(db_rec->quantity);
		}
		
		switch (app.output.sequence[i])
		{
			case SEQUENCE_BARCODE:
				if ((app.output.sequence[i] == SEQUENCE_BARCODE) && (app.output.barcode_size == 0x00))
					memcpy(ptr, db_rec->barcode, field_size);
				else
					sprintf(ptr, "%-*.*s", field_size, field_size, db_rec->barcode);
				break;
				
			case SEQUENCE_QUANTITY:
				sprintf(ptr, "%-*.*s", field_size, field_size, db_rec->quantity);
				break;
				
			case SEQUENCE_SERIAL_NUMBER:
				sprintf(ptr, "%-*.*s", field_size, field_size, db_rec->serial);
				break;
				
			case SEQUENCE_TIME:
				sprintf(ptr, "%-*.*s", field_size, field_size, db_rec->time);
				break;
				
			case SEQUENCE_DATE:
				sprintf(ptr, "%-*.*s", field_size, field_size, db_rec->date);
				break;
		}
		
		ptr += field_size;
		
		if (i == (SEQUENCE_MAX - 1) || app.output.sequence[i + 1] == SEQUENCE_NONE || app.output.sequence[i + 1] > SEQUENCE_MAX)
		{
			sprintf(ptr, "\r\n");
			ptr += 2;
			break;
		}

		*ptr++ = app.output.field_separator;	// Add field separator
	}
	
	if(AppendRecord( &dbExportStorage, (uint8_t *)g_Record, (ptr - g_Record)) != DATABASE_OK ) 
		result = ERROR_FILESYSTEM;

	return result;
}

//------------------------------------------------------------------
//  StringQuantityToLong
//  ============================
//  Convert the stored quantity string to a long value
//------------------------------------------------------------------
long StringQuantityToLong(char* quantity)
{
	static char tmp[SZ_QUANTITY + 1];
	char* ptr;
	int pos;

	memset(tmp, '\0', sizeof(tmp));
	ptr = tmp;
	for (pos = 0; pos < (SZ_QUANTITY); pos++)
	{
		if (quantity[pos] == '\0')
			break;
		
		if (quantity[pos] == ' ') // cut off all spaces
			continue;
		
		*ptr++ = quantity[pos];
	}
	if (atol(tmp) == 0L && tmp[0] == '-')
		return 0;
	
	return (atol(tmp));
}

//------------------------------------------------------------------
//  FieldSize
//  ============================
//  Returns the string length of the specified field type
//------------------------------------------------------------------
int FieldSize(int field_type)
{
	switch (field_type)
	{
		case SEQUENCE_BARCODE:	
			return (app.output.barcode_size>>4)*10 + (app.output.barcode_size & 0xF);
			
		case SEQUENCE_QUANTITY:
			return app.output.quantity_size;
			
		case SEQUENCE_SERIAL_NUMBER:
			return SZ_SERIAL;

		case SEQUENCE_TIME:
			if ((app.output.date_time_format & TIME_FORMAT_MASK) == TIME_FORMAT_HH_MM)
				return 5;

			return 8;		// TIME_FORMAT_HH_MM_SS:

		case SEQUENCE_DATE:

			switch (app.output.date_time_format & DATE_FORMAT_MASK)
			{
				case DATE_FORMAT_DASH_DD_MM_YYYY:
				case DATE_FORMAT_DASH_MM_DD_YYYY:
				case DATE_FORMAT_DASH_YYYY_MM_DD:
				case DATE_FORMAT_SLASH_DD_MM_YYYY:
				case DATE_FORMAT_SLASH_MM_DD_YYYY:
				case DATE_FORMAT_SLASH_YYYY_MM_DD:
					return 10;

				case DATE_FORMAT_DASH_DD_MM_YY:
				case DATE_FORMAT_DASH_MM_DD_YY:
				case DATE_FORMAT_DASH_YY_MM_DD:
				case DATE_FORMAT_SLASH_DD_MM_YY:
				case DATE_FORMAT_SLASH_MM_DD_YY:
				case DATE_FORMAT_SLASH_YY_MM_DD:
					return 8;

				case DATE_FORMAT_DASH_DD_MM:
				case DATE_FORMAT_DASH_MM_DD:
				case DATE_FORMAT_SLASH_DD_MM:
				case DATE_FORMAT_SLASH_MM_DD:
					return 5;
			}
			break;
	}
	
	return 0;
}

//-------------------------------------------------------------------
//  FindBarcodeInDatabase
//  ============================
//  Searches for the specified barcode in the database and returns it
//-------------------------------------------------------------------
/*long FindBarcodeInDatabase(char *barcode, char *quantity)
{
	long lFound = -1L;
	int barcode_field, quantity_field;
	unsigned char deleted = FALSE;

	barcode_field = GetBarcodeFieldNr();

	if (barcode_field == -1)
		return -1L;
	
	quantity_field = GetQuantityFieldNr();
	
	dbItemStorage.keyfield = barcode_field;
	dbItemStorage.csvChar =	app.output.field_separator;

	if( quantity_field == -1 && (app.output.quantity_options & QUANTITY_DELETE_KEY_DISABLED))
	{
		dbItemStorage.idxFilename = NULL;		// No index file needed
		dbItemStorage.dbFilename = FNAME_SCANNED_EXPORT;
	}
	else
	{
		dbItemStorage.idxFilename =	FNAME_SCANNED_IDX;
		dbItemStorage.dbFilename = FNAME_SCANNED;
	}

	if (quantity_field == -1)
	{
		//
		// Avoid doing a slow database search when it's not even needed
		//
		if (last_key_pressed != CLR_KEY || (app.output.quantity_options & QUANTITY_DELETE_KEY_DISABLED))
		{
			return -1L;							// Return -1, so record will be appended
		}

		//
		// Find first matching barcode from the bottom up
		//
		if (SearchDatabase(&dbItemStorage, &dbStorage, barcode, &lFound, g_Record, SEEK_END, 0) != DATABASE_OK)
		{
			return -1L;							// Barcode not found
		}

		deleted = (g_Record[0] == 'X');

   		if (deleted) // Found code is already deleted
		{
			// Search for older barcode that's not already deleted
			while(SearchDatabase(&dbItemStorage, &dbStorage, barcode, &lFound, g_Record, SEEK_CUR, -1) == DATABASE_OK)
			{
				if (g_Record[0] == 'O')
				{
					deleted = FALSE;
					break;
				}
			}
		}

		if (deleted)		// Found codes are all already deleted
			return -1L;
	}
	else
	{
		//
		// Find first matching barcode from the bottom up (when quantity field length changes (i.e from 9 to 10), the old record is marked as deleted and a new record is added)
		//
		if (SearchDatabase(&dbItemStorage, &dbStorage, barcode, &lFound, g_Record, SEEK_END, 0) == DATABASE_OK)
		{
			// Fill the quantity string
			GetField(&dbItemStorage, quantity, g_Record, quantity_field, SZ_QUANTITY);
		}
	}
	return lFound;
}*/

//------------------------------------------------------------------
//  ReplaceLineEndings
//  ============================
//  - str, in/out,  NULL terminated string to modify.
//  - c,   in 		Replacement character.
//------------------------------------------------------------------
static void ReplaceLineEndings(char* str, int *str_len, char c)
{
	// remove line ending from end of record,
	int len = *str_len;

	if (str[len - 1] == '\n' || str[len - 1] == '\r')
		--len;

	if (str[len - 1] == '\n' || str[len - 1] == '\r')
		--len;

	// replace all line feeds in the record  with 'c'.
	char* next = strchr(str, '\n');
	char* end = str + len;
	while (next)
	{
		if (next <= end) {
			*next = c;
			next = strchr(next, '\n');
		}
		else
			break;
	}
	// now remove remove left over '\r'.
	end = strrchr(str, '\r');
	next = strchr(str, '\r');
	while (next)
	{
		if (next < end)
		{
			memmove(next, next + 1, --len);
			next = strchr(next, '\r');
		}
		else
			break;
		
	}

	*str_len = len;
}

//------------------------------------------------------------------
//  ExportBarcode
//  ============================
//  Exports a barcode in the database
//------------------------------------------------------------------
int ExportBarcode(struct barcode *code)
{
	// Because we now the year aint 32182 and month not 254
	_Pragma("GCC diagnostic ignored \"-Wformat-overflow=\"") 

	static db_record db_rec;
	struct date *date = &code->date;
	struct time *time = &code->time;

	if(code->quantity == 0 && (app.output.quantity_options & QUANTITY_DELETE_ZERO_QNTY))
		return OK;

	//memset(&db_rec, '\0', sizeof(db_record));

	// fill the barcode into the record structure
	db_rec.barcode = code->text;
	db_rec.barcode_len = code->length;

	if(db_rec.barcode_len < 0 || db_rec.barcode_len > code->max)
		return  ERROR;
		
	// Remove possible trailing CR (suffix)
	if(db_rec.barcode[db_rec.barcode_len-1] == '\r')
		db_rec.barcode[--db_rec.barcode_len] = '\0';

	// Replace line ends with tabs.
	ReplaceLineEndings(  db_rec.barcode, &(db_rec.barcode_len), '\t' );

	// When ok fill the quantity of the db_record
	sprintf(db_rec.quantity, "%*d", app.output.quantity_size, code->quantity);

	sprintf(db_rec.serial, "%-6.6lu", GetSerialNumber());

	switch (app.output.date_time_format & TIME_FORMAT_MASK)
	{
		case TIME_FORMAT_HH_MM:
			sprintf(db_rec.time, "%02d:%02d", time->ti_hour, time->ti_min);
			break;

		case TIME_FORMAT_HH_MM_SS:
			sprintf(db_rec.time, "%02d:%02d:%02d", time->ti_hour, time->ti_min, time->ti_sec);
			break;
	}

	switch (app.output.date_time_format & DATE_FORMAT_MASK)
	{
		case DATE_FORMAT_SLASH_DD_MM_YYYY:
			sprintf(db_rec.date, "%02d/%02d/%04d", date->da_day, date->da_mon, date->da_year);
			break;
		case DATE_FORMAT_DASH_DD_MM_YYYY:
			sprintf(db_rec.date, "%02d-%02d-%04d", date->da_day, date->da_mon, date->da_year);
			break;
		case DATE_FORMAT_SLASH_MM_DD_YYYY:
			sprintf(db_rec.date, "%02d/%02d/%04d", date->da_mon, date->da_day, date->da_year);
			break;
		case DATE_FORMAT_DASH_MM_DD_YYYY:
			sprintf(db_rec.date, "%02d-%02d-%04d", date->da_mon, date->da_day, date->da_year);
			break;
		case DATE_FORMAT_SLASH_YYYY_MM_DD:
			sprintf(db_rec.date, "%04d/%02d/%02d", date->da_year, date->da_mon, date->da_day);
			break;
		case DATE_FORMAT_DASH_YYYY_MM_DD:
			sprintf(db_rec.date, "%04d-%02d-%02d", date->da_year, date->da_mon, date->da_day);
			break;
		case DATE_FORMAT_DASH_DD_MM_YY:
			sprintf(db_rec.date, "%02d-%02d-%02d", date->da_day, date->da_mon, date->da_year % 100);
			break;
		case DATE_FORMAT_SLASH_DD_MM_YY:
			sprintf(db_rec.date, "%02d/%02d/%02d", date->da_day, date->da_mon, date->da_year % 100);
			break;
		case DATE_FORMAT_SLASH_MM_DD_YY:
			sprintf(db_rec.date, "%02d/%02d/%02d", date->da_mon, date->da_day, date->da_year % 100);
			break;
		case DATE_FORMAT_DASH_MM_DD_YY:
			sprintf(db_rec.date, "%02d-%02d-%02d", date->da_mon, date->da_day, date->da_year % 100);
			break;
		case DATE_FORMAT_SLASH_YY_MM_DD:
			sprintf(db_rec.date, "%02d/%02d/%02d", date->da_year % 100, date->da_mon, date->da_day);
			break;
		case DATE_FORMAT_DASH_YY_MM_DD:
			sprintf(db_rec.date, "%02d-%02d-%02d", date->da_year % 100, date->da_mon, date->da_day);
			break;
		case DATE_FORMAT_SLASH_MM_DD:
			sprintf(db_rec.date, "%02d/%02d", date->da_mon, date->da_day);
			break;
		case DATE_FORMAT_DASH_MM_DD:
			sprintf(db_rec.date, "%02d-%02d", date->da_mon, date->da_day);
			break;
		case DATE_FORMAT_SLASH_DD_MM:
			sprintf(db_rec.date, "%02d/%02d", date->da_day, date->da_mon);
			break;
		case DATE_FORMAT_DASH_DD_MM:
			sprintf(db_rec.date, "%02d-%02d", date->da_day, date->da_mon);
			break;
	}

	// Store the input data into our database
	return ExportBarcodeData(&db_rec);
}

//------------------------------------------------------------------
//  ProcessBarcode
//  =====================
//  Processes a barcode that has been read. This function
//  makes is possible to read set-up labels from the universal
//  book as well. (Changed settings will be lost after a restart)
//------------------------------------------------------------------
int ProcessBarcode(struct barcode *code)
{
	int result = 0;
#ifndef HAS_2D_ENGINE
	int aiming = -1;
#endif
	if (code->id == MENU_CODE)	// Check for standard Code-39 menu labels (from the OSE Universal menu book)
	{
		while(result != EXITING_MENU_MODE && result != ERROR)
		{
#ifndef HAS_2D_ENGINE
			if(TriggerPressed() && aiming == true)
			{
				aiming = false;
				AimingOff();
				ScannerPower(ON, -1);
			}
			else if(!TriggerPressed() && aiming == false)
			{
				aiming = true;
				ScannerPower(OFF, 0);
				AimingOn();				
			}
#endif
			if(result == 0 || ReadBarcode(code) == OK)
			{
				switch( (result=ExecuteMenuLabel(code)) )
				{
					case ENTERING_MENU_MODE:
#ifndef HAS_2D_ENGINE
						aiming = !TriggerPressed();
#endif
						Sound(TSTANDARD, VHIGH, SHIGH, SMEDIUM, SHIGH, 0);
						break;

					case EXITING_MENU_MODE:
						Sound(TSTANDARD, VHIGH, SHIGH, SMEDIUM, SHIGH, 0);
						Delay(TSTANDARD * 5);	// Wait till buzzer sound if finished before saving (saving flash settings turns off sound timer temporarily)
						SystemSetting("Z2");		// Save changed settings in flash memory and apply possibly changed interface
						break;

					case INVALID_OPTION_READ:
						Sound(TLONG, VHIGH, SLOW, 0);
						break;

					case VALID_OPTION_READ:
						Sound(TSTANDARD, VHIGH, SHIGH, SMEDIUM, SHIGH, 0);
						break;

					case LABEL_IGNORED:
						break;

					case ERROR:
						break;
				}
			}
		}

		return MENU_CODE;
	}
	else if (code->id == MENU_CODE_PDF || code->id == MENU_CODE_C128 || code->id == MENU_CODE_QR || code->id == MENU_CODE_AZTEC)		// Check for Code-128 / PDF417 / QR / Aztec menu labels)
	{
		switch ((result = ExecuteMenuLabel(code)))
		{
			case INVALID_OPTION_READ:
				Sound(TLONG, VHIGH, SLOW, 0);
				break;

			case VALID_OPTION_READ:
				Sound(TSTANDARD, VHIGH, SMEDIUM, SHIGH, SMEDIUM, SHIGH, 0);
				Delay(TSTANDARD * 5);	// Wait till buzzer sound if finished before saving (saving flash settings turns off sound timer temporarily)
				SystemSetting("Z2");	// Save changed settings in flash memory and apply possibly changed interface
				break;

			case LABEL_IGNORED:
			default:
				break;
		}

#ifdef HAS_AIMING
		if(Aiming_Enabled)
			AimingOn();
		else
			AimingOff();
#endif
		
		return MENU_CODE;
	}

	//
	// Store barcode
	//

	if (last_key_pressed == CLR_KEY && !(app.output.quantity_options & QUANTITY_DELETE_KEY_DISABLED)) // If the CLR_KEY is pressed
		code->quantity = -1;
	else
		code->quantity = 1;

	if( (result = UpdateBarcodeInMemory(code)) == OK || result == WARNING )
	{
		if(code->quantity >= 0)
		{
			ShowGoodReadLed((result == WARNING) ? RED : GREEN);	// Warning = negative quantity
			OkBeep();		// Barcode added, quantity >= 0 : 3000 Hz
		}
		else if (result == OK)
		{
			ShowGoodReadLed(ORANGE);		// ORANGE led indicates a succesfully deleted/substracted barcode
			RemoveBeep();
		}
		else if(result == WARNING)
		{
			ShowGoodReadLed(RED);	// RED led indicates a succesfully substracted barcode, resulting in a negative value
			WarningBeep();			// Barcode deleted, quantity < 0 : 1500 Hz
		}
	}
	else
	{
		ShowGoodReadLed(RED);
		ErrBeep();
	}
	
	return FALSE;
}

//------------------------------------------------------------------
//  EnableAiming
//  =====================
//  Enables aiming help for reading the set-up sheet of the manual
//------------------------------------------------------------------
int EnableAiming(void)
{
	Aiming_Enabled = TRUE;
	return OK;
}

//------------------------------------------------------------------
//  DisableAiming
//  =====================
//  Disable aiming help for reading the set-up sheet of the manual
//------------------------------------------------------------------
int DisableAiming(void)
{
	Aiming_Enabled = FALSE;
	return OK;
}

//------------------------------------------------------------------
//  UsbSetMsd
//  =====================
//  Select OseComm default (batch mode)
//------------------------------------------------------------------
int OseCommDefault(void)
{
	Application_Default(COM0);
	return OK;
}

//------------------------------------------------------------------
//  ReinitBarcodeStorage
//  =====================
//  Configure storage with application specific database options
//------------------------------------------------------------------
int ReinitBarcodeStorage(void)
{
	uint8_t quantity_options = 0;

	quantity_options |= (app.output.quantity_options & QUANTITY_ALLOW_NEGATIVE_QNTY) ? QNT_OPT_ALLOW_NEGATIVE_QNTY : 0;
	quantity_options |= (app.output.quantity_options & QUANTITY_DELETE_ZERO_QNTY) ? 0 : QNT_OPT_KEEP_ZERO_QNTY;
	quantity_options |= (GetQuantityFieldNr() == -1) ? QNT_OPT_ALLOW_DUPLICATES : 0;
	quantity_options |= QNT_OPT_FAST_OPEN;	// We don't need to know exactly how many barcodes are in memory when we have over 1024 records, so we'd rather be fast
	quantity_options |= (app.special_options & PERFORMANCE_MODE) ? QNT_OPT_FAST_READ : 0;	// Skips the f_sync after writing each barcode

	InitStorage(quantity_options);
	return OK;
}

int StoreApplicationSettings(void)
{
	ReinitBarcodeStorage();	// Configure storage with application specific database options
	SystemSetting("Z2");
	return OK;
}

//------------------------------------------------------------------
//  UsbSetMsd
//  =====================
//  Select USB MSD Interface
//------------------------------------------------------------------
int UsbMsdDefault(void)
{
	Application_Default(COM11);
	return OK;
}

//------------------------------------------------------------------------------
//	GetBarcodeFieldNr
//	=====================
//
//------------------------------------------------------------------------------
/*int GetBarcodeFieldNr(void)
{
	int i, barcode_field;

	barcode_field = -1;
	
	for (i = 0; i < SEQUENCE_MAX; ++i)
	{
		if (app.output.sequence[i] == SEQUENCE_BARCODE)
		{
			barcode_field = i+1;
			break;
		}
		
		if (i == SEQUENCE_MAX - 1 || app.output.sequence[i + 1] == SEQUENCE_NONE || app.output.sequence[i + 1] > SEQUENCE_MAX)
			break;
	}
	
	return barcode_field;
}*/

//------------------------------------------------------------------------------
//	GetQuantityFieldNr
//	=====================
//
//------------------------------------------------------------------------------
int GetQuantityFieldNr(void)
{
	int i, quantity_field;

	quantity_field = -1;
	
	for (i = 0; i < SEQUENCE_MAX; ++i)
	{
		if (app.output.sequence[i] == SEQUENCE_QUANTITY)
		{
			quantity_field = i+1;
			break;
		}
		
		if (i == SEQUENCE_MAX - 1 || app.output.sequence[i + 1] == SEQUENCE_NONE || app.output.sequence[i + 1] > SEQUENCE_MAX)
			break;
	}
	
	return quantity_field;
}

//------------------------------------------------------------------------------
//	ShutdownCallback
//  =====================
// 	Closes the database file before shutting down
//------------------------------------------------------------------------------
int ShutdownCallback(void)
{
	CloseStorage();

	if (UsbIsPowered() && app.default_interface == COM11)	// If MSD -> Generate the export file here on connecting the USB-cable
	{
		if(!app_settings_restored) // In MSD-mode, the application may not have reached this call, when the 'ShutdownCallback' is called 
		{
			ApplicationSettingsMemorizing(ON, (void *)&app, sizeof(app), VERSION_CHECK);
			ReinitBarcodeStorage();	// Configure storage with application specific database options
			app_settings_restored = TRUE;
		}	

#ifdef HAS_2D_ENGINE
		ScannerPower(SCAN_SUSPEND, 0);			// Remember the current read mode
#else
		ScannerPower(OFF, 0);				// Disable scanning while waiting to transmit its data
#endif
		if(!TriggerPressed() || !ClearKeyPressed()) 
		{
			ExportDatabase();					// Export the database.
			GoodReadLed(GREEN_FLASH, -1);		// Show blinking green LED when connected to USB.
		}
		else
		{
			GoodReadLed(MAGENTA | LED_FLASH, -1);		// Show blinking green LED when connected to USB.
		}
	}

	return OK;
}



//------------------------------------------------------------------------------
// 	ExportDatabase
//  =====================
// 	Exports the database from the SCANNED.DAT/IDX files to the SCANNED.TXT file.
//	The .DAT/.IDX files are for internal use. The .TXT file is for export to the computer.
//	.IDX keeps track of the code id, quantity, deleted flag, date/time and record offsets
//	.DAT contains the raw barcode data (it can also contain deleted barcodes)
//------------------------------------------------------------------------------
int ExportDatabase(void)
{
	int res = 0;
	int records = 0;
	int total_records = BarcodesInMemory();

	//*********** show orange flash if data available to download ( code from OPN-2002 program) **********************
	SystemSetting("8Z");				// Disable charge indicator (or we wouldn't see the orange flashing)
	GoodReadLed(ORANGE_FLASH, -1);
	//***************************************************************
	
	remove(FNAME_SCANNED_EXPORT);

	dbExportStorage.fdDb = &SDBExportFile;   
	
   	if( CreateBarcodeDatabase((char*)FNAME_SCANNED_EXPORT, NULL, &dbExportStorage) == DATABASE_OK )
	{
		if(total_records > 0)
		{
		    if( ReadFirstBarcodeFromMemory(&code) == DATABASE_OK )
		    {
		        do
		        {
		            if((res=ExportBarcode(&code)) < 0)
		                break;

		            ++records;

		        } while (ReadNextBarcodeFromMemory(&code) == DATABASE_OK);
		    }

		    DEBUGPUTStr("ExportDatabase() Records: %d exported", records); 
		}

		// close databases (and truncates it to the actual size)
		CloseBarcodeDatabase(&dbExportStorage);

		CloseStorage();

		if(total_records == 0)
		{
			ClearBarcodes(false);
		}
	}

		
	//*********** Stop orange flashing and re-enable charge indicator ***********************
	GoodReadLed(ORANGE_FLASH, 0);	// Stop the orange flashing LED (if enabled)
	SystemSetting("7G");			// Re-enabled charge indicator
	//***************************************************************

    if(res < 0)
    {
        DEBUGPUTStr("ExportDatabase() error writing DB");
        return ERR_DB_WRITE;	// return error writing DB
    }
    return OK;
}

//------------------------------------------------------------------
//  main
//  =====================
//  Main function of this demo that allows the reading of
//  barcodes and transmit them using the USB-C cable to a virtual
//  comport on a PC or laptop or download them as file in USB-MSD mode. 
//  This application also support the
//  reading of the univeral menu-book. Since all changed
//  configurations are stored in flash, the changed configurations
//  are restored after a restart even if the battery has been empty.
//------------------------------------------------------------------
#ifdef ALLINONE_APP
void batch_main(void)
#else
void app_main(void)
#endif
{
	int state, ch;

	// Initialize the barcode structure
	InitBarcodeBuf(0, MAX_BARCODE_LENGTH);

	state = STATE_IDLE;
	reset_read_mode = FALSE;

	DEBUGPutString("Application started");

	// The following function implements SystemSetting menu options that aren't present in the OS or need to be overruled
	AddMenuOptionTable(batch_menu_option_table, BATCH_MAX_OPTIONS);

	ScannerPower(SCAN_SINGLE | SCAN_TRIGGER, -1);	// Initialize the scanner power to default and let the OS handle the rest

	// Set shutdown callback to close the barcode database
	Application_ShutdownCallback(ShutdownCallback);

	if(SystemSettingsMemorizing(ON) < 0 || set_to_known_default == FALSE)	// Enable remembering of (decoder) settings in flash and load old settings if present
	{
		// Don't call Application_Default directly, because it calls SystemSetting("OPND") without ']', 
		// because it assumes Application_Default is always called as a result of a SystemSetting() call.
		SystemSetting("]OPND");
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

	ReinitBarcodeStorage();

	SystemSetting("YC");	// Always makes sure menu-labels are enabled after a restart
	
	// If TRUE, then the small clear key acts like a trigger key, so read modes settings also apply to this key
	ClearKeyAsTrigger((app.output.quantity_options & QUANTITY_DELETE_KEY_DISABLED) ? FALSE : TRUE);
	
	//
	// Disable all timers
	//
	for(int x = 0; x < MAX_TIMERS; ++x)				
		timer[x] = TIMER_DISABLED;
	
	//
	// Install our own heartbeat timer that beats every 20ms
	//
	InstallHeartbeatHandler( Timers_Heartbeat );

	

#ifndef HAS_2D_ENGINE
	//
	// We're controlling the read mode in the app unfortunately; set the last settings from the app vars
	//
	SetReadMode(app.read_mode, app.read_time);
#endif

	AutoPowerDown(ON, IDLE_POWER_DOWN_TIME);				// Init auto power down time.
	AutoPowerDown(APD_SHUTDOWN_ON, IDLE_POWER_OFF_TIME);	// Init auto power off time.

	if (IsColdBoot() && (app.buzzer_options & STARTUP_BUZZER))
    {
        Sound(TSTANDARD, VHIGH, SLOW, SMEDIUM, SHIGH, SLOW, SMEDIUM, SHIGH, 0);	// Start-up beep
    }

    for (;;)
    {		
#if NRF_CLI_ENABLED
        NRF_CLI_PROCESS();
#endif

        NRF_LOG_PROCESS();

        //
        // Main state machine
        //
        switch (state)
        {
            case STATE_IDLE:		// Idle state (not connected)
                if ((ch = KeyOut()) > 0)
                    last_key_pressed = ch;// Used to determine whether a barcode was read using the clear key or trigger key
					
				//
				// This code allows a format of all stored data by pressing the delete key for 15 seconds
				//
				if (ClearKeyPressed() && !TriggerPressed() && app.func_key_del)
				{
					if (timer[FORMAT_TIMER] == TIMER_DISABLED)
					{
						timer[FORMAT_TIMER] = CLR_KEY_FORMAT_TIME;
					}
					else if (timer[FORMAT_TIMER] == 0)
					{
						timer[FORMAT_TIMER] = TIMER_DISABLED;
						GoodReadLed(GREEN, 100);
                        ClearBarcodes(false);
						Sound(TSTANDARD, VHIGH, SHIGH, SMEDIUM, SLOW, 0);
					}
				}
				else
				{
					timer[FORMAT_TIMER] = TIMER_DISABLED;
				}

				//
				// This code allows a safe reset when pressing both keys for 15 seconds (the OS manual reset of 20 seconds doesn't close the databases, causing possible data loss!)
				//
				if(ClearKeyPressed() && TriggerPressed())
				{
					if (timer[RESET_TIMER] == TIMER_DISABLED)
					{
						timer[RESET_TIMER] = RESET_TIME;
						GoodReadLed(ORANGE_FLASH, -1);
						ScannerPower(SCAN_SUSPEND, 0);
					}
					else if (timer[RESET_TIMER] == 0)
					{
						timer[RESET_TIMER] = TIMER_DISABLED;
						Sound(TSTANDARD, VHIGH, SHIGH, SMEDIUM, SLOW, SHIGH, 0);
						Delay(TSTANDARD * 5);

						while(ClearKeyPressed() && TriggerPressed())	// We don't want to enter boot mode or halt the application, so wait till keys are released!
							Idle();
						
						SoftReset();
					}
				}
				else if(timer[RESET_TIMER] != TIMER_DISABLED)
				{
					timer[RESET_TIMER] = TIMER_DISABLED;
					GoodReadLed(ORANGE_FLASH, 0);
					ScannerPower(SCAN_RESUME, 0);
				}

				//
				// Barcode reading and handling
				//
				if (ReadBarcode(&code) == OK)			// If barcode read
				{
					ProcessBarcode(&code);				// Process the barcode
					reset_read_mode = TRUE;
				}

				if (UsbIsConnected())	// If the USB is connected
				{
					if (app.default_interface != COM11)	// If not MSD -> Generate the export file here. Else Generate it in the shutdown-callback
					{
						CloseStorage();					// Forces a flush of the last scanned data

						if(!TriggerPressed() || !ClearKeyPressed()) 
						{
							AutoPowerDown(APD_SUSPEND, 0);
							ExportDatabase();				// Export the database
							AutoPowerDown(APD_RESUME, 0);
							GoodReadLed(GREEN_FLASH, -1);	// Show blinking green LED when connected to USB
						}
						else
						{
							GoodReadLed(MAGENTA | LED_FLASH, -1);		// Show blinking green LED when connected to USB.
						}

#ifdef HAS_2D_ENGINE
						ScannerPower(SCAN_SUSPEND, 0);			// Remember the current read mode
#else
						ScannerPower(OFF, 0);				// Disable scanning while waiting to transmit its data
#endif
						state = STATE_USB_CONNECTED;		// Switch the state to usb connected
					}
				}
                break;

            case STATE_USB_CONNECTED:
                if (!UsbIsConnected())
                {
                    if (app.default_interface == COM11)	// USB-MSD: Powerdown on disconnecting the USB cable, because the file system might have changed completely the OS will restart the application!!!!!!!
                    {
                        state = STATE_WAIT_FOR_RELEASE;
						reset_read_mode = TRUE;
						break;
                    }

					GoodReadLed(ORANGE, -1);
                    OpenStorage();					// Re-open the database
					GoodReadLed(ORANGE, 0);
#ifdef HAS_2D_ENGINE
                    ScannerPower(SCAN_RESUME, 0);		// Restore the read mode
#else
					reset_read_mode = TRUE;
#endif
                    state = STATE_WAIT_FOR_RELEASE;	// Forces an update of the read mode, which may have been changed
                    break;
                }
				else if (app.default_interface != COM11)
                {
                    Communicate();
                }
                break;

            case STATE_WAIT_FOR_RELEASE:
            default:
                if (!TriggerPressed() && !ClearKeyPressed())
                {
                    GoodReadLed(GREEN_FLASH, 0);// Stop the green flashing LED (if enabled)
                    state = STATE_IDLE;
                }
                break;
        }
		
#ifndef HAS_2D_ENGINE
		if ( reset_read_mode && !TriggerPressed() )
        {
            reset_read_mode = FALSE;
            SetReadMode(app.read_mode, app.read_time);
        }
#endif
		
		// Detect connection with OptiConnect -> Switch to default application
		if(ComOpen(-1) == COM16 || ComOpen(-1) == COM7)
		{
			SystemSetting("]EBLE"); // Causes a reset!
		}

        Idle();		// This reduces the power consumption
    }
}
