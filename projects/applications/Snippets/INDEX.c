//	INDEX.C
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

//#include "stdafx.h"

#include <stdlib.h>
#include <string.h>
#include "lib.h"
#include "ff.h"
#include "FileSystem.h"
#include "index.h"

#ifdef DEBUG
volatile int blocks_read = 0;
volatile int blocks_written = 0;
#endif

//
// file I/O for indexing routines
//
FRESULT CreatIf( const char *idxName, FIL *fp )
{
    return f_open(fp, idxName, FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
}


FRESULT OpenIf( const char *idxName, FIL *fp )
{
	return f_open(fp, idxName, FA_READ | FA_WRITE);
}

FRESULT CloseIf( FIL *fp )
{
    if(fp == NULL)
        return FR_INT_ERR;
	
    return f_close(fp);
}

int OpenIndex(const char *idxName, IX_DESC *pix)
{
    ENTRY e;
    UINT br;
	FSIZE_t offset = 0;

    if ( OpenIf(idxName, pix->ixfile) != FR_OK )
        return (IX_FAIL);

    pix->cache_offset   = -1;
    pix->cache_size     = 0;
    pix->cache_idx      = 0;
    pix->logical_entries = 0;
	pix->total_records = 0;
	pix->last_recptr = 0;

	DWORD current_size = f_size(pix->ixfile);

	if( (current_size % ENT_SIZE) != 0 )	// This should not be possible, because everything is ENT_SIZE aligned
		return IX_FAIL;

	// Backwards compatiblity....increase index file size if needed, but keep the data
	if(current_size == 0 || (current_size % FILE_ALLOC_SIZE) != 0 )
	{
		if( AllocateBlock(pix->ixfile, current_size) != IX_OK )
			return IX_FAIL;

		current_size = f_size(pix->ixfile);
	}

	/* allocated_bytes = physical file size (preallocated at MakeIndex) */
	pix->allocated_bytes = current_size;

    /* Scan entries of the current block to find first empty/unused slot */
	if(!pix->fast_open || pix->allocated_bytes <= FILE_ALLOC_SIZE)
	{
		offset = 0;
		pix->logical_entries = 0;
		pix->total_records = 0;
	}
	else	// We have more than 1 block of barcodes and we're not interested in keeping exact track of the amount of barcodes in memory
	{
		offset = pix->allocated_bytes - FILE_ALLOC_SIZE;
		pix->logical_entries = (offset / FILE_ALLOC_SIZE) * (FILE_ALLOC_SIZE / ENT_SIZE);
		pix->total_records = pix->logical_entries;
	}

	if (f_lseek(pix->ixfile, offset) != FR_OK)
		return IX_FAIL;

    while (offset + ENT_SIZE <= (FSIZE_t)pix->allocated_bytes)
    {
        if (f_read(pix->ixfile, &e, ENT_SIZE, &br) != FR_OK || br != ENT_SIZE)
            break;

		if( (e.length == 0 && e.key == 0) || (e.code_id == 0 && e.recptr == 0))	// This is a corrupt file, because this should not be possible
			return IX_FAIL;

		// Validate entry
		if((uint32_t)e.recptr == 0xFFFFFF || (e.key == 0xFFFFFFFF && e.length >= 0xFFF) )
			break;

        pix->logical_entries++;

		if(e.quantity != 0)
			pix->total_records++;

		pix->last_recptr = e.recptr + e.length;
		offset += ENT_SIZE;
    }
    
    LastKey(&e, pix);		// Loads cache with the last block
    return (IX_OK);
}

int CloseIndex( IX_DESC *pix)
{
	pix->cache_offset = -1;
    CloseIf( pix->ixfile );
    return (IX_OK);
}

int MakeIndex(const char *idxName, IX_DESC *pix)
{
    if (CreatIf(idxName, pix->ixfile) != FR_OK)
		return (IX_FAIL);

	pix->cache_offset = 0;				// Initialize empty cache
    pix->cache_size = 0;
    pix->cache_idx = 0;
    pix->logical_entries = 0;
	pix->total_records = 0;
	pix->last_recptr = 0;
	pix->allocated_bytes = 0;

    // Preallocate block of 16KB
	if(AllocateBlock(pix->ixfile, 0) != IX_OK)
		return IX_FAIL;

	pix->allocated_bytes = FILE_ALLOC_SIZE;

	f_lseek(pix->ixfile, 0); // New file so we start writing at the begin of the file
    return IX_OK;
}

int AllocateBlock(FIL *ixfile, int offset)
{
    const size_t IDX_CHUNK_SIZE = 512;
    UINT bw;
    BYTE buffer[IDX_CHUNK_SIZE];

    if (CoreLeft() < FILE_ALLOC_SIZE)
        return IX_FAIL;

    // Fill the buffer with 0xFF
    memset(buffer, 0xFF, IDX_CHUNK_SIZE);

    // Compute the next FILE_ALLOC_SIZE boundary
    size_t newSize = (offset == 0) ? FILE_ALLOC_SIZE : ((offset + FILE_ALLOC_SIZE - 1 ) / FILE_ALLOC_SIZE) * FILE_ALLOC_SIZE;

    // Get current file size
    FSIZE_t fileSize = f_size(ixfile);

    if (fileSize >= newSize)
	{   // Already large enough, nothing to do
        return IX_OK;
    }
    
   	//debug_printf("Allocate: %s %d %d", (ixfile == &mIdxFile) ? "IDX" : "DAT", fileSize, newSize);

    // Seek to current end of file
    f_lseek(ixfile, fileSize);

	size_t remaining = newSize - fileSize;

	while (remaining) 
	{
		UINT toWrite = (remaining > IDX_CHUNK_SIZE) ? IDX_CHUNK_SIZE : remaining;
		
		if (f_write(ixfile, buffer, toWrite, &bw) != FR_OK || bw != toWrite)
			return IX_FAIL;
		remaining -= toWrite;
	}

	f_sync(ixfile); // Ensure data is flushed
    return IX_OK;
}


//
// position at start/end of index routines
//

int FirstKey( ENTRY *pe, IX_DESC *pix)
{
	if(pix->cache_offset == 0)						// If cache is pointing to first block, read from cache
	{
		if(pix->cache_size < 1)						// No key available -> EOF
			return (IX_FAIL);

		pix->cache_idx = 0;				// Set pointer to first key
		memcpy(pe, pix->cache, ENT_SIZE);
		return (IX_OK);
	}

	if (f_lseek(pix->ixfile, 0) == FR_OK)
	{
        if( f_read(pix->ixfile, pix->cache, ENT_SIZE * CACHE_SIZE, &pix->cache_size) == FR_OK && pix->cache_size >= ENT_SIZE )
		{
#ifdef DEBUG
			++blocks_read;
#endif
            pix->cache_size /= ENT_SIZE;
			pix->cache_offset = 0;
			memcpy(pe, pix->cache, ENT_SIZE);
			pix->cache_idx = 0;				// Set pointer to last read key
			return (IX_OK);
		}
	}

	return (IX_FAIL);
}


int LastKey( ENTRY *pe, IX_DESC *pix)
{
	if(pix->cache_offset >= 0 && pix->cache_size != CACHE_SIZE)		// If cache is pointing to last block, read from cache
	{
		if(pix->cache_size < 1)										// No keys available -> EOF
			return (IX_FAIL);

		pix->cache_idx = pix->cache_size - 1;			// Set pointer to last read key
		memcpy(pe, &pix->cache[pix->cache_idx], ENT_SIZE);
		return (IX_OK);
	}

	if (f_lseek(pix->ixfile, pix->logical_entries * ENT_SIZE) == FR_OK)
	{
		pix->cache_offset = f_tell(pix->ixfile);			// Set offset to file size

		if((pix->cache_offset % ENT_SIZE) != 0)			// If no valid index file
		{
			pix->cache_offset = -1L;		// Invalidate cache
			return (IX_FAIL);
		}
		
		if(pix->cache_offset >= (int)ENT_SIZE)				// If index has atleast 1 key
		{ 
			pix->cache_size = (pix->cache_offset/ENT_SIZE) % CACHE_SIZE;

			if( pix->cache_size == 0 )
				pix->cache_size = CACHE_SIZE;

            // RDW TO BE TESTED
			if (f_lseek(pix->ixfile, pix->cache_offset - (long)(pix->cache_size * ENT_SIZE)) == FR_OK)
			{
                UINT bytes_read;

				if( f_read(pix->ixfile, pix->cache, ENT_SIZE * pix->cache_size, &bytes_read) == FR_OK && (ENT_SIZE * pix->cache_size) == bytes_read)
				{
#ifdef DEBUG
					++blocks_read;
#endif
					pix->cache_offset -= (pix->cache_size * ENT_SIZE);			// Set offset to beginning of cached block
					pix->cache_idx = pix->cache_size - 1;			// Set pointer to last read key
					memcpy(pe, &pix->cache[pix->cache_idx], ENT_SIZE);
					return (IX_OK);
				}
			}
		}
	}

	pix->cache_offset = -1L;		// Invalidate cache
	return (IX_FAIL);
}

//
// get next, previous entries
//
int CurrentKey( ENTRY *pe, IX_DESC *pix )
{
	if(pix->cache_offset >= 0)
	{
		memcpy(pe, &pix->cache[pix->cache_idx], ENT_SIZE);								// Read from cache
		return (IX_OK);
	}
	return (IX_FAIL);
}

//
// get next, previous entries
//
int NextKey( ENTRY *pe, IX_DESC *pix )
{
	if(pix->cache_offset >= 0)
	{
		if(pix->cache_idx + 1 >= pix->cache_size)	// Is there still a cached item available
		{
			int fsize = pix->logical_entries * ENT_SIZE;

			if( fsize < (pix->cache_offset + CACHE_SIZE_IN_BYTES))	// End of file
				return (IX_FAIL);

			pix->cache_offset += CACHE_SIZE_IN_BYTES;			// If not, fill cache with next block
			pix->cache_idx = 0;									// Set pointer to first key in cache

#ifdef DEBUG
			++blocks_read;
#endif
			int read_size = MIN(CACHE_SIZE_IN_BYTES, fsize - pix->cache_offset);

			if( f_lseek(pix->ixfile, pix->cache_offset) != FR_OK 
				|| f_read(pix->ixfile, pix->cache, read_size, &pix->cache_size) != FR_OK 
					|| pix->cache_size < ENT_SIZE )											// No next key available -> EOF
			{
				pix->cache_offset = -1L;			// Invalidate cache
				return (IX_FAIL);
			}

            pix->cache_size /= ENT_SIZE;
		}
		else
		{
			++pix->cache_idx;												// Move pointer to next key
		}

		memcpy(pe, &pix->cache[pix->cache_idx], ENT_SIZE);					// Read from cache
		return (IX_OK);
	}

	return (IX_FAIL);
}

int PrevKey( ENTRY *pe, IX_DESC *pix)
{
	if(pix->cache_offset >= 0)
	{
		if(pix->cache_idx < 1)				// Is there no cached item available? If not, fill cache with previous block
		{
			if(pix->cache_offset == 0)						// If no previous block -> SOF
				return (IX_FAIL);

			pix->cache_offset -= CACHE_SIZE_IN_BYTES;
			pix->cache_idx = 0;

			if ( f_lseek(pix->ixfile, pix->cache_offset) != FR_OK 
                    || f_read(pix->ixfile, pix->cache, ENT_SIZE * CACHE_SIZE, &pix->cache_size) != FR_OK 
						|| pix->cache_size < ENT_SIZE )
			{
				pix->cache_offset = -1L;			// Invalidate cache
				return (IX_FAIL);
			}
#ifdef DEBUG
			++blocks_read;
#endif
			pix->cache_size /= ENT_SIZE;
			pix->cache_idx = pix->cache_size - 1;	// Set pointer to last key in cache
		}
		else
		{
			--pix->cache_idx;						// Move pointer to previous key
		}

		memcpy(pe, &pix->cache[pix->cache_idx], ENT_SIZE);		// Read from cache
		return (IX_OK);
	}

	return (IX_FAIL);
}

ENTRY *FindFirstKey( ENTRY *pe, IX_DESC *pix )
{
	static ENTRY r_pe;

	if( FirstKey( &r_pe, pix ) == (IX_OK) )
	{
		do
		{
			if( r_pe.quantity != 0 && r_pe.code_id == pe->code_id && r_pe.key == pe->key )
				return &r_pe;
		} while ( NextKey( &r_pe, pix ) == (IX_OK) );
	}

	return NULL;
}

ENTRY *FindLastKey( ENTRY *pe, IX_DESC *pix )
{
	static ENTRY r_pe;

	if( LastKey( &r_pe, pix ) == (IX_OK) )
	{
		do
		{
			if( r_pe.quantity != 0 && r_pe.code_id == pe->code_id && r_pe.key == pe->key )
				return &r_pe;
		} while ( PrevKey( &r_pe, pix ) == (IX_OK) );
	}


	return NULL;
}

ENTRY *FindNextKey( ENTRY *pe, IX_DESC *pix )
{
	static ENTRY r_pe;

	while ( NextKey( &r_pe, pix ) == (IX_OK) )
	{
		if( r_pe.quantity != 0 && r_pe.code_id == pe->code_id && r_pe.key == pe->key )
			return &r_pe;
	}


	return NULL;
}

ENTRY *FindPrevKey( ENTRY *pe, IX_DESC *pix )
{
	static ENTRY r_pe;

	while ( PrevKey( &r_pe, pix ) == (IX_OK) )
	{
		if( r_pe.quantity != 0 && r_pe.code_id == pe->code_id && r_pe.key == pe->key )
			return &r_pe;
	}

	return NULL;
}

int AddKey( ENTRY *pe, IX_DESC *pix )
{
    UINT bytes_written;

	DWORD offset = pix->logical_entries * ENT_SIZE;

    // Check if the logical write crosses the allocated region
    if (offset + ENT_SIZE > pix->allocated_bytes)
    {
		// Allocate next 64-KB chunk
		if(AllocateBlock(pix->ixfile, offset + ENT_SIZE) != IX_OK)
			return IX_FAIL;
	                
        pix->allocated_bytes = f_size(pix->ixfile);
    }

	if (f_lseek(pix->ixfile, pix->logical_entries * ENT_SIZE) == FR_OK)
	{
		if(pix->cache_offset >= 0)		// If cache is initialized
		{
			if((UINT)(pix->cache_offset + (long)(pix->cache_size * ENT_SIZE)) == f_tell(pix->ixfile))		// If cache reaches end of the file
			{
				if(pix->cache_size == CACHE_SIZE)				// If cache is full
				{
					pix->cache_size = 0;						// Start a new cache
					pix->cache_offset += CACHE_SIZE_IN_BYTES;
				}

				pix->cache_size += 1;
				pix->cache_idx = pix->cache_size - 1;		// Add key to the end of cache
				memcpy(&pix->cache[pix->cache_idx], pe, ENT_SIZE);
			}
		}
		else if(f_tell(pix->ixfile) == 0)
		{
			pix->cache_idx = 0;
			pix->cache_size = 1;						// Start a new cache
			pix->cache_offset = 0;
			memcpy(&pix->cache[pix->cache_idx], pe, ENT_SIZE);
		}
				
		if (f_write(pix->ixfile, pe, ENT_SIZE, &bytes_written) == FR_OK && bytes_written == ENT_SIZE)		// Write key to file
		{
			pix->logical_entries++;
			pix->last_recptr = pe->recptr + pe->length;
			f_sync(pix->ixfile);
#ifdef DEBUG
			++blocks_written;
#endif
            return (IX_OK);
		}
}

	pix->cache_offset = -1L;		// Invalidate cache
	return (IX_FAIL);
}

int UpdateKey( ENTRY *pe, IX_DESC *pix )
{
	UINT bytes_written;

	if(pix->cache_offset >= 0)		// If cache is initialized
	{
		memcpy(&pix->cache[pix->cache_idx], pe, ENT_SIZE);	// Update cache
		
		if (f_lseek(pix->ixfile, pix->cache_offset + (pix->cache_idx * ENT_SIZE)) == FR_OK)
		{
			if (f_write(pix->ixfile, pe, ENT_SIZE, &bytes_written) == FR_OK && bytes_written == ENT_SIZE)		// Write key to file
				return (IX_OK);
		}
	}
	
	return (IX_FAIL);
}

int GetTotalKeys( IX_DESC *pix )
{
	if(pix->ixfile == NULL)
		return 0;

	return pix->logical_entries;
}


