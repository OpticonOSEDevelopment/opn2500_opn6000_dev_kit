/***********************************************************************
 *
 *  Copyright (C) Opticon Sensors Europe B.V.
 *
 *  lib.h For OPN2004 and OPN2005
 *
 ***********************************************************************/

#ifndef __LIB_H__
#define __LIB_H__

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "lib_legacy.h"		// Use Lower-case functions (deprecated)

// Hardware dependend defines
#include "boards.h"

// Location used by OS and Appload to retrieve the application version from memory
#define OS_PREAMBLE_SECT __attribute__ (( section (".os_preamble")))
#define OS_VERSION_SECT __attribute__ (( section (".os_version")))
#define APP_VERSION_SECT __attribute__ (( section (".app_version")))

#ifndef NULL
#define NULL ((void*)0)
#endif

//Function pointers without arguments
typedef void (*func_ptr)(void);
typedef int  (*ret_func_ptr)(void);
typedef int  (*heartbeat_function)(void);			 //!< Typedef of the heartbeat handlers.

//Function pointers with arguments
typedef void (*func_ptr_arg)(int argument);
typedef int  (*ret_func_ptr_arg)(int argument);

/** @defgroup General_defs General definitions.
 *  @{
 */
#define WARNING             2       //!< Value indicating something didn't went as expected.
#define OK                  0       //!< Value indicating something went OK
#define ERROR               -1      //!< Value indicating an error occurred.

#define OFF                 0       //!< Value indicating something is OFF
#define ON                  1       //!< Value indicating something is ON

#define YES                 1
#define NO                  0

#ifndef TRUE
#define TRUE  				1       //!< Value indicating TRUE
#endif
#ifndef FALSE
#define FALSE 				0       //!< Value indicating FALSE
#endif

//#define DISABLE             0       //!< Value indicating to disable something.
//#define ENABLE				1       //!< Value indicating to enable something.

#define NG					100
#define STANDBY				2
#define	DISABLED			-2

#define HIGH                1       //!< Value indicating high value.
#define LOW                 0       //!< Value indicating low value.
#define INPUT				3		//!< Value indicating input state
#define BLUETOOTH			4		//!< Value used by SystemSettingsMemorizing

#define FOREVER             -1

#define HEARTBEAT_PERIOD    20      //!< Time in milliseconds between heartbeats.

#ifndef MIN
/** Leaves the minimum of the two 32-bit arguments */
/*lint -emacro(506, MIN) */ /* Suppress "Constant value Boolean */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
/** Leaves the maximum of the two 32-bit arguments */
/*lint -emacro(506, MAX) */ /* Suppress "Constant value Boolean */
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#endif
/*
 *--------------------------------------
 *  ASCII definitions
 *--------------------------------------
 */
#define SOH             0x01
#define STX             0x02
#define ETX             0x03
#define EOT             0x04
#define ENQ             0x05
#define ACK             0x06
#define BEL             0x07
#define BS              0x08
#define HT              0x09
#define LF              0x0A
#define VT              0x0B
#define FF              0x0C
#define CR              0x0D
#define SO              0x0E
#define SI              0x0F
#define DLE             0x10
#define DC1             0x11
#define DC2             0x12
#define DC3             0x13
#define XOFF            DC3
#define DC4             0x14
#define NAK             0x15
#define SYN             0x16
#define ETB             0x17
#define CAN_CHAR        0x18
#define EM              0x19
#define SUB             0x1A
#define ESC             0x1B
#define ASCII_FS        0x1C
#define GS              0x1D
#define ASCII_RS        0x1E
#define US              0x1F
#define ASCII_SP        0x20
#define DEL             0x7F

/**
 * \defgroup KeysDef Definitions of the keys.
 * @{ */
#define TRIGGER_KEY     0x80
#define UP_KEY          0x81
#define CLR_KEY         UP_KEY
#define ENT_KEY			TRIGGER_KEY
/*
 *--------------------------------------
 *  scnanner functions
 *--------------------------------------
 */
#define EAN13           0x01
#define UPCA            0x02
#define EAN8            0x03
#define UPCE            0x04
#define UPCE1			0x05
#define ISBN			0x06
#define ISSN			0x07
#define ISMN			0x08
#define CODE39          0x09
#define CODABAR         0x0A
#define NW7             CODABAR
#define D2OF5           0x0B                    // industrial 2 of 5
#define I2OF5           0x0C                    // interleaved 2 of 5
#define CODE93          0x0D
#define CODE128         0x0E
#define MSI_PLESSEY     0x0F
#define IATA            0x10
#define UK_PLESSEY      0x11
#define TELEPEN         0x12
#define M2OF5           0x13
#define CHINPOST        0x14
#define NW7_ABC         0x15
#define NW7_CX          0x16
#define SCODE           0x17
#define LAETUS          0x18
#define TRIOPTIC        LAETUS
#define C39_FA          0x19                    // Full ascii
#define C39_IT          0x1A                    // Italian pharma code
#define EAN128          0x1B

#define RSS_14          0x1C
#define RSS_LIMITED     0x1D
#define RSS_EXPANDED    0x1E

#define PDF417          0x1F
#define CCC_COMPOSITE   0x20

#define MICRO_PDF417    0x21
#define CCA_COMPOSITE   0x22
#define CCB_COMPOSITE   0x23

#define CODE11          0x24

#define EAN128_CCC      0x25                    // EAN-128 with CC-C composite
#define CODE3OF5        0x26                    // Code 3 of 5 A.K.A. Korean Postal Code
#define CODABLOCK_F		0x27

#define QR_CODE			0x30
#define DATAMATRIX		0x31
#define MAXICODE		0x32
#define AZTEC			0x33
#define OCR				0x34
#define CHINESE_SENSIBL 0x35
#define INTERCODE		0x37
#define DOTCODE			0x38
#define INTELLIGENT_MAIL 0x39
#define POSTNET			0x3A


/*** ADD ON +2 ***/
#define ADD_ON_2        0x40
#define EAN13_2         0x41
#define UPCA_2          0x42
#define EAN8_2          0x43
#define UPCE_2          0x44
#define UPCE1_2         0x45
#define CODE128_2       0x4E                    // the same group CODE-128

/*** ADD ON +5 ***/
#define ADD_ON_5        0x80
#define EAN13_5         0x81
#define UPCA_5          0x82
#define EAN8_5          0x83
#define UPCE_5          0x84
#define UPCE1_5         0x85
#define CODE128_5       0x8E                    // the same group CODE-128

#define PARTIAL_READ	0x40
#define ADD_ON_MASK     0xC0
#define MENU_CODE       0xF0                    // OSE Code-39 Menu label
#define MENU_CODE_PDF   0xF1                    // OSE (u)PDF417 Menu label
#define MENU_CODE_C128  0xF2                    // OSE Code-128 Menu label
#define MENU_CODE_QR	0xF3					// QR Menu label
#define MENU_CODE_AZTEC	0xF4					// Aztec Menu label

#define NO_LABEL_ID     0xF8                    // Internal code-id for a No-label message
#define NO_DECODE_ID    0xF9                    // Internal code-id for a No-decode message

#define DATAWIZARD_MASK 0xD0                    // Code-id mask used for parallel and multirow label reading to bind datawizard configurations to the label sequence

/*
 * Constants for ScannerPower
 */
#define SCAN_SINGLE              2
#define SCAN_MULTIPLE            4
#define SCAN_CONTINUOUS          6
#define SCAN_TRIGGER             8
#define SINGLE_READ_TRIGGER      (SCAN_TRIGGER|SCAN_SINGLE)
#define MULTIPLE_READ_TRIGGER    (SCAN_TRIGGER|SCAN_MULTIPLE)
#define SCAN_SUSPEND             16
#define SCAN_RESUME              32

/* AutoPowerDown*/
#define APD_OFF             0       //!< Turn Auto Power Down Off
#define APD_ON              1       //!< Turn Auto Power Down On
#define APD_RESET           3       //!< Reset Auto Power Down timer.
#define APD_NOW             5       //!< Go into power down now.
#define APD_SHUTDOWN_ON     6      //!< Turn Auto Shutdown On.
#define APD_SHUTDOWN_OFF    7      //!< Turn Auto Shutdown Off.
#define APD_SHUTDOWN_NOW    8

#define APD_SUSPEND         16      //!< Suspend auto power down timer (16 to keep SUSPEND / ADP_SUSPEND / SCAN_SUSPEND the same to minimize backwards compatilbity issues)
#define APD_RESUME          32      //!< Resume auto power down timer. (32 to keep RESUME / ADP_RESUMED / SCAN_RESUME the same to minimize backwards compatilbity issues)

/*
 *--------------------------------------
 *  Led/sound functions
 *--------------------------------------
 */
#define RED                 0xFF0000                //!< Turn Red led continuously on.
#define GREEN               0x00FF00                //!< Turn Green led continuously on.
#define BLUE                0x0000FF                //!< Turn Blue led continuously on.
#define YELLOW              0xFFFF00                //!< Turn Yellow ( RED and GREEN) led continuously on.
#define ORANGE              0xFF8000                //!< Turn Orange ( RED and GREEN @ 50%) led continuously on.
#define DIMMED_ORANGE       0x080200                //!< Turn Orange ( RED and GREEN @ 50%) led continuously on.
#define WHITE               0xFFFFFF                //!< Turn White on
#define CYAN				0x00FFFF
#define MAGENTA             0xFF00FF
#define WHITE               0xFFFFFF
#define GOOD_READ_COLOR		0xFEFEFEFE				//!< When passed to ShowGoodReadLed, it uses the configured good read color

#define LED_FLASH           0x1000000               //!< Turn led on flashing.
#define LED_FLASH_SLOW      0x8000000               //!< Turn led on flashing (slow)
#define RED_FLASH           (RED | LED_FLASH)       //!< Turn Red led on flashing.
#define GREEN_FLASH         (GREEN | LED_FLASH)     //!< Turn Green led on flashing.
#define ORANGE_FLASH        (ORANGE | LED_FLASH)    //!< Turn Orange(Red and Green) led on flashing.
#define BLUE_FLASH          (BLUE | LED_FLASH)      //!< Turn Blue led on flashing.
#define BLUE_FLASH_SLOW     (BLUE | LED_FLASH_SLOW) //!< Turn Blue led on flashing.
#define WHITE_FLASH         (WHITE | LED_FLASH)      //!< Turn Blue led on flashing.

#define VIBRATE             0x2000000
#define VIBRATE_TOGGLE      0x4000000

/*
 * definitions for Sound volume level
 */
#define VOFF                0    //!< Sets sound volume off.
#define VVLOW               10    //!< Sets sound volume very low.
#define VLOW                25    //!< Sets sound volume low.
#define VSTANDARD           50    //!< Sets sound volume normal.
#define VMEDIUM             75    //!< Sets sound volume medium.
#define VHIGH               100   //!< Sets sound volume high.
#define VSYSTEM             0xFE  //!< Sets sound volume the System volume set by Menu-labels & Systemsetting()

 /*
 * definitions for Sound tone duration
 */
#define TCLICK              1   //!< Sets duration to one click.
#define TSHORT              3   //!< Sets duration to short buzz.
#define TSTANDARD           5   //!< Sets duration to normal buzz.
#define TLONG               10  //!< Sets duration to long buzz.
#define TVLONG              20  //!< Sets duration to very long buzz.
#define ARG_POINTER         0x80

/*
 * definitions for Sound tone
 */
#define SPAUSE              0x7FFF  //!< Pause of 100 msec.
#define REPEAT              0xFFFE  //!< Repeat previous.
#define REPLAY              0xFFFD  //!< Replay complete sound
#define REPLAY_X            0xFFFC  //!< Repeat sound from index X
#define ADD_PAUSE           0x8000  //!< Flag to add a pause behind a tone
#define SERROR              124     //!< Very low error sound.
#define SVLOW               247    //!< Low sound.
#define SLOW                1047   //!< Low sound.
#define SMEDIUM             2489   //!< Medium sound.
#define SHIGH               3520   //!< High sound.

/* Ischarging */
#define NOT_CONNECTED       0       //!< Not connect USB cable(VBUS).
#define CHARGING            1       //!< Charging
#define CHARGING_FULL       2       //!< Charge completed
#define CHARGING_ERROR      3
#define CHARGING_INIT       4      //!< Charging Not initialized
#define CHARGING_RESUME     5      //!< Resume charging at next poll
#define CHARGING_DISABLED   NOT_CONNECTED  

#define TEMP_OUT_OF_RANGE   CHARGING_ERROR       //!< Connected but not able to charge for various reasons.

/* UsbStatus Return codes */
#define USB_OK                  0
#define USB_ERR                 -1
#define USB_ERR_NOTREADY        -2
#define USB_NOT_CONNECTED		-3
#define USB_ERR_VCPNOTOPEN		-4
#define USB_ERR_BUSY			-5

#define USB_OPENED				USB_OK		// Legacy

/*
 * Return values for 'ExecuteMenuLabel()'
 */
#define	ENTERING_MENU_MODE 		1
#define	EXITING_MENU_MODE		2
#define	INVALID_OPTION_READ		3
#define VALID_OPTION_READ		4
#define LABEL_IGNORED			5

/*
 *--------------------------------------
 *  Memory/file functions
 *--------------------------------------
 */
/*  file system */
#define D_NAME              8
#define D_EXT               3
#define MAX_FNAME			13

#define ERR_NOT_SUPPORTED   (-2)
#define ERR_PARAMETER       (-3)
#define ERR_NOT_INSERTED    (-4)
#define ERR_NOT_FORMAT      (-5)

#ifndef O_BINARY
	#define O_BINARY        0x00    /*   binary file                 */
#endif

/*
 *--------------------------------------
 * Serial communication functions
 *--------------------------------------
 */
typedef enum
{
    COM0  = 0 ,      //!< USB-VCP (OPN2001 mode)
    COM1  = 1 ,      //!< USB-CDC (OPN2001 mode)
	COM2  = 2 ,      //!< RS232 / Cradle
    COM7  = 7 ,      //!< USB-CDC (OptiConnect)
	COM8  = 8 ,      //!< USB-CDC    
    COM9  = 9 ,      //!< USB-VCP
    COM10 = 10,      //!< USB-HID
    COM11 = 11,      //!< USB-MSD
    COM12 = 12,      //!< Bluetooth HID (No whitelist)
    COM13 = 13,      //!< Bluetooth HID (With whitelist)
    COM14 = 14,		 //!< Bluetooth LE UART Service (No whitelist)
    COM15 = 15,		 //!< Bluetooth LE UART Service (With whitelist)
    COM16 = 16,		 //!< Bluetooth LE OptiConnect Service (No whitelist)
    COM17 = 17,		 //!< Bluetooth LE OptiConnect Service (With whitelist)
	COM18 = 18,		 //!< Bluetooth LE UART Service (No whitelist) (OptiConnect Service disabled)
    COM19 = 19,		 //!< Bluetooth LE UART Service (With whitelist) (OptiConnect Service disabled)
    COM99 = 99,		 //!< Scanner UART (for debug only)
}ComPort_t;

#define IsUsbPort(x)		(x == COM0 || x == COM1 || x == COM7 || x == COM8 ||  x == COM9 || x == COM10 || x == COM11)
#define IsUsbSerialPort(x)	(x == COM0 || x == COM1 || x == COM7 || x == COM8 ||  x == COM9)
#define IsUsbCdcPort(x)		(x == COM1 || x == COM7 || x == COM8)
#define IsUsbVcpPort(x)		(x == COM0 || x == COM9)
#define IsUsbHidPort(x)		(x == COM10)
#define IsBluetoothPort(x)		(x == COM12 || x == COM13 || x == COM14 || x == COM15 || x == COM16 || x == COM17 || x == COM18  || x == COM19)
#define IsBluetoothSerialPort(x)(x == COM14 || x == COM15 || x == COM16 || x == COM17 || x == COM18 || x == COM19)
#define IsBluetoothHidPort(x)	(x == COM12 || x == COM13)
#define IsOptiConnectPort(x)	(x == COM7 || x == COM16 || x == COM17)
#define IsBleSppPort(x)			(x == COM14 || x == COM15 || x == COM18 || x == COM19)



/*
 * definitions for xmodem receive
 */
#define XMR_OK              0
#define XMR_CREATE_ERR      1
#define XMR_POLL_ERR        2
#define XMR_ABORT_ERR       3
#define XMR_BLOCK_ERR       4
#define XMR_CRC_ERR         5
#define XMR_CHECKSUM_ERR    6
#define XMR_WRITE_ERR       7
#define XMR_RESPONSE_ERR    8
#define XMR_MEM_ERR         9

/*
 * definitions for xmodem transmit
 */
#define XMT_OK              0
#define XMT_OPEN_ERR        1
#define XMT_RESPONSE_ERR    2
#define XMT_POLL_ERR        3
#define XMT_READ_ERR        4
#define XMT_ACK_ERR1        5
#define XMT_ACK_ERR2        6
#define XMT_MEM_ERR         7

#define ERR_USER_BREAK      -3
#define ERR_EOT_HOST        -5
#define ERR_HOST_REQ        -6
#define ERR_OPEN_FILE       -8
#define ERR_WRITE_FILE      -9
#define ERR_READ_FILE       -10
#define ERR_OUT_SYNC        -12
#define ERR_ALOC_MEM        -13
#define ERR_BCC_FAIL        -2          // checksum error (internal)
#define ERR_TIME_OUT        -4          // time out error (internal)
#define ERR_HOST_LOST       -7          // time out on first char (internal)
#define ERR_NAK_RECV        -11         // NAK received from host (internal)


/*
 ***********************************************************************
 *  Type Define
 ***********************************************************************
 */
struct time
{
    unsigned char ti_hour;
    unsigned char ti_min;
    unsigned char ti_sec;
};

struct date
{
    unsigned short  da_year;
    unsigned char da_mon;
    unsigned char da_day;
};

struct barcode
{
    char *text;
    int length;
    int id;
	int quantity;
    int min;
    int max;
	struct date date;
	struct time time;
};

struct ffblk
{
    char name[D_NAME+1];
    char ext[D_EXT+1];
    int filesize;
    char search_name[D_NAME+1];
    char search_ext[D_EXT+1];
    int slot;
    int drive;
};

// Barcode / scanner functions
void ScannerPower(int mode, int time);
int ReadBarcode(struct barcode *barcodep);

/** @brief Returns if the scanner is on/off.
 *
 *  @retval TRUE(1)     Scanner is on.
 *  @retval FALSE(0)    Scanner is off.
 */
int IsScannerOn(void);

/** @brief Returns if the scanner is off/on.
 *
 *  @retval TRUE(1)     Scanner is off.
 *  @retval FALSE(0)    Scanner is on.
 */
int IsScannerOff(void);

/** @brief Turns the aiming of the scanner on.
 *
 *  @return void
 */
void AimingOn(void);

/** @brief Turns the aiming of the scanner on.
 *
 *  @return void
 */
void AimingOff(void);

/** @brief Returns if aiming is on/off.
 *
 *  @retval TRUE(1)     Scanner is on.
 *  @retval FALSE(0)    Scanner is off.
 */
int IsAimingOn(void);

void ResetReadTime(void);
int SystemSettingsMemorizing(int on_off);
int SystemSetting(const char *cstring);
int ExecuteSystemsetting(const char *option);
int ExecuteMenuLabel(struct barcode *code);
int GetStoredCmds(char* cmd_string);

/*
 * Battery functions
 */

/** @brief Returns if the battery voltage in mV */
int GetBatteryVoltage(void);

/** @brief Returns if the battery percentage (0...100) */
int GetBatteryLevel(void);

/** @brief Returns if the battery capacity in mAh */
int GetBatteryCapacity(void);

/** @brief Returns if the charging current in mA */
int GetBatteryCurrent(void);

/** @brief Returns if the battery temperature in C */
int GetBatteryTemperature(void);

/** @brief Returns if the battery is nearly depleted.
 *
 * @retval TRUE     Battery is low
 * @retval FALSE    Battery is not low.
 */
int IsBatteryLow(void);

/** @brief This will set the date and the time which will be stored in the RTC. It will also check the time if it is possible.
 *
 * @param[in] datep     Pointer to date structure where the date will be stored.
 * @param[in] timep     Pointer to date structure where the time will be stored.
 *
 * @retval  OK(0)       Date And Time are set.
 * @retval  ERROR(-1)   Failed to set the time and date.
 */
int SetDateTime(struct date *datep, struct time *timep);

/** @brief This will get the date and the time. It will also check the time if it is possible.
 *
 * @param[out] datep    Pointer to date structure where the date will be stored.
 * @param[out] timep    Pointer to time structure where the time will be stored.
 *
 * @retval OK(0)       Date And Time in structures are correct.
 * @retval ERROR(-1)    Failed to get a possible time and date.
 */
int GetDateTime(struct date *datep, struct time *timep);

/** @brief Sets the current time
 *
 * @param[in] timep     Pointer to time structure holding the current time.
 *
 * @retval OK(0)        Time in successfully set.
 * @retval ERROR(-1)    Failed to set the time.
 */
int SetTime(struct time *timep);

/** @brief Returns the current time.
 *
 * @param[out] timep    Pointer to time structure where the time will be stored.
 *
 * @retval OK(0)        Time in structure is correct.
 * @retval ERROR(-1)    Failed to get a possible time.
 */
int GetTime(struct time *timep);

/** @brief Sets the current date
 *
 * @param[in] datep     Pointer to date structure holding the current date.
 *
 * @retval OK(0)        Date in successfully set.
 * @retval ERROR(-1)    Failed to set the date.
 */
int SetDate(struct date *datep);

/** @brief Returns the current date which.
 *
 * @param[out] datep    Pointer to date structure where the time will be stored.
 *
 * @retval  OK(0)       Date in structure is correct.
 * @retval  ERROR(-1)   Failed to get a possible date.
 */
int GetDate(struct date *datep);

/** @brief Checks if the specified time is possible.
 *
 *  @param[in] timep    Pointer to time structure holding the time to be checked
 *
 * @retval OK(0)        Time in structure is correct.
 * @retval ERROR(-1)    The time in the structure is wrong.
 */
int CheckTime(struct time *timep);

/** @brief Checks if the specified date is possible.
 *
 *  @param[in] datep    Pointer to date structure holding the date to be checked
 *
 * @retval OK(0)        Date in structure is correct.
 * @retval ERROR(-1)    The date in the structure is wrong
 */
int CheckDate(struct date *datep);

/** @brief Converts a date/time into seconds since Unix Epoch (=1/1/1970)
 *
 *  @param[in] date    Pointer to structure holding the date
 *  @param[in] time    Pointer to structure holding the time
 *
 *  @retval unsigned long Number of seconds since Unix Epoch
 */
unsigned long SecondsSinceEPOCH(void);

//------------------------------------------------------------------
//  SetAlarm
//  =====================
//  Sets alarm on/off.
//------------------------------------------------------------------
void SetAlarm(struct time *timep, unsigned short onoff);

//------------------------------------------------------------------
//  GetAlarmTime
//  =====================
//  Gets the current alarm time, returns ERROR if no alarm is set
//  else it returns OK
//------------------------------------------------------------------
int GetAlarm(struct date *datep, struct time *timep);

//------------------------------------------------------------------
//  IsAlarmOn
//  =====================
//  Returns whether an alarm has been set
//  Note: After the alarm has occured, the function returns FALSE
//------------------------------------------------------------------
int IsAlarmOn(void);

//------------------------------------------------------------------
//  DateTimeToTimeStamp
//  =====================
//  Converts a date/time to a 4-byte timestamp
//------------------------------------------------------------------
void DateTimeToTimeStamp(struct date *pDate, struct time *pTime, unsigned char *timestamp);

//------------------------------------------------------------------
//  TimeStampToDateTime
//  =====================
//  Converts a 4-byte timestamp to a date/time
//------------------------------------------------------------------
void TimeStampToDateTime(unsigned char *timestamp, struct date *pDate, struct time *pTime);

/** @brief  The function that controls the auto power down functionality
 *
 *  @param[in] what \ref AutoPowerDownDef
 *  @param[in] time The time before the terminal powers down automatically in \ref HEARTBEAT_PERIOD periods.
 *
 *  @return void
 */
void AutoPowerDown(int what, unsigned long time);

/** @brief  Switches to software standby mode
 *
 *  @param[in] deepsleep  Turns off all pheripherals, including the scan engine
 *
 *  @return void
 */
void PowerOff(int deepsleep);

/** @brief Switches the processor into sleep mode to save power. Any interrupt will wake the processor.
 *
 *  @return void
 */
void Idle(void);

/** @brief Returns the charging state
 *
 *  @return  \ref IsChargingDef
 */
int IsCharging(void);

// Timer functions

/** @brief This function starts the time-out timer.
 *
 *  @param[in] timevalue This is the time in steps of 20 milliseconds.
 *
 *  @return void
 */
void StartTimer(unsigned int timevalue);

/** @brief This function checks if the time specified with the StartTimer() function has elapsed
 *
 *  @retval FALSE (0) The specified time has not yet elapsed.
 *  @retval TRUE  (1) The specified time has elapsed.
 */
int EndTimer(void);

/** @brief Returns the time that is left of the time that was specified with
 * the starttimer() function. The time is specified in units of 20 milliseconds.
 *
 *  @return Remaining timer value in units of 20 milliseconds.
 */
unsigned int TimerValue(void);

/** @brief Returns the nr of heartbeat ticks
 *
 *  @return Nr of heartbeat ticks.
 */
unsigned long GetSystemTime(void);

/** @brief Sets the system timer
 *
 *  @param[in] value Value to set the system timer.
 *
 *  @return void
 */
void SetSystemTime(unsigned long value);

/** @brief Returns the nr of heartbeat ticks
 *
 *  @return Nr of heartbeat ticks
 */
unsigned int GetTickCount(void);

/** @brief  Sets the system timer
 *
 *  @param[in] value Value to set the sytem timer
 *
 *  @return void
 */
void SetTickCount(unsigned int value);

/** @brief Delay function in 20ms steps (includes power saving)
 *
 *  @param[in] time The delay time in 20ms steps.
 *
 *  @return void
 */
void Delay(int time);

/** @brief Delays execution for n miliseconds (software loop)
 *
 * @return void
 */
void SoftwareDelayMs(unsigned short ms);

/** @brief Delays execution for n miliseconds (includes power saving)
 *
 * @return void
 */
void DelayMs(unsigned short ms);

/** @brief Controls the red, green and blue LED's
 *
 *  @param color Which led should be switched on see \ref GoodRead_Leds
 *  @param time How long should the led be on in periods of \ref HEARTBEAT_PERIOD ms. \ref GoodRead_Time
 *
 *  @return void
 */
void GoodReadLed(int rbg, int time);

/** @brief Shows the red, green and blue LED's with the configured good read indicator time (default 200ms)
 *
 *  @param color Which led should be switched on \ref GoodRead_Leds
 *
 *  @return void
 */
void ShowGoodReadLed(uint32_t color );

/** @brief Configures the goodreadled color
 *
 *  @param color (rbg)
 *
 *  @return void
 */
int SetGoodReadIndicator(int color);

/** @brief Returns which LED is currently affected by goodreadled() without PWM
 *  Also returns the LEDs that are flashing, but are currently in the off state.
 *
 *  @return int A mask of the defined in \ref GoodRead_Leds.
 */

int IsGoodReadActive(void);

/** @brief Plays a series of beeps.
 *
 *  @param time      Length of the sound to be made. Use the \ref SoundToneDef definitions.
 *  @param vol       Volume level of the sound in a range of 1 to 100. Use the \ref SoundVolumeDef definitions.
 *  @param ...       Sequence of sound tones. Use a combination (upto 15) of \ref SoundSequenceDef definitions.
 *
 *  @return void
 */
void Sound(int time, uint8_t vol, ...);

/** @brief Plays a series of beeps from an array of notes
 *
 *  @param time      Length of the sound to be made. Use the \ref SoundToneDef definitions.
 *  @param vol       Volume level of the sound in a range of 1 to 100. Use the \ref SoundVolumeDef definitions.
 *  @param notes     Sequence of sound tones
 *  @param notes_len Nr. of notes
 *
 *  @return void
 */
void MidiSound(int time, int vol, int octave_offset, ...);

/**  @brief Checks if the buzzer is ON or OFF.
 *
 *  @retval TRUE(1)   When the buzzer is ON
 *  @retval FALSE(0)   When the buzzer is OFF
 */
int IsBuzzerOn(void);

/** @brief Returns the state of the trigger key.
 *
 *  @retval TRUE     If trigger key is pressed.
 *  @retval FALSE    If trigger key is not pressed.
 */
int TriggerPressed(void);

/** @brief Returns the state of the clear key.
 *
 *  @retval TRUE     If clear key is pressed.
 *  @retval FALSE    If clear key is not pressed.
 */
int ClearKeyPressed(void);

/** @brief Returns the state of the external trigger key.
 *
 *  @retval TRUE     If external trigger key is pressed.
 *  @retval FALSE    If external trigger key is not pressed.
 */
int ExtTriggerPressed(void);

/** @brief Puts the new key in the buffer and clears the EOF error.
 *
 *  @param[in] key Key to put into keybuffer.
 *
 *  @return void
 */
void KeyIn(unsigned short key);

/** @brief  Reads a key from the keyboard buffer. Used by the low-level read function.
 *
 *  @return Key from the keyboard buffer
 */
int KeyOut(void);

/** @brief Clears the keyboard buffer
 *
 *  @return void
 */
void ResetKey(void);

/** @brief Clears the keyboard buffer
 *
 *  @return true when a key was hit.
 */
unsigned int kbhit(void);

/** @brief Holds execution until a key is pressed.
 *
 *  @return void
 */
int WaitKey(void);

// Key functions
#define UpPressed		ClearKeyPressed
#define DownPressed		ClearKeyPressed

int ClearKeyAsTrigger(int enabled);

char *GetBootVersion(void);
char *GetOsVersion(void);
char *GetApplVersion(void);
void VersionOs(char os[10]); // legacy
unsigned int GetTerminalId(void);
void SetTerminalId(unsigned int id);

/** @brief Sound an beep which is associated with a good read (as configured with menu options)
 *
 *  @return void
 */
void OkBeep(void);
void OkBeep_adv(uint8_t volume, uint8_t vibrate);

/** @brief Sound an beep which is associated with a memorizing good read (as configured with menu options)
 *
 *  @return void
 */
void MemorizeBeep(void);

/** @brief Sound an beep to indicate successfully deleted barcode\ (as configured with menu options)
 *
 *  @return void
 */
void RemoveBeep(void);

/** @brief Sound an beep to indicate a warning
 *
 *  @return void
 */
void WarningBeep(void);

/** @brief Turns on the vibrating engine for a specific time (in \ref HEARTBEAT_PERIOD period)
 *
 *  @return void
 */
void Vibrate(int time);
void VibrateForced(int time); // Ignores vibrate enabled setting

/** @brief Sound an beep which is associated with an error.
 *
 *  @return void
 */
void ErrBeep(void);
void ErrBeep_adv(int ignore_disable);

/** @brief Sound an beep which is associated with an click.
 *
 *  @return void
 */
void ClickBeep(void);

/** @brief Sound an beep which is associated with a key press.
 *
 *  @return void
 */
void KeyBeep(void);

char *GetSerialNumber_str(void);
long GetSerialNumber(void);

unsigned long CoreLeft(void);			// Returns memory left on the file system
unsigned long Heap_CoreLeft(void);		// Returns memory left on Heap

/** @brief Open the selected communication port
 *
 *  @param[in] port         Communication port to use. Use one from \ref PortDef
 *
 *  @retval OK(0)           Successfully opened the communication port.
 *  @retval ERROR(-1)       Failed to open the communication port.
 */
int ComOpen(int port);  

/** @brief Switches the power off of the selected port
 *
 *  @param[in] port         Communication port to use. Use one from \ref PortDef
 *
 *  @retval OK(0)           Successfully closed all the communication ports.
 *  @retval ERROR(-1)       Failed to closed all the communication ports.
 */
int ComClose(int port);

/** @brief Sends a character out via the currently active port.
 *
 *  @param[in] data         Data to send.
 *
 *  @retval OK(0)           Successfully transmitted the data.
 *  @retval ERROR(-1)       Failed to transmitted the data.
 */
int PutCom(uint8_t data);

/** @brief Sends a character out via the selected port
 *
 *  @param[in] port         Port to send data. Use one from \ref PortDef
 *  @param[in] data         Data to send.
 *
 *  @retval OK(0)           Successfully transmitted the data.
 *  @retval ERROR(-1)       Failed to transmitted the data.
 */
int PutCom2(int port, uint8_t data);

/** @brief Receives a character from the currently active port
 *
 *  @param[in] timeout      Period to check for new data on active port.
 *
 *  @return >=0             The received data.
 *  @retval ERROR(-1)       Failed to receive data.
 */
int GetCom(int timeout);

/** @brief  Receives a character from the selected port
 *
 *  @param[in] port          Port to try and receive data. Use one from \ref PortDef
 *  @param[in] timeout       Period to check for new data on active port.
 *
 *  @retval >=0              The received data.
 *  @retval ERROR(-1)        Failed to receive data.
 */
int GetCom2(int port, int timeout);

/** @brief Clears the receive buffer from the (currently active) port.
 *
 *  @return void
 */
void ResetReceive(void);
void ResetReceive2(int port);

/** @brief Sends a string out via the currently active port.
 *
 *  @param[in] string       Pointer to string to transmit.
 *
 *  @retval OK(0)           Successfully transmitted the string.
 *  @retval ERROR(-1)       Failed to transmitted the string.
 */
int PutString(const char *string);

/** @brief Sends a string out via the selected port.
 *
 *  @param[in] port      Port to try and transmit data. Use one from \ref PortDef
 *  @param[in] string       Pointer to string to transmit.
 *
 *  @return void
 */
int PutnString(const char *string, unsigned int length);
int PutString2(int port, const char * string);
int PutnString2(int port, const char* string, unsigned int len);
int PutBarcode(int port, const struct barcode *code);

int PutAck(void);
int PutNak(void);
int PutAck2(int port);
int PutNak(void);
int PutNak2(int port);
int PutAckNak(int port, uint8_t ack_nak);

/** @brief Sets the level of the DTR line.
 *
 *  @param[in] on_off   \a ON or \a OFF
 *
 */
void SetDTR(int on_off);

/** @brief Sets the level of the RTS line.
 *
 *  @param[in] on_off   \a ON or \a OFF
 */
void SetRTS(int on_off);

/** @brief Sets the level of the RING line.
 *
 *  @param[in] on_off   \a ON or \a OFF
 */
void SetRING(int on_off);

/** @brief Returns the level of the CTS line.
 *
 *  @retval \a ON   The CTS line is active.
 *  @retval \a OFF  The CTS line is not active.
 */
int GetCTS(void);

/** @brief Returns the level of the DSR line.
 *
 *  @retval \a ON   The DSR line is active.
 *  @retval \a OFF  The DSR line is not active.
 */
int GetDSR(void);


/*******************************************************************************
 * DEFINES
 *******************************************************************************/
#define APPL_MAX_CMD_SIZE 4

/*******************************************************************************
 * TYPEDEFS
 *******************************************************************************/
typedef struct
{
    char    name[APPL_MAX_CMD_SIZE];
    void*    variable;
    unsigned char clear_mask;
    unsigned char set_mask;
	bool dont_store;			// if true, then these menu options are not stored in the array of stored commands (usually options that use printf's or beep)
}OPTION;


/*******************************************************************************
 * PROTOTYPES
 *******************************************************************************/
int AddMenuOptionTable( const OPTION menu_table[], int nr_of_entries );
int ApplicationSettingsMemorizing(int on_off, void * application_settings, int size, char * match_string);

// USB functions
int UsbStatus(void);

// Returns TRUE if the USB cable is physically connected, FALSE otherwise
int UsbIsPowered(void);

// Returns TRUE if the USB connection is open
int UsbIsConnected(void);

/** @brief Checks whether the device was powered down.
 *
 * @retval TRUE     Device was powered down in the mean time.
 * @retval FALSE    Device wasn't powered down in the mean time.
 */
int PoweredDown(void);


uint8_t IsColdBoot(void);		// True is the reset was caused by a power off
uint8_t IsSoftReset(void);		// True is the reset was caused by a software reset

void Application_ShutdownCallback(ret_func_ptr callback);

// File and memory functions
//int findnext(struct ffblk *ffblk);
//int findfirst(const char *fname,struct ffblk *ffblk);
int format(void);
size_t fsize(const char *filename);
//long lseek(int handle, long offset, int orgin);
//long tell(int handle);
int chsize(int handle, long size);

// Interrupts
void DisableInts(void);
void EnableInts(void);

// Heartbeat
int InstallHeartbeatHandler( heartbeat_function userfunction );
int RemoveHeartbeatHandler( heartbeat_function userfunction );

typedef int (*func_ptr_serial_cmd)(int port, uint8_t ch);
typedef int(*serial_command_exe_function)(int port, uint8_t * data, uint16_t len); //!< Typedef of the serial command execute handlers.

#define FORWARD_TO_OS		1
#define FORWARD_TO_APPL		2
#define FORWARD_TO_ALL		3

int InstallSerialCommandHandler(func_ptr_serial_cmd function, serial_command_exe_function exe_function, uint8_t *result_buf, uint8_t forward);

// Output functions
void PutByte(unsigned char c);
void PutShort(unsigned short c);
void PutLong(unsigned long l);

// Reset
void SoftReset(void);

// Bluetooth function
char *GetBltAddress(void);                      // Legacy: has no effect on BLE
void SetBltAddress(const char *address);        // Legacy: has no effect on BLE
char *GetBltLocalName( void );
void SetBltLocalName(const char *localname );
char *GetBltLocalAddress(void);
void SetBltLocalAddress(const char *address );
uint16_t GetOpcServiceId(void);


void SetBltSppServiceUUIDs(const uint8_t *service_uuid, const uint8_t *tx_char_uuid, const uint8_t *rx_char_uuid);

int BltIsConnected(void);
void BltUnpair(void);
int BltGetRssi(void);
unsigned char GetBltConnectableTime(void);		// Conntectable time in seconds
unsigned char GetBltAutoReconnectTime(void);	// Reonntectable time in seconds
void SetBltAuthMode(int bt_options);

// defines for SetBltAuthMode
#define BT_AUTH_DISABLE         0x00
#define BT_AUTH_AUTO            0x01    // Note:  AUTO = interface dependend.
#define BT_AUTH_ENABLE          0x02

// Defines for OseComm()
#define CONNECTION_ABORT		0x0100		// Physical connection removed
#define CRADLE_ABORT			CONNECTION_ABORT
#define ANY_KEYS_ABORT			0x0200
#define CONNECTION_CLOSE		0x0400		// Connection closed


// Return values for OseComm()
#define COMM_OK					(0)		// Communication OK
#define ERR_PR_NO_RESPONSE		(-1)	// No response
#define ERR_PR_CRC				(-2)	// CRC-16 error
#define ERR_PR_CMD_MISMATCH		(-3)	// Wrong command received
#define ERR_PR_FRAME_LENGTH		(-4)	// Frame length error
#define ERR_PR_SESSION_ID		(-5)	// Session ID mismatch
#define ERR_PR_SENDING_FRAME	(-6)	// Error sending frame
#define ERR_PR_OPEN_FILE		(-7)	// Cannot open or create the requested file
#define ERR_PR_READ_FROM_FILE	(-8)	// Cannot read from file
#define ERR_PR_WRITE_TO_FILE	(-9)	// cannot write to file
#define ERR_PR_WRONG_BLOCK		(-10)	// Requested the wrong block number
#define ERR_PR_FILE_SIZE		(-11)	// Error in filesizes
#define ERR_PR_POLL_COMMAND		(-12)	// unexpected poll command
#define ERR_PR_FILE_UNAVAILABLE	(-13)	// File cannot be found on the terminal
#define ERR_PR_LINE_BUSY		(-14)	// An other terminal is already communicating
#define ERR_PR_TIME_DATE		(-15)	// Error in the time or date when it is being set

#define ERR_PR_USER_ABORT		(-20)	// User aborted the protocol.

#define SUCC_COMPLETE			1		// 

// Defines for STATUS that are used in the callback
#define STAT_GET_TIME_DATE		1
#define STAT_SET_TIME_DATE		2
#define STAT_GET_OS_VERSION		3
#define STAT_GET_APPL_VERSION	4
#define STAT_XMIT_FILE_TO_PC	5
#define STAT_RECV_FILE_FROM_PC	6
#define STAT_LIST_FILES			7

// Callback function prototype
typedef void (*pOnOseCommInfo)( int status, int errorsuccess, int progress, const char *info );

//-------------------------------------------------------------------------------------------------------
// long OseComm( long sessionID, int terminateKey, const char *appVersion, pOnOseCommInfo OnOseCommInfo )
//
// Blocking function that wait for the PC to start the communication
// 
// long sessionID -		communication identification number 
//						valid range from 0 til 16777215 ( 0x000000 til 0xFFFFFF )
//						if sessionID matches -1 the a sessionID is generated by the terminal per session
//
// int terminateKey -	terminate key, when this key is pressed the protocol is aborted
// 
// const char *appVersion - A string holding the application version. when no version string
//							or an empty version string is used the terminal will send an unknown
//							to a get application version command.
//
//	pOnOseCommInfo OnOseCommInfo - Callback function that gets the status, error and progress information
//
// return:	COMM_OK
//			ERR_PR_USER_ABORT
//
//-------------------------------------------------------------------------------------------------------
long OseComm( long sessionID, int terminateKey, const char *appVersion, pOnOseCommInfo OnOseCommInfo );

// Returns the stored systemsettings string from flash or ram
int StoredCommands_Retrieve(char **string, char from_flash);

#endif		// __LIB_H__
