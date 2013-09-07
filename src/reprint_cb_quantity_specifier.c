/* reprint_cb_specifier.c
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


/* This C file only has context within reprint.c:reprint_cb()
 * The inherited variables from the reprint_cb() context are
 *
 * const char *i;       (the specifier value)
 * unsigned total_len;  (the number of characters required to print value)
 *
 * */
#ifdef REPRINT_GUARD_reprint_cb_QUANTITY_SPECIFIER
/* Branch on integer vs noninteger. */
uint8_t num = *i;
if((num & 0xF0) == 0x40){
	if((num & 0x7) >= 0x5){
		/* This is an arch dependent int param. Translate to the proper size. */
		if((num & 0xF) > 8)
			num -= 0xD;
		else
			num -= 0x5;
		
		num = s_arch_translate[num];
	}
}
else if(num == 0x58){
	num = s_arch_translate[6];
}

if(!(num & 0x10)){

	{
		const unsigned int_size = 1 << (num & 0x7);

		/* If using struct packing, then align the pointer to the datatype. */
		if(rs->reg_flags & FLAG_REG_STRUCT_PACK)
			rs->data = s_arch_align_ptr(rs->data, int_size);

		copy_bytes(&rs->cur_data.binary, rs->data, int_size);
		rs->data += int_size;
	}

	if(rs->mini_regs & FORMAT_BIT){

		/* Default pad char is a '0' when formatting integers AND
		 * the value is right aligned. */
		if(!(rs->reg_flags & (1 << FS_REG_PAD_CHAR))
			&& rs->mini_regs & FS_FLAG_RIGHT_ALIGN)
		{
			rs->registers[FS_REG_PAD_CHAR] = '0';
		}

		/* Check if signed and negate if necessary. */
		if(num & 0x08){
			switch(num & 0x7){
#if (RP_CFG_Q_INT_SIZE_MASK & RP_CFG_Q_INT_SIZE_8)
				case 0:
					{
						int8_t* x = (int8_t*)(&rs->cur_data.binary);
						if(*x < 0){
							*x = -*x;
							rs->mini_regs |= INTERNAL_HACK_MINUS_FLAG;
							++total_len;
						}
					}
					break;
#endif

#if (RP_CFG_Q_INT_SIZE_MASK & RP_CFG_Q_INT_SIZE_16)
				case 1:
					{
						int16_t* x = (int16_t*)(&rs->cur_data.binary);
						if(*x < 0){
							*x = -*x;
							rs->mini_regs |= INTERNAL_HACK_MINUS_FLAG;
							++total_len;
						}
					}
					break;
#endif

#if (RP_CFG_Q_INT_SIZE_MASK & RP_CFG_Q_INT_SIZE_32)
				case 2:
					{
						int32_t* x = (int32_t*)(&rs->cur_data.binary);
						if(*x < 0){
							*x = -*x;
							rs->mini_regs |= INTERNAL_HACK_MINUS_FLAG;
							++total_len;
						}
					}
					break;
#endif

#if (RP_CFG_Q_INT_SIZE_MASK & RP_CFG_Q_INT_SIZE_64)
				case 3:
					{
						int64_t* x = (int64_t*)(&rs->cur_data.binary);
						if(*x < 0){
							*x = -*x;
							rs->mini_regs |= INTERNAL_HACK_MINUS_FLAG;
							++total_len;
						}
					}
					break;
#endif

#if (RP_CFG_Q_INT_SIZE_MASK & RP_CFG_Q_INT_SIZE_128)
#error "128-bit support not yet implemented"
				case 4:
					{
						/* 128 bit integers, TODO someday; not important now. */
						assert(0);
					}
					break;
#endif

				default:
					/* Unsupported type. */
					assert(0);
			}
		}

		/* Count the number of significant digits. */
		reprint_reg_t all_digits;
#if (RP_CFG_Q_RADIX & ~(RP_CFG_Q_RADIX_10))
		if(FQS_MR_RADIX_DEFINED & rs->mini_regs){
			all_digits = s_arch_calc_msb(rs->cur_data.binary) + 1;
			switch(rs->mini_regs & FQ_MR_RADIX_MASK){

#if (RP_CFG_Q_RADIX & RP_CFG_Q_RADIX_16)
				case FQ_MR_RADIX_16:
					all_digits += 0xF;
					all_digits >>= 4;
					++all_digits;
					break;
#endif

#if (RP_CFG_Q_RADIX & RP_CFG_Q_RADIX_8)
				case FQ_MR_RADIX_8:
					all_digits += 0x7;
					all_digits >>= 3;
					++all_digits;
					break;
#endif

#if (RP_CFG_Q_RADIX & RP_CFG_Q_RADIX_2)
				case FQ_MR_RADIX_2:
					/* Nothing to do. */
					break;
#endif

				default:
					assert(0);
					break;
			}
		}
		else
#endif
		{
#if (RP_CFG_Q_RADIX & RP_CFG_Q_RADIX_10)
			/* Determine number of base 10 digits. */
			all_digits = s_arch_calc_r10_digits(rs->cur_data.binary);
#endif
		}

		/* If significant digits defined by user, then choose the smaller
		 * of the two. */
		unsigned pad_zeros = 0;
		if(rs->reg_flags & (1 << FQS_REG_SIGFIGS)){
			/* If the user specified zero sig figs, then we do not print anything at all.
			 * Instead all digits are considered significant but we go
			 * back to looking for a new field. */
			if(!rs->registers[FQS_REG_SIGFIGS]){
				rs->registers[FQS_REG_SIGFIGS] = (1 << (num & 0x7)) << 3;
				goto BEGIN;
			}

			if(all_digits < rs->registers[FQS_REG_SIGFIGS]){
				pad_zeros = rs->registers[FQS_REG_SIGFIGS] - all_digits;
				rs->registers[FQS_REG_SIGFIGS] = all_digits;
			}
		}
		else{
			rs->registers[FQS_REG_SIGFIGS] = all_digits;
		}

		/* Shifting will move the break check forward. */
		unsigned break_check = 0;
#if (RP_CFG_Q_FEATURES & RP_CFG_Q_FEATURES_EXPO_FORM)
		if(rs->mini_regs & FQS_FLAG_EXPONENTIAL){
			/* One sigfig precedes the decimal point. */
			break_check = 1 | FQW_REG_BREAK_FLAG_SIG;

			/* May have to truncate sigfigs or change zero pads
			 * depending on precision. */
			if(rs->reg_flags & (1 << FQS_REG_PRECISION)){
				if(0 == rs->registers[FQS_REG_PRECISION]){
					rs->registers[FQS_REG_SIGFIGS] = 1;
					pad_zeros = 0;
					/* No decimal point. */
				}
				else if(rs->registers[FQS_REG_PRECISION] <
						rs->registers[FQS_REG_SIGFIGS] - 1)
				{
					/* This just reduces the number of sigfigs to prec. */
					rs->registers[FQS_REG_SIGFIGS] =
						rs->registers[FQS_REG_PRECISION] + 1;

					/* No padding zeros. */
					pad_zeros = 0;

					/* Add decimal point. */
					++total_len;
				}
				else{
					/* TODO Add more padding zeros. */
				}
			}


			/* If exponential form, then exponent is just number of sigfigs - 1.
			 * For 32-bit integers, this is < 10, so just add e and number char. */
			if(rs->mini_regs & FQS_FLAG_EXPONENTIAL)
				total_len += 2;

		}
		else 
#endif
		if(rs->reg_flags & ((1 << FQS_REG_SHIFT) | (1 << FQS_REG_PRECISION))){
			/* TODO apply the precision register?? Definitely OK for
			 * desktop systems, embedded systems probably don't need it. */

			/* If shift is greater than sigfigs + zeros, then we have
			 * zeroes in front of the sig digits.  * */
			if(rs->registers[FQS_REG_SHIFT] > all_digits){
				/* Break value is number of leading zeros between sig figs
				 * and decimal point. */
				break_check = rs->registers[FQS_REG_SHIFT] - all_digits;

				/* Make sure our we didn't reach into flag territory. */
				assert(!(break_check & FQW_REG_BREAK_FLAG_SIG));

				/* Print leading '0' */
				rs->mini_regs |= FQW_REG_PRINT_LZ;
				total_len += 2;
			}
			else if(rs->registers[FQS_REG_SHIFT] > pad_zeros){
				/* Shifting will split sigfigs. */
				break_check = rs->registers[FQS_REG_SHIFT] - pad_zeros;

				/* Make sure our we didn't reach into flag territory. */
				assert(!(break_check & FQW_REG_BREAK_FLAG_SIG));

				break_check |= FQW_REG_BREAK_FLAG_SIG;
				++total_len;
			}
			else if(rs->registers[FQS_REG_SHIFT] < pad_zeros){
				/* Breaking on the zeros.. */
				break_check = rs->registers[FQS_REG_SHIFT];
				++total_len;
			}

		}

		rs->registers[FQW_REG_BREAK] = break_check;
		rs->registers[FQW_REG_ZEROS] = pad_zeros;
		rs->cur_label = &&QUANT_SIGN;

		total_len +=
			rs->registers[FQW_REG_ZEROS] + rs->registers[FQS_REG_SIGFIGS];
	}
	else{
		/* TODO Output is in binary */
		assert(0);
	}
}
else{
	if(!(num & 0xF)){
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

	}
	else{
		/* TODO Floating point... */
		assert(0);
	}
}

#endif
