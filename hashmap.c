/*******************************************************************************
***
***	 Author: Tyler Barrus
***	 email:  barrust@gmail.com
***
***	 Version: 0.7.4
***
***	 License: MIT 2015
***
*******************************************************************************/

#include "hashmap.h"


#define INITIAL_NUM_ELEMENTS 1024
#define MAX_FULLNESS_PERCENT 0.25       /* arbitrary */


/*******************************************************************************
***		PRIVATE FUNCTIONS
*******************************************************************************/
static uint64_t default_hash(char *key);
static inline float __get_fullness(HashMap *h);
static inline int __calc_big_o(uint64_t num_nodes, uint64_t i, uint64_t idx);
static int   __allocate_hashmap(HashMap *h, uint64_t num_els, HashFunction hash_function);
static int   __relayout_nodes(HashMap *h);
static void* __get_node(HashMap *h, char *key, uint64_t hash, uint64_t *i, int *error);
static void  __assign_node(HashMap *h, char *key, void *value, short mallocd, uint64_t i, uint64_t hash);
static void* __hashmap_set(HashMap *h, char *key, void *value, short mallocd);
static void  __calc_stats(HashMap *h, uint64_t *worst_case, uint64_t *max_big_o, float *avg_big_o, float *avg_used_big_o, unsigned int *hash, unsigned int *idx);
static void __merge_sort(uint64_t *arr, uint64_t length);
static void __m_sort_merge(uint64_t *arr, uint64_t length, uint64_t mid);

/*******************************************************************************
***		FUNCTION DEFINITIONS
*******************************************************************************/
int hashmap_init_alt(HashMap *h, HashFunction hash_function) {
	return __allocate_hashmap(h, INITIAL_NUM_ELEMENTS, hash_function);
}

void hashmap_destroy(HashMap *h) {
	uint64_t i;
	for (i = 0; i < h->number_nodes; i++) {
		if (h->nodes[i] != NULL) {
			free(h->nodes[i]->key);
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
	uint64_t i, hash = h->hash_function(key);
	i = hash % h->number_nodes;
	int e;
	return __get_node(h, key, hash, &i, &e);
}

void* hashmap_remove(HashMap *h, char *key) {
	uint64_t i, hash = h->hash_function(key);
	i = hash % h->number_nodes;
	int e;
	void* ret = __get_node(h, key, hash, &i, &e);
	if (ret != NULL) {
		free(h->nodes[i]->key);
		if (h->nodes[i]->mallocd == 0) {
			free(h->nodes[i]->value);
			ret = NULL;
		}
		free(h->nodes[i]);
		h->nodes[i] = NULL;
		h->used_nodes--;
		// TODO: Refactor relayout nodes to allow for re-layout from a particular position (i)
		__relayout_nodes(h);
	}
	return ret;
}

void hashmap_stats(HashMap *h) {
	uint64_t max, wc;
	float avg, avg_used;
	unsigned int hc, ic;
	__calc_stats(h, &wc, &max, &avg, &avg_used, &hc, &ic);
	/* size is the size of a single hashmap
	   plus the size of the array of pointers
	   plus the size of the number of allocated nodes
	   NOTE: this does NOT include the key and value sizes */
	uint64_t size = sizeof(HashMap) + (sizeof(hashmap_node*) * h->number_nodes) + (sizeof(hashmap_node) * h->used_nodes);
	printf("HashMap:\n\
	Number Nodes: %" PRIu64 "\n\
	Used Nodes: %" PRIu64 "\n\
	Fullness: %f%%\n\
	Average O(n): %f\n\
	Average Used O(n): %f\n\
	Max O(n): %" PRIu64 "\n\
	Max Consecutive Buckets Used: %" PRIu64 "\n\
	Number Hash Collisions: %d\n\
	Number Index Collisions: %d\n\
	Size on disk (bytes): %" PRIu64 "\n", h->number_nodes, h->used_nodes,
	__get_fullness(h) * 100.0, avg, avg_used, max, wc, hc, ic, size);
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
	return keys;
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
static uint64_t default_hash(char *key) { // FNV-1a hash (http://www.isthe.com/chongo/tech/comp/fnv/)
	int i, len = strlen(key);
	char *p = calloc(len + 1, sizeof(char));
	strncpy(p, key, len);
	uint64_t h = 14695981039346656073ULL; // FNV_OFFSET 64 bit
	for (i = 0; i < len; i++){
		h = h ^ (unsigned char) p[i];
		h = h * 1099511628211ULL; // FNV_PRIME 64 bit
	}
	free(p);
	return h;
}

static int  __allocate_hashmap(HashMap *h, uint64_t num_els, HashFunction hash_function) {
	uint64_t i;
	if (num_els == INITIAL_NUM_ELEMENTS) {
		h->nodes = (hashmap_node**) malloc(num_els * sizeof(hashmap_node*));
		if (h->nodes == NULL) {return HASHMAP_FAILURE;}
		h->number_nodes = num_els;
		for (i = 0; i < h->number_nodes; i++) {
			h->nodes[i] = NULL;
		}
		h->used_nodes = 0;
		h->hash_function = (hash_function == NULL) ? &default_hash : hash_function;
	} else {
		hashmap_node** tmp = realloc(h->nodes, num_els * sizeof(hashmap_node*));
		if (h->nodes == NULL) {return HASHMAP_FAILURE;}
		h->nodes = tmp;
		uint64_t orig_num_els = h->number_nodes;
		for (i = orig_num_els; i < num_els; i++) {
			h->nodes[i] = NULL;
		}
		h->number_nodes = num_els;
		int q = 0, j = 0;
		// TODO: The math to see if this ever needs to be done more than once
		while (q == 0) {
			q = __relayout_nodes(h);
		}
	}
	return HASHMAP_SUCCESS;
}

static int  __relayout_nodes(HashMap *h) {
	int moved_one = 1;
	uint64_t i;
	for (i = 0; i < h->number_nodes; i++) {
		if(h->nodes[i] != NULL) {
			uint64_t id;
			int error;
			void *tn = __get_node(h, h->nodes[i]->key, h->nodes[i]->hash, &id, &error);
			if (id != i) {
				moved_one = 0;
				h->nodes[id] = h->nodes[i];
				h->nodes[i] = NULL;
			}
		}
	}
	return moved_one;
}

static void* __get_node(HashMap *h, char *key, uint64_t hash, uint64_t *i, int *error) {
	*error = 0; // no errors
	uint64_t idx = *i = hash % h->number_nodes;
	while (true) {
		if (h->nodes[*i] == NULL) { //not found
			return NULL;
		} else if (h->nodes[*i]->hash == hash && strlen(key) == strlen(h->nodes[*i]->key) && strncmp(key, h->nodes[*i]->key, strlen(key)) == 0) {
			return  h->nodes[*i]->value;
		} else {
			// lets see if we need to continue or if we have already gone all the way around
			*i = (*i + 1 == h->number_nodes) ? 0 : *i + 1;
			if (*i == idx) {	// This can only have this happen if there are NO open locations
				*error = -1;    // which signifies that the hashmap is full
				return NULL;
			}
		}
	}
}

static void* __hashmap_set(HashMap *h, char *key, void *value, short mallocd) {
	// check to see if we need to expand the hashmap
	if (__get_fullness(h) >= MAX_FULLNESS_PERCENT) {
		uint64_t num_nodes = h->number_nodes;
		__allocate_hashmap(h, num_nodes * 2, h->hash_function);
	}
	// get the hash value
	uint64_t hash = h->hash_function(key);
	uint64_t i;
	int error;
	void *tmp = __get_node(h, key, hash, &i, &error);
	if (tmp == NULL && error == -1) {
		printf("Error: Unable to insert due to the hashmap being full\n");
		return NULL;
	} else  if (tmp != NULL) {
		if (h->nodes[i]->mallocd == 0) {free(h->nodes[i]->value);}
		h->nodes[i]->value = value;
	} else {
		__assign_node(h, key, value, mallocd, i, hash);
	}
	return value;
}

static void  __assign_node(HashMap *h, char *key, void *value, short mallocd, uint64_t i, uint64_t hash) {
	h->nodes[i] = malloc(sizeof(hashmap_node));;
	h->nodes[i]->key = calloc(strlen(key) + 1, sizeof(char));
	strncpy(h->nodes[i]->key, key, strlen(key));
	h->nodes[i]->value = value;
	h->nodes[i]->hash = hash;
	h->nodes[i]->mallocd = mallocd;
	h->used_nodes++;
}

static inline float __get_fullness(HashMap *h) {
	return h->used_nodes / (float) h->number_nodes;
}

static void __calc_stats(HashMap *h, uint64_t *worst_case, uint64_t *max_big_o, float *avg_big_o, float *avg_used_big_o, unsigned int *hash, unsigned int *idx) {
	uint64_t i, sum = 0, max = 0, cur = 0, wc = 0, sum_used = 0, j = 0;
	unsigned int hash_col = 0, idx_col = 0;
	uint64_t *hashes = calloc(h->used_nodes, sizeof(uint64_t));
	uint64_t *idxs = calloc(h->used_nodes, sizeof(uint64_t));
	for (i = 0; i < h->number_nodes; i++) {
		if (h->nodes[i] != NULL) {
			cur++;
			uint64_t idx = h->nodes[i]->hash % h->number_nodes;
			uint64_t O = __calc_big_o(h->number_nodes, i, idx);
			sum_used += O;
			sum += O;
			if (O > max) {
				max = O;
			}
			hashes[j] = h->nodes[i]->hash;
			idxs[j] = idx;
			j++;
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

	// sort the results
	__merge_sort(hashes, h->used_nodes);
	__merge_sort(idxs, h->used_nodes);

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

static inline int __calc_big_o(uint64_t num_nodes, uint64_t i, uint64_t idx) {
	return (i < idx) ? i + num_nodes - idx + 1 : 1 + i - idx;
}

void __merge_sort(uint64_t *arr, uint64_t length) {
	if (length < 2) {
		return;
	}
	uint64_t mid = length / 2;
	__merge_sort(arr, mid);
	__merge_sort(arr + mid, length - mid);
	__m_sort_merge(arr, length, mid);
}


void __m_sort_merge(uint64_t *arr, uint64_t length, uint64_t mid) {
	uint64_t *tmp = malloc((length) * sizeof(uint64_t));
	uint64_t l = 0, r = mid, i = 0;
	while(l < mid && r < length) { // sort until one half is empty
		tmp[i++] = (arr[l] > arr[r]) ? arr[r++] : arr[l++];
	}
	// move the rest of the remaining array to tmp
	while (l < mid) {
		tmp[i++] = arr[l++];
	}
	while (r < length) {
		tmp[i++] = arr[r++];
	}
	// move it over
	for (i = 0; i < length; i++) {
		arr[i] = tmp[i];
	}
	free(tmp);
}
