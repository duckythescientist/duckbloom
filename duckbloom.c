#include <unistd.h>
// #include <string.h>
// #include <fcntl.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <stdint.h>

// #include <sys/types.h>
// #include <sys/stat.h>
#include <sys/mman.h>

#include "duckbloom.h"
#include "mmapf.h"

#define XXH_INLINE_ALL
#include "xxHash/xxhash.h"

#include "logc/log.h"


struct bloom_ctx {
    mmapf_ctx mapping;
    size_t size;
    int hashcount;
};


void bloom_sync(bloom_ctx * ctx) {
    // TODO error checking
    if (ctx->mapping.fd >= 0) {
        msync(ctx->mapping.mem, ctx->mapping.file_sz, MS_SYNC);
        fsync(ctx->mapping.fd);
    }
}


void bloom_close(bloom_ctx * ctx) {
    munmapf(&ctx->mapping);
}


bloom_ctx * bloom_malloc() {
    return (bloom_ctx *)malloc(sizeof(bloom_ctx));
}

void bloom_free(bloom_ctx * ctx) {
    free(ctx);
}


int _bloom_open_create(bloom_ctx * ctx, const char * fname, size_t size, int hashcount, int flags) {
    if(size & (size - 1)) {
        LOGF("Bloom filter size is non-power-of-two: %zu", size);
        return -1;
    }

    int mmapf_ret = mmapf(&ctx->mapping, fname, size, flags);
    if(mmapf_ret != MMAPF_OKAY) {
        LOGF("Can't open/create bloom file %s: %s", fname, mmapf_strerror(mmapf_ret));
        return -1;
    }
    ctx->size = size;
    ctx->hashcount = hashcount;
    return 0;
}


int bloom_create(bloom_ctx * ctx, const char * fname, size_t size, int hashcount) {
    int flags = MMAPF_RW|MMAPF_CR|MMAPF_EX|MMAPF_RND|MMAPF_PRE|MMAPF_WILLNEED;
    return _bloom_open_create(ctx, fname, size, hashcount, flags);
}


int bloom_open(bloom_ctx * ctx, const char * fname, size_t size, int hashcount) {
    int flags = MMAPF_RW|MMAPF_CR|MMAPF_RND|MMAPF_PRE|MMAPF_WILLNEED;
    return _bloom_open_create(ctx, fname, size, hashcount, flags);
}


void bloom_add160(bloom_ctx * ctx, uint8_t * data) {
    uint64_t mask = (uint64_t)ctx->size;
    mask -= 1;
    uint64_t hash = 0;
    uint8_t * bloom = (uint8_t *)ctx->mapping.mem;
    for(int i=0; i<ctx->hashcount; i++) {
        hash = XXH64(data, 20, i);
        uint64_t offset = hash & mask;
        bloom[offset >> 3] |= 1<<(offset & 7);
    }
}


void bloom_add(bloom_ctx * ctx, uint8_t * data, size_t len) {
    uint64_t mask = (uint64_t)ctx->size;
    mask -= 1;
    uint64_t hash = 0;
    uint8_t * bloom = (uint8_t *)ctx->mapping.mem;
    for(int i=0; i<ctx->hashcount; i++) {
        hash = XXH64(data, len, i);
        uint64_t offset = hash & mask;
        bloom[offset>>3] |= 1<<(offset & 7);
    }
}


bool bloom_check160(bloom_ctx * ctx, uint8_t * data) {
    uint64_t mask = (uint64_t)ctx->size;
    mask -= 1;
    uint64_t hash = 0;
    uint8_t * bloom = (uint8_t *)ctx->mapping.mem;
    for(int i=0; i<ctx->hashcount; i++) {
        hash = XXH64(data, 20, i);
        uint64_t offset = hash & mask;
        if(!(bloom[offset>>3] & (1<<(offset & 7)))) {
            return false;
        }
    }
    return true;
}


bool bloom_check(bloom_ctx * ctx, uint8_t * data, size_t len) {
    uint64_t mask = (uint64_t)ctx->size;
    mask -= 1;
    uint64_t hash = 0;
    uint8_t * bloom = (uint8_t *)ctx->mapping.mem;
    for(int i=0; i<ctx->hashcount; i++) {
        hash = XXH64(data, len, i);
        uint64_t offset = hash & mask;
        if(!(bloom[offset>>3] & (1<<(offset & 7)))) {
            return false;
        }
    }
    return true;
}
