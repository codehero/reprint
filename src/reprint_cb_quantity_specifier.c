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
 * reprint_state* rs;   (reprint state)
 * unsigned total_len;  (the number of characters required to print value)
 *
 * */
#ifdef REPRINT_GUARD_reprint_cb_QUANTITY_SPECIFIER

/* Ignore break flag. */
rs->reg_flags &= ~(1 << _FQW_REG_BREAK);

/* Populate the data field and advance the pointer.
 * Note: if this is a bitfield, just the use the value already present. */
{
	unsigned int_size = 0;

	/* Ambiguous values are either ambiguous integers
	 * or concrete integers with ambiguous specifier (fast or least). */
	if(!(rs->input_flags & IFLAG_CONCRETE)){
		int_size = s_arch_int_amb_size[*(rs->fmt) & SFLAG_SIZE_MASK];
	}
	else if((*rs->fmt & SFLAG_SIZE_MASK) != SFLAG_UC_BITFIELD){
		/* This is a non bitfield concrete type. */

		/* If any of the Aux0 flags are set then input type is specialized. */
		if(rs->input_flags & (IFLAG_AUX0_B0 | IFLAG_AUX0_B1)){
			/* Aux0 flags select which part of the table we index. */
			uint8_t index = rs->input_flags & (IFLAG_AUX0_B0 | IFLAG_AUX0_B1);
			index |= *(rs->fmt) & SFLAG_SIZE_MASK;

			/* First 8 are defined by format itself. */
			index -= 8;

			int_size = s_arch_int_conc_size[index];
		}
		else{
			/*  */
			int_size = 1 << (*(rs->fmt) & SFLAG_SIZE_MASK);
		}
	}

	if(int_size){
		/* If using struct packing, then align the pointer to the datatype. */
		if(!(FLAG_REG_TIGHT_PACK & rs->reg_flags))
			rs->data = s_arch_align_ptr(rs->data, int_size);

		/* TODO apply endian transformation from EE flags if concrete. */
		copy_bytes(&rs->cur_data.binary, rs->data, int_size);
		rs->data += int_size;

		/* Check if signed and negate if necessary. */
		if((rs->selectors & FORMAT_BIT) && !(rs->input_flags & IFLAG_UNSIGNED)){
			switch(*(rs->fmt) & SFLAG_SIZE_MASK){
#if (RP_CFG_Q_INT_SIZE_MASK & RP_CFG_Q_INT_SIZE_8)
				case 0:
					{
						int8_t* x = (int8_t*)(&rs->cur_data.binary);
						if(*x < 0){
							*x = -*x;
							rs->selectors |= _FQ_FLAG_INTERNAL_HACK_MINUS_FLAG;
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
							rs->selectors |= _FQ_FLAG_INTERNAL_HACK_MINUS_FLAG;
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
							rs->selectors |= _FQ_FLAG_INTERNAL_HACK_MINUS_FLAG;
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
							rs->selectors |= _FQ_FLAG_INTERNAL_HACK_MINUS_FLAG;
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

	}
	else{
		/* This is a bitfield. Assume that the user loaded up
		 * cur_data with a previous field. This section will
		 * strip out only the bits we are interested in printing.
		 * We will save the remaining bits in the last two registers,
		 * use the normal numeric printing process, and after the bitfield
		 * is done printing, restore the values in the registers to cur_data. */

		/* Don't print bitfields in exponential form. Hust don't. */
		if(rs->selectors & FQS_FLAG_EXPONENTIAL)
			assert(0);

		/* Outputting bit field. First drop bits if necessary. */
		if(rs->reg_flags & (1 << FQB_REG_BDROP)){
			/* If dropping more bits than we have, this is an error. */
			if(rs->registers[FQS_REG_SIGFIGS] < rs->registers[FQB_REG_BDROP]){
				assert(0);
			}
			rs->registers[FQS_REG_SIGFIGS] -= rs->registers[FQB_REG_BDROP];
			rs->cur_data.binary >>= rs->registers[FQB_REG_BDROP];

			/* No longer need this. */
			rs->reg_flags &= ~(1 << FQB_REG_BDROP);
			rs->registers[FQB_REG_BDROP] = 0;
		}

		/* Clear sigfigs flag (after bitfield prints, it will be set again
		 * if any bits remain.
		 * Precision also does not apply to bitfield values. */
		rs->reg_flags &= ~((1 << FQS_REG_SIGFIGS) | (1 << FQS_REG_PRECISION));

		/* If bit count is not set then just outputing the remaining bits. */
		if(!(rs->reg_flags & (1 << FQB_REG_BCOUNT))){
			rs->registers[FQB_REG_BCOUNT] = rs->registers[FQS_REG_SIGFIGS];
		}
		else{
			/* Do not preserve this value. */
			rs->reg_flags &= ~(1 << FQB_REG_BCOUNT);

			/* Make sure there are enough bits left for the count. */
			if(rs->registers[FQS_REG_SIGFIGS] < rs->registers[FQB_REG_BCOUNT])
				assert(0);

			/* If bitcount is zero, then do not print this bitfield */
			if(!rs->registers[FQB_REG_BCOUNT]){
				++rs->fmt;
				goto BEGIN;
			}
		}

		/* Compose bitfield value. */
		reprint_uint_t bitfield = rs->cur_data.binary
			& ((1 << rs->registers[FQB_REG_BCOUNT]) - 1);
		reprint_reg_t tmp = rs->registers[FQB_REG_BCOUNT];

		/* Shift out bitfield value. */
		rs->cur_data.binary >>= rs->registers[FQB_REG_BCOUNT];
		rs->registers[FQS_REG_SIGFIGS] -= rs->registers[FQB_REG_BCOUNT];

		/* Remember the binary value and the sigfig count. */
		rs->registers[2] = rs->registers[FQS_REG_SIGFIGS];
		memcpy(rs->registers + 6, &rs->cur_data.binary
			,sizeof(rs->cur_data.binary));

		/* Ready bitfield value for printing. */
		rs->cur_data.binary = bitfield;
		rs->registers[FQS_REG_SIGFIGS] = tmp;
	}
}

if(rs->selectors & FORMAT_BIT){

	/* Default pad char is a '0' when formatting integers AND
	 * the value is right aligned. */
	if(!(rs->reg_flags & (1 << F_REG_PAD_CHAR))
		&& rs->selectors & F_FLAG_RIGHT_ALIGN)
	{
		rs->registers[F_REG_PAD_CHAR] = '0';
	}

	/* Count the number of significant digits. */
	reprint_reg_t all_digits;
	unsigned pad_zeros = 0;
#if (RP_CFG_Q_RADIX & ~(RP_CFG_Q_RADIX_10))
	if(FQS_S_RADIX_DEFINED & rs->selectors){
		/* Note if this is a bitfield we will want to print ALL the bits
		 * out. */
		all_digits = s_arch_calc_msb(rs->cur_data.binary) + 1;
		switch(rs->selectors & _FQ_S_RADIX_MASK){

#if (RP_CFG_Q_RADIX & RP_CFG_Q_RADIX_16)
			case FQ_S_RADIX_16:
				all_digits += 3;
				all_digits >>= 2;
				if((rs->input_flags & IFLAG_CONCRETE)
						&& (*rs->fmt & SFLAG_SIZE_MASK) == SFLAG_UC_BITFIELD)
				{
					pad_zeros = rs->registers[FQS_REG_SIGFIGS] + 3;
					pad_zeros >>= 2;
					pad_zeros -= all_digits;
				}
				break;
#endif

#if (RP_CFG_Q_RADIX & RP_CFG_Q_RADIX_8)
			case FQ_S_RADIX_8:
				all_digits += 2;
				all_digits /= 3;
				if((rs->input_flags & IFLAG_CONCRETE)
					&& (*rs->fmt & SFLAG_SIZE_MASK) == SFLAG_UC_BITFIELD)
				{
					pad_zeros = rs->registers[FQS_REG_SIGFIGS] + 2;
					pad_zeros /= 3;
					pad_zeros -= all_digits;
				}
				break;
#endif

#if (RP_CFG_Q_RADIX & RP_CFG_Q_RADIX_2)
			case FQ_S_RADIX_2:
				/* Nothing to do. */
				if((rs->input_flags & IFLAG_CONCRETE)
					&& (*rs->fmt & SFLAG_SIZE_MASK) == SFLAG_UC_BITFIELD)
				{
					pad_zeros = rs->registers[FQS_REG_SIGFIGS] - all_digits;
				}
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
	if(rs->reg_flags & (1 << FQS_REG_SIGFIGS)){
		/* If the user specified zero sig figs, then we do not print anything at all.
		 * Instead all digits are considered significant but we go
		 * back to looking for a new field. */
		if(!rs->registers[FQS_REG_SIGFIGS]){
			rs->registers[FQS_REG_SIGFIGS] = (1 << (*(rs->fmt) & SFLAG_SIZE_MASK)) << 3;
			goto ST_FIELD_DONE;
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
#if (RP_CFG_Q_FEATURES & RP_CFG_Q_FEATURES_EXPO_FORM)
	if(rs->selectors & FQS_FLAG_EXPONENTIAL){
		/* One sigfig precedes the decimal point. */

		rs->reg_flags |= 1 << _FQW_REG_BREAK;
		rs->registers[_FQW_REG_BREAK] = 1;

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
		if(rs->selectors & FQS_FLAG_EXPONENTIAL)
			total_len += 2;

	}
	else 
#endif
	if(rs->reg_flags & ((1 << FQS_REG_SHIFT) | (1 << FQS_REG_PRECISION))){
		/* TODO apply the precision register?? */

		unsigned break_check = 0;
		/* If shift is greater than sigfigs + zeros, then we have
		 * zeroes in front of the sig digits.  * */
		if(rs->registers[FQS_REG_SHIFT] > all_digits){
			/* Break value is number of leading zeros between sig figs
			 * and decimal point. */
			break_check = rs->registers[FQS_REG_SHIFT] - all_digits;

			/* Print leading '0' */
			rs->selectors |= _FQW_REG_PRINT_LZ;
			total_len += 2;
		}
		else if(rs->registers[FQS_REG_SHIFT] > pad_zeros){
			/* Shifting will split sigfigs. */
			break_check = rs->registers[FQS_REG_SHIFT] - pad_zeros;

			rs->reg_flags |= 1 << _FQW_REG_BREAK;
			++total_len;
		}
		else if(rs->registers[FQS_REG_SHIFT] < pad_zeros){
			/* Breaking on the zeros.. */
			break_check = rs->registers[FQS_REG_SHIFT];
			++total_len;
		}
		rs->registers[_FQW_REG_BREAK] = break_check;
		rs->reg_flags |= 1 << _FQW_REG_BREAK;
	}

	rs->registers[_FQW_REG_ZEROS] = pad_zeros;

	/* If this is a bitfield and there leading zeros, then just go directly
	 * to printing zeros. (bitfields cannot print negative or fractional) */
	if((rs->input_flags & IFLAG_CONCRETE) && (*rs->fmt & SFLAG_SIZE_MASK) == SFLAG_UC_BITFIELD && pad_zeros){
		rs->cur_label = &&ST_QUANT_ZERO_PAD;
	}
	else{
		rs->cur_label = &&ST_QUANT_SIGN;
	}

	total_len +=
		rs->registers[_FQW_REG_ZEROS] + rs->registers[FQS_REG_SIGFIGS];
}
else{
	/* TODO Output is in binary */
	assert(0);
}

#endif
