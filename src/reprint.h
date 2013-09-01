/* reprint.h
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
#ifndef __REPRINT_H__
#define __REPRINT_H__

#include <stdint.h>
#include <stddef.h>

#include "arch.h"

/** @brief reprint_reg_t  */

/** @brief reprint_uint_t is the maximum signed integer type size
 * supported by the reprint implementation.
 *
 * This may be smaller than the * maximum size supported by the platform
 * (for efficiency reasons). */

/** @brief reprint_uint_t is the maximum integer type size
 * supported by the reprint implementation.  */

/** @brief Init, but pack with no gaps between each datum.
 *  @return Pointer to first byte following last packed byte. */
void* reprint_init_pack(const char* fmt, void* pack, ...);

/** @brief  */
void reprint_init(const char* fmt, const void* data, uint8_t struct_pack);

int reprint_cb(char* dest);

#endif
