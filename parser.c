#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>


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
	for (unsigned int i_lod_entry = 0; i_lod_entry < header.num_files; i_lod_entry ++) {
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
	struct blv_compressed_blv6_header compr_header;
	//printf("pos: %lu, start_offset: %lu\n", pos, p_dir_entry->start_offset);

	//unsigned long tot_offset = curr_pos + p_dir_entry->start_offset;
	unsigned long tot_offset = sizeof(struct lod_gheader) + p_dir_entry->start_offset;
	fseek(fp, tot_offset, SEEK_SET);
	fread(&compr_header, sizeof(compr_header), 1, fp);

	//printf("compressed_size: %lu, uncompressed_size: %lu\n", 
	// 	compr_header.compressed_size, compr_header.uncompressed_size);

	unsigned char *compr_data = (unsigned char *)malloc(compr_header.compressed_size);
	fread(compr_data, (int)compr_header.compressed_size, 1, fp);

	void *blv_data = malloc(compr_header.uncompressed_size);
	unsigned long read_len = compr_header.uncompressed_size;
 	int rc = uncompress((unsigned char *)blv_data, &read_len, compr_data, compr_header.compressed_size);
 	struct blv_header *p_blv_header = (struct blv_header *)blv_data;

 	if (read_len != compr_header.uncompressed_size) {
 		printf("!! error uncompressing BLV: %.16s; read_len:%lu; unc_len: %lu \n", 
 			p_dir_entry->name, read_len, compr_header.uncompressed_size);
 		goto cleanup_blv;
 	}

 	printf("-- description: [%.76s] \n", p_blv_header->description);

cleanup_blv:
	//if (compr_data)
	//	free(compr_data);

	if (blv_data)
		free(blv_data);
}
