#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <inttypes.h>

//----------------------------------------------------------------------

struct lod_header lod_header;
struct blv_data blv_data;

//----------------------------------------------------------------------

void read_lod_header(FILE *fp, struct lod_header *p_header);
void read_lod_dir_entry(FILE *fp, unsigned long pos, struct lod_dir_entry *p_dir_data);

void read_blv(FILE *fp, unsigned long curr_pos, struct lod_dir_entry *p_dir_entry);

void parse_blv_fields(struct blv_data* bv);

void print_lod_header(struct lod_header* lh);
void print_blv_data(struct blv_data* bv);

//----------------------------------------------------------------------

void parse_level(const char lod_name[], const char level_name[]) {
	FILE *fp;
	struct lod_dir_entry dir_entry;
	unsigned long curr_lod_pos;


	//TODO: prepend default dir
	fp = fopen(lod_name, "rb");
	if (!fp) {
		fprintf(stderr, " !! Could not open file: %s \n", lod_name);
		exit(-1);
	}

	read_lod_header(fp, &lod_header);


	curr_lod_pos = lod_header.dir_start;
	for (unsigned int i_lod_entry = 0; i_lod_entry < lod_header.num_files; i_lod_entry ++) {
		read_lod_dir_entry(fp, curr_lod_pos, &dir_entry);
		//printf("Reading %.16s, level_name: %s\n", dir_entry.name);

		char *ext = strrchr(dir_entry.name, '.');
		if (!ext) {
			//printf("!! No extension, file skipped\n");
			goto next_entry;
		}


		if ( (level_name != NULL) && strcmp(level_name, dir_entry.name)) {
			goto next_entry;
		}


		if (!strcmp(ext, ".blv")) {
			read_blv(fp, curr_lod_pos, &dir_entry);

		} else {
			//TODO: dlv odm ddm
			//printf("!! Unknown extension, file skipped\n");
			goto next_entry;
		}

next_entry:
		curr_lod_pos += sizeof(struct lod_dir_entry);
	}

	fclose(fp);
}


void read_lod_header(FILE *fp, struct lod_header *p_header) {
	fseek(fp, 0L, SEEK_SET);
	fread(p_header, sizeof(struct lod_header), 1, fp);
}


void read_lod_dir_entry(FILE *fp, unsigned long pos, struct lod_dir_entry *p_dir_entry) {
	fseek(fp, pos, SEEK_SET);
	fread(p_dir_entry, sizeof(struct lod_dir_entry), 1, fp);
}

void read_blv(FILE *fp, unsigned long curr_pos, struct lod_dir_entry *p_dir_entry) {
	struct blv_compressed_blv6_header compr_header;

	unsigned long tot_offset = sizeof(struct lod_header) + p_dir_entry->start_offset;
	fseek(fp, tot_offset, SEEK_SET);
	fread(&compr_header, sizeof(compr_header), 1, fp);

	unsigned char compr_data[compr_header.compressed_size];
	fread(compr_data, (int)compr_header.compressed_size, 1, fp);

	blv_data.size = compr_header.uncompressed_size;
	blv_data.buffer = malloc(compr_header.uncompressed_size);
	
	unsigned long read_len = compr_header.uncompressed_size;
 	int rc = uncompress(blv_data.buffer, &read_len, compr_data, compr_header.compressed_size);

 	if (read_len != compr_header.uncompressed_size) {
 		fprintf(stderr, "!! error uncompressing BLV: %.16s; read_len:%lu; unc_len: %"PRIu32" \n", 
 			p_dir_entry->name, read_len, compr_header.uncompressed_size);
 		return;
 	}

 	parse_blv_fields(&blv_data);
}


void parse_blv_fields(struct blv_data* bd) {
 	void *curr_pos = bd->buffer;

 	bd->p_blv_header = (struct blv_header *)curr_pos;
 	curr_pos += sizeof(struct blv_header);

 	bd->p_vertex_section = (struct vertex_section *)curr_pos;
 	curr_pos += sizeof(struct vertex_section);
 	curr_pos += bd->p_vertex_section->count * sizeof(struct vertex);

 	bd->p_wall_section = (struct wall_section *)curr_pos;
 	curr_pos += sizeof(struct wall_section);
 	bd->walls = (struct wall *)curr_pos;

 	curr_pos += bd->p_wall_section->count * sizeof(struct wall);
 	curr_pos += bd->p_blv_header->wall_vertex_size;

 	curr_pos += (sizeof(struct texture) * bd->p_wall_section->count);

 	bd->p_face_section = (struct face_section *)curr_pos;
 	curr_pos += sizeof(struct face_section);
 	curr_pos += bd->p_face_section->count * (sizeof(struct face_param_data) + sizeof(struct face_param_data_2));

 	bd->p_sectors_section = (struct sectors_section *)curr_pos;
 	curr_pos += sizeof(struct sectors_section);
 	curr_pos += bd->p_sectors_section->count * sizeof(struct sector);
 	curr_pos += bd->p_blv_header->r_datasize;
 	curr_pos += bd->p_blv_header->rl_datasize;

 	bd->p_object_section = (struct object_section *)curr_pos;
 	curr_pos += sizeof(struct object_section);
 	curr_pos += bd->p_object_section->count * (sizeof(struct obj_unk) + sizeof(struct object));

 	bd->p_light_section = (struct light_section *)curr_pos;
 	curr_pos += sizeof(struct light_section);
 	curr_pos += bd->p_light_section->count * (sizeof(struct light));

 	bd->p_unknown_section = (struct unknown_section *)curr_pos;

 	curr_pos += sizeof(struct unknown_section);
 	curr_pos += bd->p_unknown_section->count * (sizeof(struct unknown_item));

 	bd->p_spawn_section = (struct spawn_section *)curr_pos;
 	curr_pos += sizeof(struct spawn_section);
 	curr_pos += bd->p_spawn_section->count * (sizeof(struct spawn));

 	bd->p_outline_section = (struct outline_section *)curr_pos;
 	curr_pos += sizeof(struct outline_section);

 	//curr_pos += (sizeof(struct outline) * bd->p_outline_section->count);
}



void dump_lod_header() {
	print_lod_header(&lod_header);
}


void print_lod_header(struct lod_header* lh) {
	printf("id:   %.4s\n", lh->id);
	printf("game: %.9s\n", lh->game);
	printf("dir:  %.16s\n", lh->dir);
	printf("num_files:  %"PRIu32"\n", lh->num_files);
	printf("--------------------------------------\n");
}


void dump_blv() {
	print_blv_data(&blv_data);
}

void print_blv_data(struct blv_data* bd) {
	if (bd->size == 0) {
		printf(" !! not found \n");
		return;
	}

 	printf("   description: [%.76s] \n", bd->p_blv_header->description);
 	printf("   num_vertices: %"PRIu32" \n", bd->p_vertex_section->count);
 	printf("   num_walls: %"PRIu32" \n", bd->p_wall_section->count);

	//printf("   textures: \n");
 	// for (unsigned int i_wall=0; i_wall < bd->p_wall_section->count; i_wall++) {
 	// 	struct texture *p_texture = (struct texture *)curr_pos;
		// //skip portals, which have no texture string
		// //if (!(walls[i_wall].bits & 0x01)) {
		// 	//printf("     %.10s \n", bd->p_texture->name);
		// //}

	 // 	curr_pos += sizeof(struct texture);
 	// }
 	
 	printf("   num_faces: %"PRIu32" \n", bd->p_face_section->count);
 	printf("   num_sectors: %"PRIu32" \n", bd->p_sectors_section->count);
 	printf("   num_obj_unknown: %"PRIu32", num_objects: %"PRIu32" \n", bd->p_object_section->num_unknown, bd->p_object_section->count);
 	printf("   num_lights: %"PRIu32" \n", bd->p_light_section->count);
 	printf("   num_spawn: %"PRIu32" \n", bd->p_spawn_section->count);
 	printf("   num_outlines: %"PRIu32" \n", bd->p_outline_section->count);

 	//  	for (unsigned int i_outline=0; i_outline < bd->p_outline_section->count; i_outline++) {
 	// 	struct outline *p_outline = (struct outline *)curr_pos;
		// // printf("     %"PRIi16" %"PRIi16" %"PRIi16" %"PRIi16" %"PRIi16" \n", 
		// // 	p_outline->x1, bd->p_outline->x2, bd->p_outline->y1, bd->p_outline->y2, blv_data.p_outline->z);

	 // 	curr_pos += sizeof(struct outline);
 	// }

 }

