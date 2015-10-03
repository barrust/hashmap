/*******************************************************************************
***
***	 Author: Tyler Barrus
***	 email:  barrust@gmail.com
***
***	 Version: 0.5.0
***
***	 License: MIT 2015
***
*******************************************************************************/

#include "hashmap.h"
#include "sorting_algos.h"

#define INITIAL_NUM_ELEMENTS 1024
#define MAX_FULLNESS_PERCENT 0.25       /* arbitrary */


/*******************************************************************************
***		PRIVATE FUNCTIONS
*******************************************************************************/
static uint64_t md5_hash_default(char *key);
static void  __setup_nodes(hashmap_node **nodes, uint64_t length);
static void* __insert_key(HashMap *h, char *key, void *value, uint64_t hash, short mallocd);
static void* __hashmap_set(HashMap *h, char *key, void *value, short mallocd);
static inline float __get_fullness(HashMap *h);
static void  __get_stats(HashMap *h, uint64_t *worst_case, uint64_t *max_big_o, float *avg_big_o, float *avg_used_big_o);
static void  __get_collision_stats(HashMap *h, unsigned int *hash, unsigned int *idx);
static int   __calc_big_o(uint64_t num_nodes, uint64_t i, uint64_t idx);

/*******************************************************************************
***		FUNCTION DEFINITIONS
*******************************************************************************/
int hashmap_init(HashMap *h) {
	return hashmap_init_alt(h, NULL);
}


int hashmap_init_alt(HashMap *h, HashFunction hash_function) {
	// init the original nodes
	h->nodes = (hashmap_node**) malloc(INITIAL_NUM_ELEMENTS * sizeof(hashmap_node*));
	if (h->nodes == NULL) {
		return HASHMAP_FAILURE;
	}
	h->number_nodes = INITIAL_NUM_ELEMENTS;
	h->used_nodes = 0;
	__setup_nodes(h->nodes, h->number_nodes);
	h->hash_function = (hash_function == NULL) ? &md5_hash_default : hash_function;

	return HASHMAP_SUCCESS;
}


void hashmap_destroy(HashMap *h) {
	uint64_t i;
	for (i = 0; i < h->number_nodes; i++) {
		if (h->nodes[i] != NULL) {
			free(h->nodes[i]->key); // no issue of calling free(NULL);
			if (h->nodes[i]->mallocd == 0) {
				free(h->nodes[i]->value);
			}
			free(h->nodes[i]);
		}
	}
	free(h->nodes);
	h->number_nodes = 0;
	h->used_nodes = 0;
	h->hash_function = NULL;
}


void* hashmap_set(HashMap *h, char *key, void *value) {
	return __hashmap_set(h, key, value, -1);
}


void* hashmap_set_alt(HashMap *h, char *key, void * value) {
	return __hashmap_set(h, key, value, 0);
}


void* hashmap_get(HashMap *h, char *key) {
	// get the hash value
	uint64_t hash = h->hash_function(key);

	// get the index
	uint64_t idx = hash % h->number_nodes;
	uint64_t i = idx;

	// starting at idx, search for until there is an unused spot
	while (true) {
		if (h->nodes[i] == NULL) { //not found
			return NULL;
		} else if (h->nodes[i]->hash == hash && strlen(key) == strlen(h->nodes[i]->key) && strncmp(key, h->nodes[i]->key, strlen(key)) == 0) {
			return  h->nodes[i]->value;
		} else {
			// lets see if we need to continue or if we have already gone all the way around
			i = (i + 1 == h->number_nodes) ? 0 : i + 1;
			if (i == idx) {	// We can only have this happen if there are NO open locations
				return NULL;
			}
		}
	}
}


void hashmap_stats(HashMap *h) {
	uint64_t max, wc;
	float avg, avg_used;
	unsigned int hc, ic;
	__get_collision_stats(h, &hc, &ic);
	__get_stats(h, &wc, &max, &avg, &avg_used);
	// size is the size of a single hashmap
	// plus the size of the array of pointers
	// plus the size of the number of allocated nodes
	uint64_t size = sizeof(HashMap) + (sizeof(hashmap_node*) * h->number_nodes) + (sizeof(hashmap_node) * h->used_nodes);
	printf("HashMap:\n\
	Number Nodes: %" PRIu64 "\n\
	Used Nodes: %" PRIu64 "\n\
	Fullness: %f\n\
	Average O(n): %f\n\
	Average Used O(n): %f\n\
	Max O(n): %" PRIu64 "\n\
	Max Consecutive Buckets Used: %" PRIu64 "\n\
	Number Hash Collisions: %d\n\
	Number Index Collisions: %d\n\
	Size on disk (bytes): %" PRIu64 "\n", h->number_nodes, h->used_nodes,
	__get_fullness(h), avg, avg_used, max, wc, hc, ic, size);
}


char** hashmap_keys(HashMap *h) {
	char** keys = calloc(h->used_nodes, sizeof(char*));
	uint64_t i, j = 0;
	for (i = 0; i < h->number_nodes; i++) {
		if (h->nodes[i] != NULL) {
			keys[j] = calloc(strlen(h->nodes[i]->key) + 1, sizeof(char));
			strncpy(keys[j], h->nodes[i]->key, strlen(h->nodes[i]->key));
			j++;
		}
	}
	return keys; // wouldn't it be nice if they were sorted?
}

/*******************************************************************************
***		UTILITY INSERTS
*******************************************************************************/

int* hashmap_set_int(HashMap *h, char *key, int value) {
	int *ptr = malloc(sizeof(int));
	*ptr = value;
	return (int*) __hashmap_set(h, key, ptr, 0);
}


char* hashmap_set_string(HashMap *h, char *key, char *value) {
	char *ptr = calloc(strlen(value) + 1, sizeof(char));
	strncpy(ptr, value, strlen(value));
	return (char*) __hashmap_set(h, key, ptr, 0);
}



/*******************************************************************************
***		PRIVATE FUNCTIONS
*******************************************************************************/

static uint64_t md5_hash_default(char *key) {
	unsigned char digest[MD5_DIGEST_LENGTH];
	MD5_CTX md5_ctx;
	MD5_Init(&(md5_ctx));
	MD5_Update(&(md5_ctx), key, strlen(key));
	MD5_Final(digest, &(md5_ctx));
	return (uint64_t) *(uint64_t *)digest % UINT64_MAX;
}


static void __setup_nodes(hashmap_node **nodes, uint64_t length) {
	uint64_t i;
	for (i = 0; i < length; i++) {
		nodes[i] = NULL;
	}
}

// TODO: Rethink this... should probably reorganize if there isn't a hash collision (I.e., when a new key collides in a bucket that it shouldn't need to)
static void* __insert_key(HashMap *h, char *key, void *value, uint64_t hash, short mallocd) {
	uint64_t idx = hash % h->number_nodes;
	uint64_t i = idx;
	unsigned int o = 1; // it has to at least be 1!
	while (true) {
		if (h->nodes[i] == NULL) {
			h->nodes[i] = malloc(sizeof(hashmap_node));
			h->nodes[i]->key = calloc(strlen(key) + 1, sizeof(char));
			strncpy(h->nodes[i]->key, key, strlen(key));
			h->nodes[i]->value = value;
			h->nodes[i]->hash = hash;
			h->nodes[i]->mallocd = mallocd;
			h->used_nodes++;
			h->nodes[i]->O = o; // __calc_big_o(h->number_nodes, i, idx);
			h->nodes[i]->idx = idx;
			return value;
		} else if (h->nodes[i]->hash == hash && strlen(key) == strlen(h->nodes[i]->key) && strncmp(key, h->nodes[i]->key, strlen(key)) == 0) {
			void *ret = NULL;
			if (h->nodes[i]->mallocd == 0) { // we need to free it...
				free(h->nodes[i]->value);
			}
			h->nodes[i]->value = value;
			h->nodes[i]->hash = hash;
			h->nodes[i]->mallocd = mallocd;
            return value;
		} else {
			i = (i + 1 == h->number_nodes) ? 0 : i + 1;
			if (i == idx) {	// We can only have this happen if there are NO open locations
				return NULL;
			}
		}
		o++;
	}
}


static void* __hashmap_set(HashMap *h, char *key, void *value, short mallocd) {
	// check to see if we need to expand the hashmap
	if (__get_fullness(h) >= MAX_FULLNESS_PERCENT) {
		printf("RESIZE!\n\n\n");
		uint64_t num_nodes = h->number_nodes;
		hashmap_node **old_nodes = h->nodes;
		h->nodes = NULL;
		h->number_nodes = num_nodes * 2;
		h->used_nodes = 0;
		h->nodes = malloc(sizeof(hashmap_node*) * h->number_nodes); // double each time
		__setup_nodes(h->nodes, h->number_nodes);
		// move over all the original elements
		unsigned int O = 0;
		uint64_t i;
		for (i = 0; i < num_nodes; i++) {
			if (old_nodes[i] != NULL) {
				// this would be better to just pass pointers around...
				__insert_key(h, old_nodes[i]->key, old_nodes[i]->value, old_nodes[i]->hash, old_nodes[i]->mallocd);
				free(old_nodes[i]->key);
				free(old_nodes[i]);
			}
		}
		// final cleanup
		free(old_nodes);
	}
	// get the hash value
	uint64_t hash = h->hash_function(key);
	return __insert_key(h, key, value, hash, mallocd);
}


static inline float __get_fullness(HashMap *h) {
	return h->used_nodes / (1.0 * h->number_nodes);
}


static void __get_stats(HashMap *h, uint64_t *worst_case, uint64_t *max_big_o, float *avg_big_o, float *avg_used_big_o) {
	uint64_t i, sum = 0, max = 0, cur = 0, wc = 0, sum_used = 0;
	for (i = 0; i < h->number_nodes; i++) {
		if (h->nodes[i] != NULL) {
			cur++;
			sum_used += h->nodes[i]->O;
			sum += h->nodes[i]->O;
			if (h->nodes[i]->O > max) {
				max = h->nodes[i]->O;
			}
		} else {
			sum += 1;
			if (wc < cur) { wc = cur; }
			cur = 0;
		}
	}
	*worst_case = wc;
	*max_big_o = max;
	*avg_big_o = sum / (h->number_nodes * 1.0);
	*avg_used_big_o = sum_used / (h->used_nodes * 1.0);
}


// consolidate to save some loops
static void __get_collision_stats(HashMap *h, unsigned int *hash, unsigned int *idx) {
	unsigned int hash_col = 0, idx_col = 0;
	uint64_t *hashes = calloc(h->used_nodes, sizeof(uint64_t));
	uint64_t *idxs = calloc(h->used_nodes, sizeof(uint64_t));
	uint64_t i, j = 0;
	for (i = 0; i < h->number_nodes; i++) {
		if (h->nodes[i] != NULL) {
			hashes[j] = h->nodes[i]->hash;
			idxs[j] = h->nodes[i]->idx;
			j++;
		}
	}
	// sort the results
	merge_sort(hashes, h->used_nodes);
	merge_sort(idxs, h->used_nodes);

	// then do some maths to see if there are actual collisions
	for (i = 0; i < h->used_nodes - 1; i++) {
		if(hashes[i] == hashes[i + 1]) {
			hash_col++;
		}
		if(idxs[i] == idxs[i + 1]) {
			idx_col++;
		}
	}
	free(hashes);
	free(idxs);
	*hash = hash_col;
	*idx = idx_col;
}

static int __calc_big_o(uint64_t num_nodes, uint64_t i, uint64_t idx) {
	if (i < idx) { // it wrapped around
		uint64_t t = num_nodes - idx;
		return i + t + 1;
	} else {
		return 1 + i - idx;
	}
}
