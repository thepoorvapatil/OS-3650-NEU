#include <stdint.h>
#include <sys/mman.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include "opt_malloc.h"
#include "xmalloc.h"

typedef struct husky_node {
	size_t size;
	struct husky_node* next;
} husky_node;

const size_t PAGE_SIZE = 4096;
__thread  hm_stats stats; // This initializes the stats to 0.
__thread husky_node* bckts[7] = {0, 0, 0, 0, 0, 0, 0};

void coalesce_husky_list(int bucket);
void add_to_bckts(husky_node* bckt);

int
LOG(long x)
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
powerof2 (long x)
{
    long num = 1;
    while (x > 0) {
    num <<= 1;
    x--;
    }
    return num;
}

void
add_to_bckts(husky_node* bckt)
{	
	size_t size = bckt->size;
	bckt->next = NULL;
	int s = size;
	char* cbckt = (char*)bckt;
	while (s > 0) {

        //calculations
        int X;
        if (s == 0) 
            X = 0;
        else {
            int counter = -1;
            long num = 1;
            while (num <= s)
            {
                num <<= 1;
                counter++;
            }
            X = counter;
        }
        // end of calcs

		int bucket = X - 5;
		if (bucket > 6) {
			bucket = 6;
		}
		size_t nextSize = powerof2(bucket + 5);
		husky_node* newbckt = (husky_node*)cbckt;
		cbckt += nextSize;
		s -= nextSize;
		newbckt->size = nextSize;
		newbckt->next = NULL;

		husky_node* AddToBucket = bckts[bucket];


		if (AddToBucket == NULL || (AddToBucket > newbckt)) {
			newbckt->next = bckts[bucket];
			bckts[bucket] = newbckt;
		} else {
			while(AddToBucket->next != NULL && (AddToBucket->next < newbckt)) {
				AddToBucket = AddToBucket->next;
			}
			AddToBucket->next = newbckt;
		}

		if (bucket != 6) {
			coalesce_husky_list(bucket);
		}
	}
}


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
coalesce_husky_list(int bucket)
{
	husky_node* head = bckts[bucket];
	while(head != NULL && head->next != NULL) {
		if (((size_t)head + head->size == ((size_t)head->next))
				&& (head->size + head->next->size != 96)) {
			head->size += head->next->size;
			head->next = head->next->next;
		}
		head = head->next;
	}

	// Add coalesced buckets
	husky_node* prev = NULL;
	head = bckts[bucket];
	husky_node* retbckt = NULL;
	int size = powerof2(bucket + 5);
	while(head != NULL) {
		if (head->size > size) {
			if (prev == NULL) {
				bckts[bucket] = head->next;
			} else if (head->next == NULL) {
				prev->next = NULL;
			} else {
				prev->next = head->next;
			}
			retbckt = head;
			head = head->next;
			retbckt->next = NULL;
			int newbucket = LOG(retbckt->size) - 5;
			husky_node* AddToBucket = bckts[newbucket];

            // add_to_bckts_helper();
			if (AddToBucket == NULL || (AddToBucket > retbckt)) {
				retbckt->next = bckts[newbucket];
				bckts[newbucket] = retbckt;
			} else {
				while(AddToBucket->next != NULL && (AddToBucket->next < retbckt)) {
					AddToBucket = AddToBucket->next;
				}
				AddToBucket->next = retbckt;
			}

		} else {
			prev = head;
			head = head->next;
		}
	}
}


void*
remove_from_bckts(size_t size)
{
	int bucket = LOG(size) - 5;
	husky_node* RemoveFromBucket = bckts[bucket];
	if (RemoveFromBucket != NULL) {
		// remove first entry
		bckts[bucket] = RemoveFromBucket->next;
		RemoveFromBucket->next = NULL;
		return (void*)RemoveFromBucket;
	} else {
		void* tmp = NULL;
		for (int ii = bucket + 1; ii < 7; ++ii) {
			if (bckts[ii] != NULL) {
				husky_node* bigger = bckts[ii];
				bckts[ii] = bigger->next;
				size_t prevSize = bigger->size;
				// split
				husky_node* new = bigger;
				new->size = size;
				new->next = NULL;
				size_t newSize = prevSize - size;
				char* ctmp = (char*)new;
				ctmp += size;
				husky_node* old = (husky_node*)ctmp;
				old->size = newSize;
				old->next = NULL;
				// add leftover into the buckets
				add_to_bckts(old);
				tmp = (void*)new;
				break;
			}
		}
		return tmp;
	}
}

void*
xmalloc(size_t size)
{
	stats.chunks_allocated += 1;
	size += sizeof(size_t);
	if (size < PAGE_SIZE && size <= 2048) {
		// memeory of size power of 2
		// size = size <= 32 ? 32 : powerof2(LOG(size);
        if (size <= 32)
		    size=32;
        else
		    size = powerof2(LOG(size));
        
		void* tmp = remove_from_bckts(size);
		if (tmp != NULL) {
			char* ctmp = (char*)tmp;
			ctmp += sizeof(size_t);
			return (void*)ctmp;
		} 
        //not enough mem
        else { 
			tmp = mmap(0, PAGE_SIZE, PROT_READ | PROT_WRITE,
					MAP_SHARED | MAP_ANONYMOUS, -1, 0);
			stats.pages_mapped += 1;
			char* ctmp = tmp;
			ctmp += size;
			void* remaining = (void*)ctmp;
			husky_node* ftmp = (husky_node*)tmp;
			ftmp->size = size;
			ftmp->next = NULL;
			// add leftover mem to buckets
			husky_node* newNode = (husky_node*)remaining;
			newNode->size = PAGE_SIZE - size;
			newNode->next = NULL;
			add_to_bckts(newNode);
			char* retMem = (char*)ftmp;
			retMem += sizeof(size_t);
			return (void*)retMem;
		}
	} 
    else {
		if (size > 2048 && size < PAGE_SIZE) {
			size = PAGE_SIZE;
		}
		int numOfPages = div_up(size, PAGE_SIZE);
		stats.pages_mapped += numOfPages;
		int numOfMem = numOfPages * PAGE_SIZE;
		void* tmp = mmap(0, numOfMem, PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_ANONYMOUS, -1, 0);
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

	char* start = item;
	start -= sizeof(size_t);
	husky_node* newList = (husky_node*)start;
	if (newList->size < PAGE_SIZE) {
		newList->next = NULL;
		add_to_bckts(newList);
	} else {
		int numOfPages = div_up(newList->size, PAGE_SIZE);
		stats.pages_unmapped += numOfPages;
		munmap(item, newList->size);
	}
}

void* 
xrealloc(void* prev, size_t bytes)
{
	char* cPrev = (char*)prev;
	cPrev -= sizeof(size_t);
	size_t sizePrev = *((size_t*)cPrev);
	//new memory
	void* newMemory = xmalloc(bytes);
	char* cNewMemory = (char*)newMemory;
	cNewMemory -= sizeof(size_t);
	size_t sizeNewMem = *((size_t*)cNewMemory);
	if (sizeNewMem >= sizePrev)
		memcpy(newMemory, prev, sizePrev - sizeof(size_t));
	else 
		memcpy(newMemory, prev, sizePrev - sizeNewMem - sizeof(size_t));
	
	xfree(prev);
	return newMemory;
}
