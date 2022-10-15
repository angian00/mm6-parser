#include "tga.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <png.h>
#include <SDL2/SDL_image.h>

#include "compression.h"


//--------------------------------------------------------------
//--- Data structures
//--------------------------------------------------------------

struct px {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};


struct bitmap
{
    struct px *pixels;
    size_t width;
    size_t height;
};

struct tga_data
{
    uint32_t size;
    size_t width;
    size_t height;
    uint8_t *pixels;
    struct px palette[256];
};


//----   Gamefile data format  ----------

struct lod_bin_entry_header
{
    char fname[16];
    uint32_t image0_size;
    uint32_t compressed_size;
    uint16_t width0;
    uint16_t height0;

    uint16_t unknown1[6];  // seems to be =0
    uint32_t uncompressed_size;
    uint32_t unknown2;      // seems to be =0
};


//----------------------------------------------------------------------
//--- Internal function declarations
//----------------------------------------------------------------------

char *get_file_type(struct lod_bin_entry_header compr_header);

int tga2bitmap(struct tga_data *p_tga, struct bitmap *p_bitmap);
int bitmap2png(struct bitmap *p_bitmap, char *png_path);
int bitmap2bmp(struct bitmap *p_bitmap, char *bmp_path);

static struct px * pixel_at(struct bitmap *p_bitmap, int x, int y);
void print_palette(struct px *palette);
void print_pixel_indices(struct tga_data *p_tga);


//----------------------------------------------------------------------
//--- Implementation
//--------------------------------------------------------------


struct tga_data *tga_parse(char *raw_data)
{
	//printf("tga_parse\n");

	char *p_data = raw_data;

	struct lod_bin_entry_header compr_header;
	printf("lod_bin_entry_header size: %llu\n", sizeof(struct lod_bin_entry_header)); //
	
	memcpy(&compr_header, p_data, sizeof(struct lod_bin_entry_header));
	p_data += sizeof(struct lod_bin_entry_header);

	char *file_type = get_file_type(compr_header);
	assert(file_type && !strcmp(file_type, "TGA"));

	char compr_data[compr_header.compressed_size];
	memcpy(compr_data, p_data, compr_header.compressed_size);
	p_data += compr_header.compressed_size;


	struct tga_data *p_tga = malloc(sizeof(struct tga_data));
	
	p_tga->size = compr_header.uncompressed_size;
	p_tga->width = compr_header.width0;
	p_tga->height = compr_header.height0;

	printf("uncompressed_size: %d\n", compr_header.uncompressed_size);
	printf("compressed_size: %d\n", compr_header.compressed_size);
	printf("p_tga->width: %llu\n", p_tga->width);
	printf("p_tga->height: %llu\n", p_tga->height);

	p_tga->pixels = z_uncompress(compr_data, compr_header.compressed_size, compr_header.uncompressed_size);

	//TODO: parse multi-resolution images

	if (p_tga->size != p_tga->width * p_tga->height) {
		fprintf(stderr, "!! maybe a multiscale image \n");
		return NULL;
	}

	memcpy(p_tga->palette, p_data, 256 * sizeof(struct px));
	//print_palette(p_tga->palette);
	print_pixel_indices(p_tga);

	//p_data += 256 * sizeof(struct px); assert(p_data - raw_data == compr_header.uncompressed_size + ...);

	return p_tga;
}


void tga_export_png(char *out_path, struct tga_data *p_tga)
{
	printf("Saving image file to %s\n", out_path);

	struct bitmap bitmap;

/*
	printf("image0_size: %u \n", p_compr_header->image0_size);
	printf("compressed_size: %u \n", p_compr_header->compressed_size);
	printf("width0: %hu \n", p_compr_header->width0);
	printf("height0: %hu \n", p_compr_header->height0);
	printf("uncompressed_size: %u \n", p_compr_header->uncompressed_size);
	printf("n_large_tga: %d \n", n_large_tga);
	printf("\n");
*/

	if (tga2bitmap(p_tga, &bitmap))
		return;

	//printf("after tga2bitmap \n");
	if (bitmap2png(&bitmap, out_path))
		return;

	//print_palette(p_tga->palette);
	print_pixel_indices(p_tga);

	printf("Saved image file to %s\n", out_path);
}


void tga_export_bmp(char *out_path, struct tga_data *p_tga)
{
	printf("Saving image file to %s\n", out_path);

	struct bitmap bitmap;

	if (tga2bitmap(p_tga, &bitmap))
		return;

	//printf("after tga2bitmap \n");
	if (bitmap2bmp(&bitmap, out_path))
		return;

	printf("Saved image file to %s\n", out_path);
}


//--- Internal functions definition

char *get_file_type(struct lod_bin_entry_header compr_header)
{
	if (!strncmp(compr_header.fname + strlen(compr_header.fname) + 1, "TGA", 3))
		return "TGA";

	//TODO: check for other types

	return NULL;
}



int tga2bitmap(struct tga_data *p_tga, struct bitmap *p_bitmap) {
	int w = p_tga->width;
	int h = p_tga->height;

	p_bitmap->width  = w;
	p_bitmap->height = h;
	p_bitmap->pixels = malloc(sizeof(struct px) * w * h);

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			//lookup pixel data for (x,y) in palette
			//memcpy(pixel_at(p_bitmap, x, y), p_tga->palette + p_tga->buffer[w * y + x], sizeof(struct px));
			memcpy(pixel_at(p_bitmap, x, y), &p_tga->palette[(int)p_tga->pixels[w * y + x]], sizeof(struct px));
		}
	}

	return 0;
}



static struct px * pixel_at(struct bitmap *p_bitmap, int x, int y)
{
	return p_bitmap->pixels + p_bitmap->width * y + x;
}


int bitmap2png(struct bitmap *p_bitmap, char *png_path)
{
	FILE * fp;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	size_t x, y;

	png_byte ** row_pointers = NULL;

	int status = -1;
	int pixel_size = 3;
	int depth = 8;

	fp = fopen(png_path, "wb");
	if (!fp) {
		fprintf(stderr, "!! fopen_failed");
		goto fopen_failed;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fprintf(stderr, "!! png_create_write_struct_failed");
		goto png_create_write_struct_failed;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fprintf(stderr, "!! png_create_info_struct_failed");
		goto png_create_info_struct_failed;
	}

	if (setjmp(png_jmpbuf (png_ptr))) {
		fprintf(stderr, "!! png_failure");
		goto png_failure;
	}

	png_set_IHDR(
		png_ptr,
		info_ptr,
		p_bitmap->width,
		p_bitmap->height,
		depth,
		PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);


	row_pointers = png_malloc(png_ptr, p_bitmap->height * sizeof (png_byte *));

	for (y = 0; y < p_bitmap->height; y++) {
		png_byte *row = png_malloc(png_ptr, sizeof (uint8_t) * p_bitmap->width * pixel_size);
		row_pointers[y] = row;

		for (x = 0; x < p_bitmap->width; x++) {
			struct px * pixel = pixel_at(p_bitmap, x, y);
			*row++ = pixel->r;
			*row++ = pixel->g;
			*row++ = pixel->b;
		}
	}

	png_init_io(png_ptr, fp);
	png_set_rows(png_ptr, info_ptr, row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	status = 0; //OK!

	for (y = 0; y < p_bitmap->height; y++) {
		png_free(png_ptr, row_pointers[y]);
	}
	png_free(png_ptr, row_pointers);


png_failure:
png_create_info_struct_failed:
	png_destroy_write_struct(&png_ptr, &info_ptr);

png_create_write_struct_failed:
	fclose(fp);

fopen_failed:
	return status;
}

int bitmap2bmp(struct bitmap *p_bitmap, char *bmp_path)
{
	//FIXME: some rgb encoding error, maybe the RGBA masks

    SDL_Surface * image = SDL_CreateRGBSurfaceFrom(
		p_bitmap->pixels,                 // dest_buffer from CopyTo
		p_bitmap->width,        // in pixels
		p_bitmap->height,       // in pixels
		3 * 8,                  // in bits, so should be dest_depth * 8
		3 * p_bitmap->width,    // dest_row_span from CopyTo
		0xff0000,        // RGBA masks, see docs
		0x00ff00,
		0x0000ff,
		0x000000
	);
	SDL_SaveBMP(image, bmp_path);
	SDL_FreeSurface(image);

	return 0;
}


void print_palette(struct px *palette)
{
	for (int i=0; i < 256; i++) {
		printf("\t %02x %02x %02x\n", palette[i].r,palette[i].g, palette[i].b);
	}
}

void print_pixel_indices(struct tga_data *p_tga)
{
	int w = p_tga->width;
	int h = p_tga->height;

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			printf("\t (%d,%d) 0x%02x\n", x, y, p_tga->pixels[w * y + x]);
		}
	}

}
