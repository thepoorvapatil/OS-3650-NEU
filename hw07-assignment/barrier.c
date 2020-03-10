// Author: Nat Tuck
// CS3650 starter code

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>

#include "barrier.h"

barrier*
make_barrier(int nn)
{
    barrier* bb = malloc(sizeof(barrier));
    assert(bb != 0);

    int rv = pthread_cond_init(&(bb->cv), 0);
    assert(rv == 0);

    rv = pthread_mutex_init(&(bb->mutex), 0);
    assert(rv == 0);

    bb->count = nn;
    bb->seen  = 0;
    return bb;
}

void
barrier_wait(barrier* bb)
{
    // while (1) {
    //     sleep(1);
    //     // TODO: Stop waiting.
    //     // TODO: Don't sleep here.
    // }
    
    pthread_mutex_lock(&(bb->mutex));

    bb->seen += 1;
    int seenbb = bb->seen;

    if(seenbb < bb->count)
        pthread_cond_wait(&(bb->cv), &(bb->mutex));    
    
    pthread_cond_broadcast(&(bb->cv));
    pthread_mutex_unlock(&(bb->mutex));
}

void
free_barrier(barrier* bb)
{
    free(bb);
}
