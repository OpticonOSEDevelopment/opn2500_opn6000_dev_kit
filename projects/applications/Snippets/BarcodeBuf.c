#include "lib.h"
#include "BarcodeBuf.h"

// Locals
static char bcr_buf[MAX_BARCODE_LENGTH];

struct barcode code = {0};


int InitBarcodeBuf(int min_code_size, int max_barcode_size)
{
	code.min = min_code_size;
	code.max = max_barcode_size;
	code.text = bcr_buf;
	return OK;
}

