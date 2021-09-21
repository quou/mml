#include "mml.h"
#include "keytable.h"

void init_key_table(struct mml_key_table* table) {
	int i;

	for (i = 0; i < MML_KEY_COUNT; i++) {
		table->elements[i] = (struct mml_key_table_item) { -1, 0 };
	}
}

int search_key_table(struct mml_key_table* table, int key) {
	int hash_idx, i;

	hash_idx = key % MML_KEY_COUNT;

	i = 0;

	while (table->elements[hash_idx].key != key && i < MML_KEY_COUNT) {
		hash_idx++;
		hash_idx %= MML_KEY_COUNT;

		i++;
	}

	if (i >= MML_KEY_COUNT) { return -1; }

	return table->elements[hash_idx].value;
}

void key_table_insert(struct mml_key_table* table, int key, int value) {
	int hash_idx;
	struct mml_key_table_item item = { key, value };

	hash_idx = key % MML_KEY_COUNT;

	while (table->elements[hash_idx].key != -1) {
		hash_idx++;
		hash_idx %= MML_KEY_COUNT;
	}

	table->elements[hash_idx] = item;
}
