## Current Version

### Master

* Fix segfault when calculating stats on empty hashmap

### Version 0.7.7

* Add clear hashmap functionality
* Performance improvements
    * Reduce calls to strlen
    * strncpy replaced with memcpy since non-overlapping memory

### Version 0.7.6

* Fix for possible race condition when **OpenMP** used
* Remove `#define` construct to allow functions to better be used directly in
if-statements
* Start re-laying out nodes from removal location
* Unique HashFunction type when using with related libraries

### Version 0.7.5

* Add support for thread safety when using **OpenMP**
    * Only available when compiling hashmap.c with the *-fopenmp* flag
    * **Note:** Does not speed up insertion or lookup, just makes it so multiple
    threads can access the same hashmap at the same time
* Updated the repository directory structure
* Improvements to Makefile

### Version 0.7.4

* Unified header added
* Minor fixes for Mac OS X

### Version 0.7.3

* Remove need for sorting_algos.h
* Converted hashing algorithm to FNV-1a
    * Remove need for the *-lcrypto* flag

#### Earlier Versions

* Setup hashmap with add, remove, and retrieve functionality
