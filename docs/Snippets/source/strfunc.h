//
// strfunc.h
//
// Special string functions
//
// Martin Jansen
//

#ifndef __STRFUNC_H__
#define __STRFUNC_H__

//
// Remove all specific Trailing character(s) from the source input string
//
char *TrimRight( char *pszSource, char removeChar );

//
// Remove all specific Leading character(s) from the source input string
//
char *TrimLeft( char *pszSource, char removeChar );

//
// Remove all specific Leading and Trailing character(s) from the source input string
//
char *Trim( char *pszSource, char removeChar );

//
// Center a string
// IMPORTANT: the input string should hold enough space for the size
//
char *Center( char *pszSource, int nSize );


#endif // __STRFUNC_H__
