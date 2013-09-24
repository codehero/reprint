/* This C file only has context within reprint.c:reprint_cb()
 * The inherited variables from the reprint_cb() context are
 *
 * reprint_state* rs;   (reprint state)
 * unsigned total_len;  (the number of characters required to print value)
 *
 * */

#ifdef REPRINT_GUARD_reprint_cb_FLOAT_SPECIFIER
#include <math.h>
#include <stdlib.h>

static const char s_infinity[] = "-Infinity";
static const char s_nan[] = "NaN";

rs->reg_flags &= ~(1 << FQW_REG_BREAK);

{
	/* For floats, the size of the data */
	unsigned size = 1 << (*(rs->fmt) & 0x7);

	/* If using struct packing, then align the pointer to the datatype. */
	if(rs->reg_flags & FLAG_REG_STRUCT_PACK)
		rs->data = s_arch_align_ptr(rs->data, size);

	copy_bytes(&rs->cur_data.binary, rs->data, size);
	rs->data += size;

	/* TODO complex and decimal floating point. */

	int exp = 0;
	unsigned sigfigs;
	switch(*(rs->fmt) & 0x7){
#if (RP_CFG_Q_FLOAT_SIZE_MASK & RP_CFG_Q_FLOAT_SIZE_16)
#error "16-bit float support not yet implemented"
		case 1:
			assert(0);
			break;
#endif

#if (RP_CFG_Q_FLOAT_SIZE_MASK & RP_CFG_Q_FLOAT_SIZE_32)
		case 2:
			{
				float* x = (float*)(&rs->cur_data.binary);
				if(isnan(*x)){
					/* Going to output string instead of number. */
					total_len = sizeof(s_nan) - 1;
					rs->cur_data.text = (uint8_t*)s_nan;

					rs->cur_label = &&ST_TEXT;
				}
				else if(isinf(*x)){
					/* Going to output string instead of number. */
					const char* str = s_infinity;
					total_len = sizeof(s_infinity) - 1;
					rs->cur_data.text = (uint8_t*)str;
					if(*x > 0){
						++str;
						--total_len;
					}

					rs->cur_label = &&ST_TEXT;
				}
				else{
					if(*x < 0){
						*x = -*x;
						rs->mini_regs |= INTERNAL_HACK_MINUS_FLAG;
						++total_len;
					}

					if(isnormal(*x)){
						/* Handle power of 2 radices differently than base 10. */
						if(FQS_MR_RADIX_DEFINED & rs->mini_regs){
							/* TODO check negative exponents. */
							float sig = frexpf(*x, &exp);

							switch(rs->mini_regs & FQ_MR_RADIX_MASK){
								case FQ_MR_RADIX_16:
									sigfigs = 5;
									sig *= exp2f(exp % 4);
									sig *= exp2f(sigfigs * 4);
									exp /= 4;
									break;

								case FQ_MR_RADIX_8:
									sigfigs = 7;
									sig *= exp2f(exp % 3);
									sig *= exp2f(sigfigs * 3);
									exp /= 3;
									break;

								case FQ_MR_RADIX_2:
									sigfigs = 23;
									sig *= exp2f(sigfigs);
									break;

								default:
									assert(0);
							}
							rs->cur_data.binary = fabs(sig);
						}
						else{
							sigfigs = 6;

							/* Determine exponent and prep digits for decimal display. */
							exp = floorf(log10f(*x));
							*x *= pow10f(-exp + (int)(sigfigs));

							/* TODO Select rounding method from oflags!
							 * Defaulting to round to even. */
							rs->cur_data.binary = round(fabs(*x));
						}
						total_len += sigfigs;
					}
					else{
						/* Subnormal number. Number of sigfigs depends on magnitude. */
						/* Put into its equivalent exponential mode. */
						/* TODO */
						assert(0);
					}

					if(rs->mini_regs & FQS_FLAG_EXPONENTIAL){
						rs->registers[FQS_REG_SIGFIGS] = sigfigs + 1;

						/* One sigfig precedes the decimal point. */
						rs->reg_flags |= 1 << FQW_REG_BREAK;
						rs->registers[FQW_REG_BREAK] = sigfigs;

						/* May have to truncate sigfigs or change zero pads
						 * depending on precision. */
						if(rs->reg_flags & (1 << FQS_REG_PRECISION)){
							if(0 == rs->registers[FQS_REG_PRECISION]){
								rs->registers[FQS_REG_SIGFIGS] = 1;
								rs->registers[FQW_REG_ZEROS] = 0;
								/* No decimal point. */
							}
							else if(rs->registers[FQS_REG_PRECISION] <
									rs->registers[FQS_REG_SIGFIGS] - 1)
							{
								/* This just reduces the number of sigfigs to prec. */
								rs->registers[FQS_REG_SIGFIGS] =
									rs->registers[FQS_REG_PRECISION] + 1;

								/* No padding zeros. */
								rs->registers[FQW_REG_ZEROS] = 0;

								/* Add decimal point. */
								++total_len;
							}
							else{
								/* TODO Add more padding zeros. */
							}
						}

						/* Add exponent char. */
						++total_len;

						/* Add exponent value. */
						if(exp < 0)
							++total_len;
						int* e = (int*)(rs->registers + FQW_REG_EXP);
						*e = exp;
						total_len += s_arch_calc_r10_digits(abs(exp));
					}
					else{
#if 0

						/* If significant digits defined by user, then choose the smaller
						 * of the two. */
						if(rs->reg_flags & (1 << FQS_REG_SIGFIGS)){
							if(!rs->registers[FQS_REG_SIGFIGS])
								assert(0);

							if(all_digits < rs->registers[FQS_REG_SIGFIGS]){
								pad_zeros = rs->registers[FQS_REG_SIGFIGS] - all_digits;
								rs->registers[FQS_REG_SIGFIGS] = all_digits;
							}
						}

						/* First digit position is at exp. */
						/* Calculate where last digit of sigfigs ends up. */
						int last_digit_pos = exp - (sigfigs + 1) + 1;
						trailing_zero_count = precision + last_digit_pos;

						/* Remove sigfigs that extend past precision. */
						if(trailing_zero_count < 0)
							sigfigs += trailing_zero_count;
#endif
					}
				}
			}
			break;
#endif

#if (RP_CFG_Q_FLOAT_SIZE_MASK & RP_CFG_Q_FLOAT_SIZE_64)
		case 3:
			{
				double* x = (double*)(&rs->cur_data.binary);
				if(isnan(*x)){
					/* Going to output string instead of number. */
					total_len = sizeof(s_nan) - 1;
					rs->cur_data.text = (uint8_t*)s_nan;

					rs->cur_label = &&ST_TEXT;
				}
				else if(isinf(*x)){
					/* Going to output string instead of number. */
					const char* str = s_infinity;
					total_len = sizeof(s_infinity) - 1;
					rs->cur_data.text = (uint8_t*)str;
					if(*x > 0){
						++str;
						--total_len;
					}

					rs->cur_label = &&ST_TEXT;
				}
				else{
					if(*x < 0){
						*x = -*x;
						rs->mini_regs |= INTERNAL_HACK_MINUS_FLAG;
						++total_len;
					}
				}
			}
			break;
#endif

#if (RP_CFG_Q_FLOAT_SIZE_MASK & RP_CFG_Q_FLOAT_SIZE_128)
#error "128-bit float support not yet implemented"
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

if(!rs->cur_label)
	rs->cur_label = &&ST_QUANT_SIGN;
#endif

#if 0
{
	/* Binary 64 bit. */
	{
		case PINT_FLOAT_64:
			if(isnan(v->f64)){
				/* Set exponent to highest value (1023)
				 * and set highest significand bit. */
				v->u64 = 0x7FF8L << 48;
			}
			else if((exp = isinf(v->f64))){
				/* Set exponent to highest value (1023)
				 * and set significand to zero value. */
				v->u64 = 0x7FFL << 52;
				minus = (exp < 0);
			}

			/* TODO */
			assert(0);

			/* FALLTHROUGH to common floating point code. */
	}

	/* Binary 32 bit. */
	if(0){
		case PINT_FLOAT_32:

			
	}

	/* COMMON FLOATING POINT CODE. */

	if(cur->ctl.oflags & PONF_EXP){
		/* If more than one digit then add in fracpoint. */
		if(precision > 0)
			++len;

		/* Calculate decimal exponent. */
		unsigned explen = 0;
		if(exp < 0){
			/* Add in minus sign for exponent. */
			++len;
		}

		/* Determine exponent length.
		 * Must be at least 2 digits. */
		if(abs(exp) < 100){
			len += 2;
		}
		else{
			len += (abs(exp) < 1000) ? 3 : 4;
		}

		/* Add in exponent character. */
		++len;

		/* Add in leading digit. */
		++len;

		/* Add sigfigs */
		len += sigfigs;

		/* Bias exponent. */
		exp += 1023;
	}
	else{
		/* First digit position is at exp. */
		/* Calculate where last digit of sigfigs ends up. */

		if(cur->ctl.auxiliary & PA_AUX_INV_FLAG){
			/* Number of digits determined by the greater of
			 * precision or the exponent of the significand. */
			len = (exp < precision) ? precision + 1 : exp + 1;

			/* Number of trailing zeros determined by precision. */
			exp = precision + 1;
		}
		else{
			/* At least some number of frac digits + fracpoint. */
			len = precision + 1;

			/* If exp less than 0, then need "0." leading chars
			 * Otherwise, will show exp + 1 leading digits. */
			len += (exp < 0) ? 2 : (exp + 1);

			exp = trailing_zero_count;
		}

		/* NOTE exp now stores
		 * the number of zeros TRAILING the least significant digit. */
	}

	v->u64 |= (uint64_t)exp << 52;
}


if(PI_NUMERIC == (pst->cur_field->ctl.itype & PI_FLAG_ALL)){
	unsigned size = 32;
	uint64_t x64 = pst->cur->u64;
	uint8_t* out_begin = outp;
	uint8_t* out_tail = outp + *pst->cur_len - 1;
	int exp;

	/* Handle the minus sign. */
	if(PINT_SIGNED_64 != (pst->cur_field->ctl.itype & PINT_FLAG_MASK)){
		/* If high bit set then write minus sign.
		 * clear the bit. */
		if(x64 >> 63){
			*(out_begin++) = '-';
			x64 &= 0x7FFFFFFFFFFFFFFF;
		}
	}
	else{
		if(pst->cur->i64 < 0){
			/* Write minus and flip sign. */
			*(out_begin++) = '-';
			size = 64;
			x64 = -pst->cur->i64;
		}
	}

	switch((pst->cur_field->ctl.itype & PINT_FLAG_MASK)){

		case PINT_FLOAT_64:
			size = 64;

		case PINT_FLOAT_32:

			/* Output minus, exponent and exponent character. */
			exp = ((x64 >> 52) & 0xFFF);

			if(PONF_EXP == (pst->cur_field->ctl.oflags & PONF_FLAG_MASK)){
				/* Subtract bias. */
				exp -= 1023;

				/* 5 is an invalid length in this case. */
				assert(*pst->cur_len != 5);

				/* Determine exponent length. */
				unsigned ewidth = 2;
				if(abs(exp) > 100)
					ewidth += (abs(exp) < 1000) ? 1 : 2;

				/* Write exponent. All digits must be consumed. */
				assert(!write_int(ewidth, out_tail, abs(exp), PONR_10));
				out_tail -= ewidth;

				/* Write exp minus if necessary */
				if(exp < 0){
					*out_tail = '-';
					--out_tail;
					++ewidth;
				}

				/* FIXME handle hex/octal/binary floats here! */
				*out_tail = pst->config.decimal_exp;
				--out_tail;

				/* Output trailing digits and '.' if above minimum length. */
				if(*pst->cur_len > 4){
					/* Output section following decimal point. */
					ewidth = *pst->cur_len - ewidth - 1 - 2;
					x64 = write_int(ewidth, out_tail, x64,
							pst->cur_field->ctl.oflags);

					/* This must be a non-zero value. */
					assert(x64);

					/* Output '.'. */
					*(out_begin + 1) = '.';
				}

				write_int(1, out_begin, x64, pst->cur_field->ctl.oflags);
			}
			else{
				/* 
				 * The number of digits is a function of the length:
				 * content length - (minus_char + '.' + significand_len)
				 * = padding zeros at end of number.
				 *
				 * Unfortunately, have to recalculate significand_len
				 * here because there is no space to cache it in prep_apply.
				 *
				 * The decimal point will occur within the padding, or
				 * the significand digits, or right between.
				 * */

				/* If precision <= 0, there is not fracpoint.. */
				uint16_t precision =
					pst->cur_field->ctl.auxiliary & PANP_FLAG_MASK;
				if(!(pst->cur_field->ctl.auxiliary & PA_AUX_INV_FLAG)){
					uint8_t* fracpoint = out_tail - precision;
					/* Output trailing zeros first. */
					if(exp){
						/* Write all zero's before decimal point if necessary. */
						if(exp > precision){
							memset(out_tail - precision + 1, '0', precision);
							exp -= precision;

							/* Skip over decimal point. */
							out_tail = fracpoint - 1;
						}

						/* Write remaining zeros. */
						memset(out_tail - exp + 1, '0', exp);
						out_tail -= exp;
					}

					/* Write portion of sigfigs following fracpoint. */
					if(out_tail > fracpoint){
						x64 = write_int(out_tail - fracpoint,
								out_tail, x64, pst->cur_field->ctl.oflags);
					}

					/* Output significant digits. */
					*fracpoint = '.';
					out_tail = fracpoint - 1;

					/* Write portion of significand preceding fracpoint.
					 * Note that this will also write all leading zeros
					 * behind the fracpoint. */
					if(x64){
						x64 = write_int(out_tail - out_begin + 1,
								out_tail, x64, pst->cur_field->ctl.oflags);
					}
					else{
						*out_tail = '0';

						/* Make sure we end up at beginning our section. */
						assert(out_tail == out_begin);
					}
				}
				else{
					/* Output trailing zeros. */
					memset(out_tail - precision, '0', precision);
					out_tail -= precision;

					/* Output significand. */
					if(x64){
						x64 = write_int(out_tail - out_begin + 1,
								out_tail, x64, pst->cur_field->ctl.oflags);
					}
				}
			}
			break;

			/* Fall through to print significand portion preceding decimal */
		case PINT_SIGNED_64:
		case PINT_SIGNED_8:
		case PINT_SIGNED_16:
		case PINT_SIGNED_32:
			/* Choose output function based on highest significant bit */
			if(size == 32){
				unsigned nlen = s_ulen(pst->cur->u32);
				/* Adjust for output radix and store length. */
				nlen =
					s_intlen(nlen, pst->cur->u32, pst->cur_field->ctl.oflags);

				if(PONF_EXP == (pst->cur_field->ctl.oflags & PONF_FLAG_MASK)){
					/* 5 is an invalid length in this case. */
					assert(*pst->cur_len != 5);

					/* Exponent is based on number of digits.  */
					unsigned exp = nlen - 1;

					/* Write exponents first. */
					*out_tail = exp % 10 + '0';
					--out_tail;
					*out_tail = exp / 10 + '0';
					--out_tail;
					*out_tail = pst->config.decimal_exp;
					--out_tail;

					if(*pst->cur_len > 4){

						/* Divide out part that won't be printed. */
						pst->cur->u32 /= PowersOf10_2[nlen - (*pst->cur_len - 4)];

						/* Write fractional part. */
						exp = write_int(*pst->cur_len - 3 - 2, out_tail,
								pst->cur->u32, pst->cur_field->ctl.oflags);
						assert(exp > 0);

						/* Write fracpoint and MSD. */
						*(out_begin + 1) = '.';
						exp = write_int(1, out_begin, exp,
								pst->cur_field->ctl.oflags);
					}
					else{
						/* Handle precision of 0 (which is when len == 4). */
						pst->cur->u32 /= PowersOf10_2[nlen - 1];
						exp = write_int(1, out_begin, pst->cur->u32,
								pst->cur_field->ctl.oflags);
					}
				}
				else{
					uint16_t precision =
						pst->cur_field->ctl.auxiliary & PANP_FLAG_MASK;
					if(precision &&
							!(pst->cur_field->ctl.auxiliary & PA_AUX_INV_FLAG))
					{
						out_tail -= precision - 1;
						memset(out_tail, '0', precision);
						--out_tail;
						*out_tail = '.';
						--out_tail;
					}

					write_int(nlen, out_tail, pst->cur->u32,
							pst->cur_field->ctl.oflags);
				}
			}
			else{
				write_int64(out_begin, out_tail, x64,
						pst->cur_field->ctl.oflags);
			}
			break;


			/* TODO Not a priority right now... */
		case PINT_FLOAT_DECIMAL_32:
		case PINT_FLOAT_DECIMAL_64:
		case PINT_FLOAT_16:
		default:
			assert(0);
			return NULL;
	}
}
#endif
