//*****************************************************************************
//				File Name : memorize.c
//*****************************************************************************
//				Modified Information
//
//		Date   | Version  |					Contents
//  -----------+----------+----------------------------------------------------
//	    /  /   |          |
//  -----------+----------+----------------------------------------------------
//	2024/10/01 | FAxV0127 | OPN6000 / OPN2500
//
//*****************************************************************************
#include <string.h>
#include <stdlib.h>
#include "lib.h"
#include "ff.h"
#include "Debug.h"
#include "BarcodeDBase.h"
#include "memorize.h"

SDBOutVal dbFile;

static FIL mOutFile;
static FIL mIdxFile;

static int lExportFileSize = 0;

int InitStorage(uint8_t quantity_options)
{
	dbFile.quantity_options = quantity_options;

	if(IsBarcodeDatabaseOpen(&dbFile))		// Forces a reinit of the database 
		return CloseStorage();

	return OK;
}

//------------------------------------------------------------------------------
//	OpenStorage
//	=====================
//	Opens the database and creates a new one if it doesn't exist
//-----------------------------------------------------------------------------
int OpenStorage(void)
{
	if(IsBarcodeDatabaseOpen(&dbFile))		// If already open
		return OK;

	dbFile.fdDb = &mOutFile;
	dbFile.ix.ixfile = &mIdxFile;

	if(dbFile.quantity_options & QNT_OPT_FAST_OPEN)
		dbFile.ix.fast_open = TRUE;

	if(dbFile.quantity_options & QNT_OPT_FAST_READ)
		dbFile.ix.fast_read = TRUE;

#ifdef DBASE_EXPORT
	lExportFileSize = MAX(0, fsize(DBASE_EXPORT));
#endif

	if( OpenBarcodeDatabase((char*)DBASE_NAME, (char*)DBASE_IDX, &dbFile ) != DATABASE_OK)
	{
		if( CreateBarcodeDatabase((char*)DBASE_NAME, (char*)DBASE_IDX, &dbFile ) != DATABASE_OK )
		{
			return ERROR;
		}
	}

	return OK;
}

//------------------------------------------------------------------------------
//	CloseStorage
//	=====================
//	Closes the main database
//-----------------------------------------------------------------------------
int CloseStorage(void)
{
	if(IsBarcodeDatabaseOpen(&dbFile))
		CloseBarcodeDatabase( &dbFile );

	dbFile.fdDb = NULL;
	dbFile.ix.ixfile = NULL;
	return OK;
}

//------------------------------------------------------------------------------
//	DeleteInternalStorage
//	=====================
//	Deletes all internal storage files from memory (but not the export)
//-----------------------------------------------------------------------------
void DeleteInternalStorage(void)
{
	CloseStorage();				// Just to be sure

	remove(DBASE_NAME);
	remove(DBASE_IDX);
	remove(DBASE_EXPORT);
}

//------------------------------------------------------------------------------
//	DeleteStorage
//	=====================
//	Deletes all barcodes from memory
//-----------------------------------------------------------------------------
void DeleteStorage(void)
{
	CloseStorage();				// Just to be sure
	format();					// Much quicker than a file delete
}
		
//------------------------------------------------------------------------------
//	BarcodesInMemory
//	=====================
//	Returns number of (untransmitted) barcodes in memory
//-----------------------------------------------------------------------------
int BarcodesInMemory(void)
{
	if(OpenStorage() != OK)	// Re-opens if closed
		return ERROR;

	return BarcodesInMemoryISR();
}

int BarcodesInMemoryISR(void)
{
	int records = GetTotalRecords( &dbFile );

	return (records > 0) ? records : 0;
}

//------------------------------------------------------------------------------
//	UpdateBarcodeInMemory
//	=====================
//	Updates a barcode in the database file
//-----------------------------------------------------------------------------
int UpdateBarcodeInMemory(struct barcode *pCode)
{
	int ret = OK;
	ENTRY *foundEntry = NULL;;

	if(OpenStorage() != OK)	// Re-opens if closed
		return ERROR;

	if(CoreLeft() + lExportFileSize < f_size(dbFile.fdDb) + (f_size(dbFile.ix.ixfile)/16 * 30))	// 30/16, because index file is 16 bytes per record and maximum export overhead is 30 bytes
		return ERROR;

	if( !(dbFile.quantity_options & QNT_OPT_ALLOW_DUPLICATES) )	// if Store quantities
	{
		if( SearchBarcodeDatabase(&dbFile, pCode, SEEK_END, 0, &foundEntry) != DATABASE_OK)
		{
			// If not found and we don't allow negative quantities -> ERROR
			if(pCode->quantity < 0 && !(dbFile.quantity_options & QNT_OPT_ALLOW_NEGATIVE_QNTY))
				return ERROR;

			// fall through to writing new entry
		}
	}
	else
	{
		if(pCode->quantity < 0) // Negative quantity -> is remove -> find frst
		{
			if( SearchBarcodeDatabase(&dbFile, pCode, SEEK_END, 0, &foundEntry) != DATABASE_OK)
				return ERROR;	// If not found, can't delete
		}
	}

	if( WriteBarcode(&dbFile, foundEntry, pCode) != DATABASE_OK )
	{
		ret = ERROR;
	}
	// else : We don't flush/close the database on each barcode, since it's faster

	
	return ret;
}

//------------------------------------------------------------------------------
//	DeleteCurrentBarcodeFromMemory
//	========================
//	Deletes the barcode that was last retrieved by 'GetCurrentBarcodeFromMemory'
//-----------------------------------------------------------------------------
int DeleteCurrentBarcodeFromMemory(void)
{
	if(OpenStorage() != OK)	// Re-opens if closed
		return ERROR;

	if( DeleteCurrentBarcode( &dbFile ) < 0 )
		return ERROR;

	if(!BarcodesInMemory())
	{
		DeleteStorage();
	}
	
	return OK;
}

int IsLastBarcodeInMemory(struct barcode *pCode)
{
	ENTRY e = {0};

	if(OpenStorage() != OK)	// Re-opens if closed
		return ERROR;

	if (ReadLastEntry(&dbFile, &e) == DATABASE_OK)
	{
		HASHKEY hKey = hash(pCode->text, pCode->length);

		if(e.code_id == pCode->id && e.key == hKey)
			return true;
	}

	return false;
}

int FindFirstBarcodeInMemory(struct barcode *pCode)
{
	ENTRY *foundEntry;

	if(OpenStorage() != OK)	// Re-opens if closed
		return ERROR;

	if(SearchBarcodeDatabase(&dbFile, pCode, SEEK_SET, 0, &foundEntry) == DATABASE_OK)
	{
		do
		{
			if(foundEntry->quantity > 0)
				return OK;

		} while (SearchBarcodeDatabase(&dbFile, pCode, SEEK_CUR, 1, &foundEntry) == DATABASE_OK); // Search upwards
	}

	return ERROR;
}


int FindLastBarcodeInMemory(struct barcode *pCode)
{
	ENTRY *foundEntry;

	if(OpenStorage() != OK)	// Re-opens if closed
		return ERROR;

	if(SearchBarcodeDatabase(&dbFile, pCode, SEEK_END, 0, &foundEntry) == DATABASE_OK)
	{
		do
		{
			if(foundEntry->quantity > 0)
				return OK;

		} while (SearchBarcodeDatabase(&dbFile, pCode, SEEK_CUR, -1, &foundEntry) == DATABASE_OK); // Search upwards
	}

	return ERROR;
}

//------------------------------------------------------------------------------
//	ReadCurrentBarcodeFromMemory
//	========================
//	Returns the first available (not-deleted) barcode from memory
//-----------------------------------------------------------------------------
int ReadCurrentBarcodeFromMemory(struct barcode *pCode)
{
	if(OpenStorage() != OK)	// Re-opens if closed
		return ERROR;

	if( ReadCurrentBarcode(&dbFile, pCode) == DATABASE_OK )
	{
		return ERROR;
	}

	return OK;
}

//------------------------------------------------------------------------------
//	GetNexBarcodeFromMemory
//	========================
//	Returns the next available (not-deleted) barcode from memory
//-----------------------------------------------------------------------------
int ReadNextBarcodeFromMemory(struct barcode *pCode)
{
	if(OpenStorage() != OK)	// Re-opens if closed
		return ERROR;

	return (ReadNextBarcode(&dbFile, pCode) == DATABASE_OK) ? OK : ERROR;
}

int ReadFirstBarcodeFromMemory(struct barcode *pCode)
{
	if(OpenStorage() != OK)	// Re-opens if closed
		return ERROR;

	return (ReadFirstBarcode(&dbFile, pCode) == DATABASE_OK) ? OK : ERROR;
}

int ReadLastBarcodeFromMemory(struct barcode *pCode)
{
	if(OpenStorage() != OK)	// Re-opens if closed
		return ERROR;

	return (ReadLastBarcode(&dbFile, pCode) == DATABASE_OK) ? OK : ERROR;
}
