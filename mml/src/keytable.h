#pragma once

#include "mml.h"

struct mml_key_table_item {
	int key, value;
};

struct mml_key_table {
	struct mml_key_table_item elements[MML_KEY_COUNT];
};

void init_key_table(struct mml_key_table* table);
int search_key_table(struct mml_key_table* table, int key);
void key_table_insert(struct mml_key_table* table, int key, int value);
