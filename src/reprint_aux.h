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

#ifndef __REPRINT_AUX_H__
#define __REPRINT_AUX_H__

#include <stdarg.h>
#include "reprint.h"

enum {
	REP_FLAG_SPECIFIER_IS_REGISTER = 0x8000
	,REP_SPECIFIER_OFFSET_TYPE = 4
	,REP_SPECIFIER_OFFSET_AUX1 = 7
	,REP_SPECIFIER_OFFSET_AUX2 = 9
};

/** @brief Search format string for next input specifier.
 *  @param dest Where to store input specifier
 *  @param fmt format string
 *  @return NULL if no specifier found;
 *  pointer to first char succeeding specifier otherwise.
 *
 *  If REP_FLAG_SPECIFIER_IS_REGISTER is set then expecting a register value,
 *  not an input specifier value. */
const char* reprint_get_next_param(uint16_t* dest, const char* fmt);


/* Marshalling functions. */

/** @brief Marshall an unsigned integer value.
 *  @return pointer to byte following last written byte. NULL if speci*/
void* reprint_marshall_unsigned(void* dest, uint16_t specifier, unsigned long long value);

/** @brief Marshall a signed integer value.
 *  @return pointer to byte following last written byte. */
void* reprint_marshall_signed(void* dest, uint16_t specifier, signed long long value);

/** @brief Marshall a binary floating point value.
 *  @return pointer to byte following last written byte. */
void* reprint_marshall_bin_floating_pt(void* dest, uint16_t specifier, double value);

/** @brief Marshall a pointer, including string pointers.
 *  @return pointer to byte following last written byte. */
void* reprint_marshall_pointer(void* dest, uint16_t specifier, const void* ptr);

/** @brief Marshall a single character.
 *  @return pointer to byte following last written byte. */
void* reprint_marshall_char(void* dest, uint16_t specifier, unsigned code_point);

/** @brief Marshall parameters from va list into packed form.
 *  @param dest Where to store packed parameters
 *  @param dest_len available space.
 *  @param fmt Format string containing specifiers
 *  @param ap Parameters from stack
 *  @return Pointer to first byte after last packed;
 *  if NULL, then bad specifier or insufficient space.*/
void* reprint_pack_va(void* dest, unsigned dest_len, const char* fmt, va_list ap);

/** @brief Marshall specified parameters into packed form.
 *  @param dest Where to store packed parameters
 *  @param dest_len available space.
 *  @param fmt Format string containing specifiers
 *  @return Pointer to first byte after last packed;
 *  if NULL, then bad specifier or insufficient space.*/
void* reprint_pack(void* dest, unsigned dest_len, const char* fmt, ...);

int reprint_buff(reprint_state* rs, uint8_t* dest, unsigned dest_length);

#endif
