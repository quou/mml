#include <stdio.h>
#include <stdlib.h>

#include "mml.h"

void mml_init() {
	mml_init_time();
}

void mml_deinit() {

}

int mml_read_file(const char* path, void** buffer, unsigned int* size, int terminate) {
	FILE* file;
	unsigned int file_size;

	file = fopen(path, "rb");
	if (!file) {
		printf("failed to open `%s'.\n", path);
		return MML_FILE_NOT_FOUND;
	}

	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);

	*buffer = malloc(file_size + (terminate ? 1 : 0));
	fread(*buffer, sizeof(char), file_size, file);
	if (terminate) {
		*((char*)*buffer + file_size) = '\0';
	}

	if (size) {
		*size = file_size + (terminate ? 1 : 0);
	}

	fclose(file);
	return MML_OK;
}
