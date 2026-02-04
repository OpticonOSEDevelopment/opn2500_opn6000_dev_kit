
// Error values

#define IS_ERROR( x ) ( (int)x < 0 )
#define ERROR_ILLEGAL_QUANTITY		(-1)  	// 0xFFFF
#define ERROR_DATABASE				(-2)	// 0xFFFE
#define ERROR_FILESYSTEM			(-3)	// 0xFFFD
#define ERROR_INVALID_FIELD			(-4)	// 0xFFFC
#define ERROR_INVALID_VALUE			(-5)	// 0xFFF0
#define ERROR_FORMAT_CHANGED 		(-6)


// Database record size
#define SZ_BARCODE					(MAX_BARCODE_LENGTH+1)
#define SZ_QUANTITY					9
#define SZ_SERIAL					6
#define SZ_TIME						(2+1+2+1+2)
#define SZ_DATE						(4+1+2+1+2)
#define SZ_DELETED					1
#define SZ_RECORD					(SZ_BARCODE+1+SZ_QUANTITY+1+SZ_SERIAL+1+SZ_TIME+1+SZ_DATE+1+SZ_DELETED+1+1)

#define FNAME_SCANNED_EXPORT		"SCANNED.TXT"

// Defines for app.read_time
#define READ_TIME_INDEFINITELY			0xFE		// Read time indefinitely
// Defines for app.buzzer_options
#define BUZZER_ENABLED					0x01
#define STARTUP_BUZZER					0x02
#define BUZZER_TONE_SINGLE				0x00
#define BUZZER_TONE_HIGH_LOW			0x10
#define BUZZER_TONE_LOW_HIGH			0x20
#define BUZZER_TONE_MASK				0x30

// Defines for app.output.date_time_format
#define TIME_FORMAT_HH_MM_SS			0x00
#define TIME_FORMAT_HH_MM				0x01
#define TIME_FORMAT_MASK				0x03

#define DATE_FORMAT_DASH_DD_MM_YYYY		0x00
#define DATE_FORMAT_DASH_MM_DD_YYYY		0x10
#define DATE_FORMAT_DASH_DD_MM_YY		0x20
#define DATE_FORMAT_DASH_MM_DD_YY		0x30
#define DATE_FORMAT_DASH_YYYY_MM_DD		0x40
#define DATE_FORMAT_DASH_YY_MM_DD		0x50
#define DATE_FORMAT_DASH_DD_MM			0x60
#define DATE_FORMAT_DASH_MM_DD			0x70
#define DATE_FORMAT_SLASH_DD_MM_YYYY	0x80
#define DATE_FORMAT_SLASH_MM_DD_YYYY	0x90
#define DATE_FORMAT_SLASH_DD_MM_YY		0xA0
#define DATE_FORMAT_SLASH_MM_DD_YY		0xB0
#define DATE_FORMAT_SLASH_YYYY_MM_DD	0xC0
#define DATE_FORMAT_SLASH_YY_MM_DD		0xD0
#define DATE_FORMAT_SLASH_DD_MM			0xE0
#define DATE_FORMAT_SLASH_MM_DD			0xF0
#define DATE_FORMAT_MASK				0xF0

// Defines for app.output.quantity_options
#define QUANTITY_DELETE_KEY_ENABLED		0x00
#define QUANTITY_DELETE_KEY_DISABLED	0x01
#define QUANTITY_DELETE_ZERO_QNTY		0x04
#define QUANTITY_ALLOW_NEGATIVE_QNTY	0x08

// Defines for app.output.field_separator

#define FIELD_SEPARATOR_ZERO			0x00
#define FIELD_SEPARATOR_TAB				0x09
#define FIELD_SEPARATOR_LF				0x0A
#define FIELD_SEPARATOR_CR				0x0D
#define FIELD_SEPARATOR_GS				0x1D
#define FIELD_SEPARATOR_COMMA			','
#define FIELD_SEPARATOR_SEMICOLON		';'
#define FIELD_SEPARATOR_SLASH			'/'
#define FIELD_SEPARATOR_PIPE			'|'
#define FIELD_SEPARATOR_SPACE			' '

// Defines for app.output.sequence
#define SEQUENCE_NONE					0x00
#define SEQUENCE_BARCODE				0x01
#define SEQUENCE_DATE					0x02
#define SEQUENCE_TIME					0x03
#define SEQUENCE_SERIAL_NUMBER			0x04
#define SEQUENCE_QUANTITY				0x05
#define SEQUENCE_MAX					SEQUENCE_QUANTITY

// Defines for app.special_options
#define NETO_COMMUNICATE				0x00
#define OSECOMM_COMMUNICATE				0x01

#define PERFORMANCE_MODE				0x80

//
// public functions
//
extern int OseCommDefault(void);
extern int UsbMsdDefault(void);
extern int ReinitBarcodeStorage(void);
extern int StoreApplicationSettings(void);

extern int EnableAiming(void);
extern int DisableAiming(void);

//
// Defines a structure of configuration settings that is stored in non-volatile (flash) memory
// to make it possible that certain configurations aren't lost after a restart
//
typedef struct 
{
		unsigned char field_separator;
		unsigned char date_time_format;
		unsigned char barcode_size;
		unsigned char quantity_size;
		unsigned char quantity_options;
		unsigned char serial_enabled;
		unsigned char sequence[5];		
}output_config_vars;

typedef struct
{
		unsigned char default_interface;						// Interface: USB-VCP/Cradle or USB-MSD
		unsigned char goodread_duration_old;					// Good read led duration (stored in 20ms steps)
		unsigned char buzzer_volume_old;						// Buzzer volume
		unsigned char buzzer_duration_old;						// Buzzer duration (stored in 40ms steps)
		unsigned char buzzer_options;							// Buzzer options
		unsigned char special_options;							// RFU: Special options (Reserved for future use)
#ifndef HAS_2D_ENGINE
		unsigned char read_mode;                                // Read mode
		unsigned char read_time;                                // Read time (stored in 0,1s steps)
#endif
		unsigned char func_key_del;								// Controls whether or not holding the function key for CLR_KEY_FORMAT_TIME will format the flash disk
		output_config_vars output;
} batch_appl_vars;

//#define MAX_RECORD_SIZE		(SZ_RECORD + 1)
//extern char	g_Record[MAX_RECORD_SIZE];
