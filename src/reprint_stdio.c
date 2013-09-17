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
