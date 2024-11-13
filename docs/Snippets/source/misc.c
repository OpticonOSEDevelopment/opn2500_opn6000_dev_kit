//
// misc.c
//
// Miscellaneous functions from the snippets section in the C-library kit
//
// Martin Jansen
//

#include "lib.h"


//
// Returns the current day of the week
// 0 = Sunday 
// 1 = Monday 
// 2 = Tuesday 
// 3 = Wednesday 
// 4 = Thursday 
// 5 = Friday 
// 6 = Saterday 
//
int WhatDayOfWeek( void )
{
	int m, y, a;
	struct date curr_date = {0};

	GetDate( &curr_date );
	a = (14 - curr_date.da_mon ) / 12;
	y = curr_date.da_year - a;
	m = curr_date.da_mon + (12*a) - 2;

	return((curr_date.da_day + y + (y/4) - (y/100) + (y/400) + ((31*m)/12))%7 );
}

//
// Returns the current week number
//
short GetWeekNumber( void )
{
	unsigned long ulF, ulS, ulT;
	unsigned long m, y, a;
	unsigned long ulJDN;
	struct date d = {0};

	GetDate( &d );

	a = (14UL - d.da_mon ) / 12UL;
	y = d.da_year + 4800UL - a;
	m = d.da_mon + (12UL*a) - 3UL;

	ulJDN = d.da_day + (( 153UL * m + 2UL) / 5UL) + (365UL * y) + (y / 4UL) - (y / 100UL) + (y / 400UL) - 32045UL;

	ulF = ( ulJDN + 31741UL - ( ulJDN % 7UL )) % 146097UL % 36524UL % 1461UL;
	ulS = ulF / 1460UL;
	ulT = (( ulF - ulS ) % 365UL) + ulS;

	return (short)((ulT / 7UL) + 1UL);
}
