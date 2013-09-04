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

/* Note: sprintf has broken semantics. Will never be emulated by reprint.
 * snprintf semantics are "repaired". */

/** @brief Print var args data to string. */
int resnprintf(char* dest, unsigned dest_len, const char* fmt, ...);

/** @brief Print var args to file. */
int refprintf(FILE* output, const char* fmt, ...);

/** @brief Print var args to stdout. */
int reprintf(const char* fmt, ...);


/** @brief Print struct packed data to string. */
int resnprintsf(char* dest, unsigned dest_len, const char* fmt, const void* data);

/** @brief Print struct packed data to file. */
int refprintsf(FILE* output, const char* fmt, const void* data); 

/** @brief Print struct packed data to stdout. */
static int reprintsf(const char* fmt, const void* data);


/** @brief Print byte packed data to string. */
int resnprintpf(char* dest, unsigned dest_len, const char* fmt, const uint8_t* data);

/** @brief Print byte packed data to file. */
int refprintpf(FILE* output, const char* fmt, const uint8_t* data); 

/** @brief Print byte packed data to stdout. */
static int reprintpf(const char* fmt, const uint8_t* data);

/* Marshalling functions. */

/** @brief Marshall an unsigned integer value.
 *  @return pointer to byte following last written byte. NULL if speci*/
void* reprint_marshall_unsigned(void* dest, uint8_t specifier, unsigned value);

/** @brief Marshall a signed integer value.
 *  @return pointer to byte following last written byte. */
void* reprint_marshall_signed(void* dest, uint8_t specifier, signed value);

/** @brief Marshall a binary floating point value.
 *  @return pointer to byte following last written byte. */
void* reprint_marshall_bin_floating_pt(void* dest, uint8_t specifier, double value);

/** @brief Marshall a pointer, including string pointers.
 *  @return pointer to byte following last written byte. */
void* reprint_marshall_pointer(void* dest, uint8_t specifier, const void* ptr);

/* Inlines. */
static inline int reprintsf(const char* fmt, const void* data){
	return refprintsf(stdout, fmt, data);
}

static inline int reprintpf(const char* fmt, const uint8_t* data){
	return refprintpf(stdout, fmt, data);
}

#endif
