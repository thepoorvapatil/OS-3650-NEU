#include <stdint.h>
#include <sys/mman.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include "opt_malloc.h"
#include "xmalloc.h"

int
ilog2(long x)
{
    int counter = 0;
    long num = 1;
    while (num < x)
    {
        num <<= 1;
        counter++;
    }
    return counter;
}

int
ilog2floor(long x)
{
    if (x == 0) {
        return 0;
    }
    
    int counter = -1;
    long num = 1;
    while (num <= x)
    {
        num <<= 1;
        counter++;
    }
    return counter;
}


int ipow2 (long x)
{
    long num = 1;
    while (x > 0) {
    num <<= 1;
    x--;
    }
    return num;
}

typedef struct husky_node {
	size_t size;
	struct husky_node* next;
} husky_node;

const size_t PAGE_SIZE = 4096;
__thread  hm_stats stats; // This initializes the stats to 0.
__thread husky_node* bckts[7] = {0, 0, 0, 0, 0, 0, 0};

void add_mem_to_bckts(husky_node* cell);

void
free_list_length()
{
	int vals[7] = {32, 64, 128, 256, 512, 1024, 2048};
	int counts[7] = {0, 0, 0, 0, 0, 0, 0};
	husky_node* head = NULL;
	for (int ii = 0; ii < 7; ++ii) {
		head = bckts[ii];
		while (head != NULL) {
			counts[ii]++;
			head = head->next;
		}
		fprintf(stderr, "Freelen for size %i: %i\n", vals[ii], counts[ii]);
	}
}

//CHANGED
hm_stats*
hgetstats()
{
    return &stats;
}

//UNCHANGED
void
hprintstats()
{
	fprintf(stderr, "\n== husky malloc stats ==\n");
	fprintf(stderr, "Mapped:   %ld\n", stats.pages_mapped);
	fprintf(stderr, "Unmapped: %ld\n", stats.pages_unmapped);
	fprintf(stderr, "Allocs:   %ld\n", stats.chunks_allocated);
	fprintf(stderr, "Frees:    %ld\n", stats.chunks_freed);
	free_list_length();
}

//UNCHANGED
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

void
coalesce_free_list(int bin)
{
	// Coalesce the bin
	husky_node* head = bckts[bin];
	while(head != NULL && head->next != NULL) {
		if (((int64_t)head + head->size == ((int64_t)head->next))
				&& (head->size + head->next->size != 96)) {
			head->size += head->next->size;
			head->next = head->next->next;
		}
		head = head->next;
	}
	// Add the coalesced cells to their larger-sized bckts
	husky_node* prev = NULL;
	head = bckts[bin];
	husky_node* retCell = NULL;
	int size = ipow2(bin + 5);
	while(head != NULL) {
		if (head->size > size) {
			if (prev == NULL) {
				bckts[bin] = head->next;
			} else if (head->next == NULL) {
				prev->next = NULL;
			} else {
				prev->next = head->next;
			}
			retCell = head;
			head = head->next;
			retCell->next = NULL;
			// A little duplicate code from add_mem_to_bckts to quickly add it
			int newBin = ilog2(retCell->size) - 5;
			husky_node* binToAddTo = bckts[newBin];
			if (binToAddTo == NULL || (binToAddTo > retCell)) {
				retCell->next = bckts[newBin];
				bckts[newBin] = retCell;
			} else {
				while(binToAddTo->next != NULL && (binToAddTo->next < retCell)) {
					binToAddTo = binToAddTo->next;
				}
				binToAddTo->next = retCell;
			}
		} else {
			prev = head;
			head = head->next;
		}
	}
}

size_t
round_to_next_power_of_two(size_t size)
{
	if (size <= 32) {
		return 32;
	} else {
		return ipow2(ilog2(size));
	}
}

void
add_mem_to_bckts(husky_node* cell)
{	
	size_t size = cell->size;
	cell->next = NULL;
	int s = size;
	char* cCell = (char*)cell;
	while (s > 0) {
		int bin = ilog2floor(s) - 5;
		if (bin > 6) {
			bin = 6;
		}
		size_t nextSize = ipow2(bin + 5);
		husky_node* newCell = (husky_node*)cCell;
		cCell += nextSize;
		s -= nextSize;
		newCell->size = nextSize;
		newCell->next = NULL;

		husky_node* binToAddTo = bckts[bin];
		if (binToAddTo == NULL || (binToAddTo > newCell)) {
			newCell->next = bckts[bin];
			bckts[bin] = newCell;
		} else {
			while(binToAddTo->next != NULL && (binToAddTo->next < newCell)) {
				binToAddTo = binToAddTo->next;
			}
			binToAddTo->next = newCell;
		}
		if (bin != 6) {
			coalesce_free_list(bin);
		}
	}
}

void*
remove_mem_from_bckts(size_t size)
{
	int bin = ilog2(size) - 5;
	husky_node* binToRemoveFrom = bckts[bin];
	if (binToRemoveFrom != NULL) {
		// remove the first entry in list
		bckts[bin] = binToRemoveFrom->next;
		binToRemoveFrom->next = NULL;
		return (void*)binToRemoveFrom;
	} else {
		void* cell = NULL;
		for (int ii = bin + 1; ii < 7; ++ii) {
			if (bckts[ii] != NULL) {
				husky_node* largerCell = bckts[ii];
				bckts[ii] = largerCell->next;
				size_t prevSize = largerCell->size;
				// split the larger cell into the correct sizes
				husky_node* new = largerCell; // the mem to be returned
				new->size = size;
				new->next = NULL;
				size_t newSize = prevSize - size;
				char* cCell = (char*)new;
				cCell += size;
				husky_node* old = (husky_node*)cCell;
				old->size = newSize;
				old->next = NULL;
				// add the leftover chunk back into the bckts
				add_mem_to_bckts(old);
				cell = (void*)new;
				break;
			}
		}
		return cell;
	}
}

void*
xmalloc(size_t size)
{
	stats.chunks_allocated += 1;
	size += sizeof(size_t);
	if (size < PAGE_SIZE && size <= 2048) {
		// Try to remove a power of 2 sized memory from bckts
		size = round_to_next_power_of_two(size);
		void* cell = remove_mem_from_bckts(size);
		if (cell != NULL) {
			char* cCell = (char*)cell;
			cCell += sizeof(size_t);
			return (void*)cCell;
		} else { // Not enough memory in bckts
			cell = mmap(0, PAGE_SIZE, PROT_READ | PROT_WRITE,
					MAP_SHARED | MAP_ANONYMOUS, -1, 0);
			stats.pages_mapped += 1;
			char* cCell = cell;
			cCell += size;
			void* leftover = (void*)cCell;
			husky_node* fCell = (husky_node*)cell;
			fCell->size = size;
			fCell->next = NULL;
			// add leftover memory to bckts
			husky_node* loCell = (husky_node*)leftover;
			loCell->size = PAGE_SIZE - size;
			loCell->next = NULL;
			add_mem_to_bckts(loCell);
			char* retMem = (char*)fCell;
			retMem += sizeof(size_t);
			return (void*)retMem;
		}
	} else {
		if (size > 2048 && size < PAGE_SIZE) {
			size = PAGE_SIZE;
		}
		// Calculate number of pages needed for this block
		int numOfPages = div_up(size, PAGE_SIZE);
		stats.pages_mapped += numOfPages;
		// Allocate that many pages with mmap
		int numOfMem = numOfPages * PAGE_SIZE;
		void* cell = mmap(0, numOfMem, PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_ANONYMOUS, -1, 0);
		// Fill in the size of the block as (# of pages * 4096)
		size_t* sizePtr = cell;
		*sizePtr = numOfMem;
		sizePtr++;
		// Return a pointer to the block *after* the size field
		void* start = sizePtr;
		return start;
	}
}

void
xfree(void* item)
{
	stats.chunks_freed += 1;
	char* start = item;
	start -= sizeof(size_t);
	husky_node* freedCell = (husky_node*)start;
	if (freedCell->size < PAGE_SIZE) {
		freedCell->next = NULL;
		add_mem_to_bckts(freedCell);
	} else {
		int numOfPages = div_up(freedCell->size, PAGE_SIZE);
		stats.pages_unmapped += numOfPages;
		munmap(item, freedCell->size);
	}
}

void* 
xrealloc(void* prev, size_t bytes)
{
	char* cPrev = (char*)prev;
	cPrev -= sizeof(size_t);
	size_t sizePrev = *((size_t*)cPrev);
	// hmalloc new memory
	void* newMem = xmalloc(bytes);
	// copy over data to new memory
	char* cNewMem = (char*)newMem;
	cNewMem -= sizeof(size_t);
	size_t sizeNewMem = *((size_t*)cNewMem);
	if (sizeNewMem >= sizePrev) {
		memcpy(newMem, prev, sizePrev - sizeof(size_t));
	} else {
		memcpy(newMem, prev, sizePrev - sizeNewMem - sizeof(size_t));
	}
	// hfree old memory
	xfree(prev);
	return newMem;
}
