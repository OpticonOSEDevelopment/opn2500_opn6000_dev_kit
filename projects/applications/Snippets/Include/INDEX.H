//	INDEX.H
//
//	Version: 0.1
//
//	---------------------------------------------------------------------------
//	File indexing routines
//			by
//	Ronny de Winter (Opticon Sensors Europe BV)
//	---------------------------------------------------------------------------
//
//	Copyright (c) 2016
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any damages
//  arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source distribution.
//
//


#ifndef ___INDEX__H___
#define ___INDEX__H___

#if defined(__cplusplus)
#define ANSI_C
extern "C"{
#endif
#include <setjmp.h>

//
// macros, constants, data types
//
#define IX_OK       1
#define IX_FAIL     0

typedef long RECPOS;
typedef unsigned long HASHKEY;

typedef struct						// entry structure in index       
{  
    RECPOS   recptr :24;			// points to data record 
    uint8_t code_id;
	HASHKEY  key;					// hash key             
	uint16_t length;
	int16_t quantity;
	uint8_t timestamp[4];
}  __attribute__((__packed__ )) ENTRY;

#define	 ENT_SIZE		sizeof(ENTRY)

#define	 CACHE_SIZE_IN_BYTES		512		// For optimal performance use flash block size of the file system
											// CACHE_SIZE must be devidable by ENT_SIZE

#define  CACHE_SIZE  (CACHE_SIZE_IN_BYTES/ENT_SIZE)

#define FILE_ALLOC_SIZE				(16 * 1024)		// We allocate chucks of 16kb for files to reduce writes to the FAT table

typedef struct						// in-memory index descriptor 
{  
	FIL 	*ixfile;				// file descriptor
    int		duplicate;				// no duplicate keys if 0        
	ENTRY   cache[CACHE_SIZE];						// Used to speed up reading (especially search from bottom to top)
	UINT    cache_idx;								// Pointer to current position in cache
	int		cache_offset;							// Offset of cache block in index file
	UINT	cache_size;								// Size of current cache block

	DWORD	logical_entries;			// Number of valid entries in the file
    DWORD	allocated_bytes;			// Total allocated file size of index file (64k, 128k, etc)
	DWORD	allocated_dbase_bytes;		// Total allocated file size of database file (64k, 128k, etc)
	DWORD	last_recptr;				// Contains the end-of-file of the database, because we can't ise f_size anymore
} IX_DESC;

int  OpenIndex( const char *idxName, IX_DESC *pix, int dup);
int  MakeIndex( const char *idxName, IX_DESC *pix, int dup);
int  CloseIndex( IX_DESC *pix);

int  AllocateBlock(FIL *ixfile, int offset);

int  GetTotalKeys( IX_DESC *pix );

int  FirstKey( ENTRY *pe, IX_DESC *pix);
int  LastKey( ENTRY *pe, IX_DESC *pix);
int  NextKey( ENTRY *pe, IX_DESC *pix);
int  PrevKey( ENTRY *pe, IX_DESC *pix);
int  CurrentKey( ENTRY *pe, IX_DESC *pix);

#define FindKey		FindFirstKey

ENTRY *FindFirstKey( ENTRY *pe, IX_DESC *pix);
ENTRY *FindLastKey( ENTRY *pe, IX_DESC *pix);
ENTRY *FindPrevKey( ENTRY *pe, IX_DESC *pix);
ENTRY *FindNextKey( ENTRY *pe, IX_DESC *pix);

int  AddKey( ENTRY *pe, IX_DESC *pix);
int  UpdateKey( ENTRY *pe, IX_DESC *pix);

#if defined(__cplusplus)
}
#endif

#endif // ___INDEX__H___

