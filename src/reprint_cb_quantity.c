/* reprint_cb_quantity.c
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
 *
 * */
#ifdef REPRINT_GUARD_reprint_cb_QUANTITY


{
	/* Display sign if applicable. */
QUANT_SIGN:
	if(rs->mini_regs & INTERNAL_HACK_MINUS_FLAG){
		*dest = '-';
		rs->cur_label = &&QUANT_CHECK_PREFIX;
		return 1;
	}
	else if(rs->mini_regs & FQ_MR_PREFIX_FORCE_SIGN){
		*dest = '+';
		rs->cur_label = &&QUANT_CHECK_PREFIX;
		return 1;
	}

QUANT_CHECK_PREFIX:
	if(rs->mini_regs & FQ_MR_PREFIX_FORCE_SIGN){
		/* Obviously if we are here, we should have a non-decimal base. */

		if(FQ_MR_RADIX_2 == (rs->mini_regs & FQ_MR_RADIX_MASK)){
			*dest = 'b';
			rs->cur_label = &&QUANT_LEAD_ZERO;
			return 1;
		}

		*dest = '0';
		rs->cur_label = (FQ_MR_RADIX_16 == (rs->mini_regs & FQ_MR_RADIX_MASK))
			? &&QUANT_PREFIX_x : &&QUANT_LEAD_ZERO;
		return 1;

QUANT_PREFIX_x:
		*dest = 'x';
		rs->cur_label = &&QUANT_LEAD_ZERO;
		return 1;
	}

QUANT_LEAD_ZERO:
	if(rs->mini_regs & FQW_REG_PRINT_LZ){
		*dest = '0';
		rs->cur_label = &&QUANT_DECIMAL;
		return 1;
	}

	if(0){
QUANT_DECIMAL:
		*dest = '.';

		/* Just added a dec point. If the break is nonzero, that means
		 * will be breaking a sequence of zeros to insert the sigfigs.
		 * Otherwise, finishing the sigfigs. */
		rs->cur_label = (rs->registers[FQW_REG_BREAK])
			? &&QUANT_ZERO_PAD : &&QUANT_SIGFIGS;
		return 1;
	}

	rs->cur_label = &&QUANT_SIGFIGS;
QUANT_SIGFIGS:
	{
		/* Must have at least one sigfig to be in this section. */
		assert(rs->registers[FQS_REG_SIGFIGS]);

		/* Obviously, the general case for calculating what digit
		 * to print is faster using a division based method on
		 * most platforms. The key is MOST. I would like this to
		 * work on MSP430 and AVR without code bloat. */

		/* Will store the highest radix power into the delta value,
		 * which is computed based on number of sigfigs. */
		reprint_uint_t delta = rs->registers[FQS_REG_SIGFIGS] - 1;
		if(FQS_MR_RADIX_DEFINED & rs->mini_regs){
			uint16_t radix = rs->mini_regs & FQ_MR_RADIX_MASK;
			switch(radix){
#if (RP_CFG_Q_RADIX & RP_CFG_Q_RADIX_16)
				case FQ_MR_RADIX_16:
					assert(delta < INTERNAL_16_POWER_COUNT);
					delta = s_16_powers[rs->registers[FQS_REG_SIGFIGS] - 1];
					break;
#endif

#if (RP_CFG_Q_RADIX & RP_CFG_Q_RADIX_8)
				case FQ_MR_RADIX_8:
					assert(delta < INTERNAL_8_POWER_COUNT);
					delta = s_8_powers[rs->registers[FQS_REG_SIGFIGS] - 1];
					break;
#endif

#if (RP_CFG_Q_RADIX & RP_CFG_Q_RADIX_2)
				case FQ_MR_RADIX_2:
					assert(delta <= sizeof(reprint_uint_t));
					delta = 1 << (rs->registers[FQS_REG_SIGFIGS] - 1);
					break;
#endif

				default:
					assert(0);
			}
		}
		else
		{
#if (RP_CFG_Q_RADIX & RP_CFG_Q_RADIX_10)
			assert(delta < INTERNAL_10_POWER_COUNT);
			delta = s_10_powers[delta];
#endif
		}

		/* Select increment value. */
		reprint_uint_t tmp = 0;

		/* Compute digit by incrementing until we exceed current value. */
		uint8_t output = '0';

		while(rs->cur_data.binary - tmp >= delta){
			++output;
			tmp += delta;
		}
		rs->cur_data.binary -= tmp;

		/* Shift to capital letters. */
		if(output > '9')
			output += 7;

		/* TODO If this was the last digit to output, (and something is left)
		 * need to do proper rounding. */
		if(1 == rs->registers[FQS_REG_SIGFIGS] && rs->cur_data.binary
				&& *rs->fmt != Q_BITFIELD)
		{
		}

		*dest = output;

		--rs->registers[FQS_REG_SIGFIGS];

		/* Check if breaking the sigfigs. */
		if(rs->registers[FQW_REG_BREAK] & FQW_REG_BREAK_FLAG_SIG){
			/* Check if it is time to break. */
			if((rs->registers[FQW_REG_BREAK] & FQW_REG_BREAK_MASK)
				== rs->registers[FQS_REG_SIGFIGS])
			{
				/* Matched the break, so clear it. */
				rs->registers[FQW_REG_BREAK] = 0;

				/* If just printing bit sequence, then done.
				 * Clear out */
				if(*rs->fmt == Q_BITFIELD){
					/* If sigfigs is zero, clear the set flag. */
					if(!rs->registers[FQS_REG_SIGFIGS])
						rs->reg_flags &= ~(1 << FQS_REG_SIGFIGS);
					rs->reg_flags &= ~(
								(1 << FQB_REG_BDROP)
							| (1 << FQB_REG_BCOUNT)
							);
					goto FIELD_DONE;
				}
				else{
					/* Printing decimal next. */
					rs->cur_label = &&QUANT_DECIMAL;
				}
			}
		}
		else if(0 == rs->registers[FQS_REG_SIGFIGS]){
			/* Finished printing sigfigs. */

			/* If there are zeros to print, then go to that point. */
			if(rs->registers[FQW_REG_ZEROS]){
				rs->cur_label = &&QUANT_ZERO_PAD;
			}
			else{
				/* Unset shift, sigfig, and precision registers if not
				 * printing out bit fields. */
				rs->reg_flags &= ~(
							(1 << FQS_REG_PRECISION)
						| (1 << FQS_REG_SIGFIGS)
						| (1 << FQS_REG_SHIFT)
						);

				/* It may be exponent time. If not field is done. */
				rs->cur_label = (rs->mini_regs & FQS_FLAG_EXPONENTIAL)
					?  &&EXPONENT : &&FIELD_DONE;
			}
		}

		return 1;
	}

	/* Check if break register applies to zeros. */

	/* Skip over this section if calculating sigfigs */
	if(0){
QUANT_ZERO_PAD:
		/* Must have at least one zero to be in this section. */
		assert(rs->registers[FQW_REG_ZEROS]);

		*dest = '0';
		--rs->registers[FQW_REG_ZEROS];

		/* If not time to break then just return. */
		if(rs->registers[FQW_REG_ZEROS] != rs->registers[FQW_REG_BREAK])
			return 1;

		/* Matched the break, so clear it. */
		rs->registers[FQW_REG_BREAK] = 0;

		/* If there are sigfigs to print, then go onto that state. */
		if(rs->registers[FQS_REG_SIGFIGS]){
			rs->cur_label = &&QUANT_SIGFIGS;
			return 1;
		}

		/* If there are still zeros left, inserting decimal. */
		if(rs->registers[FQW_REG_ZEROS]){
			rs->cur_label = &&QUANT_DECIMAL;
			return 1;
		}

		/* Finished numerical portion */

		/* Unset shift, sigfig, and precision registers. */
		rs->reg_flags &= ~(
					(1 << FQS_REG_PRECISION)
				| (1 << FQS_REG_SIGFIGS)
				| (1 << FQS_REG_SHIFT)
				);

		/* It may be time for an exponent. If not field is done. */
		rs->cur_label = (rs->mini_regs & FQS_FLAG_EXPONENTIAL)
			? &&EXPONENT : &&FIELD_DONE;
		return 1;
	}

	/* Handle exponent (if applicable) */
	{
EXPONENT:
		/* Setup the number printing registers for the exponent. */
		if(rs->registers[FQW_REG_EXP] < INTERNAL_MID_EXPONENT){
			rs->cur_data.binary = INTERNAL_MID_EXPONENT - rs->registers[FQW_REG_EXP];
			rs->cur_label = &&QUANT_SIGN;
			rs->mini_regs |= INTERNAL_HACK_MINUS_FLAG;
		}
		else{
			rs->cur_data.binary = rs->registers[FQW_REG_EXP] - INTERNAL_MID_EXPONENT;
			rs->cur_label = &&QUANT_SIGFIGS;
		}

		/* Determine exponent sig figs. Always Base 10. */
		if(rs->cur_data.binary >= 100){
			rs->registers[FQS_REG_SIGFIGS] =
				rs->cur_data.binary >= 1000 ? 4 : 3;
		}
		else{
			rs->registers[FQS_REG_SIGFIGS] =
				rs->cur_data.binary >= 10 ? 2 : 1;
		}

		/* Break when finished with sigfigs. */
		rs->registers[FQW_REG_BREAK] = 0;

		/* Output the letter. select 'p' for power of 2 radices. */
#if (RP_CFG_Q_RADIX & ~(RP_CFG_Q_RADIX_10))
		if(FQS_MR_RADIX_DEFINED & rs->mini_regs){
			*dest = 'p';
		}
		else
#endif
		{
#if (RP_CFG_Q_RADIX & RP_CFG_Q_RADIX_10)
			*dest = 'e';
#endif
		}
		return 1;
	}
}

#endif
