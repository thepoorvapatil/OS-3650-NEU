#include <stdint.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include "opt_malloc.h"

typedef struct nu_free_cell {
    int64_t              size;
    struct nu_free_cell* next;
} nu_free_cell;

static const int64_t CHUNK_SIZE = 65536;
static const int64_t CELL_SIZE  = (int64_t)sizeof(nu_free_cell);

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static nu_free_cell* nu_bins[8];  // array of bins that will hold several free lists

void*
opt_realloc(void* item, int64_t size) {
  void* result = (void*) opt_malloc(size + sizeof(int64_t));
  pthread_mutex_lock(&mutex);

  int64_t* original_size_ptr = (item - sizeof(int64_t));
  int64_t original_size = *original_size_ptr;

  memcpy(result, item, original_size - sizeof(int64_t));

  pthread_mutex_unlock(&mutex);
  opt_free(item);

  return result;
}

int64_t
nu_free_list_length() {
  int len = 0;

  for (int ii = 0; ii < 8; ++ii) {
    for (nu_free_cell* pp = nu_bins[ii]; pp != 0; pp = pp->next) {
      len++;
    }
  }

  return len;
}

void
nu_print_free_list() {
  for (int ii = 0; ii < 8; ++ii) {
    nu_free_cell* pp = nu_bins[ii];
    printf("= Free list #%d: =\n", ii);

    for (; pp != 0; pp = pp->next) {
      printf("%lx: (cell %ld %lx)\n", (int64_t) pp, pp->size, (int64_t) pp->next);
    }
  }
}

// will insert the given cell into the given bin
static
void
nu_free_list_insert(nu_free_cell* cell, int64_t bin) {
  cell->next = nu_bins[bin];
  nu_bins[bin] = cell;
  return;
}

// finds the nearest power of two that is greater than the given size
// will use that to choose which bin the given size belongs in
int64_t
choose_bin(int64_t size) {
  int64_t n = 32;
  int64_t bin = 0;
  while(n != size) {
    n *= 2;
    ++bin;
  }
  return bin;
}

// gets the first cell in the free list with a size >= the given size
// will now take a bin that is the free list it will take from
static
nu_free_cell*
free_list_get_cell(int64_t bin) {
  if (!nu_bins[bin]) {  // if the given bin is empty, return 0
    return 0;
  }
  nu_free_cell* result = nu_bins[bin];
  nu_bins[bin] = result->next;

  return result;
}

// finds the next larger bin with a free cell in it, then puts it in the given bin
// will assume that the given bin is empty
// if none have any, will allocate space in the largest, and break it down
void
split_cells(int64_t bin) {
  if (bin == 7) { // if the final bin is empty,
    allocate_more_bins(); // allocate more space in the final bin
    return;
  }

  if (nu_bins[bin+1] == 0) { // if bin[bin+1] is empty, split into it first
    split_cells(bin+1);  // will recursively call split_cells(bin+1) until reaching one that has something in it
  }

  nu_free_cell* cell = free_list_get_cell(bin+1);  // gets the first cell in nu_bins[bin+1]
  cell->size = (cell->size / 2);

  nu_free_cell* cell2 = (nu_free_cell*) ((size_t) cell + cell->size);
  cell2->size = cell->size;

  nu_free_list_insert(cell, bin);
  nu_free_list_insert(cell2, bin);
  return;
}

void
allocate_more_bins() {
  nu_free_cell* new_alloc = mmap(0, CHUNK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  int64_t limit = CHUNK_SIZE / 4096;

  // inserts however many cells can fit into one chunk allocation
  for (int64_t ii = 0; ii < limit; ++ii) {
    nu_free_cell* cell = (nu_free_cell*) ((size_t) new_alloc + (ii * 4096));
    cell->size = 4096;
    nu_free_list_insert(cell, 7);
  }
  return;
}

// rounds the given number to the nearest power of 2,
// that is also a size of a bin
int64_t
round_to_nearest_power_of_two(int64_t num) {
  int64_t ii = 32;

  while (ii < num) {
    ii = ii * 2;
  }
  return ii;
}

void*
opt_malloc(size_t usize) {
  int64_t size = (int64_t) usize;

  // space for size
  int64_t alloc_size = size + sizeof(int64_t);

  // space for free cell when returned to list
  if (alloc_size < CELL_SIZE) {
      alloc_size = CELL_SIZE;
  }

  if (alloc_size > 4096) {
      void* addr = mmap(0, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      *((int64_t*)addr) = alloc_size;
      pthread_mutex_unlock(&mutex);
      return addr + sizeof(int64_t);
  }

  alloc_size = round_to_nearest_power_of_two(alloc_size);

  int64_t bin = choose_bin(alloc_size); // the bin that the given size will take from

  pthread_mutex_lock(&mutex);
  nu_free_cell* cell = free_list_get_cell(bin);

  while (!cell) {
    split_cells(bin);
    cell = free_list_get_cell(bin);
  }

  pthread_mutex_unlock(&mutex);

  *((int64_t*)cell) = alloc_size;
  return ((void*)cell) + sizeof(int64_t);
}

void
opt_free(void* addr) {
  nu_free_cell* cell = (nu_free_cell*)(addr - sizeof(int64_t));
  int64_t size = cell->size;// *((int64_t*) cell);
  size = round_to_nearest_power_of_two(size);

  if (size > 4096) {
    munmap((void*) cell, size);
  }
  else {
    cell->size = size;
    int64_t bin = choose_bin(size);

    pthread_mutex_lock(&mutex);
    nu_free_list_insert(cell, bin);
    pthread_mutex_unlock(&mutex);
  }
}
