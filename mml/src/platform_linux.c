#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "mml.h"

static clockid_t mml_clock;
static unsigned long long mml_freq;

void mml_init_time() {
	struct timespec ts;

	mml_clock = CLOCK_REALTIME;
	mml_freq = 1000000000;

#if defined(_POSIX_MONOTONIC_CLOCK)
	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
		mml_clock = CLOCK_MONOTONIC;
	}
#endif
}

unsigned long long mml_get_frequency() {
	return mml_freq;
}

unsigned long long mml_get_time() {
	struct timespec ts;

	clock_gettime(mml_clock, &ts);
	return (unsigned long long)ts.tv_sec * mml_freq + (unsigned long long)ts.tv_nsec;
}

int mml_list_directory(struct mml_directory_list* list, const char* path) {
	DIR* dir;
	struct dirent* entry;
	int capacity, len;

	list->entries = NULL;
	list->count = 0;

	dir = opendir(path);
	if (!dir) {
		return MML_FILE_NOT_FOUND;
	}

	capacity = 0;
	while ((entry = readdir(dir))) {
		if (strcmp(entry->d_name, ".") == 0)  { continue; }
		if (strcmp(entry->d_name, "..") == 0) { continue; }

		if (list->count >= capacity) {
			capacity = capacity < 8 ? 8 : capacity * 2;
			list->entries = realloc(list->entries, capacity * sizeof(char*));
		}

		len = strlen(entry->d_name);

		list->entries[list->count] = malloc(len + 1);
		strcpy(list->entries[list->count], entry->d_name);
		list->entries[list->count][len] = '\0';
		list->count++;
	}

	return MML_OK;
}

void mml_deinit_directory_list(struct mml_directory_list* list) {
	int i;

	for (i = 0; i < list->count; i++) {
		free(list->entries[i]);
	}
	if (list->count > 0) {
		free(list->entries);
	}
	list->entries = NULL;
	list->count = 0;
}

int mml_stat(struct mml_stat* ostat, const char* filename) {
	struct stat path_stat;

	if (stat(filename, &path_stat) == -1) {
		return MML_FILE_NOT_FOUND;
	}

	if      (S_ISDIR(path_stat.st_mode)) { ostat->type = MML_FILE_DIR; }
	else if (S_ISREG(path_stat.st_mode)) { ostat->type = MML_FILE_NORMAL; }
	else                                 { ostat->type = MML_FILE_UNKNOWN; }

	ostat->mod_time = path_stat.st_mtim.tv_nsec;
	ostat->access_time = path_stat.st_atim.tv_nsec;
	ostat->create_time = path_stat.st_ctim.tv_nsec;

	return MML_OK;
}

int mml_mkdir(const char* filename) {
	struct stat st;

	if (stat(filename, &st) == -1) {
		mkdir(filename, 0700);
		return MML_OK;
	}

	return MML_FILE_EXISTS;
}

int mml_mkfile(const char* filename) {
	struct stat st;
	int f;

	if (stat(filename, &st) == -1) {
		f = open(filename, O_RDWR | O_CREAT, 0777);
		if (f != -1) {
			close(f);
			return MML_OK;
		}
		return MML_FAILURE_UNKNOWN_REASON;
	}

	return MML_FILE_EXISTS;
}
