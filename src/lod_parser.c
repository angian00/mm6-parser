#include "lod_parser.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <assert.h>

#include "constants.h"

//--------------------------------------------------------------
//--- Local constants
//--------------------------------------------------------------

#define MAX_LODS 20
#define MAX_LOD_ENTRIES 10000

//--------------------------------------------------------------
//--- Data structures
//--------------------------------------------------------------

struct lod_dir_entry {
	char file_name[16];
	uint32_t bin_offset;
	uint32_t length;
};

struct lod_dir_data {
	char *gamedir;
	char *lod_name;

	struct lod_dir_entry dir_entries[MAX_LOD_ENTRIES];
	int n_dir_entries;
};


//----   Gamefile data format  ----------
struct lod_bin_header {
    char id[4];
    char game[9];
    char unknown[256-13];
    char dir[16];
    uint32_t dir_start;
    uint32_t dir_length;
    uint32_t unknown2;
    uint32_t num_files;
};


struct lod_bin_dir_entry {
    char name[16];
    uint32_t start_offset;
    uint32_t length;
    uint32_t u1;
    uint32_t u2;
};


//--------------------------------------------------------------
//--- Global variables
//--------------------------------------------------------------

static struct lod_dir_data lods[MAX_LODS];
static int n_lods;



//----------------------------------------------------------------------
//--- Internal function declarations
//----------------------------------------------------------------------

struct lod_dir_data * search_lods(const char *gamedir, const char *lod_name);
struct lod_dir_entry * search_lod_entries(const char *gamedir, const char *lod_name, const char *lod_entry_name);


//----------------------------------------------------------------------
//--- Implementation
//--------------------------------------------------------------


void lod_init()
{
	//printf("lod_init\n");

	n_lods = 0;
}


void lod_dir_load(const char *gamedir, const char *lod_name)
{
	//printf("lod_dir_load\n");

	struct lod_dir_data *p_lod_data = lods + n_lods;
	p_lod_data->gamedir = strdup(gamedir);
	p_lod_data->lod_name = strdup(lod_name);
	p_lod_data->n_dir_entries = 0;

	char full_lod_path[MAX_PATH_LEN];
	sprintf(full_lod_path, "%s/%s", gamedir, lod_name);


	FILE *fp;
	struct lod_bin_header lod_header;
	struct lod_bin_dir_entry bin_dir_entry;
	unsigned long curr_lod_pos;

	fp = fopen(full_lod_path, "rb");
	if (!fp) {
		fprintf(stderr, "!! Could not open file: %s \n", full_lod_path);
		return;
	}

	fread(&lod_header, sizeof(struct lod_bin_header), 1, fp);

	curr_lod_pos = lod_header.dir_start;
	for (unsigned int i_lod_entry = 0; i_lod_entry < lod_header.num_files; i_lod_entry ++) {
		struct lod_dir_entry *p_dir_entry = p_lod_data->dir_entries + p_lod_data->n_dir_entries;
		fread(&bin_dir_entry, sizeof(struct lod_bin_dir_entry), 1, fp);
		
		memcpy(p_dir_entry->file_name, bin_dir_entry.name, 16);
		p_dir_entry->bin_offset = bin_dir_entry.start_offset + sizeof(struct lod_bin_header); //NOTICE we need to add lod header to offset here
		p_dir_entry->length = bin_dir_entry.length;

		p_lod_data->n_dir_entries ++;
		curr_lod_pos += sizeof(struct lod_bin_dir_entry);
	}

	fclose(fp);

	n_lods ++;
}


void lod_dir_print(const char *gamedir, const char *lod_name)
{
	struct lod_dir_data *p_lod_data = search_lods(gamedir, lod_name);
	assert(p_lod_data);

	printf("\n");
	printf("--- LOD %s/%s ---\n", p_lod_data->gamedir, p_lod_data->lod_name);
	for (unsigned int i_lod_entry = 0; i_lod_entry < p_lod_data->n_dir_entries; i_lod_entry ++) {
		printf("\t %s \n", p_lod_data->dir_entries[i_lod_entry].file_name);
	}

	printf("\n");
}

void lod_dir_iterate(const char *gamedir, const char *lod_name, lod_entry_cbk cbk)
{
	struct lod_dir_data *p_lod_data = search_lods(gamedir, lod_name);
	assert(p_lod_data);

	printf("\n");
	for (unsigned int i_lod_entry = 0; i_lod_entry < p_lod_data->n_dir_entries; i_lod_entry ++) {
		char *filename = p_lod_data->dir_entries[i_lod_entry].file_name;
		printf("Processing lod entry [%s]\n", filename);
		cbk(gamedir, lod_name, filename);
	}
	printf("\n");
}

char *lod_file_load(const char *gamedir, const char *lod_name, const char *lod_entry_name)
{
	//printf("lod_file_load\n");

	struct lod_dir_entry *p_dir_entry = search_lod_entries(gamedir, lod_name, lod_entry_name);
	assert(p_dir_entry);

	char full_lod_path[MAX_PATH_LEN];
	sprintf(full_lod_path, "%s/%s", gamedir, lod_name);

	FILE *fp;
	fp = fopen(full_lod_path, "rb");
	if (!fp) {
		fprintf(stderr, "!! Could not open file: %s \n", full_lod_path);
		return NULL;
	}

	int buffer_len = p_dir_entry->length;
	printf("buffer_len: %d\n", buffer_len);

	char *buffer = malloc(buffer_len);
	fseek(fp, p_dir_entry->bin_offset, SEEK_SET);
	fread(buffer, buffer_len, 1, fp);
	fclose(fp);

	return buffer;
}


//--- Internal functions definition

struct lod_dir_data * search_lods(const char *gamedir, const char *lod_name)
{
	for (unsigned int i_lod = 0; i_lod < n_lods; i_lod ++) {
		if (!strcmp(gamedir, lods[i_lod].gamedir) && !strcmp(lod_name, lods[i_lod].lod_name))
			return lods + i_lod;
	}

	fprintf(stderr, "!! LOD not found: %s/%s", gamedir, lod_name);
	return NULL;
}


struct lod_dir_entry * search_lod_entries(const char *gamedir, const char *lod_name, const char *lod_entry_name)
{
	//printf("search_lod_entries\n");

	struct lod_dir_data *p_lod_data = search_lods(gamedir, lod_name);
	if (!p_lod_data)
		return NULL;

	for (unsigned int i_entry = 0; i_entry < p_lod_data->n_dir_entries; i_entry ++) {
		if (!strcmp(lod_entry_name, p_lod_data->dir_entries[i_entry].file_name))
			return p_lod_data->dir_entries + i_entry;
	}

	fprintf(stderr, "!! LOD entry not found: %s/%s [%s]", gamedir, lod_name, lod_entry_name);
	return NULL;
}

