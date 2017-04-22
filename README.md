# hashmap
Hashmap implementation written in C.

Hashmaps are a key value store with quick look up times. The hashmap_stats()
function will provide a summary of the current lookup times including average,
worst case key not found, and worst case key found.

This hashmap implementation is a simple and, generally, quick method to include
a hashmap in C programs. It was developed to provide a basis for testing and
benchmarking performance along with providing a purposeful, low overhead
library.

## License
MIT 2016

## Usage:
```c
#include "hashmap.h"

HashMap h;
hashmap_init(&h);

hashmap_set_string(&h, "google", "search engine, android, web ads");
hashmap_set_string(&h, "facebook", "social media site");
hashmap_set_string(&h, "twitter", "the sound of little birds");

char* tmp = (char*)hashmap_get(&h, "google");
if (tmp == NULL) {
	printf("'google' was not in the hashmap\n");
} else {
	printf("key: test\tvalue: %s\n", tmp);
	hashmap_set_string(&h, "google", "search engine, android, web ads, and automobiles");
}

hashmap_stats(&h);
hashmap_destroy(&h);
```

## Required Compile Flags:
None

### Future Enhancements:
* Allow for sorting from the `hashmap_keys` function
* Prove if relaying out nodes needs to do more than 1 loop
* Add ability to merge Hashmaps
* Add additional utility functions
  * ???
