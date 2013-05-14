/*
 * CFile1.c
 *
 * Created: 15.04.2013 20:09:18
 *  Author: Avega
 */ 


#include "compilers.h"



// Converts uint16_t value into decimal string with right alignment
// len = number of digits output (without terminating '\0')
// len[7] = 1 -> no terminating '\0' at buffer[len]
// fill_char -> char used for string padding
void u16toa_align_right(uint16_t val, char *buffer, uint8_t len,char fill_char)
{
	if (len & 0x80)
	{
		len = len & 0x7F;
		buffer += len;	
	}
	else
	{
		buffer += len;	
		*buffer = 0;
	}
	
	do
	{
		*--buffer = val % 10 + '0';
		val /= 10;
		len--;
	}
	while ((val != 0) && len);
	
	// Padding
	while(len--)
		*--buffer = fill_char;
}

