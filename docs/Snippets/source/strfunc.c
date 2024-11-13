//
// strfunc.c
//
// Special string functions
//
// Martin Jansen
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strfunc.h"


//
// Remove all specific Trailing character(s) from the source input string
//
char *TrimRight( char *pszSource, char removeChar )
{
	char *ptr = pszSource;

	if( ptr && *ptr != '\0' )
	{
		// set ptr to last place
		ptr += strlen( pszSource ) -1;

		// find last non-specific character
		while(( ptr >= pszSource) && ( *ptr  == removeChar ))
			*ptr-- = '\0';
	}
	return pszSource;
}

//
// Remove all specific Leading character(s) from the source input string
//
char *TrimLeft( char *pszSource, char removeChar )
{
	char *ptr = pszSource;

	if( ptr && *ptr )
	{
		// find first non-specific character
		while( *ptr == removeChar )
			ptr++;

		if( ptr != pszSource)
			memmove( pszSource, ptr, ( strlen( ptr )+1));
	}
	return pszSource;

}

//
// Remove all specific Leading and Trailing character(s) from the source input string
//
char *Trim( char *pszSource, char removeChar )
{
	if( TrimRight( pszSource, removeChar) == NULL )
		return NULL;
	return TrimLeft( pszSource, removeChar);
}

//
// Center a string
// IMPORTANT: the input string should hold enough space for the size
//
char *Center( char *pszSource, int nSize )
{
	int nLength = (int)strlen( pszSource );
	int nPos;
	if( nLength >= nSize )
		return pszSource;

	nPos = nSize - nLength;
	nPos >>= 1; // divide by 2

	memmove( pszSource + nPos, pszSource, nLength +1);
	for( nLength = 0; nLength < nPos; nLength++ )
		pszSource[ nLength ] = ' ';
	return pszSource;
}
