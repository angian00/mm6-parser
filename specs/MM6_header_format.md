# MM6 Compressed Header

Most of the files in Might and Magic 6 are zlib-compressed and have a small 48 byte header before the actual compressed data.

Here's the header format:

	struct CompressedHeader {
	  char fname[16];
	  unsigned long image0Size;
	  unsigned long compressedSize;
	  unsigned short width0;
	  unsigned short height0;
	  unsigned short unknown1[6];  // seems to be =0
	  unsigned long uncompressedSize;
	  unsigned long unknown2;      // seems to be =0
	}

This header is used for images/textures, as well as other files which don't contain bitmap data. `fname` is the ASCIIZ-filename of the compressed file. On image files, it is directly followed by the ASCIIZ-string "TGA", which, however, might just be an artefact from the compression. On images, `Image0Size` seems to be a copy of the field uncompressedSize, while on all other file types, `Image0Size` is zero.

`compressedSize` and `uncompressedSize` are the only meaningful fields for non-image files, they contain the size of the compressed data (directly following the header in the file, i.e. at offset 0x30) and the size for the uncompressed data, respectively. All other fields are zeroed out on non-image files.

Image files additionally have `width0` and `height0`, which contain the size of the first image in the file. See also .TGA for more info on how images are laid out in the uncompressed buffer.

Uncompressing such a compressed file is relatively simple: read the compressed and uncompressed sizes, allocate two buffers for the data, read the compressed data from offset 0x30 into the first buffer and call the zlib function uncompress() to uncompress the data into the second buffer.
