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


float*
readinput(int fd, long count){
    float* data = malloc(4 * count);

    for(int ii = 0; ii < count; ++ii)
	    read(fd, &data[ii], 4);
    
    return data;
}

void
writeoutput(const char* file, long count, float* data){

    int fd = open(file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    check_rv(fd);

    write(fd, &count, 8);
    for(int ii = 0; ii < count; ++ii)
	    write(fd, &data[ii], 4);
    
    close(fd);
}



long 
sumsizes_start(long* sizes, int pp){
    if(pp == 0) 
        return 0;

    long result = 0;
    for (int cc = 0; cc <= pp - 1; cc++) {
	    result += sizes[cc];
    }

    return result;
}



long
sumsizes_end(long* sizes, int pp){
    //Summing sizes to calculate the end index ont he file to copy the sorted data. 
    long result = 0;
    for(int cc = 0; cc <= pp; ++cc)
	    result += sizes[cc];

    return result;
}

//done
int 
compare (const void * a, const void * b)
{
    return ( ((*(const float*)a > *(const float*)b) - (*(const float*)b < *(const float*)b)));
}

//done
void
qsort_floats(floats* xs)
{
    // TODO: call qsort to sort the array
    // see "man 3 qsort" for details
    qsort(xs->data, xs->size, 4, compare);
}

// floats*
// sample(float* data, long size, int P)
// {

//     // TODO: sample the input data, per the algorithm decription
//     // 
//     floats* float_arr = make_floats(3 * (P - 1));

//     //stores samples
//     floats* samples_arr = make_floats(P + 1);

//     //push all float_arr in one floats struct array
//     for(int ii = 0; ii < float_arr->cap; ii++){
//         // samp_arr[ii] = data[rand()%size];
//         floats_push(float_arr, data[rand() % size]);
//     }
//     qsort_floats(float_arr);
    
//     floats_push(samples_arr, 0);
//     int median = 1;
//     for(int ii = 0; ii < P-1; ++ii){
//         floats_push(samples_arr, float_arr->data[median]);
//         median += 3;
//     }

//     free_floats(float_arr);

//     floats_push(samples_arr, FLT_MAX);

//     return samples_arr;
// }




// void*
// sort_worker(void *args) {

//     job* arg = ((job*)args);

//     floats* xs = make_floats(arg->size / arg->P);


//     // TODO: select the floats to be sorted by this worker
//     float start = arg->samps->data[arg->pnum];
//     float end = arg->samps->data[arg->pnum + 1];

//     for(int ii = 0; ii < arg->size; ii++){
//         if (arg->data[ii] >= start && arg->data[ii] < end){
// 	        floats_push(xs, arg->data[ii]);
//         }
//     }   

    
//     printf("%d: start %.04f, count %ld\n", arg->pnum, arg->samps->data[arg->pnum], arg->sizes[arg->pnum]);


//     arg->sizes[arg->pnum] = xs->size;
//     qsort_floats(xs);

//     //Synchronizing
//     barrier_wait(arg->bb);

//     //Copy local array to input.
//     // long Start = sumsizes_start(arg->sizes, arg->pnum);
//     // long End = sumsizes_end(arg->sizes, arg->pnum); 

//     //Each process copies its sorted array to input[start...end].
//     long index = start_sumsizes(arg->sizes, arg->pnum);
//     long kk = 0; 
//     while (index < end_sumsizes(arg->sizes, arg->pnum)){
//         //Copying sorted element to file. 
//         arg->data[index] = xs->data[kk];
//         //Incrementing counters.
//         kk++;			
//         index++;
//     }

//     free_floats(xs);

//     free(arg);

//     return 0;
// }




// void
// run_sort_workers(float* data, long size, int P, floats* samps, long* sizes, barrier* bb)
// {
//     pthread_t threads[P];

//     int rv;

//     // Spawn P processes
// 	for(int pp = 0; pp < P; ++pp){
// 		job* args = malloc(sizeof(job));
// 		args->pnum = pp;
// 		args->data = data;
// 		args->size = size;
// 		args->P = P;
// 		args->samps = samps;
// 		args->sizes = sizes;
// 		args->bb = bb;

// 		rv = pthread_create(&(threads[pp]), 0,  sort_worker, args);
// 		assert (rv == 0);
//     }

// 	for(int ii = 0; ii < P; ++ii){
// 		rv = pthread_join(threads[ii], 0);
// 		assert (rv == 0);
//     }
// }



// void
// sample_sort(float* data, long size, int P, long* sizes, barrier* bb)
// {
//     floats* samps = sample(data, size, P);
//     run_sort_workers(data, size, P, samps, sizes, bb);
//     free_floats(samps);
// }

// int
// main(int argc, char* argv[])
// {
//     alarm(95);

//     if (argc != 4) {
//         printf("Usage:\n");
//         printf("\t%s P data.dat\n", argv[0]);
//         return 1;
//     }
	
//     const int P = atoi(argv[1]); 
//     const char* fname = argv[2];
//     const char* output = argv[3];

//     seed_rng();

//     int rv;
//     struct stat st;
//     rv = stat(fname, &st);
//     check_rv(rv);

//     const int fsize = st.st_size;
//     if (fsize < 8) {
//         printf("File too small.\n");
//         return 1;
//     }
	
//     int fd = open(fname, O_RDWR);
//     check_rv(fd);

//     long count;
//     read(fd, &count, 8);
//     float* data = Read(fd, count);

//     long sizes_bytes = P * sizeof(long);
//     long* sizes = malloc(sizes_bytes);
    
//     barrier* bb = make_barrier(P);
//     sample_sort(data, count, P, sizes, bb);

//     Write(output, count, data);

//     free_barrier(bb);
//     free(data);
//     free(sizes);

//     close(fd);

//     return 0;
// }

// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/mman.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <sys/wait.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <math.h>
// #include <float.h>
// #include <assert.h>
// #include "float_vec.h"
// #include "barrier.h"
// #include "utils.h"

// typedef struct job {
// int pnum;
// float* data;
// long size;
// int P;
// floats* samps;
// long* sizes;
// barrier* bb;
// } job;


// float*
// readinput(int fd, long count){
// float* data = malloc(4 * count);

// for(int ii = 0; ii < count; ++ii){
// 	read(fd, &data[ii], 4);
// }

// return data;

// }



// void
// writeoutput(const char* file, long count, float* data){

// int fd = open(file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
// check_rv(fd);

// write(fd, &count, 8);
// for(int ii = 0; ii < count; ++ii){
// 	write(fd, &data[ii], 4);
// }

// close(fd);
// }



// long 
// sumsizes_start(long* sizes, int pp){
// //Summing sizes to calculate the start index on the file to copy the sorted data. 
// if(pp == 0) return 0;

// long result = 0;
// for (int cc = 0; cc <= pp - 1; ++cc) {
// 	result += sizes[cc];
// }

// return result;
// }



// long
// sumsizes_end(long* sizes, int pp){
// //Summing sizes to calculate the end index ont he file to copy the sorted data. 
// long result = 0;
// for(int cc = 0; cc <= pp; ++cc){
// 	result += sizes[cc];
// }

// return result;
// }

// int
// comparator(const void* p1, const void* p2){

// //Comparator implemented for qsort function. 
// float f1 = *(const float *)p1;
// float f2 = *(const float *)p2;

// if(f1 < f2){
// 	return -1;
// } else if (f1 == f2){
// 	return 0;
// } else {
// 	return 1;
// }
// }


// void
// qsort_floats(floats* xs)
// {
// // Call qsort to sort the array.
// qsort(xs->data, xs->size, 4, comparator);
// }




floats*
sample(float* data, long size, int P)
{

// 1- Randomly select 3*(P-1) items from the array.
floats* sample_items = make_floats(3 * (P - 1));

for(int ii = 0; ii < sample_items->cap; ++ii){
float chosen = data[rand() % size];
floats_push(sample_items, chosen);
}

// 2 - Sort chosen items.
qsort_floats(sample_items);
//floats_print(sample_items);


// 3 - Take the median of each group of 3 in the sorted sample_items producing our array of floats samps. 
floats* samps = make_floats(P + 1);
// 4 - Adding 0 at start.
floats_push(samps, 0);
int median = 1;
for(int ii = 0; ii < P-1; ++ii){
    floats_push(samps, sample_items->data[median]);
    median += 3;
}


free_floats(sample_items);

// 4 - Adding maximun value at the end.
floats_push(samps, FLT_MAX);


return samps;
}




void*
sort_worker(void *args) {

//NOTE: maybe create a wrapper function to make it easier to work with input arguments. 

job* arg = ((job*)args);

floats* xs = make_floats(arg->size / arg->P);


//Selecting fooats to be sorted by this worker. 
float start = arg->samps->data[arg->pnum];
float end = arg->samps->data[arg->pnum + 1];

for(int ii = 0; ii < arg->size; ++ii){
if (arg->data[ii] >= start && arg->data[ii] < end){
	floats_push(xs, arg->data[ii]);
}
}

//Writting the number of items taken to a shared array of sizes at slot p.
arg->sizes[arg->pnum] = xs->size;

printf("%d: start %.04f, count %ld\n", arg->pnum, arg->samps->data[arg->pnum], arg->sizes[arg->pnum]);

//Sort locally.
qsort_floats(xs);

//SYNCRONIZING
barrier_wait(arg->bb);

//Copy local array to input.
long start_idx = sumsizes_start(arg->sizes, arg->pnum);
long end_idx = sumsizes_end(arg->sizes, arg->pnum); 

//Each process copies its sorted array to input[start...end].
long idx = start_idx;
long kk = 0; 
while (idx < end_idx){
    //Copying sorted element to file. 
    arg->data[idx] = xs->data[kk];
    //Incrementing counters.
    kk++;			
    idx++;
}

free_floats(xs);

free(arg);

return 0;
}




void
run_sort_workers(float* data, long size, int P, floats* samps, long* sizes, barrier* bb)
{
pthread_t threads[P];


// Spawn P processes, each running sort_worker
	for(int pp = 0; pp < P; ++pp){
		job* args = malloc(sizeof(job));
		args->pnum = pp;
		args->data = data;
		args->size = size;
		args->P = P;
		args->samps = samps;
		args->sizes = sizes;
		args->bb = bb;

		int rv = pthread_create(&(threads[pp]), 0,  sort_worker, args);
		assert (rv == 0);


}

//free(args);

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
    alarm(95);

    if (argc != 4) {
        printf("Usage:\n");
        printf("\t%s P data.dat\n", argv[0]);
        return 1;
    }
	
    //First argument.
    const int P = atoi(argv[1]); 
    //Second argument. 
    const char* fname = argv[2];
    //Third argument.
    const char* output = argv[3];

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
	
    //Open the input file.
    int fd = open(fname, O_RDWR);
    check_rv(fd);


    //Reading from input file 
    long count;
    read(fd, &count, 8);
    float* data = readinput(fd, count);

    //Sizes for chunk of array in sample sort. 
    long sizes_bytes = P * sizeof(long);
    long* sizes = malloc(sizes_bytes);
    
    barrier* bb = make_barrier(P);

    //Sorting the data
    sample_sort(data, count, P, sizes, bb);

    //Writting in output file.
    writeoutput(output, count, data);

    //Freeing. 
    free_barrier(bb);
    free(data);
    free(sizes);

    //Closing file descriptor.
    close(fd);

    return 0;
}

