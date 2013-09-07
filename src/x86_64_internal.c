/* x86_64_internal.c
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


enum {
	INTERNAL_2_POWER_COUNT = 64
	,INTERNAL_8_POWER_COUNT = 22
	/* FIXME */
	,INTERNAL_10_POWER_COUNT = 16
	,INTERNAL_16_POWER_COUNT = 16


	,INTERNAL_MID_EXPONENT = 127

};

static const reprint_uint_t s_8_powers[INTERNAL_8_POWER_COUNT] = {
	 0x0000000000000001ULL
	,0x0000000000000008ULL
	,0x0000000000000040ULL
	,0x0000000000000200ULL

	,0x0000000000001000ULL
	,0x0000000000008000ULL
	,0x0000000000040000ULL
	,0x0000000000200000ULL

	,0x0000000001000000ULL
	,0x0000000008000000ULL
	,0x0000000040000000ULL
	,0x0000000200000000ULL

	,0x0000001000000000ULL
	,0x0000008000000000ULL
	,0x0000040000000000ULL
	,0x0000200000000000ULL

	,0x0001000000000000ULL
	,0x0008000000000000ULL
	,0x0040000000000000ULL
	,0x0200000000000000ULL

	,0x1000000000000000ULL
	,0x8000000000000000ULL
};

static const reprint_uint_t s_10_powers[INTERNAL_10_POWER_COUNT] = {
	1ULL
	,10ULL
	,100ULL
	,1000ULL

	,10000ULL
	,100000ULL
	,1000000ULL
	,10000000ULL

	,100000000ULL
	,1000000000ULL
	,10000000000ULL
	,100000000000ULL

	,1000000000000ULL
	,10000000000000ULL
	,100000000000000ULL
	,1000000000000000ULL
};

static const reprint_uint_t s_16_powers[INTERNAL_16_POWER_COUNT] = {
	 0x0000000000000001ULL
	,0x0000000000000010ULL
	,0x0000000000000100ULL
	,0x0000000000001000ULL

	,0x0000000000010000ULL
	,0x0000000000100000ULL
	,0x0000000001000000ULL
	,0x0000000010000000ULL

	,0x0000000100000000ULL
	,0x0000001000000000ULL
	,0x0000010000000000ULL
	,0x0000100000000000ULL

	,0x0001000000000000ULL
	,0x0010000000000000ULL
	,0x0100000000000000ULL
	,0x1000000000000000ULL
};

static uint8_t s_arch_translate[7] = {
	0x41
	,0x42
	,0x41
	,0x41
	,0x42
	,0x41
	,0x41
};

static inline reprint_reg_t s_arch_calc_msb(reprint_uint_t v){
	/* Power of 2 base. Determine greatest 1 bit position.
	 * Shamelessly ripped from bit twiddling hacks:
	 * http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog
	 *
	 * */

	uint8_t r;
	uint8_t shift;
	r = (v > 0xFF) << 3; v >>= r;
	shift = (v > 0xF) << 2; v >>= shift; r |= shift;
	shift = (v > 0x3) << 1; v >>= shift; r |= shift;

	r |= (v >> 1);

	return r;
}

static inline reprint_reg_t s_arch_calc_r10_digits(reprint_uint_t v){
	/* TODO make more efficient with bit twiddling trick. */
	unsigned tmp = INTERNAL_10_POWER_COUNT;
	while(tmp > 0){
		if(v >= s_10_powers[tmp - 1])
			break;
		--tmp;
	}
	return tmp;
}


static inline const void* s_arch_align_ptr(const void* p, size_t size){
	if(1 == size)
		return p;

	uintptr_t x = (uintptr_t)p;
	x += size - 1;
	x &= ~(size - 1);
	return (const void*)x;
}

