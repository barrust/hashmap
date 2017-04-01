
### Version 0.7.5

* Add support for thread safe when using **OpenMP**
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
