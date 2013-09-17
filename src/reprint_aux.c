/* reprint_stdio.h
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

#include <string.h>
#include "reprint_aux.h"
#include "arch_internal.h"

const char* reprint_get_specifier(uint16_t* dest, const char* fmt){
	/* Find first format character. */
	while(*fmt && *fmt < 0x60)
		++fmt;

	if(!*fmt)
		return NULL;

	*dest = 0;
	while(*fmt < 0x70){
		/* This is an error. */
		if(*fmt < 0x60)
			return NULL;

		if(*fmt < 0x68){
			*dest |= (*fmt & 0x7);
		}
		else if(*fmt < 0x6C){
			*dest |= (*fmt & 0x3) << 3;
		}
		else{
			*dest |= (*fmt & 0x3) << 5;
		}
		++fmt;
	}

	*dest <<= 4;
	*dest |= *fmt & 0xF;
	++fmt;

	return fmt;
}

void* reprint_marshall_unsigned(void* dest, uint16_t specifier, unsigned long long value){
	/* Either dealing with ambiguous or concrete types. */
	unsigned int_size;
	if(specifier & 0x10){
		/* Possibly concrete type. */
		if(specifier & (0x3 << (2 + 3 + 3))){
			/* non concrete variation thereof. */
			/* FIXME */
			return NULL;
		}
		else{
			int_size = 1 << (specifier & 0x7);
		}
	}
	else{
		int_size = s_arch_int_amb_size[specifier & 0x7];
	}
	memcpy(dest, &value, int_size);

	return dest + int_size;
}

void* reprint_marshall_signed(void* dest, uint16_t specifier, signed long long value){
	/* Either dealing with ambiguous or concrete types. */
	if(specifier & 0x10){
		/* Possibly concrete type. */
		if(specifier & (0x3 << (2 + 3 + 3))){
			/* non concrete variation thereof. */
			/* FIXME */
			return NULL;
		}
		else{
			switch(specifier & 0x7){
				/* int8_t */
				case 0x0:
					{
						int8_t v = value;
						*(int8_t*)dest = v;
						return dest + 1;
					}

				/* int16_t */
				case 0x1:
					{
						int16_t v = value;
						memcpy(dest, &v, 2);
						return dest + 2;
					}

				/* int32_t */
				case 0x2:
					{
						int32_t v = value;
						memcpy(dest, &v, 4);
						return dest + 4;
					}

				/* int64_t */
				case 0x3:
					{
						int64_t v = value;
						memcpy(dest, &v, 8);
						return dest + 8;
					}

				/* TODO 128-bit. */

				default:
					return NULL;
			}

		}
	}
	else{
		switch(specifier & 0x7){
			/* char */
			case 0x0:
				{
					/* char is not 1 byte on all platforms! */
					char v = value;
					memcpy(dest, &v, sizeof(char));
					return dest + sizeof(char);
				}

			/* short */
			case 0x1:
				{
					short v = value;
					memcpy(dest, &v, sizeof(short));
					return dest + sizeof(short);
				}

			/* signed */
			case 0x2:
				{
					int v = value;
					memcpy(dest, &v, sizeof(int));
					return dest + sizeof(int);
				}

			/* long */
			case 0x3:
				{
					long v = value;
					memcpy(dest, &v, sizeof(long));
					return dest + sizeof(long);
				}

			/* long long */
			case 0x4:
				{
					long long v = value;
					memcpy(dest, &v, sizeof(long long));
					return dest + sizeof(long long);
				}

			default:
				return NULL;
		}
	}
}

void* reprint_marshall_bin_floating_pt(void* dest, uint16_t specifier, double value){
	switch(specifier & 0x7){
		/* half precision*/
		case 0x1:
			{
				/* TODO */
				return NULL;
			}

		/* 32 bit fp*/
		case 0x2:
			{
				float f = value;
				memcpy(dest, &f, 4);
				return dest + 4;
			}

		/* 64 bit. */
		case 0x3:
			{
				memcpy(dest, &value, 8);
				return dest + 8;
			}

		default:
			return NULL;
	}
}

void* reprint_marshall_pointer(void* dest, uint16_t specifier, const void* ptr){
	memcpy(dest, &ptr, sizeof(ptr));
	return dest + sizeof(ptr);
}

void* reprint_marshall_char(void* dest, uint16_t specifier, unsigned code_point){
	switch(specifier){
		/* char */
		case 0x24:
			{
				char v = code_point;
				*(char*)dest = v;
				return dest + 1;
			}

		default:
			return NULL;
	}

}
