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

#define REPRINTF_REGISTER_COUNT 8

/** @brief reprint_uint_t is the maximum signed integer type size
 * supported by the reprint implementation.
 *
 * This may be smaller than the * maximum size supported by the platform
 * (for efficiency reasons). */
typedef struct reprint_state_s {
	/** @brief Format string. Points to current character. */
	const uint8_t* fmt;

	/** @brief Packed data for reprint */
	const void* data;

	/** @brief Where to jump into the reprint_cb function.
	 * Only valid if nonzero. */
	const void* cur_label;

	union {
		const uint8_t* text;
		reprint_uint_t binary;
	} cur_data;

	reprint_reg_t registers[REPRINTF_REGISTER_COUNT];

	uint16_t mini_regs;

	uint8_t reg_flags;

	uint8_t input_flags;

} reprint_state;

/** @brief  */
void reprint_init(reprint_state* rs, const char* fmt, const void* data
	,uint8_t struct_pack);

int reprint_cb(reprint_state* rs, uint8_t* dest);

#endif
