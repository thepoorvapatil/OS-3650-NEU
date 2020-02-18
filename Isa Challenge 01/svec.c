/* This file is lecture notes from CS 3650, Fall 2018 */
/* Author: Nat Tuck */

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "svec.h"

svec*
make_svec()
{
    svec* sv = malloc(sizeof(svec));
    sv->data = malloc(2 * sizeof(char*));
    sv->size = 0;
    sv->capacity = 2;
    return sv;
}

void
free_svec(svec* sv)
{
    free(sv->data);
    free(sv);
}

char*
svec_get(svec* sv, int ii)
{
    assert(ii >= 0 && ii < sv->size);
    return sv->data[ii];
}

void
svec_put(svec* sv, int ii, char* item)
{
    assert(ii >= 0 && ii < sv->size);
    sv->data[ii] = strdup(item);
}

void
svec_push_back(svec* sv, char* item)
{
    int ii = sv->size;

    if (ii == sv->capacity) {
	sv->data = realloc(sv->data, 2*sv->capacity*(sizeof(char*)));
	sv->capacity = 2*sv->capacity;
    }
    sv->size = ii + 1;
    svec_put(sv, ii, item);
}


void 
print_reverse(svec *sv){
	for(int ii = sv->size - 1; ii >=0; --ii){
		printf("%s\n", sv->data[ii]);
	}
}
