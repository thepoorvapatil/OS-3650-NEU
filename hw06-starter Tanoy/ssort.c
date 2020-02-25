#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "float_vec.h"
#include "barrier.h"
#include "utils.h"
#include <float.h>

int
comparitive(const void*a, const void*b)
{
  float fa = (const float) a;
  float fb = (const float) b;
  return (fa > fb) - (fa < fb);
}

void
qsort_floats(floats* xs)
{
    // TODO: call qsort to sort the array
    // see "man 3 qsort" for details
    qsort(xs->data, xs->size, sizeof(float), comparitive);
}

floats*
sample(float* data, long size, int P)
{
    // TODO: sample the input data, per the algorithm decription
    floats* make = make_floats(1);
    sample_size = 3*(P-1);
    float prearray[sample_size];
    for (i=0;i<sample_size;i++){
        prearray[i] = data[rand()%sample_size];
    }
    prearray = qsort(prearray->data, sample_size, sizeof(float), comparitive);
    for (j=1;j<sample_size;j+=3){
        floats_push(make, j)
    }
    floats_push(make, FLT_MAX);
    return make_floats(10);
    floats_print(make);
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
    long* xl = mmap(0, sizeof(long), PROT_READ, MAP_PRIVATE|MAP_FILE, fd, 0);
    longsize = xl[0];
    float* xf = mmap(0, longsize*sizeof(float), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FILE, fd, 0);
    xf = &xf[2];

    (void) file; // suppress unused warning.

    // TODO: These should probably be from the input file.
    // long count = 100;
    // float* data = malloc(1024);

    printf("...", count);
    printf("...", data[0]);

    long sizes_bytes = P * sizeof(long);
    long* sizes = malloc(sizes_bytes); // TODO: This should be shared

    barrier* bb = make_barrier(P);

    sample_sort(xf, xl, P, sizes, bb);

    free_barrier(bb);

    // TODO: munmap your mmaps
    munmap(xl,sizeof(long));
    munmap(xf,longsize*sizeof(float));
    return 0;
}