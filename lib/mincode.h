/*
 * mincode.h
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
 
#ifndef __MINCODE_H__
#define __MINCODE_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

unsigned char ctox(char src);
unsigned char atox(char *src, int len);
void incode_ctoxs(unsigned char *dest , char *src, int len);
void incode_xtocs(char *dest , unsigned char *src, int len);
void incode_ctox16(unsigned short *dest, char *src);
void incode_xtoc16(char *dest, unsigned short src);
void incode_ctox32(unsigned int *dest, char *src);
void incode_xtoc32(char *dest, unsigned int src);
unsigned int gen_rand(unsigned char *seed);

#ifdef __cplusplus
}
#endif

#endif //  __MINCODE_H __
