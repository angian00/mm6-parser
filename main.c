#define MAP_LOD "Games.lod"

#include "parser.h"
#include <stdlib.h>
#include <string.h>


void usage_exit();
void cmd_dump(char *lod_name, char *level_name, char *section_name);



int main(int argc, char *argv[]) {
	if (argc <= 1) {
		printf(" !! Missing command \n");
		usage_exit(argv, 0);
	}

	char* cmd = argv[1];
	char* arg1;
	char* arg2;
	char* arg3;

	if (!strcmp(cmd, "help")) {
		usage_exit(argv, 0);

	} else if (!strcmp(cmd, "dump")) {
		if (argc <= 3) {
			printf(" !! Missing argument \n");
			usage_exit(argv, -1);
		}

		arg1 = argv[2];
		arg2 = argv[3];

		if (argc >= 5)
			arg3 = argv[4];
		else
			arg3 = NULL;

		cmd_dump(arg1, arg2, arg3);

	} else if (!strcmp(cmd, "uncompress")) {
		printf("TODO uncompress\n");
		usage_exit(argv, 0);

	} else if (!strcmp(cmd, "visualize")) {
		printf("TODO visualize\n");
		usage_exit(argv, 0);
	} else {
		printf(" !! Unknown command: %s \n", cmd);
		usage_exit(argv, 0);
	}

}


void usage_exit(char *argv[], int rc) {
	printf("\n");
	printf("    Usage: %s   help \n", argv[0]);
	printf("             \t | dump <lod_name> <level_name> [section_name] \n");
	printf("             \t | uncompress TODO\n");
	printf("             \t | visualize TODO \n");
	printf("\n");

	exit(rc);
}



void cmd_dump(char *lod_name, char *level_name, char *section_name) {
	parse_level(lod_name, level_name);
	printf("LOD %s: \n", lod_name);
	dump_lod_header();

	printf("level %s: \n", level_name);
	dump_blv();
}

