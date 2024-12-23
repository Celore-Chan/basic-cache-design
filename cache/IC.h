#ifndef CACHE
#define CACHE

#include "stdio.h"
#include "mips.h"
#include "stdbool.h"
#include "stdint.h"
typedef struct 
{
    uint8_t block[32];
}Data;

typedef struct 
{
    bool valid;//有效位 
    uint8_t rank;//组内排名，排名越大，即相对地越少被使用过 
    uint16_t tag;//标记号，即memory中的组群号 
    Data data;//instruction数据 
}Cache_line;

typedef struct 
{
    Cache_line cache_line[4];//4组路
}Cache_set;
typedef struct 
{
    Cache_set cache_set[64];//cache组号 
}Cache_Instruction;
extern Cache_Instruction cache;
void init();
void get_set(uint32_t address);
void Cache_Instruction_write(uint32_t address);
uint32_t Cache_Instruction_read(uint32_t address);

extern int waiting;

#endif
