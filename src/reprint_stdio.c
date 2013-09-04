/* reprint_stdio.c
 * Copyright (c) 2013 David Bender. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:

 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * */

#define _GNU_SOURCE
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "reprint_stdio.h"

#define BUFFER_SIZE 1024

int resnprintf(char* dest, unsigned dest_len, const char* fmt, ...){
	/* TODO function to pack va_args data into array with no gaps. */
	return -1;
}

int refprintf(FILE* output, const char* fmt, ...){
	/* same TODO as resnprintf */
	return -1;
}

int reprintf(const char* fmt, ...){
	/* same TODO as resnprintf */
	return -1;
}


int resnprintsf(char* dest, unsigned dest_len, const char* fmt, const void* data){
	/* Expect at least 1 char sized output. */
	if(dest_len < 1)
		return -1;

	reprint_init(fmt, data, 1);
	char* end = dest + dest_len - 1;
	while(dest != end){
		int ret = reprint_cb(dest);
		++dest;
		if(!ret)
			break;
	}

	/* Add null terminator. */
	*dest = '\0';

	return dest - end;
}

int refprintsf(FILE* output, const char* fmt, const void* data){
	reprint_init(fmt, data, 1);
	char buffer[BUFFER_SIZE];
	int acc = 0;
	while(1){
		char *x;
		for(x = buffer; x != buffer + BUFFER_SIZE; ++x){
			if(!reprint_cb(x))
				break;
		}
		int ret = fwrite(buffer, 1, x - buffer, output);
		if(ret < 0)
			return ret;
		acc += ret;
		if(x != buffer + BUFFER_SIZE)
			break;
	}

	return acc;
}


int resnprintpf(char* dest, unsigned dest_len, const char* fmt, const uint8_t* data){
	/* Expect at least 1 char sized output. */
	if(dest_len < 1)
		return -1;

	reprint_init(fmt, data, 0);
	char* end = dest + dest_len - 1;
	char* o = dest;
	while(o != end){
		int ret = reprint_cb(o);
		if(!ret)
			break;
		++o;
	}

	/* Add null terminator. */
	*o = '\0';

	return o - dest;
}

int refprintpf(FILE* output, const char* fmt, const uint8_t* data){
	/* lazy whitebox job from refprintsf...only changed 3rd parameter. */
	reprint_init(fmt, data, 0);
	char buffer[BUFFER_SIZE];
	int acc = 0;
	while(1){
		char *x;
		for(x = buffer; x != buffer + BUFFER_SIZE; ++x){
			if(!reprint_cb(x))
				break;
		}
		int ret = fwrite(buffer, 1, x - buffer, output);
		if(ret < 0)
			return ret;
		acc += ret;
		if(x != buffer + BUFFER_SIZE)
			break;
	}

	return acc;
}

void* reprint_marshall_unsigned(void* dest, uint8_t specifier, unsigned value){
	switch(specifier){
		/* uint8_t */
		case 0x40:
			{
				uint8_t v = value;
				*(uint8_t*)dest = v;
				return dest + 1;
			}
			break;

		/* uint16_t */
		case 0x41:
			{
				uint16_t v = value;
				memcpy(dest, &v, 2);
				return dest + 2;
			}
			break;

		/* uint32_t */
		case 0x42:
			{
				uint32_t v = value;
				memcpy(dest, &v, 4);
				return dest + 4;
			}
			break;

		default:
			break;
	}
}

void* reprint_marshall_signed(void* dest, uint8_t specifier, signed value){
	switch(specifier){
		/* int8_t */
		case 0x48:
			{
				int8_t v = value;
				*(int8_t*)dest = v;
				return dest + 1;
			}

		/* int16_t */
		case 0x49:
			{
				int16_t v = value;
				memcpy(dest, &v, 2);
				return dest + 2;
			}

		/* int32_t */
		case 0x4A:
			{
				int32_t v = value;
				memcpy(dest, &v, 4);
				return dest + 4;
			}

		default:
			return NULL;
	}
}

void* reprint_marshall_bin_floating_pt(void* dest, uint8_t specifier, double value){
	switch(specifier){
		/* half precision*/
		case 0x50:
			{
				/* TODO */
				return NULL;
			}

		/* 32 bit fp*/
		case 0x41:
			{
				float f = value;
				memcpy(dest, &f, 4);
				return dest + 4;
			}

		/* 64 bit. */
		case 0x42:
			{
				memcpy(dest, &value, 8);
				return dest + 8;
			}

		default:
			return NULL;
	}
}

void* reprint_marshall_pointer(void* dest, uint8_t specifier, const void* ptr){
	switch(specifier){
		/* strings */
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:

		/* pointer data. */
		case 0x70:
			memcpy(dest, &ptr, sizeof(ptr));
			return dest + sizeof(ptr);

		default:
			return NULL;
	}
}

void* reprint_marshall_char(void* dest, uint8_t specifier, unsigned code_point){
	switch(specifier){
		/* char */
		case 0x68:
			{
				char v = code_point;
				*(char*)dest = v;
				return dest + 1;
			}

		case 0x69:
			return NULL;

		case 0x6A:
			return NULL;

		case 0x6B:
			return NULL;

		default:
			return NULL;
	}

}
