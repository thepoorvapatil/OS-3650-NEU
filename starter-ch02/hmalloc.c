// #include <stdlib.h>
// #include <sys/mman.h>
// #include <stdio.h>
// #include <assert.h>
// #include "hmalloc.h"

// /*
//   typedef struct hm_stats {
//   long pages_mapped;
//   long pages_unmapped;
//   long chunks_allocated;
//   long chunks_freed;
//   long free_length;
//   } hm_stats;
// */

// typedef struct husky_node {
//   size_t size;
//   struct husky_node* next;
// } husky_node;

// const size_t PAGE_SIZE = 4096;
// static hm_stats stats; // This initializes the stats to 0.
// static husky_node* husky_head;

// husky_node* free_list = NULL;
// pthread_mutex_t free_list_lock;
// int free_list_lock_initialized = 0;

// void*
// hrealloc(void* item, size_t size) {
//   char* result = (char*) hmalloc(size);

//   char* copy = (char*) item;

//   size_t* original_size_ptr = (item - sizeof(size_t));
//   size_t original_size = *original_size_ptr;

//   memcpy(result, copy, original_size);

//   hfree(item);
//   return result;
// }

// //OKAY
// long
// free_list_length() 
// {
//     // TODO: Calculate the length of the free list.
//     long length = 0;
//     husky_node* curr = husky_head;

//     while (curr != 0) {
//       length += 1;
//       curr = curr->next;
//     }

//     return length;
//     // return 0;
// }

// //OKAY
// hm_stats*
// hgetstats()
// {
//     stats.free_length = free_list_length();
//     return &stats;
// }

// //OKAY
// void
// hprintstats()
// {
//     stats.free_length = free_list_length();
//     fprintf(stderr, "\n== husky malloc stats ==\n");
//     fprintf(stderr, "Mapped:   %ld\n", stats.pages_mapped);
//     fprintf(stderr, "Unmapped: %ld\n", stats.pages_unmapped);
//     fprintf(stderr, "Allocs:   %ld\n", stats.chunks_allocated);
//     fprintf(stderr, "Frees:    %ld\n", stats.chunks_freed);
//     fprintf(stderr, "Freelen:  %ld\n", stats.free_length);
// }

// //OKAY
// static
// size_t
// div_up(size_t xx, size_t yy)
// {
//     // This is useful to calculate # of pages
//     // for large allocations.
//     size_t zz = xx / yy;

//     if (zz * yy == xx) {
//         return zz;
//     }
//     else {
//         return zz + 1;
//     }
// }

// void
// insert_list(husky_node* node) {

//     // if empty
//     if (husky_head == 0) {
//         husky_head = node;
//         return;
//     }

//     husky_node* curr = husky_head;
//     husky_node* prev = 0;
//     while (curr != 0) {
//         // current node greater than address,
//         if ((void*) curr > (void*) node) {

//             size_t prev_size = 0;
//             if (prev != 0) {
//                 prev_size = prev->size;
//             }

//             if (((void*) prev + prev_size == (void*) node) && ((void*) node + node->size == (void*) curr)) {
//                 prev->size = prev->size + node->size + curr->size;
//                 prev->next = curr->next;
//             }

//             // node is adjacent to end of prev
//             else if ((void*) prev + prev_size == (void*) node) {
//                 prev->size = prev->size + node->size;
//             }

//             //node is adjacent to beginning of curr
//             else if ((void*) node + node->size == (void*) curr) {
//                 node->size = node->size + curr->size;

//                 if (prev != 0) {
//                 prev->next = node; 
//                 }
//                 node->next = curr->next; 
//             }

//             else {
            
//                 if (prev != 0) {
//                 prev->next = node;
//                 }
//                 node->next = curr;
//             }

//             // insert at beginning of the free list
//             if (prev == 0) {
//                 husky_head = node;
//             }

//             break;

//         }
//         prev = curr; 
//         curr = curr->next;
//     }
// }

// void*
// hmalloc(size_t size)
// {
//     stats.chunks_allocated += 1;
//     size += sizeof(size_t);

//     // TODO: Actually allocate memory with mmap and a free list.

//     if (size < PAGE_SIZE) {
//         husky_node* new_block = 0; 
//         husky_node* curr = husky_head;
//         husky_node* prev = 0;
//         while (curr != 0) {
//             if (curr->size >= size) {
//             new_block = curr;

//             if (prev != 0) {
//                 prev->next = curr->next;
//             } else {
                
//                 husky_head = curr->next;
//             }

//             break;
//             }
//             prev = curr;
//             curr = curr->next;
//         }

//         if (new_block == 0) {
//             new_block = mmap(0, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
//             assert(new_block != MAP_FAILED);
//             stats.pages_mapped += 1;
//             new_block->size = PAGE_SIZE;
//         }

//         if ((new_block->size > size) && (new_block->size - size >= sizeof(husky_node))) {
            
//             void* address = (void*) new_block + size;

//             // create new node from address
//             husky_node* leftover = (husky_node*) address;
//             leftover->size = new_block->size - size;

//             insert_list(leftover);

//             new_block->size = size;
//         }

//         return (void*) new_block + sizeof(size_t);
//     }

//     //greater than/equal to page size
//     else {
//         int num_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

//         size_t size = num_pages * PAGE_SIZE;
//         husky_node* block = mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
//         assert(block != MAP_FAILED);
//         stats.pages_mapped += num_pages;

//         block->size = size; 
//         return (void*) block + sizeof(size_t);
//     }
// }

// //original
// void
// hfree(void* item)
// {
//     stats.chunks_freed += 1;

//     // TODO: Actually free the item.

//     //beginning
//     husky_node* block = (husky_node*) (item - sizeof(size_t));

//     if (block->size < PAGE_SIZE) {
//         //back on the free list
//         insert_list(block);
//     }
//     else {
//         int num_pages = (block->size + PAGE_SIZE - 1) / PAGE_SIZE;
//         int rv = munmap((void*) block, block->size);
//         assert(rv != -1);
//         stats.pages_unmapped += num_pages;
//     }
// }


#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stddef.h>

#include "hmalloc.h"

/*
  typedef struct hm_stats {
  long pages_mapped;
  long pages_unmapped;
  long chunks_allocated;
  long chunks_freed;
  long free_length;
  } hm_stats;
*/

typedef struct husky_node {
	size_t size;
	struct husky_node* next;
} husky_node;

const size_t PAGE_SIZE = 4096;
static hm_stats stats; // This initializes the stats to 0.
husky_node* free_list = NULL;
pthread_mutex_t free_list_lock;
int free_list_lock_initialized = 0;

long
free_list_length()
{
    	husky_node* nxt = free_list;
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
removeMemFromFreeList(size_t size)
{
	if (free_list == NULL) {
		return 0;
	}
	husky_node* prev = NULL;
	husky_node* head = free_list;
	void* cell = NULL;
	while(head != NULL) {
		if (head->size >= size) {
			if (prev == NULL) {
				free_list = head->next;
			} else if (head->next == NULL) {
				prev->next = NULL;
			} else {
				prev->next = head->next;
			}
			head->next = NULL;
			cell = head;
			return cell;
		} else {
			prev = head;
			head = head->next;
		}
	}
	return cell;
}

void
sort_free_list()
{
	husky_node* x, *y, *e;

        x = free_list;
        free_list = NULL;

        while(x != NULL) {
                e = x;
                x = x->next;
                if (free_list != NULL) {
                        if (e > free_list) {
                                y = free_list;
                                while ((y->next != NULL) && (e > y->next)) {
                                        y = y->next;
                                }
                                e->next = y->next;
                                y->next = e;
                        } else {
                                e->next = free_list;
                                free_list = e;
                        }
                } else {
                        e->next = NULL;
                        free_list = e;
                }
        }
}

void
coalesce_free_list()
{
	husky_node* head = free_list;
	size_t size = head->size;
	husky_node* next = head->next;
	// long length = free_list_length();
	while(next != NULL) {
		// size_t nextSize = next->size;
		char* cHead = (char*)head;
		char* cNext = (char*)next;
		// int differenceInBytes = (cNext - cHead);
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
hmalloc(size_t size)
{
	if (!free_list_lock_initialized) {
		pthread_mutex_init(&free_list_lock, 0);
		free_list_lock_initialized = 1;
	}
    	stats.chunks_allocated += 1;
    	size += sizeof(size_t);
	if (size < PAGE_SIZE) {
		pthread_mutex_lock(&free_list_lock);
		// Try to remove a *size* amount of memory from the free_list
		void* cell = NULL;
		int memLeftover;
		if (free_list != NULL) {
			cell = removeMemFromFreeList(size);
		}
		if (cell != NULL) { // Enough memory on free_list
			size_t* cellSize = cell;
			memLeftover = *cellSize - size;
		} else { // Not enough memory on free_list
			// Allocate a page of memory
			cell = mmap(0, PAGE_SIZE, PROT_READ | PROT_WRITE,
					 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
			stats.pages_mapped += 1;
			memLeftover = PAGE_SIZE - size;
		}
		// Add any extra memory back onto the free_list
		if (memLeftover >= sizeof(husky_node)) {
			char* cCell = cell;
			cCell += size;
			husky_node* fCell = (husky_node*)cCell;
			fCell->size = memLeftover;
			fCell->next = NULL;
			if (free_list != NULL) {
				husky_node* head = free_list;
				while(head->next != NULL) {
					head = head->next;
				}
				head->next = fCell;
			} else {
				free_list = fCell;
			}
		}
		pthread_mutex_unlock(&free_list_lock);
		// Store the new size at the beginning of the memory
		size_t* sizePtr = cell;
		*sizePtr = size;
		sizePtr++;
		void* start = sizePtr;
		return start;
	} else {
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
hfree(void* item)
{
    	stats.chunks_freed += 1;
	void* start = item - sizeof(size_t);
    	size_t* size = start;
	husky_node* freedCell = (husky_node*)start;
	if (*size < PAGE_SIZE) {
		pthread_mutex_lock(&free_list_lock);
		husky_node* head = free_list;
		husky_node* next = head->next;
		while(next != NULL) {
			head = next;
			next = next->next;
		}
		head->next = freedCell;
		head->next->size = *size;
		head->next->next = NULL;

                sort_free_list();
                coalesce_free_list();
		pthread_mutex_unlock(&free_list_lock);
	} else {
		int numOfPages = div_up(*size, PAGE_SIZE);
		stats.pages_unmapped += numOfPages;
		munmap(item, *size);
	}
}

void* 
hrealloc(void* prev, size_t bytes)
{
	if (prev == NULL) {
		return hmalloc(bytes);
	}
	void* start = prev - sizeof(size_t);
	size_t* size = start;
	if (*size > bytes) {
		int memLeftover = bytes - *size;
		if (memLeftover >= sizeof(husky_node)) {
			pthread_mutex_lock(&free_list_lock);
			start += *size;
			husky_node* fCell = (husky_node*)start;
			fCell->size = memLeftover;
			fCell->next = NULL;
			if (free_list != NULL) {
				husky_node* head = free_list;
				while(head->next != NULL) {
					head = head->next;
				}
				head->next = fCell;
			} else {
				free_list = fCell;
			}
			pthread_mutex_unlock(&free_list_lock);
			*size = bytes;
			return prev;
		}
	} else if (*size == bytes) {
		return prev;
	} else {
		void* start = prev - sizeof(size_t);
		size_t* size = start;
		// hmalloc new memory
		void* newMem = hmalloc(bytes);
		// copy over data to new memory
		memcpy(newMem, prev, *size);
		// hfree old memory
		hfree(prev);
		return newMem;
	}
	return 0;
}

