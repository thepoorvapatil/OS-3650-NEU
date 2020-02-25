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

void
qsort_floats(floats* xs)
{
    qsort(xs->data, xs->size, sizeof(float), compare);
    int compare (const void * aa, const void * bb)
    {
        float ffa = *(const float*) aa;
        float ffb = *(const float*) bb;
        return (ffa > ffb) - (ffa < ffb);
    }
}

floats*
sample(float* data, long size, int P)
{
    // TODO: sample the input data, per the algorithm decription
    floats* ana = make_floats(0);
    floats_push(ana, FLT_MIN);
    float array [3*(P-1)];
    for (int ii = 0; ii < 3*(P-1); ii++){
        array[ii] = data[rand()% size];
    }
    array = qsort(array->data, 3*(P-1), sizeof(float), compare);

    for (int ii = 1; ii < 3*(P-1); ii++){
        floats_push(ana,array[ii]);
    }
    floats_push(ana, FLT_MAX);
    floats_print(ana);
    return make_floats(10);
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

    void* file = malloc(1024); 
    long* aa = mmap(0, sizeof(long), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FILE, fd, 0);
    long size = aa[0];
    float* ss = mmap(0, size*sizeof(float), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    ss = &b[2];

    (void) file; // suppress unused warning.

    // TODO: These should probably be from the input file.
    long count = 100;
    float* data = malloc(1024);

    printf("...", count);
    printf("...", data[0]);

    long sizes_bytes = P * sizeof(long);
    long* sizes = malloc(sizes_bytes); // TODO: This should be shared

    barrier* bb = make_barrier(P);

    sample_sort(ss, size, P, sizes, bb);

    free_barrier(bb);
    munmap(aa, sizeof(long));
    munmap(bb, size*sizeof(float));
    

    return 0;
}