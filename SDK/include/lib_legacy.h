/***********************************************************************
 *
 *  Copyright (C) Opticon Sensors Europe B.V.
 *
 *  lib_legacy.h For OPN-2500 / OPN-6000
 *
 ***********************************************************************/

#ifndef __LIB_LEGACY_H__
#define __LIB_LEGACY_H__


#define SINGLE						SCAN_SINGLE
#define TRIGGER						SCAN_TRIGGER
#define CONTINUOUS					SCAN_CONTINUOUS

#define SUSPEND						SCAN_SUSPEND		//!< Suspend auto power down timer (16 to keep SUSPEND / ADP_SUSPEND / SCAN_SUSPEND the same to minimize backwards compatilbity issues)
#define RESUME						SCAN_RESUME			//!< Resume auto power down timer. (32 to keep RESUME / ADP_RESUMED / SCAN_RESUME the same to minimize backwards compatilbity issues)

//-------------------------------------------------------------------
//            Public Operating System functions
//-------------------------------------------------------------------

// Barcode / scanner functions
#define scannerpower                ScannerPower         // void scannerpower(int mode, int time);
#define readbarcode                 ReadBarcode          // int readbarcode(struct barcode *barcodep);
#define scanneroff                  IsScannerOff         // int scanneroff(void);

// Configuration function(s)
#define systemsetting               SystemSetting        // int systemsetting( const char *option_string );

// Battery & temperature functions
#define ischarging                  IsCharging           // int ischarging(void);

// Date & time functions
#define GetDate                     GetDate              // void getdate(struct date *dp);
#define gettime                     GetTime              // void gettime(struct time *tp);
#define checktime                   CheckTime            // int checktime(struct time *timep);
#define checkdate                   CheckDate            // int checkdate(struct date *datep);
#define setdate                     SetDate              // int setdate(struct date *dp);
#define settime                     SetTime              // int settime(struct time *timep);

// Timing functions
#define delay                       Delay                // void delay(int time);
#define starttimer                  StartTimer           // void starttimer(unsigned int timevalue);
#define endtimer                    EndTimer             // int endtimer(void);
#define timervalue                  TimerValue           // unsigned int timervalue(void);

// LED / Sound functions
#define goodreadled                 GoodReadLed          // void goodreadled(int on_off, int time);
#define sound                       Sound                // void sound(int time, int vol, ...);

// Key functions
#define triggerpressed              TriggerPressed       // int triggerpressed(void);
#define uppressed                   UpPressed            // int uppressed(void);
#define downpressed                 DownPressed          // int downpressed(void);
#define resetkey                    ResetKey             // void resetkey(void);

// Version & identification functions
#define getterminalid               GetTerminalId        // unsigned int getterminalid(void);
#define setterminalid               SetTerminalId        // void setterminalid(unsigned int id);
#define version_os					VersionOs

// Communication functions
#define comopen						ComOpen				// int comopen(unsigned int port);
#define comclose					ComClose			// void comclose(unsigned int port);
#define getcom						GetCom				// int getcom(int timeout);
#define GetCom2						GetCom2				// int GetCom2(int port, int timeout);
#define putcom						PutCom				// int putcom(int data);
#define putcom2						PutCom2				// int putcom(int port, int data);
#define resetreceive				ResetReceive		// void resetreceive(void);
#define setRTS						SetRTS				// void setRTS(int on_off);
#define setDTR						SetDTR				// void setDTR(int on_off);
#define getCTS						GetCTS				// int getCTS(void);
#define getDSR						GetDSR				// int getDSR(void);
#define PutBuffer                   PutnString2         // int PutBuffer(unsigned short port,const char *data, unsigned short len);

// Power management functions
#define idle						Idle				// void idle(void);
#define powereddown					PoweredDown			// int powereddown(void);
#define autopowerdown				AutoPowerDown		// void autopowerdown(int what, unsigned long time);


// File and memory functions
//int findnext(struct ffblk *ffblk);
//int findfirst(const char *fname,struct ffblk *ffblk);
//long tell(int handle);

#define tell						ftell					// long tell(int handle);
#define	lseek						fseek					// long lseek(int handle, long offset, int orgin);
#define coreleft 					CoreLeft				// unsigned long coreleft(void);	
#define far_malloc 					malloc					// void *far_malloc (unsigned long nbytes);

#define putstring					PutString				// int putstring(char *data);
#define putnstring					PutnString				// int putnstring(const char *data, int length);

#define blt_is_connected			BltIsConnected			// int blt_is_connected( void );
#define blt_unpair					BltDeletePairs			// void blt_unpair(void);

#define GetBatteryVoltage_mV		GetBatteryVoltage
#define System_SetShutdownCallback	Application_ShutdownCallback

#define SetBltPin					// Bluetooth LE doesn't support PIN

#define main						app_main				// void main(void)

#endif		// __LIB_LEGACY_H__
