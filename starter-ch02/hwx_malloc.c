#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stddef.h>

#include "xmalloc.h"
// TODO: This file should be replaced by another allocator implementation.
//
// If you have a working allocator from the previous HW, use that.
//
// If your previous homework doesn't work, you can use the provided allocator
// taken from the xv6 operating system. It's in xv6_malloc.c
//
// Either way:
//  - Replace xmalloc and xfree below with the working allocator you selected.
//  - Modify the allocator as nessiary to make it thread safe by adding exactly
//    one mutex to protect the free list. This has already been done for the
//    provided xv6 allocator.
//  - Implement the "realloc" function for this allocator.

typedef struct hm_stats {
    long pages_mapped;
    long pages_unmapped;
    long chunks_allocated;
    long chunks_freed;
    long free_length;
} hm_stats;

typedef struct husky_node {
	size_t size;
	struct husky_node* next;
} husky_node;



const size_t PAGE_SIZE = 4096;
static hm_stats stats; // This initializes the stats to 0.
husky_node* husky_list = NULL;
pthread_mutex_t husky_list_lock;
int husky_list_lock_initialized = 0;

long
free_list_length()
{
    husky_node* nxt = husky_list;
	int count = 0;
	while(nxt != NULL) {
		count++;
		nxt = nxt->next;
	}
 	return count;
}

hm_stats*
hgetstats()
{
    stats.free_length = free_list_length();
    return &stats;
}

void
hprintstats()
{
    stats.free_length = free_list_length();
    fprintf(stderr, "\n== husky malloc stats ==\n");
    fprintf(stderr, "Mapped:   %ld\n", stats.pages_mapped);
    fprintf(stderr, "Unmapped: %ld\n", stats.pages_unmapped);
    fprintf(stderr, "Allocs:   %ld\n", stats.chunks_allocated);
    fprintf(stderr, "Frees:    %ld\n", stats.chunks_freed);
    fprintf(stderr, "Freelen:  %ld\n", stats.free_length);
}

static
size_t
div_up(size_t xx, size_t yy)
{
    // This is useful to calculate # of pages
    // for large allocations.
    size_t zz = xx / yy;

    if (zz * yy == xx) {
        return zz;
    }
    else {
        return zz + 1;
    }
}

void*
removeFromList(size_t size)
{
	if (husky_list == NULL) {
		return 0;
	}

	husky_node* prev = NULL;
	husky_node* head = husky_list;
	void* tmp = NULL;

	while(head != NULL) {
		if (head->size >= size) {
			if (prev == NULL) {
				husky_list = head->next;
			} else if (head->next == NULL) {
				prev->next = NULL;
			} else {
				prev->next = head->next;
			}
			head->next = NULL;
			tmp = head;
			return tmp;
		} else {
			prev = head;
			head = head->next;
		}
	}
	return tmp;
}

void
husky_list_sort()
{
	husky_node* x, *y, *z;

    x = husky_list;
    husky_list = NULL;

    while(x != NULL) {
        z = x;
        x = x->next;
        if (husky_list != NULL) {
            if (z > husky_list) {
                y = husky_list;
                while ((y->next != NULL) && (z > y->next)) {
                        y = y->next;
                }
                z->next = y->next;
                y->next = z;
            } else {
                z->next = husky_list;
                husky_list = z;
            }
        } else {
            z->next = NULL;
            husky_list = z;
        }
    }
}

void
coalesce_husky_list()
{
	husky_node* head = husky_list;
	husky_node* next = head->next;
    size_t size = head->size;

	while(next != NULL) {
		char* cHead = (char*)head;
		char* cNext = (char*)next;

		if (cHead + size == cNext) {
			head->size += next->size;
			head->next = next->next;
		} else {
			head = head->next;
		}
		size = head->size;
		next = head->next;
	}
}

void*
xmalloc(size_t size)
{
    stats.chunks_allocated += 1;
    size += sizeof(size_t);

	if (!husky_list_lock_initialized) {
		pthread_mutex_init(&husky_list_lock, 0);
		husky_list_lock_initialized = 1;
	}
    
	if (size < PAGE_SIZE) {
		pthread_mutex_lock(&husky_list_lock);
		//remove memory from the husky_list of size
		void* tmp = NULL;
		int remaining_mem;
		if (husky_list != NULL) {
			tmp = removeFromList(size);
		}
		if (tmp != NULL) { 
			size_t* tmpSize = tmp;
			remaining_mem = *tmpSize - size;  
		} 
        //not enough mem
        else { 
			tmp = mmap(0, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
			stats.pages_mapped += 1;
			remaining_mem = PAGE_SIZE - size;
		}
		//add extra memory
		if (remaining_mem >= sizeof(husky_node)) {
			char* ctmp = tmp;
			ctmp += size;
			husky_node* newtmp = (husky_node*)ctmp;
			newtmp->size = remaining_mem;
			newtmp->next = NULL;
			if (husky_list != NULL) {
				husky_node* head = husky_list;
				while(head->next != NULL) {
					head = head->next;
				}
				head->next = newtmp;
			} else {
				husky_list = newtmp;
			}
		}
		pthread_mutex_unlock(&husky_list_lock);
		//store at the beginning of memory
		size_t* sizePtr = tmp;
		*sizePtr = size;
		sizePtr++;
		void* start = sizePtr;
		return start;
	} else {
		int numOfPages = div_up(size, PAGE_SIZE);
		stats.pages_mapped += numOfPages;

		int numOfMem = numOfPages * PAGE_SIZE;
		void* tmp = mmap(0, numOfMem, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
		size_t* sizePtr = tmp;
		*sizePtr = numOfMem;
		sizePtr++;
		void* start = sizePtr;
		return start;
	}
}

void
xfree(void* item)
{
    stats.chunks_freed += 1;
	void* start = item - sizeof(size_t);
    size_t* size = start;
	husky_node* newList = (husky_node*)start;
	if (*size < PAGE_SIZE) {
		pthread_mutex_lock(&husky_list_lock);
		husky_node* head = husky_list;
		husky_node* next = head->next;
		while(next != NULL) {
			head = next;
			next = next->next;
		}
		head->next = newList;
		head->next->size = *size;
		head->next->next = NULL;

        husky_list_sort();
        coalesce_husky_list();
		pthread_mutex_unlock(&husky_list_lock);
	} else {
		int numOfPages = div_up(*size, PAGE_SIZE);
		stats.pages_unmapped += numOfPages;
		munmap(item, *size);
	}
}

void* 
xrealloc(void* prev, size_t bytes)
{
	if (prev == NULL) {
		return xmalloc(bytes);
	}
	void* start = prev - sizeof(size_t);
	size_t* size = start;
	if (*size > bytes) {
		int remaining_mem = bytes - *size;
		if (remaining_mem >= sizeof(husky_node)) {
			pthread_mutex_lock(&husky_list_lock);
			start += *size;
			husky_node* newList = (husky_node*)start;
			newList->size = remaining_mem;
			newList->next = NULL;
			if (husky_list != NULL) {
				husky_node* head = husky_list;
				while(head->next != NULL) {
					head = head->next;
				}
				head->next = newList;
			} else {
				husky_list = newList;
			}
			pthread_mutex_unlock(&husky_list_lock);
			*size = bytes;
			return prev;
		}
	} else if (*size == bytes) {
		return prev;
	} else {
		void* start = prev - sizeof(size_t);
		size_t* size = start;
		//new memory
		void* newMemory = xmalloc(bytes);
		memcpy(newMemory, prev, *size);
		xfree(prev);
		return newMemory;
	}
	return 0;
}