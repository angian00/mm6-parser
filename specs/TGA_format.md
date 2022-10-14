# .TGA format
This describes the .TGA file format for Might and Magic 6 as contained within its .LOD files. This is not the standard TARGA file format, which commonly has a file extension of .TGA.

## Structure
Compressed TGA files start with the 48-byte MM6 Compressed Header. Normally they contain one image in 4 resolutions (Level of Detail). The images are 8-bit palletized.

The image width and height, as stored in the header, must be multiples of 8. The four images have dimensions of

* `width x height`
* `width/2 x height/2`
* `width/4 x height/4`
* `width/8 x height/8`

The image data immediately follows the header as zlib compressed stream. All images are simply concatenated without any additional data inbetween, so the first (largest) image is at offset 0 in the uncompressed buffer, while the second image is at offset `width x height`, for example 262.144 for a 512x512 (base-)image. Palette is stored uncompressed and goes right after the image data. It consists of RGB triples:

	typedef struct
	{
	  UBYTE   Red;
	  UBYTE   Green;
	  UBYTE   Blue;

	} H3RGBTriple;
