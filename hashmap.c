/*******************************************************************************
***
***	 Author: Tyler Barrus
***	 email:  barrust@gmail.com
***
***	 Version: 0.3.0
***
***	 License: MIT 2015
***
*******************************************************************************/

#include "hashmap.h"
#include <stdbool.h>

#define INITIAL_NUM_ELEMENTS 1024
#define MAX_FULLNESS_PERCENT 0.75       /* arbitrary */

#define IS_USED 0
#define IS_NOT_USED -1

/*******************************************************************************
***		PRIVATE FUNCTIONS
*******************************************************************************/
static uint64_t md5_hash_default(char *key);
static void __setup_nodes(hashmap_node *nodes, uint64_t length);
static void* __insert_key(HashMap *h, char *key, void *value, uint64_t hash, short mallocd);
static void* __hashmap_set(HashMap *h, char *key, void *value, short mallocd);
static float get_fullness(HashMap *h);
static uint64_t worst_case(HashMap *h);
static float average_case_O(HashMap *h);
static uint64_t max_case_O(HashMap *h);



int hashmap_init(HashMap *h) {
	return hashmap_init_alt(h, NULL);
}

int hashmap_init_alt(HashMap *h, HashFunction hash_function) {
	// init the original nodes
	h->nodes = (hashmap_node*) malloc(INITIAL_NUM_ELEMENTS * sizeof(hashmap_node));
	if(h->nodes == NULL) {
		return HASHMAP_FAILURE;
	}
	h->number_nodes = INITIAL_NUM_ELEMENTS;
	h->used_nodes = 0;
	// set everything to blank
	__setup_nodes(h->nodes, h->number_nodes);
	h->hash_function = (hash_function == NULL) ? &md5_hash_default : hash_function;

	return HASHMAP_SUCCESS;
}

void hashmap_destroy(HashMap *h) {
	uint64_t i;
	for (i = 0; i < h->number_nodes; i++) {
		free(h->nodes[i].key); // no issue of calling free(NULL);
		if(h->nodes[i].mallocd == 0) {
			free(h->nodes[i].value);
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

void* hashmap_get(HashMap *h, char *key) {
	// get the hash value
	uint64_t hash = h->hash_function(key);

	// get the index
	uint64_t idx = hash % h->number_nodes;
	uint64_t i = idx;

	// starting at idx, search for until there is an unused spot
	while (true) {
		if (h->nodes[i].is_used == IS_NOT_USED) { //not found
			return NULL;
        } else if (h->nodes[i].hash == hash && strlen(key) == strlen(h->nodes[i].key) && strncmp(key, h->nodes[i].key, strlen(key)) == 0) {
    		return  h->nodes[i].value;
        } else {
	        // lets see if we need to continue or if we have already gone all the way around
			i = (i + 1 == h->number_nodes) ? 0 : i + 1;
			if (i == idx) {	/* We can only have this happen if there are NO open locations */
				return NULL;
			}
		}
	}
}


void hashmap_stats(HashMap *h) {
	printf("HashMap:\n\
	Number Nodes: %" PRIu64 "\n\
	Used Nodes: %" PRIu64 "\n\
	Fullness: %f\n\
	Average O(n): %f\n\
	Max O(n): %" PRIu64 "\n\
	Max Consecutive Buckets Used: %" PRIu64 "\n", h->number_nodes, h->used_nodes,
	get_fullness(h), average_case_O(h), max_case_O(h), worst_case(h));
}

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

static void __setup_nodes(hashmap_node *nodes, uint64_t length) {
	uint64_t i;
	for (i = 0; i < length; i++) {
		nodes[i].is_used = IS_NOT_USED;
		nodes[i].key = NULL;
		nodes[i].value = NULL;
		nodes[i].hash = 0; // what would be the odds?
		nodes[i].mallocd = -1;
		nodes[i].O = 0;
	}
}

// TODO: Rethink this... should probably reorganize if there isn't a collision (I.e., when a new key collides in a bucket that it shouldn't need to)
static void* __insert_key(HashMap *h, char *key, void *value, uint64_t hash, short mallocd) {
	uint64_t idx = hash % h->number_nodes;
	uint64_t i = idx;
	unsigned int o = 1; // it has to at least be 1!
	while (true) {
		if (h->nodes[i].is_used == IS_NOT_USED) {
            h->nodes[i].is_used = IS_USED;
			h->nodes[i].key = calloc(strlen(key) + 1, sizeof(char));
			strncpy(h->nodes[i].key, key, strlen(key));
    		h->nodes[i].value = value;
    		h->nodes[i].hash = hash;
			h->nodes[i].mallocd = mallocd;
			h->used_nodes++;
			h->nodes[i].O = o;
			return value;
		} else if (h->nodes[i].hash == hash && strlen(key) == strlen(h->nodes[i].key) && strncmp(key, h->nodes[i].key, strlen(key)) == 0) {
            h->nodes[i].is_used = IS_USED;
			void *ret = NULL;
			if (h->nodes[i].mallocd == 0) { // we need to free it...
				free(h->nodes[i].value);
				ret = value;
			} else {
				ret = h->nodes[i].value;
			}
    		h->nodes[i].value = value;
    		h->nodes[i].hash = hash;
			h->nodes[i].mallocd = mallocd;

            return ret;
        } else {
			i = (i + 1 == h->number_nodes) ? 0 : i + 1;
			if (i == idx) {	/* We can only have this happen if there are NO open locations */
				return NULL;
			}
		}
		o++;
	}
	return value;
}


static void* __hashmap_set(HashMap *h, char *key, void *value, short mallocd) {
	// check to see if we need to expand the hashmap
	if (get_fullness(h) >= MAX_FULLNESS_PERCENT) {
		uint64_t num_nodes = h->number_nodes;
		hashmap_node *old_nodes = h->nodes;
		h->nodes = NULL;
		h->number_nodes = num_nodes * 2;
		h->used_nodes = 0;
		h->nodes = malloc(sizeof(hashmap_node) * h->number_nodes); // double each time
		__setup_nodes(h->nodes, h->number_nodes);
		// move over all the original elements
		unsigned int O = 0;
		uint64_t i;
		for (i = 0; i < num_nodes; i++) {
			if (old_nodes[i].is_used == IS_USED) {
				printf("key: %s\n", old_nodes[i].key);
				__insert_key(h, old_nodes[i].key, old_nodes[i].value, old_nodes[i].hash, old_nodes[i].mallocd);
				free(old_nodes[i].key);
			}
		}
		// final cleanup
		free(old_nodes);
	}
	// get the hash value
	uint64_t hash = h->hash_function(key);
	return __insert_key(h, key, value, hash, mallocd);;
}


static float get_fullness(HashMap *h) {
	return h->used_nodes / (1.0 * h->number_nodes);
}

static uint64_t worst_case(HashMap *h) {
	uint64_t i, r = 0, cur = 0;
	for (i = 0; i < h->number_nodes; i++) {
		if (h->nodes[i].is_used == IS_USED) {
			cur++;
		} else {
			if (r < cur) {
				r = cur;
			}
			cur = 0;
		}
	}
	return r;
}

static float average_case_O(HashMap *h) {
	uint64_t i, sum = 0;
	for (i = 0; i < h->number_nodes; i++) {
		if (h->nodes[i].is_used == IS_USED) {
			sum += h->nodes[i].O;
		}
	}
	return sum / (h->used_nodes * 1.0);
}

static uint64_t max_case_O(HashMap *h) {
	uint64_t i, max = 1; // by definition
	for (i = 0; i < h->number_nodes; i++) {
		if (h->nodes[i].O > max) {
			max = h->nodes[i].O;
		}
	}
	return max;
}
