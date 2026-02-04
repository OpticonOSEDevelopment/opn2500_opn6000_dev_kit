

#define DBASE_NAME		"SCANNED.DAT"
#define DBASE_IDX		"SCANNED.IDX"
#define DBASE_EXPORT	"SCANNED.TXT"	// Comment this line is you never make an export file, which will increase memory available for barcode storage		
										// Change it when your export file is named differently

#define ALLOW_NEGATIVES			0x01
#define KEEP_ZERO_QUANTITY		0x02
#define SUMMED_QUANTITIES		0x04

//------------------------------------------------------------------------------
//	Initialize storage with options
//	=====================
//-----------------------------------------------------------------------------
int InitStorage(uint8_t quantity_options);

//------------------------------------------------------------------------------
//	OpenStorage
//	=====================
//	Opens the database and creates a new one if it doesn't exist
//-----------------------------------------------------------------------------
int OpenStorage(void);

//------------------------------------------------------------------------------
//	CloseStorage
//	=====================
//	Closes the main database
//-----------------------------------------------------------------------------
int CloseStorage(void);

//------------------------------------------------------------------------------
//	DeleteStorage
//	=====================
//	Deletes all barcodes from memory
//-----------------------------------------------------------------------------
void DeleteStorage(void);

//------------------------------------------------------------------------------
//	DeleteInternalStorage
//	=====================
//	Deletes all internal storage files from memory (but not the export)
//-----------------------------------------------------------------------------
void DeleteInternalStorage(void);

//------------------------------------------------------------------------------
//	BarcodesInMemory
//	=====================
//	Returns the number of (untransmitted) barcodes in memory
//-----------------------------------------------------------------------------
int BarcodesInMemory(void);

int BarcodesInMemoryISR(void);	// For heartbeats

//------------------------------------------------------------------------------
//	DeleteBarcodeFromMemory
//	========================
//	Deletes the barcode that was last retrieved by 'ReadCurrentBarcodeFromMemory' or 'ReadNextBarcodeFromMemory'
//-----------------------------------------------------------------------------
int DeleteCurrentBarcodeFromMemory(void);

// Stores, delete, adds or removes the specified barcode with the given quantity
int UpdateBarcodeInMemory(struct barcode *pCode);

// Marks the current barcode as deleted (or decrease quantity)
int DeleteCurrentBarcodeFromMemory(void);

// Checks if last barcode in memory matches the specified barcode
int IsLastBarcodeInMemory(struct barcode *pCode);

// Find the last matching barcode in memory
int FindFirstBarcodeInMemory(struct barcode *pCode);

// Find the last matching barcode in memory
int FindLastBarcodeInMemory(struct barcode *pCode);

// Returns the first available (not-deleted) barcode from memory
int ReadCurrentBarcodeFromMemory(struct barcode *pCode);

// Returns the next available (not-deleted) barcode from memory 
int ReadNextBarcodeFromMemory(struct barcode *pCode);

// Returns the first (not deteled) barcode in memory
int ReadFirstBarcodeFromMemory(struct barcode *pCode);

// Returns the last (not-deleted) barcode in memory
int ReadLastBarcodeFromMemory(struct barcode *pCode);
