
//----------Assignment 3: Cache Simulator---------
//----------Stephen Anthony Via-------------------
//----------3/29/2019-----------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cachesim.h"

counter_t accesses = 0, write_hits = 0, read_hits = 0,
          write_misses = 0, read_misses = 0, writebacks = 0;
/*  -----------------------------------------------
    added counters
    -----------------------------------------------
*/
counter_t hitsTotal = 0, missesTotal = 0, readCount = 0, writeCount = 0;

//function declerations
addr_t addressMask(addr_t, int, int);
void LRUreplacement(addr_t, addr_t, addr_t, int);

// global cache array of cache_block pointers
struct cache_block *cache;
// global variables initialzed to be used in cachesim_access
// global number of sets = (cache_size) / (line_size * associativity)
uint32_t g_blocksize = 0, g_cachesize = 0, g_ways = 0, g_sets = 0;

/*  -----------------------------------------------
    cache block struct
    -----------------------------------------------
    valid:      cache block is used or not used
    dirty:      cache block has been written to (1)
    age_bit:    counter -> # of accesses
    tag:        computed from pyhsical address
    -----------------------------------------------
*/
typedef struct cache_block {
    int *valid; //pointer to valid array
    int *dirty; //pointer to dirty array
    int *age_bit; //pointer to "age bit" array -> # of accesses
    addr_t *tag; //key || unique tag per access
} cache_block;
/* Use this method to initialize your cache data structure
* You should do all of your memory allocation here
*/
void cachesim_init(uint32_t blocksize, uint32_t cachesize, uint32_t ways)
{
    // printf("here");
    int sets = cachesize / (blocksize * ways); //number of sets in the cache
    // printf("sets: %d\n", sets);
    //set the globals to be used later in access
    g_blocksize = blocksize;
    g_cachesize = cachesize;
    g_ways = ways;
    g_sets = sets;
    //allocate memory for the entire cache array
    cache = (struct cache_block*)malloc(g_sets * sizeof(cache_block));
    //allocate memmory for the iner array for each field.
    for(int i = 0; i < g_sets; i++) {
        cache[i].valid = (int*)malloc(ways * sizeof(int)); //dynamically allocate for each row
        cache[i].dirty = (int*)malloc(ways * sizeof(int));
        cache[i].age_bit = (int*)malloc(ways * sizeof(int));
        cache[i].tag = (addr_t*)malloc(ways * sizeof(addr_t));
    }
    //establish a cold cash
    //printf("ways: %d", ways);
    for(int i = 0; i < g_sets; i++) {
        // printf("i: %d", i);
        for(int j = 0; j < g_ways; j++) {
            // printf("j: %d", j);
            cache[i].valid[j] = 0;
            cache[i].dirty[j] = 0;
            cache[i].age_bit[j] = 0;
            cache[i].tag[j] = 0;
        }
    }
}

/* Clean up dynamically allocated memory when the program terminates */
void cachesim_destruct()
{
    // freein the pointer to the head of these lists should garbage collect contents
    // free(cache);
    for (int i = 0; i < g_sets; i++) {
        free(cache[i].valid);
        free(cache[i].dirty);
        free(cache[i].age_bit);
        free(cache[i].tag);
    }
    free(cache);
}

/* Called on each access
* 'write' is a boolean representing whether a request is read(0) or write(1)
*/
void cachesim_access(addr_t physical_addr, uint32_t write)
{
    //printf("here");
    // increment access counter
    // exit(0);
    accesses++;
    // calculations for bit fields
    int offset_bits = log2(g_blocksize);
    //printf("offset bits: %d", offset_bits);
    int index_bits = log2(g_sets);
    //DEBUGGING -----------------------------------------------
    //printf("index bits: %d", index_bits);
    int tag_bits = (32) - offset_bits - index_bits; //remaining bits
    //printf("sizeof tag_bits: %d ", tag_bits);
    // MSB's
    // int offsetMSB = offset_bits - 1;
    // int indexMSB = index_bits + offset_bits - 1;
    // index into the physical address
    // -----------------------------------------------
    //                 problem here
    // printf("HERE\n");
    addr_t offset = addressMask(physical_addr, offset_bits, 0);
    // printf("address before: %lu\n", physical_addr);
    addr_t index = addressMask(physical_addr, index_bits, offset_bits);
    // printf("index: %lu\n", index);
    //exit(0);
    //addr_t index = 0;
    addr_t tag = addressMask(physical_addr, tag_bits, index_bits + offset_bits); //remaining bits
    // addr_t offset = addressMask(physical_addr, offset_bits, 0);
    // addr_t index = addressMask(physical_addr >> offset_bits, index_bits, 0);
    // addr_t tag = (physical_addr >> (b_bits_offset + index_bits));
    // -----------------------------------------------
    // save the total amount of read and writes for ratio calculation
    if (write) {
        writeCount++;
    } else {
        readCount++;
    }
    // boolean to determine if LRU is required
    int found = 0;
    for (int j = 0; j < g_ways; j++) {
        // this is a cache HIT
        // if it is a valid line and the tag matches
        if (cache[index].valid[j] == 1 && cache[index].tag[j] == tag) {
            // no need to use LRU replacement
            found = 1;
            // hit
            hitsTotal++;
            // set the age_bit to the access counter
            cache[index].age_bit[j] = accesses;
            //fix dirty
            if (write == 1) {
                // simluate write the data on HIT
                write_hits++;
                cache[index].dirty[j] = 1;
            } else {
                // simulate read the data on HIT -> nothing changes
                read_hits++;
            }
            break;
        // this is a cache MISS
        // if it is not valid (has not been used) and the tag does not match
        // this means the spot is open -> fill it with new infromation
        } else if (cache[index].valid[j] == 0 && cache[index].tag[j] != tag) {
            // no need to use LRU replacement
            found = 1;
            // miss
            missesTotal++;
            // populate the line with new meta-data
            cache[index].age_bit[j] = accesses;
            cache[index].valid[j] = 1;
            cache[index].tag[j] = tag;
            //fix dirty
            if (write) {
                write_misses++;
                cache[index].dirty[j] = 1;
            } else {
                read_misses++;
                cache[index].dirty[j] = 0; // reset this to 0 if it is dirty
                // this fills the cache with the data -> like a write; but
                // memory and cache will not differ on a cold read
            }
            break;
        }
    }
    // -----------------------------------------------
    //              COMPULSORY MISS
    // -----------------------------------------------
    // if set is full -> no cold miss or no hit
    // if all tags are valid (valid[j] == 1) and all tags are full (no match).
    // -----------------------------------------------
    if (!found) {
        missesTotal++;
        LRUreplacement(physical_addr, index, tag, write);
    }
}

/*  -----------------------------------------------
    Method for writeback policy:
        -evict the least recently used in the LRU list
            i. find the smallest access counter under age_bit fields
            ii. if it is dirty (was written to) -> increment the writeback
            iii. if it is a write -> make dirty, if it is a read -> leave 0;
            iv. set the access to the latest access counter
            v. set tag to new tag
    -----------------------------------------------
*/
void LRUreplacement(addr_t physical_addr, addr_t index, addr_t tag, int write) {
    // assume min is the first "way"
    int minLRU = cache[index].age_bit[0];
    int minIndex = 0;
    // find the minimum access # -> this is the oldest etnry in the cache set
    for (int j = 1; j < g_ways; j++) {
        if (cache[index].age_bit[j] < minLRU) {
            minLRU = cache[index].age_bit[j];
            minIndex = j;
        }
    }
    // evict the tag line at found min Index
    if (cache[index].dirty[minIndex]) {
        writebacks++; // evicting causes a writeback to memory
    }
    // increment misses counters for write and read
    if (write) {
        write_misses++;
        cache[index].dirty[minIndex] = 1;
    } else {
        read_misses++;
        cache[index].dirty[minIndex] = 0;
    }
    // update with new information
    cache[index].age_bit[minIndex] = accesses;  //update the LRU count with the newest access iteration
    cache[index].tag[minIndex] = tag; // new tag
}
/**
    calculates the appropriate bits required for the field of the address
    the left and the right bits of the appropriate field
*/
addr_t addressMask(addr_t address, int length, int shift) {
    // start with 32bit all 1's
    addr_t mask = 0xFFFFFFFF;
    // shift number of time's the length of (1's in the mask)
    mask = (mask << length);
    // negate mask
    mask = ~mask;
    // shift over to appropriate position
    mask = mask << shift;
    // and with mask -> then shift back to position
    return ((address & mask) >> shift);
}
/* You may not change this method in your final submission!!!!!
*   Furthermore, your code should not have any extra print statements
*/

void cachesim_print_stats()
{
    // printf("\n%.6f\n%.6f\n%.6f\n", (double) missesTotal / accesses, (double) read_misses /
                                                  // readCount, (double) write_misses / writeCount);
    // printf("\n%llu", missesTotal);
    // printf("\n%llu", accesses);
  printf("%llu, %llu, %llu, %llu, %llu, %llu\n", accesses, read_hits, write_hits,
                                                read_misses, write_misses, writebacks);
}
