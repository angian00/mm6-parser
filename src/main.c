#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "constants.h"
#include "lod_parser.h"
#include "tga.h"


void usage_exit();
void cmd_list(char *lod_path);
void cmd_export_tga_image(char *lod_path, char *file_name);
void cmd_export_all_tga_images(char *lod_path);

void export_tga_image(const char *gamedir, const char *lod_name, const char *lod_entry_name);


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
			printf("Processing image\n");
			cmd_export_tga_image(argv[2], argv[3]);
		}

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

	export_tga_image(gamedir, lod_name, lod_entry_name);
}


void cmd_export_all_tga_images(char *lod_path)
{
	char *gamedir = dirname(strdup(lod_path));
	char *lod_name = basename(lod_path);

	lod_init();
	lod_dir_load(gamedir, lod_name);

	lod_dir_iterate(gamedir, lod_name, export_tga_image);
}


void export_tga_image(const char *gamedir, const char *lod_name, const char *lod_entry_name)
{
	char *ext = strrchr(lod_entry_name, '.');
	if (ext) {
		//not a TGA
		return;
	}

	char *raw_data = lod_file_load(gamedir, lod_name, lod_entry_name);

	char out_dir[MAX_PATH_LEN];
	sprintf(out_dir, "images/%s/%s", gamedir, lod_name);

	//create dirs if not existing
	char mkdir_cmd[MAX_PATH_LEN];
	sprintf(mkdir_cmd, "mkdir -p \"%s\"", out_dir); //FIXME
	//printf("Executing command: [%s]\n", mkdir_cmd);
	system(mkdir_cmd);

	char image_path[MAX_PATH_LEN];

	struct tga_data *p_tga = tga_parse(raw_data);

	sprintf(image_path, "%s/%s.png", out_dir, lod_entry_name);
	tga_export_png(image_path, p_tga);

	//sprintf(image_path, "%s/%s.bmp", out_dir, lod_entry_name);
	//tga_export_bmp(image_path, p_tga);
}

