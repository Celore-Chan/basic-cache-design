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
    uint32_t tag = tmp >> 11 ;//����11λȡ����Ǻ� 
    uint32_t set = (tmp >>5) &(0x3F);//ȡ��cache��� 
    uint32_t begin = (address >> 5)<<5;
    printf("%d %d %d\n",tmp,tag,set);
}

uint32_t Cache_Instruction_read(uint32_t address)
{
    uint32_t tmp = address & 0xFFFFF;
    uint32_t tag = tmp >> 11 ;//����11λȡ����Ǻ�
    uint32_t set = (tmp >>5) &(0x3F);//����5λ������6����1����ȡ��cache���
    uint32_t inneraddress = address & 0x1f;//����5����1����ȡ�����ڵ�ַ 
    uint32_t i = 0;

    for(i=0;i<4;i++)
    {
        if(cache.cache_set[set].cache_line[i].valid==1)//���λ��Ч 
        {
            if(cache.cache_set[set].cache_line[i].tag == tag)//�ұ����cache���һ�� 
            {
                uint32_t j = inneraddress;
                uint32_t word = (cache.cache_set[set].cache_line[i].data.block[j]<<0)  |
                                (cache.cache_set[set].cache_line[i].data.block[j+1]<<8)|
                                (cache.cache_set[set].cache_line[i].data.block[j+2]<<16) |
                                (cache.cache_set[set].cache_line[i].data.block[j+3]<<24);  
                return word;//����ָ������ 
            }
        }
    }
    
    waiting = 49;
    Cache_Instruction_write(address);//����ѭ����cache���޸õ�ַ����ʱҪ�滻�������������� 
    return 0xffffffff;
}

void Cache_Instruction_write(uint32_t address)
{
    uint32_t tmp = address & 0xFFFFF;
    uint32_t tag = tmp >> 11 ;//����ʮһλȡ��Ǻ� 
    uint32_t set = (tmp >>5) &(0x3F);//������λ����6����1��ȡ����� 
    uint32_t begin = (address >> 5);
    begin = begin << 5;
    uint32_t i = 0;
    int mark = -1;
    
    for(i=0;i<4;i++)
    {
        if(cache.cache_set[set].cache_line[i].valid==0)//���λ��Ч��ͨ���ǳ�ʼ��ʱ�õ� 
        {
            cache.cache_set[set].cache_line[i].valid = 1;
            cache.cache_set[set].cache_line[i].tag = tag;
            mark = i;//��¼����Ч�������ڵ�����������һ��forѭ����Ҫ�õ� 
            cache.cache_set[set].cache_line[i].rank = 0;//��ɾ����Ϊ0�������¼��� 
            break;
        }
    }
    
    for(i=0;i<4;i++)
    {
         if(mark==i)continue;//���滻��Ч���λ������£��¼��صĲ����������� 
         else
            {
                if(cache.cache_set[set].cache_line[i].rank<3)
                cache.cache_set[set].cache_line[i].rank++;//��Ķ�Ҫ����һ������ 
            }
    }
    
    if(mark==-1){//���λ����Ч 
        uint8_t maxn = 0;
        int i;
        
        for(i=0;i<4;i++)
        {
            if(cache.cache_set[set].cache_line[i].rank==0)continue;//������ع�����������
            else
            {
                if(maxn<=cache.cache_set[set].cache_line[i].rank)
                {
                    maxn = cache.cache_set[set].cache_line[i].rank;
                    mark = i;//����������ġ�Ҫ���滻����һ�� 
                }
            }
        }
        
        cache.cache_set[set].cache_line[mark].valid = 1;
        cache.cache_set[set].cache_line[mark].tag = tag;//�滻���������һ�� 
        cache.cache_set[set].cache_line[mark].rank = 0;        
    }
    
    for(i = 0;i<8;i++)
    {
        uint32_t word = mem_read_32(begin);//�ô棬��ȡ��Ӧλ�õ�ָ�����ݣ�����д��cache 
        cache.cache_set[set].cache_line[mark].data.block[i*4]   = (uint8_t)(word>>0)&0xff;
        cache.cache_set[set].cache_line[mark].data.block[i*4+1] = (uint8_t)(word>>8)&0xff;
        cache.cache_set[set].cache_line[mark].data.block[i*4+2] = (uint8_t)(word>>16)&0xff;
        cache.cache_set[set].cache_line[mark].data.block[i*4+3] = (uint8_t)(word>>24)&0xff;
        begin+=4;
    }
}
