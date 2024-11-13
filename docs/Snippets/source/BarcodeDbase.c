
//
// Database.c(pp)
//
// Implementation Database functions that finally call the fix or the csv database functions
// 
// Martin Jansen
// 
//#include "stdafx.h"
#include <setjmp.h>
#include <string.h>
#include "lib.h"
#include "ff.h"
#include "INDEX.H"
#include "BarcodeDBase.h"

#if defined(WIN32)
#include <io.h>
#endif

char g_Record[MAX_RECORD_SIZE];

static bool IsValidQuantity(SDBOutVal *dbFile, int16_t quantity);
static int RebuildIndex(SDBOutVal *dbFile, const char *idxName );

int OpenBarcodeDatabase(const char* filename, const char *idx_file, SDBOutVal *dbFile) 
{
	if( f_open(dbFile->fdDb, filename, FA_READ | FA_WRITE) != FR_OK )
		return ERR_OPEN_DATABASE;

	if(idx_file != NULL && OpenIndex( idx_file, &(dbFile->ix), (dbFile->quantity_options & QNT_OPT_ALLOW_DUPLICATES) ) != IX_OK )
	{
		if(RebuildIndex(dbFile, idx_file ) != OK)
		{
			f_close( dbFile->fdDb );
			return ERR_OPEN_INDEX;	// Error creating the index file 
		}
	}

	dbFile->lTotalRecords = -1L;
	return DATABASE_OK;
}

int CreateBarcodeDatabase(const char* filename, const char *idx_file, SDBOutVal *dbFile) 
{
	if( f_open(dbFile->fdDb, filename, FA_CREATE_ALWAYS | FA_READ | FA_WRITE) != FR_OK )
		return ERR_OPEN_DATABASE;

	if( f_lseek(dbFile->fdDb, f_size(dbFile->fdDb)) != FR_OK )
		return ERR_OPEN_DATABASE;

	if(idx_file != NULL && MakeIndex( idx_file, &(dbFile->ix), (dbFile->quantity_options & QNT_OPT_ALLOW_DUPLICATES) ) != IX_OK )
	{
		f_close( dbFile->fdDb );
		return ERR_OPEN_INDEX;	// Error creating the index file 
	}

	dbFile->lTotalRecords = -1L;
	return DATABASE_OK;
}

void CloseBarcodeDatabase(SDBOutVal *dbFile) 
{
	// Close the index file
	if (dbFile->ix.ixfile != NULL)
		CloseIndex(&(dbFile->ix));

	// Close the database file
	f_close(dbFile->fdDb);
}

int IsBarcodeDatabaseOpen(SDBOutVal *dbFile) 
{
	if (dbFile->fdDb == NULL)
		return FALSE;

	if (dbFile->ix.ixfile == NULL)
		return FALSE;

	return TRUE;
}

int ReadBarcodeRecord(SDBOutVal *dbFile, ENTRY *e, struct barcode *pCode) 
{
	UINT bytes_read;

	pCode->length = e->length;
	pCode->id = e->code_id;
	pCode->quantity = e->quantity;

	TimeStampToDateTime(e->timestamp, &pCode->date, &pCode->time );

	if(pCode->text != NULL)
	{
		if( f_lseek(dbFile->fdDb, e->recptr) != FR_OK )
			return ERR_INDEX_KEY_NF; // TODO: maybe change this in some other value

		if( f_read(dbFile->fdDb, pCode->text, e->length, &bytes_read) != FR_OK || bytes_read != e->length )
			return ERR_INDEX_KEY_NF; // TODO: maybe change this in some other value
	}

	return DATABASE_OK;
}

int SearchBarcodeDatabase(SDBOutVal *dbFile, struct barcode *pCode, int origin, long offset, ENTRY **foundEntry ) 
{
	ENTRY e;

	*foundEntry = NULL;

	if (!IsBarcodeDatabaseOpen(dbFile))
		return ERROR;

	e.key = hash(pCode->text, pCode->length);
	e.code_id = pCode->id;
	e.quantity = pCode->quantity;

	switch(origin)
	{
		case SEEK_SET:
			if ( (*foundEntry = FindFirstKey(&e, &(dbFile->ix))) == NULL)
				return ERR_INDEX_KEY_NF;
			break;

		case SEEK_END:
			if ( (*foundEntry = FindLastKey(&e, &(dbFile->ix))) == NULL)
				return ERR_INDEX_KEY_NF;
			break;

		case SEEK_CUR:
			if(offset >= 0)
			{
				if ( (*foundEntry = FindNextKey(&e, &(dbFile->ix))) == NULL)
					return ERR_INDEX_KEY_NF;
			}
			else
			{
				if ( (*foundEntry = FindPrevKey(&e, &(dbFile->ix))) == NULL)
					return ERR_INDEX_KEY_NF;
			}
			break;

		default:
			return ERR_INDEX_KEY_NF;
	}

	return DATABASE_OK;
}

int ReadFirstBarcode(SDBOutVal *dbFile, struct barcode *pCode) 
{
	static ENTRY e;
	
	if (!IsBarcodeDatabaseOpen(dbFile))
		return ERROR;

	if (FirstKey(&e, &(dbFile->ix)) != IX_OK)
		return ERROR;

	do
	{
		if(IsValidQuantity(dbFile, e.quantity))
			return ReadBarcodeRecord(dbFile, &e, pCode);
	}
	while(NextKey(&e, &(dbFile->ix)) == IX_OK);

	return ERROR;
}

int ReadLastBarcode(SDBOutVal *dbFile, struct barcode *pCode) 
{
	static ENTRY e;
	
	if (!IsBarcodeDatabaseOpen(dbFile))
		return ERROR;

	if (LastKey(&e, &(dbFile->ix)) != IX_OK)
		return ERROR;

	do
	{
		if(IsValidQuantity(dbFile, e.quantity))
			return ReadBarcodeRecord(dbFile, &e, pCode);
	}
	while(PrevKey(&e, &(dbFile->ix)) == IX_OK);

	return ERROR;
}

int ReadPreviousBarcode(SDBOutVal *dbFile, struct barcode *pCode) 
{
	static ENTRY e;

	if (!IsBarcodeDatabaseOpen(dbFile))
		return ERROR;

	while (PrevKey(&e, &(dbFile->ix)) == IX_OK)
	{
		if(IsValidQuantity(dbFile, e.quantity))
			return ReadBarcodeRecord(dbFile, &e, pCode);
	}

	return ERROR;
}

int ReadNextBarcode(SDBOutVal *dbFile, struct barcode *pCode) 
{
	static ENTRY e;

	if (!IsBarcodeDatabaseOpen(dbFile))
		return ERROR;

	while (NextKey(&e, &(dbFile->ix)) == IX_OK)
	{
		if(IsValidQuantity(dbFile, e.quantity))
			return ReadBarcodeRecord(dbFile, &e, pCode);
	}		

	return ERROR;
}

int ReadCurrentBarcode(SDBOutVal *dbFile, struct barcode *pCode) 
{
	static ENTRY e;

	if (!IsBarcodeDatabaseOpen(dbFile))
		return ERROR;

	if (CurrentKey(&e, &(dbFile->ix)) != IX_OK)
		return ERROR;

	return ReadBarcodeRecord(dbFile, &e, pCode);
}

int DeleteCurrentBarcode(SDBOutVal *dbFile) 
{
	static ENTRY e;

	if (!IsBarcodeDatabaseOpen(dbFile))
		return ERROR;

	if (CurrentKey(&e, &(dbFile->ix)) != IX_OK)
		return ERROR;

	//if(e.quantity <= 0)
	//	return ERROR;

	e.quantity = 0;

	return WriteBarcode(dbFile, &e, NULL);
}

int WriteBarcode( SDBOutVal *dbFile, ENTRY *pEntry, struct barcode *pCode) 
{
	UINT length, bytes_written;
	ENTRY e = {0};
    
	if (dbFile->fdDb == NULL)
		return ERROR;

	if(pEntry == NULL)
	{
		length = pCode->length;

		e.key = hash(pCode->text, length);
		e.recptr =  f_size(dbFile->fdDb);
		e.length = length;
		e.quantity = pCode->quantity;
		e.code_id = pCode->id;
		DateTimeToTimeStamp(&pCode->date, &pCode->time, e.timestamp); // Update timestamp
	
		// Append to the end of the database
		if(f_lseek(dbFile->fdDb, e.recptr) != FR_OK)
			return ERR_DB_WRITE;

		if (f_write(dbFile->fdDb, pCode->text, length, &bytes_written ) != FR_OK || bytes_written != length)
			return ERR_DB_WRITE;
		
		if(AddKey(&e, &(dbFile->ix)) == IX_FAIL)
			return ERR_DB_WRITE;

		if(IsValidQuantity(dbFile, e.quantity))
			++dbFile->lTotalRecords;
	} 
	else 
	{
		bool isValid, wasValid = IsValidQuantity(dbFile, e.quantity);

		if(pCode == NULL)
		{
			--pEntry->quantity;
		}
		else
		{
			pEntry->quantity += pCode->quantity;

			if(pCode->quantity > 0)
				DateTimeToTimeStamp(&pCode->date, &pCode->time, pEntry->timestamp); // Update timestamp
		}

		isValid = IsValidQuantity(dbFile, e.quantity);

		// Overwrite the current key
		if(UpdateKey(pEntry, &(dbFile->ix)) == IX_FAIL)
			return ERR_DB_WRITE;

		if(isValid && !wasValid)
			++dbFile->lTotalRecords;
		else if(!isValid && wasValid)
			--dbFile->lTotalRecords;
	}

	return DATABASE_OK;
}


int AppendRecord( SDBOutVal *dbFile, uint8_t *data, uint32_t data_len) 
{
	UINT bytes_written;
	
	if (dbFile->fdDb == NULL)
		return ERROR;

	// Append to the end of the database
	//if(f_lseek(dbFile->fdDb, f_size(dbFile->fdDb)) != FR_OK)
	//	return ERR_DB_WRITE;

	if (f_write(dbFile->fdDb, data, data_len, &bytes_written ) != FR_OK || bytes_written != data_len)
		return ERR_DB_WRITE;

	return DATABASE_OK;
}


long GetTotalRecords( SDBOutVal *dbFile )
{
	ENTRY e;

	if(dbFile->lTotalRecords == -1L)
	{
		dbFile->lTotalRecords = 0;
		
		if (FirstKey(&e, &(dbFile->ix)) != IX_OK)
			return ERR_INDEX_KEY_NF;

		do
		{
			if(IsValidQuantity(dbFile, e.quantity))
			{
				++dbFile->lTotalRecords;
			}
		} 
		while(NextKey(&e, &(dbFile->ix)) == IX_OK);
	}

	return dbFile->lTotalRecords;
}

bool IsValidQuantity(SDBOutVal *dbFile, int16_t quantity)
{
	if(quantity > 0)
		return true;

	if(quantity == 0 && (dbFile->quantity_options & QNT_OPT_KEEP_ZERO_QNTY))
		return true;

	return (quantity < 0 && (dbFile->quantity_options & QNT_OPT_ALLOW_NEGATIVE_QNTY));
}

// adapted from String.hashCode()
unsigned long hash(const char *str, int str_len)		// 32-bit hash key
{
    unsigned long key = 5381;
    int i = str_len;

    while (i != 0)
	{
        key = ((key << 5) + key) + *(str++); /* hash * 33 + c */
		--i;
	}

    return key;
}

int RebuildIndex(SDBOutVal *dbFile, const char *idxName )
{
	static ENTRY e;
	struct date date;
	struct time time;
	int error;
	int length, recptr;

	if(dbFile->fdDb == NULL || dbFile->ix.ixfile == NULL)
		return ERR_OPEN_DATABASE;

	if( idxName == NULL )
		return ERR_OPEN_INDEX; 

	if( MakeIndex( idxName, &(dbFile->ix), (dbFile->quantity_options & QNT_OPT_ALLOW_DUPLICATES)) == IX_FAIL )
		return ERR_OPEN_INDEX; 

	error = OK;

	// We don't know the date & time, so we just set it to NOW
	GetDate(&date);	
	GetTime(&time);	
	DateTimeToTimeStamp(&date, &time, e.timestamp);

	recptr = 0;

	while( f_gets(g_Record, MAX_RECORD_SIZE, dbFile->fdDb ) != NULL )
	{
		length = strlen(g_Record);

		if(length == 0 || (length == 1 && g_Record[0] == '\n'))
			continue;
		
		e.key = hash(g_Record, length);
		e.recptr = recptr;
		e.length = length;
		e.code_id = CODE128;				// We don't know the code-id
		e.quantity = 1;						// We don't even know the quantity (deleted barcodes will be added!!!)
	
		if(AddKey(&e, &(dbFile->ix)) == IX_FAIL)
			return ERR_DB_WRITE;

		recptr = dbFile->fdDb->fptr;
	}

	return error;
}
