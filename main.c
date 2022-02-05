#define MAP_LOD "Games.lod"

#include "parser.h"
#include "graphics.h"
#include <stdlib.h>
#include <string.h>


void usage_exit();
void cmd_dump(char *lod_name, char *level_name);
void cmd_list(char *lod_name);
void cmd_uncompress(char *lod_name);
void cmd_visualize(char *lod_name, char *level_name);


int main(int argc, char *argv[]) {
	if (argc <= 1) {
		printf(" !! Missing command \n");
		usage_exit(argv, 0);
	}

	char* cmd = argv[1];

	if (!strcmp(cmd, "help")) {
		usage_exit(argv, 0);

	} else if (!strcmp(cmd, "dump")) {
		if (argc <= 3) {
			printf(" !! Missing argument \n");
			usage_exit(argv, -1);
		}

		cmd_dump(argv[2], argv[3]);

	} else if (!strcmp(cmd, "list")) {
		if (argc <= 2) {
			printf(" !! Missing argument \n");
			usage_exit(argv, -1);
		}

		cmd_list(argv[2]);

	} else if (!strcmp(cmd, "uncompress")) {
		if (argc <= 2) {
			printf(" !! Missing argument \n");
			usage_exit(argv, -1);
		}

		cmd_uncompress(argv[2]);

	} else if (!strcmp(cmd, "visualize")) {
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


void usage_exit(char *argv[], int rc) {
	printf("\n");
	printf("    Usage: %s   help \n", argv[0]);
	printf("             \t | dump <lod_name> <level_name> [section_name] \n");
	printf("             \t | list <lod_name> \n");
	printf("             \t | uncompress <lod_name> \n");
	printf("             \t | visualize <lod_name> <blv name> \n");
	printf("\n");

	exit(rc);
}


void cmd_list(char *lod_name) {
	list_levels(lod_name);
}

void cmd_dump(char *lod_name, char *level_name) {
	parse_level(lod_name, level_name);
	printf("LOD %s: \n", lod_name);
	dump_lod_header();

	printf("level %s: \n", level_name);
	dump_blv();
}

void cmd_uncompress(char *lod_name) {
	uncompress_lod(lod_name);
}

void cmd_visualize(char *lod_name, char *level_name) {
	struct point *lines;
	uint32_t n_lines;

	parse_level(lod_name, level_name);
	
	extract_blv_outlines(&n_lines, &lines);
	// printf("---- Prima ---- \n");
	// print_geometry(2*n_lines, lines);
	normalize_geometry(2*n_lines, lines);
	// printf("---- Dopo ---- \n");
	// print_geometry(2*n_lines, lines);
	visualize2d(n_lines, lines);
}


