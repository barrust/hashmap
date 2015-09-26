/*******************************************************************************
***
***	 Author: Tyler Barrus
***	 email:  barrust@gmail.com
***
***	 Version:
***	 Purpose: Simple, yet effective, hashmap implementation
***
***	 License: MIT 2015
***
***	 URL: https://github.com/barrust/hashmap
***
***	 Usage:
***
***	 Required Compile Flags:
***
*******************************************************************************/
#ifndef __SORT_ALGOS_H__
#define __SORT_ALGOS_H__

/* function definitions */
void bubble_sort(uint64_t* arr, uint64_t length);
void insertion_sort(uint64_t* arr, uint64_t length);
void merge_sort(uint64_t *arr, uint64_t length);
void __m_sort_merge(uint64_t *arr, uint64_t length, uint64_t mid);

void bubble_sort(uint64_t* arr, uint64_t length) {
    uint64_t i, j;
    for (i = 0; i < length; i++) {
        for (j = 0; j < length - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
}

void insertion_sort(uint64_t* arr, uint64_t length) {
    uint64_t i, j;
    for (i = 1; i < length -1; i++) {
        j = i - 1;
        // start shifting elements until it is no longer lower
        uint64_t tmp = arr[i];
        while (arr[j] > tmp) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = tmp;
    }
}

void merge_sort(uint64_t *arr, uint64_t length) {
    if (length < 2) {
        return;
    }
    uint64_t mid = length / 2;
    merge_sort(arr, mid);
    merge_sort(arr + mid, length - mid);
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

#endif
