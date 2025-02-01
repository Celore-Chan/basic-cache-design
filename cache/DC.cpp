#include "DC.h"
#include "pipe.h"
#include "shell.h"
#include "mips.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

Cache_data cache_data;
int waiting_data = 0;

void data_init(){
    for(int i=0;i<256;i++)
    {
        for(int j=0;j<8;j++)
        {
            cache_data.cache_data_set[i].cache_data_line[j].valid = 0;
            cache_data.cache_data_set[i].cache_data_line[j].rank = 0;
            cache_data.cache_data_set[i].cache_data_line[j].dirty = 0;
        }
    }
}

uint32_t cache_data_read(uint32_t address)
{
    uint32_t tag = address >> 13 ;//右移13位拿出标记号 
    uint32_t set = (address >>5) &(0xFF);//右移五位，与上8个“1”，取出cache的组号 
    uint32_t inneraddress = address & 0x1f;//与上5个“1”，取出块内地址 

    for(int i=0;i<8;i++)
    {
        if(cache_data.cache_data_set[set].cache_data_line[i].valid==1)//有效位为“1”才读取 
        {
            if(cache_data.cache_data_set[set].cache_data_line[i].tag == tag)//标记一致 
            {
                uint32_t j = inneraddress;
                uint32_t word = (cache_data.cache_data_set[set].cache_data_line[i].data.block[j]<<0)    |
                                (cache_data.cache_data_set[set].cache_data_line[i].data.block[j+1]<<8)  |
                                (cache_data.cache_data_set[set].cache_data_line[i].data.block[j+2]<<16) |
                                (cache_data.cache_data_set[set].cache_data_line[i].data.block[j+3]<<24);  
                return word;//返回data数据 
            }
        }
    }
    
    waiting_data = 1;
    cache_data_load(address);//没有有效位 
    return 0xffffffff;
}

void cache_data_load(uint32_t address)
{
    uint32_t tag = address >> 13 ;//取标记号 
    uint32_t set = (address >>5) &(0xFF);//取cache组号 
    uint32_t begin = (address >> 5);
    begin = begin << 5;
    int mark = -1;
    uint32_t remain_tag = 0;
    
    for(int i=0;i<8;i++)
    {
        if(cache_data.cache_data_set[set].cache_data_line[i].valid==0)//有效位无效，将被替换 
        {
            mark = i;//标记被替换行位 
            cache_data.cache_data_set[set].cache_data_line[i].valid = 1;
            cache_data.cache_data_set[set].cache_data_line[i].tag = tag;
            cache_data.cache_data_set[set].cache_data_line[i].rank = 0;//排名置零
            cache_data.cache_data_set[set].cache_data_line[mark].dirty = 0;
            break;
        }
    }
    
    for(int i=0;i<8;i++)
    {
         if(mark==i)continue;//只有在有效位无效且被替换时会跳过，这意味着该行的排名保持0 
         else
            {
                if(cache_data.cache_data_set[set].cache_data_line[i].rank<7)
                cache_data.cache_data_set[set].cache_data_line[i].rank++;//排名++ 
            }
    }
    
    if(mark==-1){//有效位都有效，意味着即将有 有效行 被替换，此时脏位要写入内存 
        uint8_t maxn = 0;
        int i;
        for(int i=0;i<8;i++)
        {
            if(cache_data.cache_data_set[set].cache_data_line[i].rank==0)continue;//跳过排名为0的
            else
            {
                if(maxn<=cache_data.cache_data_set[set].cache_data_line[i].rank)//找出排名最大的 
                {
                    mark = i;//标记排名最大的行位 
                    maxn = cache_data.cache_data_set[set].cache_data_line[i].rank;
                    remain_tag = cache_data.cache_data_set[set].cache_data_line[i].tag;//记录当前标记号 
                }
            }
        }
        
        if(cache_data.cache_data_set[set].cache_data_line[mark].dirty == 1){//脏位为0就跳过，即cache数据没有被改过 
        	uint32_t readdress = 0;
        	readdress = (remain_tag<<13)|(set<<5); //重新组合脏位为“1”那行的标记号和cache组号
        	for(i=0;i<8;i++)
        	{
            	uint32_t word = cache_data_read(readdress);//返回相应块内的内容 
            	mem_write_32(readdress,word);//向memory的块内写入内容，访存 
            	readdress+=4;
        	}
        }
        cache_data.cache_data_set[set].cache_data_line[mark].valid = 1;
        cache_data.cache_data_set[set].cache_data_line[mark].tag = tag;//加载新的标记号 
        cache_data.cache_data_set[set].cache_data_line[mark].rank = 0;        
        cache_data.cache_data_set[set].cache_data_line[mark].dirty = 0;
    }
    
    for(int i = 0;i<8;i++)
    {
        uint32_t word = mem_read_32(begin);//访存，取出data相应位置处的数据，写入cache
        cache_data.cache_data_set[set].cache_data_line[mark].data.block[i*4]   = (uint8_t)(word>>0)&0xff;
        cache_data.cache_data_set[set].cache_data_line[mark].data.block[i*4+1] = (uint8_t)(word>>8)&0xff;
        cache_data.cache_data_set[set].cache_data_line[mark].data.block[i*4+2] = (uint8_t)(word>>16)&0xff;
        cache_data.cache_data_set[set].cache_data_line[mark].data.block[i*4+3] = (uint8_t)(word>>24)&0xff;
        begin+=4;
    }
}


void cache_data_write_val(uint32_t address,uint32_t write)
{
    if(cache_data_read(address)==0xffffffff){//没有有效位 
        bool blank=0;
    }
    
    uint32_t tag = address >> 13 ;//取标记号 
    uint32_t set = (address >>5) &(0xFF);//取cache组号 
    uint32_t inneraddress = address & 0x1f;//取块内地址 
     
    for(int i=0;i<8;i++)
    {
        if(cache_data.cache_data_set[set].cache_data_line[i].valid==1)//有效位有效才能往cache内写内容 
        {
            if(cache_data.cache_data_set[set].cache_data_line[i].tag == tag)
            {
                uint32_t j = inneraddress;
                cache_data.cache_data_set[set].cache_data_line[i].dirty = 1;//脏位置“1” 
                cache_data.cache_data_set[set].cache_data_line[i].data.block[j]   = (uint8_t)(write>>0)&0xff;
                cache_data.cache_data_set[set].cache_data_line[i].data.block[j+1] = (uint8_t)(write>>8)&0xff;
                cache_data.cache_data_set[set].cache_data_line[i].data.block[j+2] = (uint8_t)(write>>16)&0xff;
                cache_data.cache_data_set[set].cache_data_line[i].data.block[j+3] = (uint8_t)(write>>24)&0xff;
                return;           
            }
        }
    }
}
