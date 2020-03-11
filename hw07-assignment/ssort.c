#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <string.h>
#include "float_vec.h"
#include "barrier.h"
#include "utils.h"

typedef struct job {
    int pnum;
    float* data;
    long size;
    int P;
    floats* samps;
    long* sizes;
    barrier* bb;
} job;

void
writeoutput(const char* file, long size, float* arr){

    int fd = open(file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    check_rv(fd);

    write(fd, &size, 8);
    for(int ii = 0; ii < size; ++ii){
        write(fd, &arr[ii], 4);
    }
    close(fd);
}

long 
start_sum(long* sizes, int pp){
    if(pp == 0) 
        return 0;

    long result = 0;
    for (int cc = 0; cc <= pp - 1; ++cc) {
        result += sizes[cc];
    }
    return result;
}


long
end_sum(long* sizes, int pp){
    long result = 0;
    for(int cc = 0; cc <= pp; ++cc){
        result += sizes[cc];
    }

    return result;
}

int compare (const void * a, const void * b)
{
    return ( ((*(const float*)a > *(const float*)b) - (*(const float*)b < *(const float*)b)));
}

void
qsort_floats(floats* xs)
{
    // TODO: call qsort to sort the array
    // see "man 3 qsort" for details
    qsort(xs->data, xs->size, 4, compare);
}



floats*
sample(float* data, long size, int P)
{
    floats* float_arr = make_floats(3 * (P - 1));

    for(int ii = 0; ii < float_arr->cap; ++ii){
        floats_push(float_arr,  data[rand() % size]);
    }

    qsort_floats(float_arr);

    floats* sample_arr = make_floats(P + 1);
    floats_push(sample_arr, 0);
    int median = 1;
    for(int ii = 0; ii < P-1; ++ii, median+=3){
        floats_push(sample_arr, float_arr->data[median]);
    }

    free_floats(float_arr);

    floats_push(sample_arr, FLT_MAX);

    return sample_arr;
}




void*
sort_worker(void *vals) {

job* val = ((job*)vals);

floats* xs = make_floats(val->size / val->P);

for(int ii = 0; ii < val->size; ++ii){
    if (val->data[ii] >= val->samps->data[val->pnum] && val->data[ii] < val->samps->data[val->pnum + 1]){
        floats_push(xs, val->data[ii]);
    }
}

val->sizes[val->pnum] = xs->size;

printf("%d: start %.04f, count %ld\n", val->pnum, val->samps->data[val->pnum], val->sizes[val->pnum]);

qsort_floats(xs);

barrier_wait(val->bb);

long index = start_sum(val->sizes, val->pnum);
long counter = 0; 

while (index < end_sum(val->sizes, val->pnum)){
    val->data[index] = xs->data[counter];
    counter++;			
    index++;
}

free_floats(xs);

free(val);

return 0;
}

void
run_sort_workers(float* data, long size, int P, floats* samps, long* sizes, barrier* bb)
{
    pthread_t threads[P];

    // TODO: spawn P processes, each running sort_worker
	for(int pp = 0; pp < P; pp++){
		job* vals = malloc(sizeof(job));
		vals->pnum = pp;
		vals->data = data;
		vals->size = size;
		vals->P = P;
		vals->samps = samps;
		vals->sizes = sizes;
		vals->bb = bb;

		int rv = pthread_create(&(threads[pp]), 0,  sort_worker, vals);
		assert (rv == 0);
    }
	for(int ii = 0; ii < P; ++ii){
		int rv = pthread_join(threads[ii], 0);
		assert (rv == 0);
    }
}

void
sample_sort(float* data, long size, int P, long* sizes, barrier* bb)
{
    floats* samps = sample(data, size, P);
    run_sort_workers(data, size, P, samps, sizes, bb);
    free_floats(samps);
}

int
main(int argc, char* argv[])
{
    alarm(120);

    if (argc != 4) {
        printf("Usage:\n");
        printf("\t%s P data.dat\n", argv[0]);
        return 1;
    }
	
    const int P = atoi(argv[1]); 
    const char* fname = argv[2];
    const char* opfile = argv[3];

    seed_rng();

    int rv;
    struct stat st;
    rv = stat(fname, &st);
    check_rv(rv);

    const int fsize = st.st_size;
    if (fsize < 8) {
        printf("File too small.\n");
        return 1;
    }
	
    int fd = open(fname, O_RDWR);
    check_rv(fd);

    long* sizePointer = mmap(0, sizeof(long), PROT_READ, MAP_PRIVATE | MAP_FILE , fd, 0);
    long size = sizePointer[0];
    float* arr = mmap(0, size*sizeof(float), PROT_READ | PROT_WRITE , MAP_SHARED , fd, 0);
    arr = &arr[2]; // offset for array on mmap

    long sizes_bytes = P * sizeof(long);
    long* sizes = mmap(0, sizes_bytes, PROT_READ | PROT_WRITE, MAP_SHARED| MAP_ANONYMOUS, -1, 0); // TODO: This should be shared

    
    barrier* bb = make_barrier(P);

    sample_sort(arr, size, P, sizes, bb);

    writeoutput(opfile, size, arr);
    // int fd = open(file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    // check_rv(fd);

    // write(fd, &size, 8);
    // for(int ii = 0; ii < size; ++ii){
    //     write(fd, &arr[ii], 4);
    // }
    // close(fd);

    //open file for output
    // int fdout = open(opfile, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    // float* final = mmap (0, size*sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, fdout, 0);

    // memcpy(final, arr, size*sizeof(float));
    free_barrier(bb);
    
    // TODO: munmap your mmaps
    munmap(sizes,sizes_bytes);
    munmap(arr, size*sizeof(float));
    munmap(sizePointer, sizeof(long));

    return 0;
}

