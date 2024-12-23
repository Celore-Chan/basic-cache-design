#include "IC.h"
#include "pipe.h"
#include "shell.h"
#include "mips.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

Cache_Instruction cache;
int waiting = 0;

void init(){
    int i =0, j = 0;
    for(i=0;i<64;i++)
    {
        for(j=0;j<4;j++)
        {
            cache.cache_set[i].cache_line[j].valid = 0;
            cache.cache_set[i].cache_line[j].rank = 0;
        }
    }
}

void get_set(uint32_t address)
{
    uint32_t tmp = address & 0xFFFFF;
    uint32_t tag = tmp >> 11 ;//右移11位取出标记号 
    uint32_t set = (tmp >>5) &(0x3F);//取出cache组号 
    uint32_t begin = (address >> 5)<<5;
    printf("%d %d %d\n",tmp,tag,set);
}

uint32_t Cache_Instruction_read(uint32_t address)
{
    uint32_t tmp = address & 0xFFFFF;
    uint32_t tag = tmp >> 11 ;//右移11位取出标记号
    uint32_t set = (tmp >>5) &(0x3F);//右移5位，与上6个“1”，取出cache组号
    uint32_t inneraddress = address & 0x1f;//与上5个“1”，取出块内地址 
    uint32_t i = 0;

    for(i=0;i<4;i++)
    {
        if(cache.cache_set[set].cache_line[i].valid==1)//标记位有效 
        {
            if(cache.cache_set[set].cache_line[i].tag == tag)//且标记与cache标记一致 
            {
                uint32_t j = inneraddress;
                uint32_t word = (cache.cache_set[set].cache_line[i].data.block[j]<<0)  |
                                (cache.cache_set[set].cache_line[i].data.block[j+1]<<8)|
                                (cache.cache_set[set].cache_line[i].data.block[j+2]<<16) |
                                (cache.cache_set[set].cache_line[i].data.block[j+3]<<24);  
                return word;//返回指令数据 
            }
        }
    }
    
    waiting = 49;
    Cache_Instruction_write(address);//跳出循环即cache内无该地址，此时要替换该组的排名最大行 
    return 0xffffffff;
}

void Cache_Instruction_write(uint32_t address)
{
    uint32_t tmp = address & 0xFFFFF;
    uint32_t tag = tmp >> 11 ;//左移十一位取标记号 
    uint32_t set = (tmp >>5) &(0x3F);//右移五位与上6个“1”取得组号 
    uint32_t begin = (address >> 5);
    begin = begin << 5;
    uint32_t i = 0;
    int mark = -1;
    
    for(i=0;i<4;i++)
    {
        if(cache.cache_set[set].cache_line[i].valid==0)//标记位无效，通常是初始化时用到 
        {
            cache.cache_set[set].cache_line[i].valid = 1;
            cache.cache_set[set].cache_line[i].tag = tag;
            mark = i;//记录下无效行在组内的行数，在下一个for循环内要用到 
            cache.cache_set[set].cache_line[i].rank = 0;//被删排名为0，即最新加载 
            break;
        }
    }
    
    for(i=0;i<4;i++)
    {
         if(mark==i)continue;//在替换无效标记位的情况下，新加载的不用增加排名 
         else
            {
                if(cache.cache_set[set].cache_line[i].rank<3)
                cache.cache_set[set].cache_line[i].rank++;//别的都要增加一次排名 
            }
    }
    
    if(mark==-1){//标记位都有效 
        uint8_t maxn = 0;
        int i;
        
        for(i=0;i<4;i++)
        {
            if(cache.cache_set[set].cache_line[i].rank==0)continue;//最近加载过，跳过该行
            else
            {
                if(maxn<=cache.cache_set[set].cache_line[i].rank)
                {
                    maxn = cache.cache_set[set].cache_line[i].rank;
                    mark = i;//标记排名最大的、要被替换的那一行 
                }
            }
        }
        
        cache.cache_set[set].cache_line[mark].valid = 1;
        cache.cache_set[set].cache_line[mark].tag = tag;//替换排名最大那一行 
        cache.cache_set[set].cache_line[mark].rank = 0;        
    }
    
    for(i = 0;i<8;i++)
    {
        uint32_t word = mem_read_32(begin);//访存，读取相应位置的指令数据，并将写入cache 
        cache.cache_set[set].cache_line[mark].data.block[i*4]   = (uint8_t)(word>>0)&0xff;
        cache.cache_set[set].cache_line[mark].data.block[i*4+1] = (uint8_t)(word>>8)&0xff;
        cache.cache_set[set].cache_line[mark].data.block[i*4+2] = (uint8_t)(word>>16)&0xff;
        cache.cache_set[set].cache_line[mark].data.block[i*4+3] = (uint8_t)(word>>24)&0xff;
        begin+=4;
    }
}
