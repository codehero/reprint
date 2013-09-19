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

#ifdef CONFIG_USE_ENDIAN_H
#define __USE_BSD
#include <endian.h>
#endif

#define ESCAPE_MASK 0x08
#define ESCAPE_SELECT 0x04
#define REPRINTF_BCD_BUFF_SIZE 10
#define REPRINTF_REGISTER_COUNT 7

#define Q_BITFIELD 0x79
#define Q_POINTER 0x78

/* These enums identify bits in the registers. */
enum {

	/* Flag/Mini Register word. */
	FLAG_0 = 0x0001
	,FLAG_1 = 0x0002
	,FLAG_2 = 0x0004
	,FLAG_3 = 0x0008

	,FLAG_MINI_REG_0_DEFINED = 0x0010
	,FLAG_MINI_REG_0_VAL_0 = 0x0020
	,FLAG_MINI_REG_0_VAL_1 = 0x0040

	,FLAG_MINI_REG_1_DEFINED = 0x0080
	,FLAG_MINI_REG_1_VAL_0 = 0x0100
	,FLAG_MINI_REG_1_VAL_1 = 0x0200

	,FLAG_MINI_REG_2_DEFINED = 0x0400
	,FLAG_MINI_REG_2_VAL_0 = 0x0800
	,FLAG_MINI_REG_2_VAL_1 = 0x1000

	,FLAG_MINI_REG_RESERVED_2000 = 0x2000
	,FLAG_MINI_REG_RESERVED_4000 = 0x4000

/* Avoid int enum warning. */
#define FLAG_MINI_REG_RESERVED_8000  0x8000


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
};

/* These enums map mini registers, flags, and register IDs to
 * bit masks and such.*/
enum {
	/* Data is packed as struct. */
	FLAG_REG_STRUCT_PACK = FLAG_REG_RESERVED_80,

	/* Whether user specified formatting '\f' vs '\b' */
	FORMAT_BIT = FLAG_MINI_REG_RESERVED_4000

	/* Common formatted flags, registers */
	,FS_REG_FIELD_WIDTH = 0
	,FS_REG_PAD_CHAR = 1
	,FS_FLAG_RIGHT_ALIGN = FLAG_0

	/* Common Formatted Quantity miniregs, flags, registers */

	,FQS_MR_RADIX_DEFINED = FLAG_MINI_REG_1_DEFINED
	,FQS_FLAG_EXPONENTIAL = FLAG_2
	,FQS_REG_SHIFT = 3
	,FQS_REG_SIGFIGS = 4
	,FQS_REG_PRECISION = 5

	/* The SHIFT value is only needed at specification time.
	 * Overwrite the value with the zero count at print time. */
	,FQW_REG_ZEROS = 3
	/* The PRECISION value is only needed at specification time.
	 * Overwrite the value with the exponent value at print time. */
	,FQW_REG_EXP = 5

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
	,FQW_REG_BREAK = 6
	,FQW_REG_BREAK_FLAG_SIG = 0x80
	,FQW_REG_BREAK_MASK = 0x7F

	,FQW_REG_PRINT_LZ = FLAG_MINI_REG_RESERVED_2000

	/* Rounding. */

	,FQ_MR_ROUND_AWAY_INF = FLAG_MINI_REG_0_DEFINED
	,FQ_MR_ROUND_TOWARD_INF = FLAG_MINI_REG_0_DEFINED
		| FLAG_MINI_REG_0_VAL_0
	,FQ_MR_ROUND_AWAY_ZERO = FLAG_MINI_REG_0_DEFINED
		| FLAG_MINI_REG_0_VAL_1
	,FQ_MR_ROUND_TOWARD_ZERO = FLAG_MINI_REG_0_DEFINED
		| FLAG_MINI_REG_0_VAL_0 | FLAG_MINI_REG_0_VAL_1


	/* Format in decimal. */
	,FQ_MR_RADIX_10 = 0x0

	/* Format in hexadecimal. */
	,FQ_MR_RADIX_16 = FLAG_MINI_REG_1_DEFINED

	/* Format in octal. */
	,FQ_MR_RADIX_8 = FLAG_MINI_REG_1_DEFINED
		| FLAG_MINI_REG_1_VAL_0

	/* Format in binary. */
	,FQ_MR_RADIX_2 = FLAG_MINI_REG_1_DEFINED
		| FLAG_MINI_REG_1_VAL_1

	,FQ_MR_RADIX_MASK = FLAG_MINI_REG_1_DEFINED
		| FLAG_MINI_REG_1_VAL_0
		| FLAG_MINI_REG_1_VAL_1


	,FQ_MR_PREFIX_DEFINED = FLAG_MINI_REG_2_DEFINED

	/* Force display of sign '+' or '-' */
	,FQ_MR_PREFIX_FORCE_SIGN = FLAG_MINI_REG_2_VAL_0

	/* Display prefix if not base10.  */
	,FQ_MR_PREFIX_SHOW = FLAG_MINI_REG_2_VAL_1

	,FQB_REG_BCOUNT = 2
	,FQB_REG_BDROP = 3


	/* Common formatted character */
	,FTC_REG_REPEAT = 3

	/* Common text (regardless of formatted)*/
	,TS_REG_START = 3
	,TS_REG_LENGTH = 4

	/* Common formatted pointer */
	,FMP_FLAG_BASE_2 = FLAG_2


	/* Only print the ptr value.. */
	,FMP_MR_LOAD_PRINT_ONLY = 0x0

	/* Print and load the data value. */
	,FMP_MR_LOAD_PRINT_DATA = FLAG_MINI_REG_1_DEFINED

	/* Print and load the format value. */
	,FMP_MR_LOAD_PRINT_FMT = FLAG_MINI_REG_1_DEFINED
		| FLAG_MINI_REG_1_VAL_0

	/* Only load the data value. */
	,FMP_MR_LOAD_DATA = FLAG_MINI_REG_1_DEFINED
		| FLAG_MINI_REG_1_VAL_1

	/* Only load the format value. */
	,FMP_MR_LOAD_FMT = FLAG_MINI_REG_1_DEFINED
		| FLAG_MINI_REG_1_VAL_0
		| FLAG_MINI_REG_1_VAL_1

	,FMP_MR_LOAD_MASK = FLAG_MINI_REG_1_DEFINED
		| FLAG_MINI_REG_1_VAL_0
		| FLAG_MINI_REG_1_VAL_1

};
#define INTERNAL_HACK_MINUS_FLAG FLAG_MINI_REG_RESERVED_8000


typedef union {
	const uint8_t* text;
	reprint_uint_t binary;
} reprint_variant_t;

/* On MSP430, this is
 * 2 + 2 + 2 + 2
 * 7
 * + 2 + 1
 *
 * 20 bytes of RAM consumed
 *
 * */
typedef struct {
	/** @brief Format string. Points to current character. */
	const uint8_t* fmt;

	/** @brief Packed data for reprint */
	const void* data;

	/** @brief Where to jump into the reprint_cb function.
	 * Only valid if nonzero. */
	const void* cur_label;

	reprint_variant_t cur_data;

	uint16_t mini_regs;

	reprint_reg_t registers[REPRINTF_REGISTER_COUNT];

	uint8_t reg_flags;

	uint8_t input_flags;

} reprint_state;

static const uint16_t s_mini_reg_bits[16] = {
	FLAG_MINI_REG_0_DEFINED
	,FLAG_MINI_REG_0_DEFINED | FLAG_MINI_REG_0_VAL_0
	,FLAG_MINI_REG_0_DEFINED | FLAG_MINI_REG_0_VAL_1
	,FLAG_MINI_REG_0_DEFINED | FLAG_MINI_REG_0_VAL_0 | FLAG_MINI_REG_0_VAL_1

	,FLAG_MINI_REG_1_DEFINED
	,FLAG_MINI_REG_1_DEFINED | FLAG_MINI_REG_1_VAL_0
	,FLAG_MINI_REG_1_DEFINED | FLAG_MINI_REG_1_VAL_1
	,FLAG_MINI_REG_1_DEFINED | FLAG_MINI_REG_1_VAL_1 | FLAG_MINI_REG_1_VAL_0

	,FLAG_MINI_REG_2_DEFINED
	,FLAG_MINI_REG_2_DEFINED | FLAG_MINI_REG_2_VAL_0
	,FLAG_MINI_REG_2_DEFINED | FLAG_MINI_REG_2_VAL_1
	,FLAG_MINI_REG_2_DEFINED | FLAG_MINI_REG_2_VAL_1 | FLAG_MINI_REG_2_VAL_0

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

uint8_t s_arch_int_conc_size[24] = {
	1
	,2
	,4
	,8
	,16
	,0
	,0
	,0

	,sizeof(uint_fast8_t)
	,sizeof(uint_fast16_t)
	,sizeof(uint_fast32_t)
	,sizeof(uint_fast64_t)
	,16
	,0
	,0
	,0

	,sizeof(uint_least8_t)
	,sizeof(uint_least16_t)
	,sizeof(uint_least32_t)
	,sizeof(uint_least64_t)
	,16
	,0
	,0
	,0
};

static reprint_state s_rs;


static inline void set_bytes(void* dest, uint8_t b, size_t size){
	for(unsigned i = 0; i < size; ++i)
		*(uint8_t*)(dest++) = b;
}

static inline void copy_bytes(void* dest, const void* src, size_t size){
	for(unsigned i = 0; i < size; ++i)
		*(uint8_t*)(dest++) = *(const uint8_t*)(src++);
}

void reprint_init(const char* fmt, const void* data, uint8_t struct_pack){
	set_bytes(&s_rs, 0, sizeof(s_rs));
	s_rs.fmt = (const uint8_t*)fmt;
	s_rs.data = data;
	if(struct_pack)
		s_rs.reg_flags |= FLAG_REG_RESERVED_80;
}

__attribute__((__noinline__,__noclone__))
int reprint_cb(char* dest){
BEGIN:
	assert(dest);
	reprint_state* rs = &s_rs;


	/* If processing an input specifier then handle that. */
	if(rs->cur_label){

		/* Decrement field width if non-zero */
		if(rs->mini_regs & FORMAT_BIT){
			if(rs->registers[FS_REG_FIELD_WIDTH]){
				--rs->registers[FS_REG_FIELD_WIDTH];

				/* If padding is on left, then write that out. */
				if(rs->mini_regs & FS_FLAG_RIGHT_ALIGN){
					*dest = rs->registers[FS_REG_PAD_CHAR];
					return 1;
				}
			}
		}

		goto *rs->cur_label;
	}

	{
		const uint8_t* i = rs->fmt;
		/* If fmt char is null terminator, just return. */
		if(*i == '\0')
			return 0;

		/* Check if this is a field header. */
		if(ESCAPE_MASK != (*i & ~ESCAPE_SELECT)){
			/* Copy character to output and advance format state. */
			*dest = *i;
			++rs->fmt;
			return 1;
		}

		/* Check for double \b\b or \f\f, which means just 
		 * send that char. */
		if(*i == *(i + 1)){
			*dest = *i;
			rs->fmt += 2;
			return 1;
		}

		/* Zero the state. Don't clear registers that are already set. */
		rs->mini_regs = 0;
		{
			uint8_t f = rs->reg_flags;
			for(unsigned k = 0; k < REPRINTF_REGISTER_COUNT; ++k){
				if(!(f & 1))
					rs->registers[k] = 0;
				f >>= 1;
			}
		}


		if(*i & ESCAPE_SELECT){
			rs->mini_regs |= FORMAT_BIT;

			/* Default pad char is a ' '.*/
			rs->registers[FS_REG_PAD_CHAR] = ' ';
		}

		/* Go onto next character. */
		++i;

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
					rs->mini_regs |= s_mini_reg_bits[*i & 0xF];
				}
				else{
					/* FIXME: this does not handle the ':' and ';' characters
					 * (which are so far undefined...) */

					/* If user specified value in fmt string, set this flag. */
					uint8_t user_flag = 0;
					if(*i <= 0x39){
						user_flag = 1;
						/* If leading zero with digit, then octal. */
						if(*i == 0x30){
							++i;

							/* Parse octal value. If no digits, then value is just zero.
							 * 8 and 9 are not expected; then will just wrap around...
							 * */
							while(*i <= 0x39){
								/* Should not mix mini-registers and flags inside a reg value. */
								assert(*i >= 0x30);

								reg_value <<= 3;
								reg_value += (*i & 0x7);
								++i;
							}
						}
						else{
							/* Parse decimal value into reg_value. */
							while(*i <= 0x39){
								/* Should not mix mini-registers and flags inside a reg value. */
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

		/* Parse input specifier flags. */
		rs->input_flags = 0;
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
		if(*(rs->fmt) < 0x78){
			if(!(rs->input_flags & 0x4)){
				/* Yes, I did just do that. Go look at the included .c file. */
#define REPRINT_GUARD_reprint_cb_QUANTITY_SPECIFIER
#include "reprint_cb_quantity_specifier.c"
#undef REPRINT_GUARD_reprint_cb_QUANTITY_SPECIFIER
			}
			else{
				/* This is a non-integer value. */

				if((rs->input_flags & 0x7) == 0x6){
					/* TODO This is a float. */
					assert(0);
				}
				else{
					/* This is a string or char. */
					if((rs->input_flags & 0x7) == 0x5){
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
						rs->cur_label = &&CHAR;
					}
					else{
						/* This is a string. */

						/* Align to ptr and assign rs->cur_data.text. */
						if(FLAG_REG_STRUCT_PACK & rs->reg_flags){
							rs->data = s_arch_align_ptr(rs->data, sizeof(const uint8_t*));
							rs->cur_data.text = (const uint8_t*)(rs->data);
						}
						else{
							copy_bytes(&rs->cur_data.text, rs->data, sizeof(rs->cur_data.text));
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

						if(rs->mini_regs & FORMAT_BIT){
							if(rs->registers[FS_REG_FIELD_WIDTH] && !total_len){
								/* Calculate string length, determine if it exceeds padding.
								 * Assuming UTF-8 string for now.
								 * TODO support char16_t, char32_t, wchar_t. */
								total_len = strlen((char*)rs->cur_data.text);
							}
						}

						rs->cur_label = &&TEXT;
					}
				}
			}
		}
		else{
			/* This is a special specifier. */
			if(*(rs->fmt) == Q_BITFIELD){
				/* Outputting bit field. First drop bits if necessary. */
				if(rs->reg_flags & (1 << FQB_REG_BDROP)){
					/* If dropping more bits than we have, this is an error. */
					if(rs->registers[FQS_REG_SIGFIGS] < rs->registers[FQB_REG_BDROP]){
						assert(0);
					}
					rs->registers[FQS_REG_SIGFIGS] -= rs->registers[FQB_REG_BDROP];
					reprint_uint_t mask = (1 << rs->registers[FQS_REG_SIGFIGS]) - 1;
					rs->cur_data.binary &= mask;
				}

				/* Now set the break. */
				rs->registers[FQW_REG_BREAK] = rs->registers[FQS_REG_SIGFIGS];

				/* Output just like an integer now. */
				rs->cur_label = &&QUANT_CHECK_PREFIX;
			}
			else if(*(rs->fmt) == Q_POINTER){
				/* TODO */
				assert(0);
			}
			else{
				assert(0);
			}
		}

		/* If total length exceeds the fieldwidth then zero fieldwidth;
		 * otherwise subtract the char count from fieldwidth. */
		if(rs->mini_regs & FORMAT_BIT){
			if(rs->mini_regs & FS_FLAG_RIGHT_ALIGN){
				if(total_len < rs->registers[FS_REG_FIELD_WIDTH]){
					rs->registers[FS_REG_FIELD_WIDTH] -= total_len;
					--rs->registers[FS_REG_FIELD_WIDTH];
					goto WRITE_CHAR;
				}
				else{
					rs->registers[FS_REG_FIELD_WIDTH] = 0;
				}
			}
		}

		/* Dispatch. */
		goto *rs->cur_label;
	}

#define REPRINT_GUARD_reprint_cb_QUANTITY
#include "reprint_cb_quantity.c"
#undef REPRINT_GUARD_reprint_cb_QUANTITY

TEXT:
	if(!*rs->cur_data.text)
		goto FIELD_DONE;
	if(rs->reg_flags & (1 << TS_REG_LENGTH)){
		if(0 == rs->registers[TS_REG_LENGTH])
			goto FIELD_DONE;
		--rs->registers[TS_REG_LENGTH];
	}

	*dest = *rs->cur_data.text;
	++rs->cur_data.text;
	return 1;

CHAR:
	if(!rs->registers[FTC_REG_REPEAT])
		goto FIELD_DONE;
	*dest = rs->cur_data.binary;
	--rs->registers[FTC_REG_REPEAT];
	return 1;

	if(0){
MISC:
		return 0;
	}

FIELD_DONE:
	{
		rs->cur_label = &&WRITE_PAD;
WRITE_PAD:
		if(rs->mini_regs & FORMAT_BIT){
			if(rs->registers[FS_REG_FIELD_WIDTH]){
	WRITE_CHAR:
				*dest = rs->registers[FS_REG_PAD_CHAR];
				return 1;
			}

			/* unset field width and pad char flags. */
			rs->reg_flags &= ~((1 << FS_REG_FIELD_WIDTH) | (1 << FS_REG_PAD_CHAR));
		}
	}

	/* Finished outputting the field!! */
	++rs->fmt;
	rs->cur_label = NULL;
	goto BEGIN;
}
