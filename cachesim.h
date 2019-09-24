#ifndef __CACHESIM_H
#define __CACHESIM_H
typedef unsigned int uint32_t;
typedef unsigned int addr_t;
typedef unsigned int uint;
typedef unsigned long long counter_t;
typedef unsigned long long uint64_t;
typedef unsigned char uint8_t;
typedef unsigned char byte_t;

void cachesim_init(uint32_t block_size, uint32_t cache_size, uint32_t ways);
void cachesim_access(addr_t physical_add, uint32_t write);

void cachesim_print_stats(void);
void cachesim_destruct(void);

//new function declerations
addr_t addressMask(addr_t, int, int);
void LRUreplacement(addr_t, addr_t, addr_t, int);

#endif
