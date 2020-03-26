#ifndef HMALLOC_H
#define HMALLOC_H

// Husky Malloc Interface
// cs3650 Starter Code

#include <stdint.h>

void* opt_realloc(void* item, int64_t size);
void* opt_malloc(size_t size);
void opt_free(void* item);
void allocate_more_bins();


#endif
