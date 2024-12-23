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
    bool valid;//��Чλ 
    bool dirty;//��λ 
    uint8_t rank;//��������������Խ�����Խ�ٱ�ʹ�� 
    uint32_t tag;//��Ǻţ���memory����Ⱥ�� 
    Data_1 data;//data���� 
}Cache_data_line;

typedef struct 
{
    Cache_data_line cache_data_line[8];//����· 
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
