#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <assert.h>
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
static husky_node* husky_head;

void*
hrealloc(void* item, size_t size) {
  char* result = (char*) hmalloc(size);

  char* copy = (char*) item;

  size_t* original_size_ptr = (item - sizeof(size_t));
  size_t original_size = *original_size_ptr;

  memcpy(result, copy, original_size);

  hfree(item);
  return result;
}

long
free_list_length()
{
    // TODO: Calculate the length of the free list.
    long length = 0;
    husky_node* curr = husky_head;

    while (curr != 0) {
      length += 1;
      curr = curr->next;
    }

    return length;
    return 0;
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

void
insert_list(husky_node* node) {

    // if empty
    if (husky_head == 0) {
        husky_head = node;
        return;
    }

    husky_node* curr = husky_head;
    husky_node* prev = 0;
    while (curr != 0) {
        // current node greater than address,
        if ((void*) curr > (void*) node) {

            size_t prev_size = 0;
            if (prev != 0) {
                prev_size = prev->size;
            }

            if (((void*) prev + prev_size == (void*) node) && ((void*) node + node->size == (void*) curr)) {
                prev->size = prev->size + node->size + curr->size;
                prev->next = curr->next;
            }

            // node is adjacent to end of prev
            else if ((void*) prev + prev_size == (void*) node) {
                prev->size = prev->size + node->size;
            }

            //node is adjacent to beginning of curr
            else if ((void*) node + node->size == (void*) curr) {
                node->size = node->size + curr->size;

                if (prev != 0) {
                prev->next = node; 
                }
                node->next = curr->next; 
            }

            else {
            
                if (prev != 0) {
                prev->next = node;
                }
                node->next = curr;
            }

            // insert at beginning of the free list
            if (prev == 0) {
                husky_head = node;
            }

            break;

        }
        prev = curr; 
        curr = curr->next;
    }
}

void*
hmalloc(size_t size)
{
    stats.chunks_allocated += 1;
    size += sizeof(size_t);

    // TODO: Actually allocate memory with mmap and a free list.

    if (size < PAGE_SIZE) {
        husky_node* new_block = 0; 
        husky_node* curr = husky_head;
        husky_node* prev = 0;
        while (curr != 0) {
            if (curr->size >= size) {
            new_block = curr;

            if (prev != 0) {
                prev->next = curr->next;
            } else {
                
                husky_head = curr->next;
            }

            break;
            }
            prev = curr;
            curr = curr->next;
        }

        if (new_block == 0) {
            new_block = mmap(0, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
            assert(new_block != MAP_FAILED);
            stats.pages_mapped += 1;
            new_block->size = PAGE_SIZE;
        }

        if ((new_block->size > size) && (new_block->size - size >= sizeof(husky_node))) {
            
            void* address = (void*) new_block + size;

            // create new node from address
            husky_node* leftover = (husky_node*) address;
            leftover->size = new_block->size - size;

            insert_list(leftover);

            new_block->size = size;
        }

        return (void*) new_block + sizeof(size_t);
    }

    //greater than/equal to page size
    else {
        int num_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

        size_t size = num_pages * PAGE_SIZE;
        husky_node* block = mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        assert(block != MAP_FAILED);
        stats.pages_mapped += num_pages;

        block->size = size; 
        return (void*) block + sizeof(size_t);
    }
}

void
hfree(void* item)
{
    stats.chunks_freed += 1;

    // TODO: Actually free the item.

    //beginning
    husky_node* block = (husky_node*) (item - sizeof(size_t));

    if (block->size < PAGE_SIZE) {
        //back on the free list
        insert_list(block);
    }
    else {
        int num_pages = (block->size + PAGE_SIZE - 1) / PAGE_SIZE;
        int rv = munmap((void*) block, block->size);
        assert(rv != -1);
        stats.pages_unmapped += num_pages;
    }
}
