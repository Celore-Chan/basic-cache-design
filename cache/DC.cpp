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
    int i =0, j = 0;
    for(i=0;i<256;i++)
    {
        for(j=0;j<8;j++)
        {
            cache_data.cache_data_set[i].cache_data_line[j].valid = 0;
            cache_data.cache_data_set[i].cache_data_line[j].rank = 0;
            cache_data.cache_data_set[i].cache_data_line[j].dirty = 0;
        }
    }
}

uint32_t cache_data_read(uint32_t address)
{
    uint32_t tag = address >> 13 ;//����13λ�ó���Ǻ� 
    uint32_t set = (address >>5) &(0xFF);//������λ������8����1����ȡ��cache����� 
    uint32_t inneraddress = address & 0x1f;//����5����1����ȡ�����ڵ�ַ 
    uint32_t i = 0;

    for(i=0;i<8;i++)
    {
        if(cache_data.cache_data_set[set].cache_data_line[i].valid==1)//��ЧλΪ��1���Ŷ�ȡ 
        {
            if(cache_data.cache_data_set[set].cache_data_line[i].tag == tag)//���һ�� 
            {
                uint32_t j = inneraddress;
                uint32_t word = (cache_data.cache_data_set[set].cache_data_line[i].data.block[j]<<0)    |
                                (cache_data.cache_data_set[set].cache_data_line[i].data.block[j+1]<<8)  |
                                (cache_data.cache_data_set[set].cache_data_line[i].data.block[j+2]<<16) |
                                (cache_data.cache_data_set[set].cache_data_line[i].data.block[j+3]<<24);  
                return word;//����data���� 
            }
        }
    }
    
    waiting_data = 49;
    cache_data_load(address);//û����Чλ 
    return 0xffffffff;
}

void cache_data_load(uint32_t address)
{
    uint32_t tag = address >> 13 ;//ȡ��Ǻ� 
    uint32_t set = (address >>5) &(0xFF);//ȡcache��� 
    uint32_t begin = (address >> 5);
    begin = begin << 5;
    int i = 0;
    int mark = -1;
    uint32_t getline = 0;
    uint32_t getset = 0;
    
    for(i=0;i<8;i++)
    {
        if(cache_data.cache_data_set[set].cache_data_line[i].valid==0)//��Чλ��Ч�������滻 
        {
            cache_data.cache_data_set[set].cache_data_line[i].valid = 1;
            cache_data.cache_data_set[set].cache_data_line[i].tag = tag;
            mark = i;//��Ǳ��滻��λ 
            cache_data.cache_data_set[set].cache_data_line[i].rank = 0;//��������
            cache_data.cache_data_set[set].cache_data_line[mark].dirty = 0;
            break;
        }
    }
    
    for(i=0;i<8;i++)
    {
         if(mark==i)continue;//ֻ������Чλ��Ч�ұ��滻ʱ������������ζ�Ÿ��е���������0 
         else
            {
                if(cache_data.cache_data_set[set].cache_data_line[i].rank<7)
                cache_data.cache_data_set[set].cache_data_line[i].rank++;//����++ 
            }
    }
    
    if(mark==-1){//��Чλ����Ч����ζ�ż����� ��Ч�� ���滻����ʱ��λҪд���ڴ� 
        uint8_t maxn = 0;
        int i;
        for(i=0;i<8;i++)
        {
            if(cache_data.cache_data_set[set].cache_data_line[i].rank==0)continue;//��������Ϊ0��
            else
            {
                if(maxn<=cache_data.cache_data_set[set].cache_data_line[i].rank)//�ҳ��������� 
                {
                    maxn = cache_data.cache_data_set[set].cache_data_line[i].rank;
                    mark = i;//�������������λ 
                    getline = cache_data.cache_data_set[set].cache_data_line[i].tag;//��¼��ǰ��Ǻ� 
                    getset = set;//��¼��ǰcache��� 
                }
            }
        }
        
        if(cache_data.cache_data_set[set].cache_data_line[mark].dirty == 1){//��λΪ0����������cache����û�б��Ĺ� 
        	uint32_t readdress = 0;
        	readdress = (getline<<13)|(set<<5); //���������λΪ��1�����еı�Ǻź�cache���
        	for(i=0;i<8;i++)�次
        	{
            	uint32_t word = cache_data_read(readdress);//������Ӧ���ڵ����� 
            	mem_write_32(readdress,word);//��memory�Ŀ���д�����ݣ��ô� 
            	readdress+=4;
        	}
        }
        cache_data.cache_data_set[set].cache_data_line[mark].valid = 1;
        cache_data.cache_data_set[set].cache_data_line[mark].tag = tag;//�����µı�Ǻ� 
        cache_data.cache_data_set[set].cache_data_line[mark].rank = 0;        
        cache_data.cache_data_set[set].cache_data_line[mark].dirty = 0;
    }
    
    for(i = 0;i<8;i++)
    {
        uint32_t word = mem_read_32(begin);//�ô棬ȡ��data��Ӧλ�ô������ݣ�д��cache
        cache_data.cache_data_set[set].cache_data_line[mark].data.block[i*4]   = (uint8_t)(word>>0)&0xff;
        cache_data.cache_data_set[set].cache_data_line[mark].data.block[i*4+1] = (uint8_t)(word>>8)&0xff;
        cache_data.cache_data_set[set].cache_data_line[mark].data.block[i*4+2] = (uint8_t)(word>>16)&0xff;
        cache_data.cache_data_set[set].cache_data_line[mark].data.block[i*4+3] = (uint8_t)(word>>24)&0xff;
        begin+=4;
    }
}


void cache_data_write_val(uint32_t address,uint32_t write)
{
    if(cache_data_read(address)==0xffffffff){//û����Чλ 
        bool blank=0;
    }
    
    uint32_t tag = address >> 13 ;//ȡ��Ǻ� 
    uint32_t set = (address >>5) &(0xFF);//ȡcache��� 
    uint32_t inneraddress = address & 0x1f;//ȡ���ڵ�ַ 
    uint32_t i = 0;
     
    for(i=0;i<8;i++)
    {
        if(cache_data.cache_data_set[set].cache_data_line[i].valid==1)//��Чλ��Ч������cache��д���� 
        {
            if(cache_data.cache_data_set[set].cache_data_line[i].tag == tag)
            {
                uint32_t j = inneraddress;
                cache_data.cache_data_set[set].cache_data_line[i].dirty = 1;//��λ�á�1�� 
                cache_data.cache_data_set[set].cache_data_line[i].data.block[j]   = (uint8_t)(write>>0)&0xff;
                cache_data.cache_data_set[set].cache_data_line[i].data.block[j+1] = (uint8_t)(write>>8)&0xff;
                cache_data.cache_data_set[set].cache_data_line[i].data.block[j+2] = (uint8_t)(write>>16)&0xff;
                cache_data.cache_data_set[set].cache_data_line[i].data.block[j+3] = (uint8_t)(write>>24)&0xff;
                return;           
            }
        }
    }
}
