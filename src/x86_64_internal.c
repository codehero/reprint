enum {
	INTERNAL_2_POWER_COUNT = 64
	,INTERNAL_8_POWER_COUNT = 22
	/* FIXME */
	,INTERNAL_10_POWER_COUNT = 16
	,INTERNAL_16_POWER_COUNT = 16


	,INTERNAL_MID_EXPONENT = 127

};

static const reprint_uint_t s_8_powers[INTERNAL_8_POWER_COUNT] = {
	 0x0000000000000001LL
	,0x0000000000000008LL
	,0x0000000000000040LL
	,0x0000000000000200LL

	,0x0000000000001000LL
	,0x0000000000008000LL
	,0x0000000000040000LL
	,0x0000000000200000LL

	,0x0000000001000000LL
	,0x0000000008000000LL
	,0x0000000040000000LL
	,0x0000000200000000LL

	,0x0000001000000000LL
	,0x0000008000000000LL
	,0x0000040000000000LL
	,0x0000200000000000LL

	,0x0001000000000000LL
	,0x0008000000000000LL
	,0x0040000000000000LL
	,0x0200000000000000LL

	,0x1000000000000000LL
	,0x8000000000000000LL
};

static const reprint_uint_t s_10_powers[INTERNAL_10_POWER_COUNT] = {
	1LL
	,10LL
	,100LL
	,1000LL

	,10000LL
	,100000LL
	,1000000LL
	,10000000LL

	,100000000LL
	,1000000000LL
	,10000000000LL
	,100000000000LL

	,1000000000000LL
	,10000000000000LL
	,100000000000000LL
	,1000000000000000LL
};

static const reprint_uint_t s_16_powers[INTERNAL_16_POWER_COUNT] = {
	 0x0000000000000001LL
	,0x0000000000000010LL
	,0x0000000000000100LL
	,0x0000000000001000LL

	,0x0000000000010000LL
	,0x0000000000100000LL
	,0x0000000001000000LL
	,0x0000000010000000LL

	,0x0000000100000000LL
	,0x0000001000000000LL
	,0x0000010000000000LL
	,0x0000100000000000LL

	,0x0001000000000000LL
	,0x0010000000000000LL
	,0x0100000000000000LL
	,0x1000000000000000LL
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

