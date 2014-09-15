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
#include <assert.h>
#include "reprint_stdio.h"

#define BUFFER_SIZE 1024

/* Assuming not too much craziness going on, though there has to be
 * a better solution...*/
#define DATA_BUFFER_SIZE 512

static reprint_state s_rs;

int resnprintf(uint8_t* dest, unsigned dest_len, const char* fmt, ...){
	va_list ap;
	va_start(ap, fmt);

	uint8_t data_buff[DATA_BUFFER_SIZE];
	if(!reprint_pack_va(data_buff, DATA_BUFFER_SIZE, fmt, ap)){
		va_end(ap);
		return -1;
	}
	va_end(ap);

	return resnprintf_packed(dest, dest_len, fmt, data_buff);
}

int refprintf(FILE* output, const char* fmt, ...){
	va_list ap;
	va_start(ap, fmt);

	uint8_t data_buff[DATA_BUFFER_SIZE];
	if(!reprint_pack_va(data_buff, DATA_BUFFER_SIZE, fmt, ap)){
		va_end(ap);
		return -1;
	}
	va_end(ap);

	return refprintf_packed(output, fmt, data_buff);
}

int reprintf(const char* fmt, ...){
	va_list ap;
	va_start(ap, fmt);

	uint8_t data_buff[DATA_BUFFER_SIZE];
	if(!reprint_pack_va(data_buff, DATA_BUFFER_SIZE, fmt, ap)){
		va_end(ap);
		return -1;
	}
	va_end(ap);

	return refprintf_packed(stdout, fmt, data_buff);
}


int resnprintf_struct(uint8_t* dest, unsigned dest_len, const char* fmt,
	const void* data)
{
	/* Expect at least 1 char sized output. */
	if(dest_len < 1)
		return -1;

	reprint_init(&s_rs, fmt, data, 1);
	uint8_t* end = dest + dest_len - 1;
	while(dest != end){
		int ret = reprint_cb(&s_rs, dest, end - dest);
		if(!ret)
			break;
		if(ret < 0)
			return ret;
		dest += ret;
	}

	/* Add null terminator. */
	*dest = '\0';

	return dest - end;
}

int refprintf_struct(FILE* output, const char* fmt, const void* data){
	reprint_init(&s_rs, fmt, data, 1);
	uint8_t buffer[BUFFER_SIZE];
	int acc = 0;
	while(1){
		uint8_t *x = buffer;
		const uint8_t* end = buffer + BUFFER_SIZE;
		while(x != end){
			int ret = reprint_cb(&s_rs, x, end - x);
			if(!ret)
				break;
			if(ret < 0)
				return ret;

			x += ret;
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


int resnprintf_packed(uint8_t* dest, unsigned dest_len, const char* fmt,
	const uint8_t* data)
{
	/* Expect at least 1 char sized output. */
	if(dest_len < 1)
		return -1;

	reprint_init(&s_rs, fmt, data, 0);
	uint8_t* end = dest + dest_len - 1;
	uint8_t* o = dest;
	while(o != end){
		int ret = reprint_cb(&s_rs, o, end - o);
		if(!ret)
			break;
		if(ret < 0)
			return ret;
		o += ret;
	}

	/* Add null terminator. */
	*o = '\0';

	return o - dest;
}

int refprintf_packed(FILE* output, const char* fmt, const uint8_t* data){
	/* lazy whitebox job from refprintsf...only changed 3rd parameter. */
	reprint_init(&s_rs, fmt, data, 0);
	uint8_t buffer[BUFFER_SIZE];
	int acc = 0;
	while(1){
		uint8_t *x = buffer;
		const uint8_t* end = buffer + BUFFER_SIZE;
		while(x != end){
			int ret = reprint_cb(&s_rs, x, end - x);
			if(!ret)
				break;
			if(ret < 0)
				return ret;

			x += ret;
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
