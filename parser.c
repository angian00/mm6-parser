#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <inttypes.h>


//----------------------------------------------------------------------

#define UNCOMPRESS_DIR "uncompressed_data/"

struct lod_header lod_header;
struct blv_data blv_data;

#define DUMP_VERTICES (1 << 0)
#define DUMP_WALLS    (1 << 1)
#define DUMP_TEXTURES (1 << 2)
#define DUMP_FACES    (1 << 3)
#define DUMP_SECTORS  (1 << 4)
#define DUMP_OBJECTS  (1 << 5)
#define DUMP_LIGHTS   (1 << 6)
#define DUMP_BSPNODES (1 << 7)
#define DUMP_SPAWNS   (1 << 8)
#define DUMP_OUTLINES (1 << 9)

#define dump_bitmask (DUMP_OUTLINES)

//----------------------------------------------------------------------

typedef void (*parse_level_cbk)(FILE *, unsigned long, struct lod_dir_entry*);

void parse_lod(const char lod_name[], const char level_name[], parse_level_cbk cbk);
void read_lod_header(FILE *fp, struct lod_header *p_header);
void read_lod_dir_entry(FILE *fp, unsigned long pos, struct lod_dir_entry *p_dir_data);

void parse_level_by_extension(FILE *fp, unsigned long curr_pos, struct lod_dir_entry *p_dir_entry);
void parse_blv(FILE *fp, unsigned long curr_pos, struct lod_dir_entry *p_dir_entry);
void parse_blv_fields(struct blv_data* bv);


void print_level_name(FILE *fp, unsigned long curr_pos, struct lod_dir_entry *p_dir_entry);
void uncompress_level(FILE *fp, unsigned long curr_pos, struct lod_dir_entry *p_dir_entry);

void print_lod_header(struct lod_header* lh);
void print_blv_data(struct blv_data* bd);
void print_blv_objects(struct blv_data* bd);
void print_blv_outlines(struct blv_data* bd);

//----------------------------------------------------------------------

void parse_level(const char lod_name[], const char level_name[]) {
	parse_lod(lod_name, level_name, parse_level_by_extension);
}

void list_levels(const char lod_name[]) {
	parse_lod(lod_name, NULL, print_level_name);
}

void uncompress_lod(const char lod_name[]) {
	parse_lod(lod_name, NULL, uncompress_level);
}


void parse_lod(const char lod_name[], const char level_name[], parse_level_cbk cbk) {

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

		if ( (level_name != NULL) && strcmp(level_name, dir_entry.name)) {
		 	goto next_entry;
		}

		cbk(fp, curr_lod_pos, &dir_entry);

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


void parse_level_by_extension(FILE *fp, unsigned long curr_pos, struct lod_dir_entry *p_dir_entry) {
	char *ext = strrchr(p_dir_entry->name, '.');
	if (!ext) {
		// no extension, file skipped
		return;
	}

	if (!strcmp(ext, ".blv")) {
	 	parse_blv(fp, curr_pos, p_dir_entry);

	} else {
	 	//TODO: dlv odm ddm
	 	//printf("!! Unknown extension, file skipped\n");
	 	return;
	}
}


void uncompress_level(FILE *fp, unsigned long curr_pos, struct lod_dir_entry *p_dir_entry) {
	//TODO: unify with parse_blv
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

 	if (rc || (read_len != compr_header.uncompressed_size)) {
 		fprintf(stderr, "!! error uncompressing BLV: %.16s; read_len:%lu; unc_len: %"PRIu32" \n", 
 			p_dir_entry->name, read_len, compr_header.uncompressed_size);
 		return;
 	}


 	FILE *wfp;
	char unc_filename[100];
	strcpy(unc_filename, UNCOMPRESS_DIR);
	strcat(unc_filename, p_dir_entry->name);

	wfp = fopen(unc_filename, "wb");
	if (!wfp) {
		fprintf(stderr, "!! could not write %s \n", unc_filename);
 		return;
	}
	fwrite(blv_data.buffer, blv_data.size, 1, wfp);
	fclose(wfp);
}


void parse_blv(FILE *fp, unsigned long curr_pos, struct lod_dir_entry *p_dir_entry) {
	struct blv_compressed_blv6_header compr_header;

	unsigned long tot_offset = sizeof(struct lod_header) + p_dir_entry->start_offset;
	fseek(fp, tot_offset, SEEK_SET);
	fread(&compr_header, sizeof(compr_header), 1, fp);

	unsigned char compr_data[compr_header.compressed_size];
	fread(compr_data, (int)compr_header.compressed_size, 1, fp);

	blv_data.size = compr_header.uncompressed_size;
	blv_data.buffer = malloc(compr_header.uncompressed_size);
	
	unsigned long read_len = compr_header.uncompressed_size;
 	uncompress(blv_data.buffer, &read_len, compr_data, compr_header.compressed_size);

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
 	bd->vertices = (struct vertex *)curr_pos;
 	curr_pos += bd->p_vertex_section->count * sizeof(struct vertex);

 	bd->p_wall_section = (struct wall_section *)curr_pos;
 	curr_pos += sizeof(struct wall_section);
 	bd->walls = (struct wall *)curr_pos;

 	curr_pos += bd->p_wall_section->count * sizeof(struct wall);
 	curr_pos += bd->p_blv_header->wall_vertex_size;

  	bd->textures = (struct texture *)curr_pos;
	curr_pos += (sizeof(struct texture) * bd->p_wall_section->count);

	//curr_pos += (sizeof(struct wall_vertex) * bd->p_wall_section->count);

 	bd->p_face_section = (struct face_section *)curr_pos;
 	curr_pos += sizeof(struct face_section);
 	bd->faces = (struct face *)curr_pos;
 	curr_pos += bd->p_face_section->count * (sizeof(struct face_param_data) + sizeof(struct face_param_data_2));

 	bd->p_room_section = (struct room_section *)curr_pos;
 	curr_pos += sizeof(struct room_section);
 	bd->rooms = (struct room *)curr_pos;
 	curr_pos += bd->p_room_section->count * sizeof(struct room);
 	curr_pos += bd->p_blv_header->r_datasize;
 	curr_pos += bd->p_blv_header->rl_datasize;

 	bd->p_object_section = (struct object_section *)curr_pos;
 	curr_pos += sizeof(struct object_section);
	curr_pos += bd->p_object_section->count * sizeof(struct obj_unk);
  	bd->objects = (struct object *)curr_pos;
	curr_pos += bd->p_object_section->count * sizeof(struct object);

 	bd->p_light_section = (struct light_section *)curr_pos;
 	curr_pos += sizeof(struct light_section);
 	curr_pos += bd->p_light_section->count * (sizeof(struct light));

 	bd->p_bsp_node_section = (struct bsp_node_section *)curr_pos;
 	curr_pos += sizeof(struct bsp_node_section);
 	bd->bsp_nodes = (struct bsp_node *)curr_pos;
 	curr_pos += bd->p_bsp_node_section->count * (sizeof(struct bsp_node));

 	bd->p_spawn_section = (struct spawn_section *)curr_pos;
 	curr_pos += sizeof(struct spawn_section);
 	bd->spawns = (struct spawn *)curr_pos;
 	curr_pos += bd->p_spawn_section->count * (sizeof(struct spawn));

 	bd->p_outline_section = (struct outline_section *)curr_pos;
 	curr_pos += sizeof(struct outline_section);

 	bd->outlines = (struct outline *)curr_pos;
 	//curr_pos += (sizeof(struct outline) * bd->p_outline_section->count);
}


void print_level_name(FILE *fp, unsigned long curr_pos, struct lod_dir_entry *p_dir_entry) {
	printf("%s \n", p_dir_entry->name);
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

 	printf("   size: %"PRIu32" \n", bd->size);
 	printf("   description: [%.76s] \n", bd->p_blv_header->description);
 	printf("   num_vertices: %"PRIu32" \n", bd->p_vertex_section->count);
 	printf("   num_walls: %"PRIu32" \n", bd->p_wall_section->count);

 	printf("   num_faces: %"PRIu32" \n", bd->p_face_section->count);
 	printf("   num_rooms: %"PRIu32" \n", bd->p_room_section->count);
 	printf("   num_obj_unknown: %"PRIu32", num_objects: %"PRIu32" \n", bd->p_object_section->num_unknown, bd->p_object_section->count);
 	printf("   num_lights: %"PRIu32" \n", bd->p_light_section->count);
 	printf("   num_spawn: %"PRIu32" \n", bd->p_spawn_section->count);
 	printf("   num_outlines: %"PRIu32" \n", bd->p_outline_section->count);

 	printf("   -- textures offset: %llu \n", (unsigned long long)((void *)bd->textures - (void *)bd->buffer));
 	printf("   -- rooms offset: %llu \n", (unsigned long long)((void *)bd->rooms - (void *)bd->buffer));
 	printf("   -- objects offset: %llu \n", (unsigned long long)((void *)bd->objects - (void *)bd->buffer));
 	printf("   -- outlines offset: %llu \n", (unsigned long long)((void *)bd->outlines - (void *)bd->buffer));

 	unsigned int i;

 	if (dump_bitmask & DUMP_TEXTURES) {
	 	printf("   textures: \n");
	 	for (i=0; i < bd->p_wall_section->count; i++) {
			//skip portals, which have no texture string
			if (!(bd->walls[i].bits & 0x01)) {
				printf("  %4u   %.10s \n", i, bd->textures[i].name);
			}
	 	}
 	}

 	if (dump_bitmask & DUMP_OBJECTS) {
	 	printf("   objects: \n");
		for (i=0; i < bd->p_object_section->count; i++) {
	 		printf("  %4u    %s \n", i, bd->objects[i].name);
		}
	}

 	if (dump_bitmask & DUMP_OUTLINES) {
	 	printf("   outlines: \n");
	 	for (i=0; i < bd->p_outline_section->count; i++) {
	 		struct outline o = bd->outlines[i];

			//printf("  %4u   %"PRIu16" %"PRIu16" \n", i, o.i_v1, o.i_v2);

			if ( (o.i_v1 == 0) && (o.i_v2 == 0) ) {
		 		printf("       xxxxxxxxx \n");
		 		continue;
			}

			struct vertex v1 = bd->vertices[o.i_v1];
			struct vertex v2 = bd->vertices[o.i_v2];

	 		printf("%"PRIi16" %"PRIi16" %"PRIi16" %"PRIi16" \n", 
	 			v1.x, v1.y, v2.x, v2.y);
	 	}
	}
 }

