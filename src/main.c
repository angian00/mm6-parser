#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "constants.h"
#include "lod_parser.h"
#include "tga.h"
#include "blv.h"
#include "geometry.h"
#include "graphics.h"


//----------------------------------------------------------------------
//--- Internal function declarations
//----------------------------------------------------------------------

void usage_exit();
void cmd_list(char *lod_path);
void cmd_export_tga_image(char *lod_path, char *file_name);
void cmd_export_all_tga_images(char *lod_path);
void cmd_visualize(char *lod_path, char *lod_entry_name);

void make_dir(char *full_path);
void export_tga_image(const char *gamedir, const char *lod_name, const char *lod_entry_name);


//----------------------------------------------------------------------
//--- Implementation
//--------------------------------------------------------------

int main(int argc, char *argv[])
{
	if (argc <= 1) {
		printf(" !! Missing command \n");
		usage_exit(argv, 0);
	}

	char* cmd = argv[1];

	if (!strcmp(cmd, "help")) {
		usage_exit(argv, 0);

	} else if (!strcmp(cmd, "list")) {
		if (argc <= 2) {
			printf(" !! Missing argument \n");
			usage_exit(argv, -1);
		}

		cmd_list(argv[2]);

	} else if (!strcmp(cmd, "export")) {
		if (argc <= 2) {
			printf(" !! Missing argument \n");
			usage_exit(argv, -1);
		}

		if (argc == 3) {
			cmd_export_all_tga_images(argv[2]);
		} else {
			cmd_export_tga_image(argv[2], argv[3]);
		}

	} else if (!strcmp(cmd, "visualize2d")) {
		if (argc <= 3) {
			printf(" !! Missing argument \n");
			usage_exit(argv, -1);
		}

		cmd_visualize(argv[2], argv[3]);

	} else {
		printf(" !! Unknown command: %s \n", cmd);
		usage_exit(argv, 0);
	}

}


void usage_exit(char *argv[], int rc)
{
	printf("\n");
	printf("    Usage: %s help \n", argv[0]);
	printf("             \t | list <gamedir/lod_name.lod> \n");
	printf("             \t | export <gamedir/lod_name.lod> [image_file_name] \n");
	printf("             \t | visualize2d <gamedir/lod_name.lod> <blv_file_name> \n");
	printf("\n");

	exit(rc);
}


void cmd_list(char *lod_path)
{
	char *gamedir = dirname(strdup(lod_path));
	char *lod_name = basename(lod_path);

	lod_init();
	lod_dir_load(gamedir, lod_name);

	lod_dir_print(gamedir, lod_name);
}


void cmd_export_tga_image(char *lod_path, char *lod_entry_name)
{
	char *gamedir = dirname(strdup(lod_path));
	char *lod_name = basename(lod_path);

	lod_init();
	lod_dir_load(gamedir, lod_name);

	char out_dir[MAX_PATH_LEN];
	sprintf(out_dir, "images/%s/%s", gamedir, lod_name);
	make_dir(out_dir);

	export_tga_image(gamedir, lod_name, lod_entry_name);
}

void cmd_export_all_tga_images(char *lod_path)
{
	char *gamedir = dirname(strdup(lod_path));
	char *lod_name = basename(lod_path);

	lod_init();
	lod_dir_load(gamedir, lod_name);

	char out_dir[MAX_PATH_LEN];
	sprintf(out_dir, "images/%s/%s", gamedir, lod_name);
	make_dir(out_dir);

	lod_dir_iterate(gamedir, lod_name, export_tga_image);
}



void cmd_visualize(char *lod_path, char *lod_entry_name)
{
	char *gamedir = dirname(strdup(lod_path));
	char *lod_name = basename(lod_path);

	lod_init();
	lod_dir_load(gamedir, lod_name);


	char *raw_data = lod_file_load(gamedir, lod_name, lod_entry_name);
	struct blv_data *p_blv = blv_parse(raw_data);

	struct point *lines;
	uint32_t n_lines;

	blv_extract_outlines(p_blv, &n_lines, &lines);

	// printf("---- Prima ---- \n");
	// print_geometry(2*n_lines, lines);
	normalize_geometry(2*n_lines, lines);
	// printf("---- Dopo ---- \n");
	// print_geometry(2*n_lines, lines);

	visualize2d(lod_entry_name, n_lines, lines);
}



void export_tga_image(const char *gamedir, const char *lod_name, const char *lod_entry_name)
{
	char *ext = strrchr(lod_entry_name, '.');
	if (ext) {
		//not a TGA
		return;
	}

	char *raw_data = lod_file_load(gamedir, lod_name, lod_entry_name);

	char image_path[MAX_PATH_LEN];
	sprintf(image_path, "images/%s/%s/%s.png", gamedir, lod_name, lod_entry_name);

	struct tga_data *p_tga = tga_parse(raw_data);
	if (!p_tga)
		return;

	tga_export_png(image_path, p_tga);

	//sprintf(image_path, "%s/%s.bmp", out_dir, lod_entry_name);
	//tga_export_bmp(image_path, p_tga);
}


void make_dir(char *full_path)
{
	//create dirs if not existing
	char mkdir_cmd[MAX_PATH_LEN];
	sprintf(mkdir_cmd, "mkdir -p \"%s\"", full_path); //FIXME
	//printf("Executing command: [%s]\n", mkdir_cmd);
	system(mkdir_cmd);
}
