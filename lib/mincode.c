/*
 * mincode.c
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#include "mincode.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned char ctox(char src)
{
	char temp = 0;

	if(src>='0' && src<='9')
		temp = src-'0';
	else if(src>='a' && src<='f')
		temp = src-'a'+10;
	else if(src>='A' && src<='F')
		temp = src-'A'+10;

	return temp;
}

unsigned char atox(char *src, int len)
{
	char temp=0, i=0, length;
	length = len;

	while(length--)
	{
		temp = ctox(*(src+i)) + (temp<<4);
		i++;
	}

	return temp;
}
 
void incode_ctoxs(unsigned char *dest ,char *src, int len)
{
	int i, temp;
	if(len<2 || dest==NULL || src==NULL)
	{
		return;
	}

	for(i=0; i<(len>>1); i++)
	{
		temp = *(src+(i<<1));
		dest[i] = (ctox(temp)<<4);
		
		temp = *(src+(i<<1)+1);
		dest[i] += ctox(temp);
	}
	 
}

void incode_xtocs(char *dest , unsigned char *src, int len)
{
    int i, temp;
	if(len<1 || dest==NULL || src==NULL)
	{
		return;
	}

	for(i=0; i<len; i++)
	{
		temp = (*(src+i)>>4);
		if(temp < 0xA)
		{
			dest[(i<<1)] = temp + '0';	
		}
		else
		{
			dest[(i<<1)] = temp - 0xA + 'A';	
		}
		
		temp = (*(src+i)&0x0F);
		if(temp < 0xA)
		{
			dest[(i<<1)+1] = temp + '0';	
		}
		else
		{
			dest[(i<<1)+1] = temp - 0xA + 'A';	
		}
	}
}


void incode_ctox16(unsigned short *dest, char *src)
{
	unsigned char dsts[2];
	incode_ctoxs(dsts, src, 4);
	*dest = dsts[0]<<8;
	*dest += dsts[1];
}

void incode_xtoc16(char *dest, unsigned short src)
{
	unsigned char val[2];
	val[0] = (src>>8);
	val[1] = (src&0xFF);
	incode_xtocs(dest, val, 2);
}

void incode_ctox32(unsigned int *dest, char *src)
{
	int i;
	char dsts[4];
	incode_ctoxs((unsigned char *)dsts, src, 8);

	*dest = 0;
	for(i=0; i<4; i++)
	{
		*dest += ((*dest)<<8)+dsts[i];
	}
}

void incode_xtoc32(char *dest, unsigned int src)
{
	unsigned char val[4];
	val[0] = ((src>>24)&0xFF);
	val[1] = ((src>>16)&0xFF);
	val[2] = ((src>>8)&0xFF);
	val[3] = (src&0xFF);
	
	incode_xtocs(dest, val, 4);
}

unsigned int gen_rand(unsigned char *seed)
{
	int i;
	unsigned int ra = 0;
	for(i=0; i<8; i+=2)
	{
		ra += seed[i]<<(i<<2);
	}

	return ra;
}

#ifdef __cplusplus
}
#endif
