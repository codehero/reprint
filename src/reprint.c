/* reprint.c
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
#include "reprint.h"
#include "arch_internal.h"

#ifdef NO_ASSERT
#define assert(x)
#else
#include <assert.h>
#endif

#include <math.h>
#include <stdlib.h>

#ifdef CONFIG_USE_ENDIAN_H
#define __USE_BSD
#include <endian.h>
#endif



/* These enums map selectors, flags, and register IDs to
 * bit masks and such.*/
enum {
	/* Data is tightly packed with no struct padding. */
	FLAG_REG_TIGHT_PACK = IFLAG_RES_B7

	/* Common formatted flags, registers */
	,F_REG_FIELD_WIDTH = 0
	,F_REG_PAD_CHAR = 1
	,F_FLAG_RIGHT_ALIGN = FLAG_0

	/* Common Formatted Quantity selectors, flags, registers */

	,FQS_S_RADIX_DEFINED = FLAG_SELECTOR_1_DEFINED
	,FQS_FLAG_EXPONENTIAL = FLAG_2
	,FQS_REG_SHIFT = 3
	,FQS_REG_SIGFIGS = 4
	,FQS_REG_PRECISION = 5

	/* The SHIFT value is only needed at specification time.
	 * Overwrite the value with the zero count at print time. */
	,_FQW_REG_ZEROS = 3
	/* The PRECISION value is only needed at specification time.
	 * Overwrite the value with the exponent value at print time. */
	,_FQW_REG_EXP = 5

	/* Writing out a number string is an alternation of 3 sequences
	 * 0 sequence
	 * S sequence (sig digits)
	 * 0 sequence
	 * There is a possible decimal point anywhere in the string.
	 * The break value indicates
	 * -when to switch sequences
	 * -or where to insert a decimal point.
	 *
	 * The MSB of the break register indicates if a break occurs
	 * in the S sequence.
	 *  */
	,_FQW_REG_BREAK = 6
	,_FQW_REG_BREAK_FLAG_SIG = 0x80
	,_FQW_REG_BREAK_MASK = 0x7F

	,_FQW_REG_PRINT_LZ = FLAG_SELECTOR_RESERVED_4000

	/* Rounding. */

	,FQ_S_ROUND_DOWN_INF = FLAG_SELECTOR_0_DEFINED
	,FQ_S_ROUND_UP_INF = FLAG_SELECTOR_0_DEFINED
		| FLAG_SELECTOR_0_VAL_0
	,FQ_S_ROUND_AWAY_ZERO = FLAG_SELECTOR_0_DEFINED
		| FLAG_SELECTOR_0_VAL_1
	,FQ_S_ROUND_TOWARD_ZERO = FLAG_SELECTOR_0_DEFINED
		| FLAG_SELECTOR_0_VAL_0 | FLAG_SELECTOR_0_VAL_1


	/* Format in decimal. */
	,FQ_S_RADIX_10 = 0x0

	/* Format in hexadecimal. */
	,FQ_S_RADIX_16 = FLAG_SELECTOR_1_DEFINED

	/* Format in octal. */
	,FQ_S_RADIX_8 = FLAG_SELECTOR_1_DEFINED
		| FLAG_SELECTOR_1_VAL_0

	/* Format in binary. */
	,FQ_S_RADIX_2 = FLAG_SELECTOR_1_DEFINED
		| FLAG_SELECTOR_1_VAL_1

	,_FQ_S_RADIX_MASK = FLAG_SELECTOR_1_DEFINED
		| FLAG_SELECTOR_1_VAL_0
		| FLAG_SELECTOR_1_VAL_1


	,_FQ_S_PREFIX_DEFINED = FLAG_SELECTOR_2_DEFINED

	/* Force display of sign '+' or '-' */
	,FQ_S_PREFIX_FORCE_SIGN = FLAG_SELECTOR_2_VAL_0

	/* Display prefix if not base10.  */
	,FQ_S_PREFIX_SHOW = FLAG_SELECTOR_2_VAL_1

	,FQB_REG_BCOUNT = 2
	,FQB_REG_BDROP = 3


	/* Common formatted character */
	,FTC_REG_REPEAT = 4

	/* Common text (regardless of formatted)*/
	,TS_REG_START = 3
	,TS_REG_LENGTH = 4

	/* Common formatted pointer */
	,FMP_FLAG_BASE_2 = FLAG_2


	/* Only print the ptr value.. */
	,FMP_S_LOAD_PRINT_ONLY = 0x0

	/* Print and load the data value. */
	,FMP_S_LOAD_PRINT_DATA = FLAG_SELECTOR_1_DEFINED

	/* Print and load the format value. */
	,FMP_S_LOAD_PRINT_FMT = FLAG_SELECTOR_1_DEFINED
		| FLAG_SELECTOR_1_VAL_0

	/* Only load the data value. */
	,FMP_S_LOAD_DATA = FLAG_SELECTOR_1_DEFINED
		| FLAG_SELECTOR_1_VAL_1

	/* Only load the format value. */
	,FMP_S_LOAD_FMT = FLAG_SELECTOR_1_DEFINED
		| FLAG_SELECTOR_1_VAL_0
		| FLAG_SELECTOR_1_VAL_1

	,FMP_S_LOAD_MASK = FLAG_SELECTOR_1_DEFINED
		| FLAG_SELECTOR_1_VAL_0
		| FLAG_SELECTOR_1_VAL_1

};
#define _FQ_FLAG_INTERNAL_HACK_MINUS_FLAG FLAG_SELECTOR_RESERVED_8000


static const uint16_t s_selector_bits[16] = {
	FLAG_SELECTOR_0_DEFINED
	,FLAG_SELECTOR_0_DEFINED | FLAG_SELECTOR_0_VAL_0
	,FLAG_SELECTOR_0_DEFINED | FLAG_SELECTOR_0_VAL_1
	,FLAG_SELECTOR_0_DEFINED | FLAG_SELECTOR_0_VAL_0 | FLAG_SELECTOR_0_VAL_1

	,FLAG_SELECTOR_1_DEFINED
	,FLAG_SELECTOR_1_DEFINED | FLAG_SELECTOR_1_VAL_0
	,FLAG_SELECTOR_1_DEFINED | FLAG_SELECTOR_1_VAL_1
	,FLAG_SELECTOR_1_DEFINED | FLAG_SELECTOR_1_VAL_1 | FLAG_SELECTOR_1_VAL_0

	,FLAG_SELECTOR_2_DEFINED
	,FLAG_SELECTOR_2_DEFINED | FLAG_SELECTOR_2_VAL_0
	,FLAG_SELECTOR_2_DEFINED | FLAG_SELECTOR_2_VAL_1
	,FLAG_SELECTOR_2_DEFINED | FLAG_SELECTOR_2_VAL_1 | FLAG_SELECTOR_2_VAL_0

	/* , */
	,FLAG_0
	/* '-' */
	,FLAG_1
	/* '.' */
	,FLAG_2
	/* '/' */
	,FLAG_3
};

uint8_t s_arch_int_amb_size[8] = {
	sizeof(char)
	,sizeof(short)
	,sizeof(int)
	,sizeof(long)
	,sizeof(long long)
	,sizeof(ptrdiff_t)
	,sizeof(intptr_t)
	,sizeof(intmax_t)
};

uint8_t s_arch_int_conc_size[32] = {
	sizeof(uint_fast8_t)
	,sizeof(uint_fast16_t)
	,sizeof(uint_fast32_t)
	,sizeof(uint_fast64_t)
	,0
	,0
	,0
	,0

	,sizeof(uint_least8_t)
	,sizeof(uint_least16_t)
	,sizeof(uint_least32_t)
	,sizeof(uint_least64_t)
	,0
	,0
	,0
	,0

	,0
	,0
	,0
	,0
	,0
	,0
	,0
	,0
};

#if 0
static inline void set_bytes(void* dest, uint8_t b, size_t size){
	for(unsigned i = 0; i < size; ++i)
		*(uint8_t*)(dest++) = b;
}

static inline void copy_bytes(void* dest, const void* src, size_t size){
	for(unsigned i = 0; i < size; ++i)
		*(uint8_t*)(dest++) = *(const uint8_t*)(src++);
}
#endif

#ifndef REPRINT_SINGLETON
void reprint_init(reprint_state* rs, const char* fmt, const void* data)
#else
static reprint_state s_rs;
void reprint_init(const char* fmt, const void* data)
#endif
{
	#ifdef REPRINT_SINGLETON
	reprint_state* rs = &s_rs;
	#endif

	memset(rs, 0, sizeof(reprint_state));
	rs->fmt = (const uint8_t*)fmt;
	rs->data = data;
}

__attribute__((__noinline__,__noclone__))
#ifndef REPRINT_SINGLETON
int reprint_cb(reprint_state* rs, uint8_t* dest, unsigned dest_len){
#else
int reprint_cb(uint8_t* dest, unsigned dest_len){
#endif

	#ifdef REPRINT_SINGLETON
	reprint_state* rs = &s_rs;
	#endif


BEGIN:
	assert(dest);

	/* If processing an input specifier then handle that. */
	/* rs->pc is only assigned labels with ST_ prefixed. */
	if(rs->pc){

		/* Decrement field width if non-zero */
		if(rs->selectors & FLAG_FORMAT_BIT){
			if(rs->registers[F_REG_FIELD_WIDTH]){
				--rs->registers[F_REG_FIELD_WIDTH];

				/* If padding is on left, then write that out. */
				if(rs->selectors & F_FLAG_RIGHT_ALIGN){
					*dest = rs->registers[F_REG_PAD_CHAR];
					return 1;
				}
			}
		}

		goto *rs->pc;
	}

	{
		const uint8_t* i = rs->fmt;
		/* If fmt char is null terminator, just return. */
		if(*i == '\0')
			return 0;

		/* Check if this is a field header. */
		uint8_t* tmp = dest;
		const uint8_t* end = dest + dest_len;
		while(tmp != end && *i && ESCAPE_MASK != (*i & ~ESCAPE_SELECT)){
			/* Copy character to output and advance format state. */
			*tmp = *i;
			++i;
			++rs->fmt;
			++tmp;
		}

		/* If wrote some string data then just return. */
		if(tmp != dest)
			return tmp - dest;

		/* Check for double \b\b or \f\f, which means just 
		 * send that char. */
		if(*i == *(i + 1)){
			*dest = *i;
			rs->fmt += 2;
			return 1;
		}

		/* Zero the state. Don't clear registers that are already set. */
		rs->selectors = 0;
		{
			/* Invert the set registers. */
			uint_fast8_t f = ~rs->reg_flags & 0x7F;
			reprint_reg_t* r = rs->registers;
			while(f){
				if(f & 1)
					*r = 0;
				++r;
				f >>= 1;
			}
		}


		if(*i & ESCAPE_SELECT){
			rs->selectors |= FLAG_FORMAT_BIT;

			/* Default pad char is a ' '.*/
			rs->registers[F_REG_PAD_CHAR] = ' ';
		}

		/* Go onto next character. */
		++i;

		/* Check for packing change directive. */
		if(PACK_MASK == (*i & ~PACK_SELECT)){
			rs->input_flags &= ~FLAG_REG_TIGHT_PACK;
			rs->input_flags |= (*i & PACK_SELECT) << 5;
			++i;
		}

		/* Parse modifiers. */
		{
			/* Do not write to output in this scope of code. */
			const uint8_t* dest = NULL;

			reprint_reg_t reg_value = 0;
			/* Otherwise, found an escape character. Parse the field. */
			while(*i < 0x40){
				if(*i < 0x20)
					assert(0);

				if(*i < 0x30){
					/* In the 0x20 column. */
					rs->selectors |= s_selector_bits[*i & 0xF];
				}
				else{
					/* If user specified value in fmt string, set this flag. */
					uint_fast8_t user_flag = 0;
					if(*i <= 0x39){
						user_flag = 1;
						/* If leading zero with digit, then octal. */
						if(*i == 0x30){
							++i;

							/* Parse octal value. If no digits, then value is just zero.
							 * 8 and 9 are not expected; then will just wrap around...
							 * */
							while(*i <= 0x39){
								/* Should not mix selectors and flags inside a reg value. */
								assert(*i >= 0x30);

								reg_value <<= 3;
								reg_value += (*i & 0x7);
								++i;
							}
						}
						else{
							/* Parse decimal value into reg_value. */
							while(*i <= 0x39){
								/* Should not mix selectors and flags inside a reg value. */
								assert(*i >= 0x30);
								reg_value *= 10;
								reg_value += (*i & 0xF);

								++i;
							}
						}
					}
					else{
						/* Registers  without a preceding numeric value are to have 
						 * their values loaded from the data. */
						/* FIXME use packing rules as per FLAG_REG_TIGHT_PACK  */
						if(*i < 0x40){
							reprint_reg_t* u = (reprint_reg_t*)(rs->data);
							reg_value = *u;
							rs->data += sizeof(*u);
						}
					}

					if(*i >= 0x40){
						/* Copy register value and break to specifier section. */
						if(user_flag){
							rs->reg_flags |= FLAG_REG_0_DEFINED;
							rs->registers[0] = reg_value;
						}
						break;
					}
					else if(*i >= 0x3A){
						unsigned id = *i - 0x39;
						rs->reg_flags |= 1 << id;
						rs->registers[id] = reg_value;
						reg_value = 0;
					}
				}

				++i;
			}
		}

		/* Parse input specifier flags.
		 * Maintain the packing type. */
		rs->input_flags &= FLAG_REG_TIGHT_PACK;
		while(*i < 0x70){
			/* This is an error. */
			if(*i < 0x60)
				assert(0);

			if(*i < 0x68){
				rs->input_flags |= (*i & 0x7);
			}
			else if(*i < 0x6C){
				rs->input_flags |= (*i & 0x3) << 3;
			}
			else{
				rs->input_flags |= (*i & 0x3) << 5;
			}
			++i;
		}

		/* Catch up to i. */
		rs->fmt = i;
	}

	/* Load data according to specifier. */
	{
		/* Track total length of field (if necessary) */
		unsigned total_len = 0;

		/* Do not write to output in this scope of code. */
		const uint8_t* dest = NULL;

		/* First branch on Integer or not. */
		if(!(*rs->fmt & SFLAG_SPECIAL)){
			if(!(rs->input_flags & IFLAG_NONINTEG)){
				/* Yes, I did just do that. Go look at the included .c file. */
#define REPRINT_GUARD_reprint_cb_QUANTITY_SPECIFIER
#include "reprint_cb_quantity_specifier.c"
#undef REPRINT_GUARD_reprint_cb_QUANTITY_SPECIFIER
			}
			else{
				/* This is a non-integer value. */

				if((rs->input_flags & IFLAG_TMASK) == IFLAG_FLOAT){
				/* Yes, I did just do that. Go look at the included .c file. */
#define REPRINT_GUARD_reprint_cb_FLOAT_SPECIFIER
#include "reprint_cb_float_specifier.c"
#undef REPRINT_GUARD_reprint_cb_FLOAT_SPECIFIER
				}
				else{
					/* This is a string or char. */
					if((rs->input_flags & IFLAG_TMASK) == IFLAG_CHAR){
						/* Assuming 8-bit character.
						 * TODO support wchar_t and unicode point ids.
						 * On some platforms (Linux et al) these will be the same types
						 * On Windows char_t is 16 bit. */

						rs->cur_data.binary = *(uint8_t*)(rs->data);
						++rs->data;

						/* Increment the repeat register.
						 * Initialized to zero with user intervention. */
						++rs->registers[FTC_REG_REPEAT];
						total_len = rs->registers[FTC_REG_REPEAT];
						rs->pc = &&ST_CHAR;
					}
					else{
						/* This is a string. */

						/* Align to ptr and assign rs->cur_data.text. */
						if(FLAG_REG_TIGHT_PACK & rs->input_flags){
							memcpy(&rs->cur_data.text, rs->data, sizeof(rs->cur_data.text));
						}
						else{
							rs->data = s_arch_align_ptr(rs->data, sizeof(const uint8_t*));
							rs->cur_data.text = (const uint8_t*)(rs->data);
						}
						rs->data += sizeof(rs->cur_data.text);

						/* Trying to avoid strlen call if at all possible. */

						/* If start range is specified, add to the start pointer.
						 * ASSUME user did not put start past actual string length!! */
						if(rs->reg_flags & (1 << TS_REG_START)){
							rs->cur_data.text += rs->registers[TS_REG_START];
						}

						/* Trust the user on its length.
						 * TODO maybe do strlen to double check??*/
						if(rs->reg_flags & (1 << TS_REG_LENGTH)){
							total_len = rs->registers[TS_REG_LENGTH];
						}

						if(rs->selectors & FLAG_FORMAT_BIT){
							if(rs->registers[F_REG_FIELD_WIDTH] && !total_len){
								/* Calculate string length, determine if it exceeds padding.
								 * Assuming UTF-8 string for now.
								 * TODO support char16_t, char32_t, wchar_t. */
								total_len = strlen((char*)rs->cur_data.text);
							}
						}

						rs->pc = &&ST_TEXT;
					}
				}
			}
		}
		else{
			if(*(rs->fmt) == SPECIFIER_POINTER){
				/* TODO */
				assert(0);
			}
			else if(*(rs->fmt) == SPECIFIER_RECURSE){
				/* TODO */
				assert(0);
			}
			else{
				assert(0);
			}
		}

		/* If total length exceeds the fieldwidth then zero fieldwidth;
		 * otherwise subtract the char count from fieldwidth. */
		if(rs->selectors & FLAG_FORMAT_BIT){
			if(rs->selectors & F_FLAG_RIGHT_ALIGN){
				if(total_len < rs->registers[F_REG_FIELD_WIDTH]){
					rs->registers[F_REG_FIELD_WIDTH] -= total_len;
					--rs->registers[F_REG_FIELD_WIDTH];
					goto ST_WRITE_CHAR;
				}
				else{
					rs->registers[F_REG_FIELD_WIDTH] = 0;
				}
			}
		}

		/* Dispatch. */
		goto *rs->pc;
	}

#define REPRINT_GUARD_reprint_cb_QUANTITY
#include "reprint_cb_quantity.c"
#undef REPRINT_GUARD_reprint_cb_QUANTITY

ST_TEXT:
	if(!*rs->cur_data.text)
		goto ST_FIELD_DONE;
	if(rs->reg_flags & (1 << TS_REG_LENGTH)){
		if(0 == rs->registers[TS_REG_LENGTH])
			goto ST_FIELD_DONE;
		--rs->registers[TS_REG_LENGTH];
	}

	*dest = *rs->cur_data.text;
	++rs->cur_data.text;
	return 1;

ST_CHAR:
	if(!rs->registers[FTC_REG_REPEAT])
		goto ST_FIELD_DONE;
	*dest = rs->cur_data.binary;
	--rs->registers[FTC_REG_REPEAT];
	return 1;

ST_FIELD_DONE:
	{
		rs->pc = &&ST_WRITE_PAD;
ST_WRITE_PAD:
		if(rs->selectors & FLAG_FORMAT_BIT){
			if(rs->registers[F_REG_FIELD_WIDTH]){
	ST_WRITE_CHAR:
				*dest = rs->registers[F_REG_PAD_CHAR];
				return 1;
			}

			/* unset field width and pad char flags. */
			rs->reg_flags &= ~((1 << F_REG_FIELD_WIDTH) | (1 << F_REG_PAD_CHAR));
		}
	}

	/* Finished outputting the field!! */
	++rs->fmt;
	rs->pc = NULL;
	goto BEGIN;
}
