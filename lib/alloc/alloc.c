#include "alloc.h"

#include <stdio.h>
#include <stdlib.h>

#define ALLOCATION_FAILURE_MSG "Failed to allocate.\n"

void *emalloc(size_t size) {
    void *ret = malloc(size);
    if (ret == NULL) {
        (void)fprintf(stderr, ALLOCATION_FAILURE_MSG);
        exit(EXIT_FAILURE);
    }
    return ret;
}

void *ecalloc(size_t nmemb, size_t size) {
    void *ret = calloc(nmemb, size);
    if (ret == NULL) {
        (void)fprintf(stderr, ALLOCATION_FAILURE_MSG);
        exit(EXIT_FAILURE);
    }
    return ret;
}
