#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lod_parser.h>


void usage_exit();
void cmd_list(char *gamedir, char *lod_name);


int main(int argc, char *argv[]) {
	if (argc <= 1) {
		printf(" !! Missing command \n");
		usage_exit(argv, 0);
	}

	char* cmd = argv[1];

	if (!strcmp(cmd, "help")) {
		usage_exit(argv, 0);

	} else if (!strcmp(cmd, "list")) {
		if (argc <= 3) {
			printf(" !! Missing argument \n");
			usage_exit(argv, -1);
		}

		cmd_list(argv[2], argv[3]);

	} else {
		printf(" !! Unknown command: %s \n", cmd);
		usage_exit(argv, 0);
	}

}


void usage_exit(char *argv[], int rc) {
	printf("\n");
	printf("    Usage: %s help \n", argv[0]);
	printf("             \t | list <gamedir> <lod_name> \n");
	printf("\n");

	exit(rc);
}


void cmd_list(char *gamedir, char *lod_name) {
	lod_init();
	lod_dir_load(gamedir, lod_name);
	lod_dir_print(gamedir, lod_name);
}
