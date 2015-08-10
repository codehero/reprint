/* reprint_stdio.h
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

#ifndef __REPRINT_STDIO_H__
#define __REPRINT_STDIO_H__

/** @brief Functions operating with stdio style semantics.
 * Intended for desktop/heavy embedded use, obviously.
 * NOT for bare metal microcontrollers. */

#include <stdio.h>
#include "reprint.h" 
#include "reprint_aux.h" 

/* Note: sprintf has broken semantics. Will never be emulated by reprint.
 * snprintf semantics are "repaired". */

/** @brief Print var args data to byte string. */
int resnprintf(uint8_t* dest, unsigned dest_len, const char* fmt, ...);

/** @brief Print var args to file. */
int refprintf(FILE* output, const char* fmt, ...);

/** @brief Print var args to stdout. */
int reprintf(const char* fmt, ...);

/*  */

/** @brief Print marshalled args to string buffer. */
int resnprintf_ptr(uint8_t* dest, unsigned dest_len, const char* fmt,
	const void* data);

/** @brief Print marshalled args to file. */
int refprintf_ptr(FILE* output, const char* fmt, const void* data);

/** @brief Print marshalled args to stdout. */
static inline int reprintf_ptr(const char* fmt, const void* data){
	return refprintf_ptr(stdout, fmt, data);
}

#endif
