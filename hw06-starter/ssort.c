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

#include "float_vec.h"
#include "barrier.h"
#include "utils.h"

int compare (const void * a, const void * b)
{
    return ( (int) ((*(const float*)a > *(const float*)b) - (*(const float*)b < *(const float*)b)));
}

void
qsort_floats(floats* xs)
{
    // TODO: call qsort to sort the array
    // see "man 3 qsort" for details
    qsort(xs->data, xs->size, sizeof(float), compare);

}

floats*
sample(float* data, long size, int P)
{
    // TODO: sample the input data, per the algorithm decription
    // 
    floats* float_arr = make_floats(1);

    //stores samples
    floats* samples_arr = make_floats(0);

    //push all samples in one floats struct array
    for (int ii=0; ii<3*(P-1); ii++){
        // samp_arr[ii] = data[rand()%size];
        floats_push(samples_arr, data[rand()%size]);
    }    
    qsort_floats(samples_arr);

    //push all medians of each sample.
    for( int ii=1; ii< 3*(P-1); ii+=3){
        floats_push(float_arr, samples_arr->data[ii]);
    }
    //push infinity end val
    floats_push(float_arr, FLT_MAX);

    floats_print(float_arr);
    return float_arr;

}

void
sort_worker(int pnum, float* data, long size, int P, floats* samps, long* sizes, barrier* bb)
{
    floats* xs = make_floats(10);
    // TODO: select the floats to be sorted by this worker

    printf("%d: start %.04f, count %ld\n", pnum, samps->data[pnum], xs->size);

    // TODO: some other stuff

    qsort_floats(xs);

    // TODO: probably more stuff

    free_floats(xs);
}

void
run_sort_workers(float* data, long size, int P, floats* samps, long* sizes, barrier* bb)
{
    pid_t kids[P];
    (void) kids; // suppress unused warning

    // TODO: spawn P processes, each running sort_worker

    for (int ii = 0; ii < P; ++ii) {
        //int rv = waitpid(kids[ii], 0, 0);
        //check_rv(rv);
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

    if (argc != 3) {
        printf("Usage:\n");
        printf("\t%s P data.dat\n", argv[0]);
        return 1;
    }

    const int P = atoi(argv[1]);
    const char* fname = argv[2];

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

    void* file = malloc(1024); // TODO: load the file with mmap.
    (void) file; // suppress unused warning.

    long* sizePointer = mmap(0, sizeof(long), PROT_READ, MAP_PRIVATE | MAP_FILE , fd, 0);
    long size = sizePointer[0];
    float* arr = mmap(0, size*sizeof(float), PROT_READ | PROT_WRITE , MAP_SHARED , fd, 0);
    arr = &arr[2]; // offset for array on mmap

    // TODO: These should probably be from the input file.
    // long count = 100;
    // float* data = malloc(1024);

    printf("...", size);
    printf("...", arr[0]);

    long sizes_bytes = P * sizeof(long);
    long* sizes = malloc(sizes_bytes); // TODO: This should be shared

    barrier* bb = make_barrier(P);

    sample_sort(arr, size, P, sizes, bb);

    free_barrier(bb);

    // TODO: munmap your mmaps
    munmap(arr, size*sizeof(float));
    munmap(sizePointer, sizeof(long));

    return 0;
}

