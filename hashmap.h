/*******************************************************************************
***
***	 Author: Tyler Barrus
***	 email:  barrust@gmail.com
***
***	 Version: 0.5.2
***	 Purpose: Simple, yet effective, hashmap implementation
***
***	 License: MIT 2015
***
***	 URL: https://github.com/barrust/hashmap
***
***	 Usage:
***
***	 Required Compile Flags: -lcrypto
***
*******************************************************************************/

#ifndef __HASH_MAP_H__
#define __HASH_MAP_H__

#include <stdlib.h>         /* malloc, etc */
#include <inttypes.h>       /* PRIu64 */
#include <stdio.h>          /* printf */
#include <string.h>         /* strncpy, strncmp */
#include <stdbool.h>        /* boolean: true false */
#include <openssl/md5.h>



#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#define HASHMAP_VERSION "0.5.2"
#define HASHMAP_MAJOR 0
#define HASHMAP_MINOR 5
#define HASHMAP_REVISION 2

#define HASHMAP_FAILURE -1
#define HASHMAP_SUCCESS 0

#define hashmap_get_version()	(HASHMAP_VERSION)
#define hashmap_number_keys(h)   (h.used_nodes)


typedef uint64_t (*HashFunction) (char *key);

/*******************************************************************************
***    Data structures
*******************************************************************************/
typedef struct hashmap_node {
	char *key;
	void *value;
	uint64_t hash;
	short mallocd; /* signals if need to deallocate the memory */
	int O;
	uint64_t idx;  /* store what node it should be part of */
} hashmap_node;

typedef struct hashmap {
	hashmap_node **nodes;
	uint64_t number_nodes;
	uint64_t used_nodes;
	HashFunction hash_function;
} HashMap;

/* initialize the hashmap using the default hashing function */
int hashmap_init(HashMap *h);

/* initialize the hashmap using the provided hashing function */
int hashmap_init_alt(HashMap *h, HashFunction hash_function);

/*
	frees all memory allocated by the hashmap library
	NOTE: If the value is malloc'd memory, it is up to the user to free it
*/
void hashmap_destroy(HashMap *h);

/*
	Adds the key to the hashmap or updates the hashmap if it is already present
	If it updates instead of adds, returns the pointer to the replaced value,
	(unless it is mallocd by the hashmap on insert) otherwise it returns the
	pointer to the new value. Returns NULL if there is an error.
*/
void* hashmap_set(HashMap *h, char *key, void *value);

/*
	Adds the key to the hashmap or updates the key if already present. Also
	signals to the system to do a simple 'free' command on the value on
	destruction.
*/
void* hashmap_set_alt(HashMap *h, char *key, void * value);

/*
	Returns the pointer to the value of the found key, or NULL if not found
*/
void* hashmap_get(HashMap *h, char *key);

/*
	Returns an array of all keys in the hashmap.
	NOTE: It is up to the caller to free the array returned.
*/
char** hashmap_keys(HashMap *h);

/*
	Prints out some basic stats about the hashmap
*/
void hashmap_stats(HashMap *h);

/*
	Easily add an int, this will malloc everything for the user and will signal
	to de-allocate the memory on destruction
*/
int* hashmap_set_int(HashMap *h, char *key, int value);

/*
	Easily add a string, this will malloc everything for the user and will signal
	to de-allocate the memory on destruction
*/
char* hashmap_set_string(HashMap *h, char *key, char *value);


#endif /* END HASHMAP HEADER */
