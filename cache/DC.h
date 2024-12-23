#ifndef CACHEDATA
#define CACHEDATA

#include "stdio.h"
#include "mips.h"
#include "stdbool.h"
#include "stdint.h"

typedef struct 
{
    uint8_t block[32];
}Data_1;

typedef struct 
{
    bool valid;//有效位 
    bool dirty;//脏位 
    uint8_t rank;//组内排名，排名越大，相对越少被使用 
    uint32_t tag;//标记号，即memory的组群号 
    Data_1 data;//data数据 
}Cache_data_line;

typedef struct 
{
    Cache_data_line cache_data_line[8];//八组路 
}Cache_data_set;

typedef struct 
{
    Cache_data_set cache_data_set[256];
}Cache_data;

extern Cache_data cache_data;
void data_init();
void cache_data_write_val(uint32_t address,uint32_t write);
void cache_data_load(uint32_t address);
uint32_t cache_data_read(uint32_t address);
extern int waiting_data;

#endif
