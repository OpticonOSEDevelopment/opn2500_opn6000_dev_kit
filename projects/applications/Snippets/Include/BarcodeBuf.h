/*******************************************************************************************************/
//
// File Name      : BarcodeBuf.h
// Author         : Ronny de Winter
//
/*******************************************************************************************************/
#ifndef _BARCODE_BUF__
#define _BARCODE_BUF__

#ifndef MAX_BARCODE_LENGTH
#ifdef OPN6000
#define MAX_BARCODE_LENGTH		3000
#else
#define MAX_BARCODE_LENGTH		250
#endif
#endif

extern struct barcode code;

int InitBarcodeBuf(int min_code_size, int max_barcode_size);


#endif // _BARCODE_BUF__
