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


/* TODO make this fit in 64 bytes (only occupy 1 cache line) */
typedef struct reprint_state_s {
	/** @brief Format string. Points to current character. */
	const uint8_t* fmt;

	/** @brief Data for reprint; packing is defined by flags */
	const void* data;

	/** @brief "Program Counter" Where to jump back into the reprint_cb function.
	 * If zero then not processing an input specifier. */
	const void* pc;

	union {
		const uint8_t* text;
		reprint_uint_t binary;
	} cur_data;

	reprint_reg_t registers[REPRINTF_REGISTER_COUNT];

	uint16_t selectors;

	uint8_t reg_flags;

	uint8_t input_flags;

} reprint_state;

#ifndef REPRINT_SINGLETON
/** @brief Initialize a reprint state context */
void reprint_init(reprint_state* rs, const char* fmt, const void* data);

/** @brief Print some data to the destination buffer.
 *  @param rs Reprint state to convert
 *  @param dest Where to write output data
 *  @param dest_len Maximum number of bytes that can be written
 *  @return 0 if finished outputting, if positive then number of characters written
 *  If negative then a runtime (not assertable logic error) occurred. */
int reprint_cb(reprint_state* rs, uint8_t* dest, unsigned dest_len);

#else

/* These variants are appropriate for very small embedded systems. */
void reprint_init(const char* fmt, const void* data);
int reprint_cb(uint8_t* dest, unsigned dest_len);

#endif

#define PACK_MASK 0x09
#define PACK_SELECT 0x04

#define ESCAPE_MASK 0x08
#define ESCAPE_SELECT 0x04
#define REPRINTF_BCD_BUFF_SIZE 10

/** @brief Input type is a pointer. */
#define SPECIFIER_POINTER        0x78
#define SPECIFIER_RECURSE        0x79

/** @brief Input type is defined and handled by reprint implementation. */
#define SPECIFIER_IMPLEMENTATION 0x7E

/** @brief Input type is defined and handled by user. */
#define SPECIFIER_USER           0x7F

/* These enums identify bits in the registers. */
enum {

	/* Flag/Selector word. */
	FLAG_0 = 0x0001
	,FLAG_1 = 0x0002
	,FLAG_2 = 0x0004
	,FLAG_3 = 0x0008

	,FLAG_SELECTOR_0_DEFINED = 0x0010
	,FLAG_SELECTOR_0_VAL_0 = 0x0020
	,FLAG_SELECTOR_0_VAL_1 = 0x0040

	,FLAG_SELECTOR_1_DEFINED = 0x0080
	,FLAG_SELECTOR_1_VAL_0 = 0x0100
	,FLAG_SELECTOR_1_VAL_1 = 0x0200

	,FLAG_SELECTOR_2_DEFINED = 0x0400
	,FLAG_SELECTOR_2_VAL_0 = 0x0800
	,FLAG_SELECTOR_2_VAL_1 = 0x1000

	,FLAG_FORMAT_BIT       = 0x2000
	,FLAG_SELECTOR_RESERVED_4000 = 0x4000

/* Avoid int enum warning. */
#define FLAG_SELECTOR_RESERVED_8000  0x8000


	/** @brief Flag 1 is the '-' character.
	 * It's special in that if it immediately precedes a register value then
	 * the FLAG_REG_MINUS_X is set, but does not affect FLAG_1. */

	/* Register flags byte (fits into 8-bit value).
	 * Whether register was supplied by the user. */
	,FLAG_REG_0_DEFINED = 0x01
	,FLAG_REG_1_DEFINED = 0x02
	,FLAG_REG_2_DEFINED = 0x04
	,FLAG_REG_3_DEFINED = 0x08
	,FLAG_REG_4_DEFINED = 0x10
	,FLAG_REG_5_DEFINED = 0x20
	,FLAG_REG_6_DEFINED = 0x40
	,FLAG_REG_RESERVED_80 = 0x80

	/* Input flags */
	,IFLAG_TYPE_B0 = 0x01
	,IFLAG_TYPE_B1 = 0x02
	,IFLAG_TYPE_B2 = 0x04
	,IFLAG_AUX0_B0 = 0x08
	,IFLAG_AUX0_B1 = 0x10
	,IFLAG_AUX1_B0 = 0x20
	,IFLAG_AUX1_B1 = 0x40
	,IFLAG_RES_B7  = 0x80

	,IFLAG_UNSIGNED = IFLAG_TYPE_B0
	,IFLAG_CONCRETE = IFLAG_TYPE_B1
	,IFLAG_NONINTEG = IFLAG_TYPE_B2

	,IFLAG_STRING = IFLAG_NONINTEG | 0x0
	,IFLAG_CHAR   = IFLAG_NONINTEG | 0x1
	,IFLAG_FLOAT  = IFLAG_NONINTEG | 0x2

	,IFLAG_TMASK  = IFLAG_TYPE_B2 | IFLAG_TYPE_B1 | IFLAG_TYPE_B0

	/* Specifier flags */
	,SFLAG_SPECIAL = 0x08

	/* Unsigned concrete bitfield */
	,SFLAG_UC_BITFIELD = 0x07

	,SFLAG_SIZE_MASK = 0x07
};

extern uint8_t s_arch_int_amb_size[8];
extern uint8_t s_arch_int_conc_size[32];

#ifdef ARCH_SPECIFIER_IMPLEMENTATION
#include "reprint_cb_implementation_specifier.h"
#endif

#endif
