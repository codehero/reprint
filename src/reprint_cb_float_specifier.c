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
						rs->selectors |= INTERNAL_HACK_MINUS_FLAG;
						++total_len;
					}

					if(isnormal(*x)){
						/* Handle power of 2 radices differently than base 10. */
						if(FQS_S_RADIX_DEFINED & rs->selectors){
							/* TODO check negative exponents. */
							float sig = frexpf(*x, &exp);

							switch(rs->selectors & FQ_S_RADIX_MASK){
								case FQ_S_RADIX_16:
									sigfigs = 5;
									sig *= exp2f(exp % 4);
									sig *= exp2f(sigfigs * 4);
									exp /= 4;
									break;

								case FQ_S_RADIX_8:
									sigfigs = 7;
									sig *= exp2f(exp % 3);
									sig *= exp2f(sigfigs * 3);
									exp /= 3;
									break;

								case FQ_S_RADIX_2:
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

					if(rs->selectors & FQS_FLAG_EXPONENTIAL){
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
						/* TODO use that C++ backend that has the awesome algorithm?  */
						assert(0);

						/* If significant digits defined by user, then choose the smaller
						 * of the two. */
						if(rs->reg_flags & (1 << FQS_REG_SIGFIGS)){
							/* TODO */
							assert(0);
#if 0
							if(!rs->registers[FQS_REG_SIGFIGS])
								assert(0);

							if(all_digits < rs->registers[FQS_REG_SIGFIGS]){
								pad_zeros = rs->registers[FQS_REG_SIGFIGS] - all_digits;
								rs->registers[FQS_REG_SIGFIGS] = all_digits;
							}
#endif
						}

						if(exp < 0){
							/* Going to print a leading zero. */
							rs->selectors |= FQW_REG_PRINT_LZ;
							total_len += 2;

							/* Number of zeros between MS sigfig and decimal point. */
							rs->registers[FQW_REG_ZEROS] = -exp - 1;
							total_len += rs->registers[FQW_REG_ZEROS];
						}
						else{
#if 0
							/* First digit position is at exp. */
							/* Calculate where last digit of sigfigs ends up. */
							int last_digit_pos = exp - (sigfigs + 1) + 1;
							trailing_zero_count = precision + last_digit_pos;

							/* Remove sigfigs that extend past precision. */
							if(trailing_zero_count < 0)
								sigfigs += trailing_zero_count;

							rs->registers[FQW_REG_BREAK] = -exp - 1;
							rs->reg_flags |= 1 << FQW_REG_BREAK;
#endif

						}
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
						rs->selectors |= INTERNAL_HACK_MINUS_FLAG;
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
