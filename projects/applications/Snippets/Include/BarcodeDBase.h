//
// BarcodeDBase.h
//
// Low-level Barcode Database functions
//
// Ronny de Winter & Martin Jansen
//

#ifndef __BARCODEDBASE__
#define __BARCODEDBASE__

// include is needed to get the IX_DESC value
#include "index.h"

//
// Error code defines, these should match the fix database error code
//
#define DATABASE_OK			(0)			// OK message
#define ERR_OPEN_DATABASE	(-1)		// Cannot open the database file
#define ERR_OPEN_INDEX		(-2)		// cannot open/create the index file
#define	ERR_NOT_OPEN		(-3)		// Error Database is not open
#define	ERR_EMPTY			(-4)		// Error Database is empty
#define ERR_INDEX_DUPLICATE	(-5)		// Duplicate key found in index while not allowed
#define ERR_INDEX_KEY_NF	(-6)		// Key has not been found.
#define ERR_PATH_NOT_FOUND	(-7)		// Folder does not exist

#define ERR_DB_READ			(-3)
#define ERR_DB_WRITE		(-4)
#define ERR_INDEX_READ		(-10)		// Cannot read from index file
#define ERR_INDEX_WRITE		(-11)		// Cannot write to index file

#define ERR_DB_WRONG_FORMAT (-12)		// Fix size database has the wrong size

#ifndef HAS_2D_ENGINE
#define MAX_RECORD_SIZE			300
#else
#define MAX_RECORD_SIZE			2500
#endif

extern char g_Record[MAX_RECORD_SIZE];

#define	QNT_OPT_ALLOW_DUPLICATES		0x01
#define	QNT_OPT_KEEP_ZERO_QNTY			0x02
#define	QNT_OPT_ALLOW_NEGATIVE_QNTY		0x04
#define	QNT_OPT_FAST_OPEN				0x08	// Reduces start-up time when having >1024 barcodes in memory. Enable when it's not required to keep exact track of the number of barcodes in memory or when deleting of barcodes is not used

//
// This is really the handle that is returned by open/create database
//
typedef struct
{
	FIL *fdDb;					// database file descriptor
	IX_DESC ix;					// holds the indexing values
	uint8_t quantity_options;
}SDBOutVal;

int IsBarcodeDatabaseOpen(SDBOutVal *dbFile);
int OpenBarcodeDatabase(const char* filename, const char *idx_file, SDBOutVal *dbFile);
int CreateBarcodeDatabase(const char* filename, const char *idx_file, SDBOutVal *dbFile);
void CloseBarcodeDatabase(SDBOutVal *dbFile); 

int ReadBarcodeRecord(SDBOutVal *dbFile, ENTRY *e, struct barcode *pCode); 
int SearchBarcodeDatabase(SDBOutVal *dbFile, struct barcode *pCode, int origin, long offset, ENTRY **pFoundEntry); 
int ReadFirstBarcode(SDBOutVal *dbFile, struct barcode *pCode); 
int ReadLastEntry(SDBOutVal *dbFile, ENTRY *e);
int ReadLastBarcode(SDBOutVal *dbFile, struct barcode *pCode);
int ReadPreviousBarcode(SDBOutVal *dbFile, struct barcode *pCode); 
int ReadNextBarcode(SDBOutVal *dbFile, struct barcode *pCode); 


int ReadCurrentBarcode(SDBOutVal *dbFile, struct barcode *pCode);
int DeleteCurrentBarcode(SDBOutVal *dbFile); 

int WriteBarcode( SDBOutVal *dbFile, ENTRY *pEntry, struct barcode *pCode); 
long GetTotalRecords( SDBOutVal *dbFile );

int AppendRecord( SDBOutVal *dbFile, uint8_t *data, uint32_t data_len);

unsigned long hash(const char *str, int str_len);	// 32-bit hash key

#endif // __BARCODEDBASE__


