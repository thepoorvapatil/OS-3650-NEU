#ifndef HMALLOC_H
#define HMALLOC_H

#include <stdint.h>
// Husky Malloc Interface
// cs3650 Starter Code

void* hrealloc(void* item, int64_t size);
void* hmalloc(size_t size);
void hfree(void* item);

#endif
