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
    bool valid;//��Чλ 
    uint8_t rank;//��������������Խ�󣬼���Ե�Խ�ٱ�ʹ�ù� 
    uint16_t tag;//��Ǻţ���memory�е���Ⱥ�� 
    Data data;//instruction���� 
}Cache_line;

typedef struct 
{
    Cache_line cache_line[4];//4��·��
}Cache_set;
typedef struct 
{
    Cache_set cache_set[64];//cache��� 
}Cache_Instruction;
extern Cache_Instruction cache;
void init();
void get_set(uint32_t address);
void Cache_Instruction_write(uint32_t address);
uint32_t Cache_Instruction_read(uint32_t address);

extern int waiting;

#endif
