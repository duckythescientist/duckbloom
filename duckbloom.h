#ifndef LIBDUCKBLOOM_H
#define LIBDUCKBLOOM_H

#ifdef __cplusplus
extern "C" {
#endif

#define __DUCKBLOOM_VERSION_MAJOR 0
#define __DUCKBLOOM_VERSION_MINOR 3

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>


typedef struct bloom_ctx bloom_ctx;


/**
    Allocate the memory for a context handle
*/
bloom_ctx * bloom_malloc();

/**
    Free the context handle memory
*/
void bloom_free(bloom_ctx * ctx);

/**
    Synchronize file writes
*/
void bloom_sync(bloom_ctx * ctx);

/**
    Sync, unmap memory, close file
*/
void bloom_close(bloom_ctx * ctx);

/**
    Open a bloom filter instance with mmapf flags
    Returns nonzero on error
*/
int _bloom_open_create(bloom_ctx * ctx, const char * fname, size_t size, int hashcount, int flags);

/**
    Open a bloom filter and force creation
    Returns nonzero on error
*/
int bloom_create(bloom_ctx * ctx, const char * fname, size_t size, int hashcount);

/**
    Open a bloom filter
    Creates the file if needed
    Pass NULL for fname for an in-memory filter
    Returns nonzero on error
*/
int bloom_open(bloom_ctx * ctx, const char * fname, size_t size, int hashcount);

/**
    Add a 160-bit (20-Byte) object to the filter
    Supposedly this is faster than the arbitrary data method
*/
void bloom_add160(bloom_ctx * ctx, uint8_t * data);

/**
    Add arbitrary data to the filter
*/
void bloom_add(bloom_ctx * ctx, uint8_t * data, size_t len);

/**
    Check if 20-Byte object is in the filter
*/
bool bloom_check160(bloom_ctx * ctx, uint8_t * data);

/**
    Check if object is in the filter
*/
bool bloom_check(bloom_ctx * ctx, uint8_t * data, size_t len);

#ifdef __cplusplus
}
#endif

#endif // LIBDUCKBLOOM