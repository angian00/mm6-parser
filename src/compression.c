#include "compression.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <zlib.h>


//----------------------------------------------------------------------
//--- Implementation
//--------------------------------------------------------------

unsigned char *z_uncompress(unsigned char *in_buffer, int compressed_size, int uncompressed_size)
{
	//printf("z_uncompress \n");

	unsigned char *out_buffer = malloc(uncompressed_size);
	unsigned long read_len = uncompressed_size;

	uncompress(out_buffer, &read_len, in_buffer, compressed_size);
	assert(read_len == uncompressed_size);

	return out_buffer;
}
