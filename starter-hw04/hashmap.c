
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// TODO: sudo apt install libbsd-dev
// This provides strlcpy
// See "man strlcpy"
#include <bsd/string.h>
#include <string.h>

#include "hashmap.h"


int
hash(char* key)
{
    // TODO: Produce an appropriate hash value.
    // sum of ASCII values of characters
    int sum=0;
    for (int ii=0; ii < strlen(key); ii++)
        sum += key[ii];
    return sum;
}

hashmap_pair*
make_hashmap_pair(const char* key, const char* val)
{
    hashmap_pair* pp = malloc(sizeof(hashmap_pair));
    pp->key = key;
    pp->val = val;
    pp->tomb = false;
    pp->used = true;
    return pp;
}

hashmap*
make_hashmap_presize(int nn)
{
    hashmap* hh = calloc(1, sizeof(hashmap));
    // TODO: Allocate and initialize a hashmap with capacity 'nn'.
    // Double check "man calloc" to see what that function does.
    hh->loadfactor=0.5;
    hh->capacity=nn;

    // map* mm = malloc(sizeof(map));
    hh->size = 0;
    // mm->capacity = 4;
    hh->pair = calloc(hh->capacity, sizeof(hashmap_pair*));
    return hh;
}

hashmap*
make_hashmap()
{
    return make_hashmap_presize(4);
}

void
free_hashmap(hashmap* hh)
{
    // TODO: Free all allocated data.
    for (int ii = 0; ii < hh->capacity; ii++) {
        //free pair
        free(hh->pair[ii]->key);
        free(hh->pair[ii]->val);
        free(hh->pair[ii]);
    }
    free(hh->pair);
    free(hh);
}

int
hashmap_has(hashmap* hh, char* kk)
{
    return hashmap_get(hh, kk) != -1;
}

int
hashmap_get(hashmap* hh, char* kk)
{
    // TODO: Return the value associated with the
    // key kk.
    // Note: return -1 for key not found.
    int hashget = hash(kk)%hh->capacity;
    for (int ii=0; ii< hh->size; ii++){
        if (strcmp(hashget, hh->pair[ii]->key) == 0) {
            return hh->pair[ii]->val;
        }
    }
    return -1;
}

void
hashmap_put(hashmap* hh, char* kk, int vv)
{
    // TODO: Insert the value 'vv' into the hashmap
    // for the key 'kk', replacing any existing value
    // for that key.
    if (hh->size/hh->capacity >= hh->loadfactor) {
		// hh->capacity *= 2;
		// hh->pair = (hashmap_pair**) realloc(hh->pair, hh->capacity * sizeof(hashmap_pair*));

        int nn = hh->capacity;
        hashmap_pair** pair = hh->pair;

        hh->capacity = 2 * nn;
        hh->pair = calloc(hh->pair, sizeof(hashmap_pair*));
        hh->size = 0;

        // for (int ii = 0; ii < nn; ii++) {
        //     for (hashmap_pair* curr = hh->pair[ii]; curr; curr = curr->next) {
        //         map_put(hh, curr->key, curr->val);
        //     }
        //     free_pair(data[ii]);
        // }
        // free(data);  
	}

    int hashval = hash(kk)%hh->capacity;
    hh->pair[hashval] = make_hashmap_pair(kk, hashval);
    hh->size += 1;
}

void
hashmap_del(hashmap* hh, char* kk)
{
    // TODO: Remove any value associated with
    // this key in the map.
    for (int ii=0; ii< hh->size; ii++){
        if (strcmp(kk, hh->pair[ii]->key) == 0) {
            hh->pair[ii]->tomb=true;
        }
    }
}

hashmap_pair
hashmap_get_pair(hashmap* hh, int ii)
{
    // TODO: Get the {k,v} pair stored in index 'ii'.
    return *hh->pair[ii];
}

void
hashmap_dump(hashmap* hh)
{
    printf("== hashmap dump ==\n");
    // TODO: Print out all the keys and values currently
    // in the map, in storage order. Useful for debugging.
    printf("Key\tValue");
    for (int ii=0; ii< hh->size; ii++){
        printf("%s\t%d\n", hh->pair[ii]->key, hh->pair[ii]->val);
    }
    
}
