#include "parser.h"

#include <stdio.h>
#include <string.h>


void parse_lod(const char filename[]) {
	FILE *fp;
	struct lod_gheader header;
	struct lod_dir_entry dir_entry;
	unsigned long curr_lod_pos;


	fp = fopen(filename,"rb");

	read_lod_header(fp, &header);
	printf("id:   %.4s\n", header.id);
	printf("game: %.9s\n", header.game);
	printf("dir:  %.16s\n", header.dir);
	printf("num_files:  %ld\n", header.num_files);
	printf("--------------------------------------\n");

	curr_lod_pos = header.dir_start;
	for (int i_lod_entry = 0; i_lod_entry < header.num_files; i_lod_entry ++) {
		read_lod_dir_entry(fp, curr_lod_pos, &dir_entry);
		//printf("%.16s  pos %ld offset %ld, length %ld\n", dir_entry.name, curr_lod_pos, dir_entry.start_offset, dir_entry.length);
		printf("Reading %.16s\n", dir_entry.name);

		char *ext = strrchr(dir_entry.name, '.');
		if (!ext) {
			printf("!! No extension, file skipped\n");
			goto next_entry;
		}


		if (!strcmp(ext, ".blv")) {
			read_blv(fp, curr_lod_pos, &dir_entry);
			//break; //DEBUG

		} else {
			//TODO: dlv odm ddm
			printf("!! Unknown extension, file skipped\n");
			goto next_entry;
		}

next_entry:
		curr_lod_pos += sizeof(struct lod_dir_entry);
	}

	fclose(fp);
}


void read_lod_header(FILE *fp, struct lod_gheader *p_header) {
	fseek(fp, 0L, SEEK_SET);
	fread(p_header, sizeof(struct lod_gheader), 1, fp);
}


void read_lod_dir_entry(FILE *fp, unsigned long pos, struct lod_dir_entry *p_dir_entry) {
	fseek(fp, pos, SEEK_SET);
	fread(p_dir_entry, sizeof(struct lod_dir_entry), 1, fp);
}

void read_blv(FILE *fp, unsigned long curr_pos, struct lod_dir_entry *p_dir_entry) {
	struct blv_compressed_blv6_header blv_header;
	//printf("pos: %lu, start_offset: %lu\n", pos, p_dir_entry->start_offset);

	//unsigned long tot_offset = curr_pos + p_dir_entry->start_offset;
	unsigned long tot_offset = sizeof(struct lod_gheader) + p_dir_entry->start_offset;
	fseek(fp, tot_offset, SEEK_SET);
	fread(&blv_header, sizeof(blv_header), 1, fp);

	// printf("curr_pos: %lu, start_offset: %lu. tot_offset: %lu \n", 
	// 	curr_pos, p_dir_entry->start_offset, tot_offset);
	// printf("compressed_size: %lu, uncompressed_size: %lu\n", 
	// 	blv_header.compressed_size, blv_header.uncompressed_size);

/*
	for (long i_offset=-50000; i_offset < 50000; i_offset += 1) {
		unsigned long my_offset = curr_pos + p_dir_entry->start_offset + i_offset;
		fseek(fp, my_offset, SEEK_SET);

		fread(&blv_header, 2*sizeof(unsigned long), 1, fp);
		//printf("compressed_size: %ld, uncompressed_size: %ld\n", 
		//	blv_header.compressed_size, blv_header.uncompressed_size);

		if (blv_header.compressed_size > 0 && 
			blv_header.compressed_size < blv_header.uncompressed_size &&
			blv_header.compressed_size < 1000*1000 &&
			blv_header.uncompressed_size < 1000*1000*10 ) {
			printf("OK!! my_offset: %lu; compressed_size: %ld, uncompressed_size: %ld\n", 
			my_offset, blv_header.compressed_size, blv_header.uncompressed_size);
			//break;
		}
	}

*/
}
