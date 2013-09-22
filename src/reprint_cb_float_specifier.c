/* This C file only has context within reprint.c:reprint_cb()
 * The inherited variables from the reprint_cb() context are
 *
 * reprint_state* rs;   (reprint state)
 * unsigned total_len;  (the number of characters required to print value)
 *
 * */

#ifdef REPRINT_GUARD_reprint_cb_FLOAT_SPECIFIER

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

			{
				minus = (v->f32 < 0) ? 1 : 0;
				if(isnan(v->f32)){
					/* Set exponent to highest value (1023)
					 * and set highest significand bit. */
					v->u64 = 0x7FF8L << 48;

					assert(0);
				}
				else if(isinf(v->f32)){
					/* Set exponent to highest value (1023)
					 * and set significand to zero value. */
					v->u64 = 0x7FFL << 52;

					assert(0);
				}
				else{
					/* Number of significant digits in corresponding base. */
					static const unsigned s_f32_sigfigs[8] = {
						/* Base 10. */
						[0] = 6,
						/* Base 16. */
						[4] = 5,
						/* Base 8. */
						[5] = 7,
						/* Base 2. */
						[6] = 23
					};

					/* Calculate number of sigfigs based on output numeric base. */

					/* If using fixed precision output, we could be outputting
					 * fewer significant digits. */
					sigfigs = (cur->ctl.oflags & PONF_EXP) ? precision :
						s_f32_sigfigs[(cur->ctl.oflags >> PONR_OFFSET) & 0x7];

					if(cur->ctl.oflags & PONR_FLAG_MASK){
						float sig = frexp(v->f32, &exp);

						if(!(cur->ctl.oflags & PONF_EXP)){
							/* First digit position is at exp. */
							/* Calculate where last digit of sigfigs ends up. */
							int last_digit_pos = exp - (sigfigs + 1) + 1;
							trailing_zero_count = precision + last_digit_pos;

							/* Remove sigfigs that extend past precision. */
							if(trailing_zero_count < 0)
								sigfigs += trailing_zero_count;
						}

						switch(cur->ctl.oflags & PONR_FLAG_MASK){
							case PONR_16:
								sig *= exp2f(exp % 4);
								sig *= exp2f(sigfigs * 4);
								exp /= 4;
								break;

							case PONR_08:
								sig *= exp2f(exp % 3);
								sig *= exp2f(sigfigs * 3);
								exp /= 3;
								break;

							case PONR_02:
								sig *= exp2f(sigfigs);
								break;

							default:
								break;
						}
						v->u32 = fabs(sig);
					}
					else{
						/* Put into exponential form. */
						exp = floorf(log10f(v->f32));

						if(!(cur->ctl.oflags & PONF_EXP)){
							/* First digit position is at exp. */
							/* Calculate where last digit of sigfigs ends up. */
							int last_digit_pos = exp - (sigfigs + 1) + 1;
							trailing_zero_count = precision + last_digit_pos;

							/* Remove sigfigs that extend past precision. */
							if(trailing_zero_count < 0)
								sigfigs += trailing_zero_count;
						}

						v->f32 *= pow10f(-exp + (int)(sigfigs));

						/* TODO Select rounding method from oflags!
						 * Defaulting to round to even. */
						v->u32 = round(fabs(v->f32));
					}
				}
			}
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
