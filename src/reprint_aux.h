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

#include "reprint.h"

/** @brief Find next specifier field.
 *  @param dest Where to store specifier parameters.
 *  @param fmt format string
 *  @return NULL if no specifier found;
 *  pointer to first char succeeding specifier otherwise. */
const char* reprint_get_specifier(uint16_t* dest, const char* fmt);


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

#endif
