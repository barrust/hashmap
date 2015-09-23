/*******************************************************************************
***
***	 Author: Tyler Barrus
***	 email:  barrust@gmail.com
***
***	 Version: 0.2.0
***
***	 License: MIT 2015
***
*******************************************************************************/

#include "hashmap.h"

#define INITIAL_NUM_ELEMENTS 1024
#define MAX_FULLNESS_PERCENT 0.75       /* arbitrary */

#define IS_USED 0
#define IS_NOT_USED -1

/*******************************************************************************
***		PRIVATE FUNCTIONS
*******************************************************************************/
static uint64_t md5_hash_default(char *key);
static void* insert_key(hashmap_node *nodes, char *key, void *value, uint64_t hash, uint64_t number_nodes, uint64_t* used_nodes, short mallocd);
static float get_fullness(HashMap *h);
static void* __hashmap_set(HashMap *h, char *key, void *value, short mallocd);
static uint64_t worst_case(HashMap *h);



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
	uint64_t i;
	for (i = 0; i < INITIAL_NUM_ELEMENTS; i++) {
		h->nodes[i].is_used = IS_NOT_USED;
		h->nodes[i].key = NULL;
		h->nodes[i].value = NULL;
		h->nodes[i].hash = 0; // what would be the odds?
		h->nodes[i].mallocd = -1;
	}
	if (hash_function == NULL) {
		h->hash_function = &md5_hash_default;
	} else {
		h->hash_function = hash_function;
	}

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
    int bl = -1;
	while (bl == -1) {
		if (h->nodes[i].is_used == IS_NOT_USED) { //not found
			return NULL;
        } else if (h->nodes[i].hash == hash && strlen(key) == strlen(h->nodes[i].key) && strncmp(key, h->nodes[i].key, strlen(key)) == 0) {
    		return  h->nodes[i].value;
        }
        // lets see if we need to continue or if we have already gone all the way around
        if (bl == -1) {
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
	O(n): \n\
	Worst Case Search: \n", h->number_nodes, h->used_nodes, get_fullness(h));
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

static void* __hashmap_set(HashMap *h, char *key, void *value, short mallocd) {
	// check to see if we need to expand the hashmap
	if (get_fullness(h) >= MAX_FULLNESS_PERCENT) {
		uint64_t new_num_nodes = h->number_nodes * 2;
		hashmap_node *new_nodes = malloc(sizeof(hashmap_node) * new_num_nodes); // double each time
		uint64_t i;
		for (i = 0; i < new_num_nodes; i++) {
			new_nodes[i].is_used = IS_NOT_USED;
			new_nodes[i].key = NULL;
			new_nodes[i].value = NULL;
			new_nodes[i].hash = 0; // what would be the odds?
			new_nodes[i].mallocd = -1;
		}
		// move over all the original elements
		uint64_t t_used_nodes = 0;
		for (i = 0; i < h->number_nodes; i++) {
			if (h->nodes[i].is_used == IS_USED) {
				insert_key(new_nodes, h->nodes[i].key, h->nodes[i].value, h->nodes[i].hash, new_num_nodes, &t_used_nodes, h->nodes[i].mallocd);
				free(h->nodes[i].key);
			}
		}
		// move everthing over
		free(h->nodes);
		h->nodes = new_nodes;
		h->number_nodes = new_num_nodes;
		h->used_nodes = t_used_nodes;
	}
	// get the hash value
	uint64_t hash = h->hash_function(key);

	// find the index to insert into
	return insert_key(h->nodes, key, value, hash, h->number_nodes, &h->used_nodes, mallocd);
}

static void* insert_key(hashmap_node *nodes, char *key, void *value, uint64_t hash, uint64_t number_nodes, uint64_t* used_nodes, short mallocd) {
	uint64_t idx = hash % number_nodes;
	uint64_t i = idx;
	int bl = -1;
	while (bl != 0) {
		if (nodes[i].is_used == IS_NOT_USED) {
            nodes[i].is_used = 0;
			nodes[i].key = calloc(strlen(key) + 1, sizeof(char));
			strncpy(nodes[i].key, key, strlen(key));
    		nodes[i].value = value;
    		nodes[i].hash = hash;
			nodes[i].mallocd = mallocd;
			*used_nodes = *used_nodes + 1;
			bl = 0;
		} else if (nodes[i].hash == hash && strlen(key) == strlen(nodes[i].key) && strncmp(key, nodes[i].key, strlen(key)) == 0) {
            nodes[i].is_used = IS_USED;
			free(nodes[i].key);
			nodes[i].key = calloc(strlen(key) + 1, sizeof(char));
			strncpy(nodes[i].key, key, strlen(key));
			void *ret = NULL;
			if (nodes[i].mallocd == 0) { // we need to free it...
				free(nodes[i].value);
			} else {
				ret = nodes[i].value;
			}
    		nodes[i].value = value;
    		nodes[i].hash = hash;
			nodes[i].mallocd = mallocd;
            return ret;
        } else {
			i = (i + 1 == number_nodes) ? 0 : i + 1;
			if (i == idx) {	/* We can only have this happen if there are NO open locations */
				return NULL;
			}
		}
	}
	return value;
}

static float get_fullness(HashMap *h) {
	return h->used_nodes / (1.0 * h->number_nodes);
}
