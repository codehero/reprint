/* Each arch must define the following functions. */

static inline reprint_reg_t s_arch_calc_msb(reprint_uint_t v);

static inline reprint_reg_t s_arch_calc_r10_digits(reprint_uint_t v);

static inline const void* s_arch_align_ptr(const void* p, size_t size);

#ifdef ARCH_MSP430
#include "msp430_internal.c"
#endif

#ifdef ARCH_X86_64
#include "x86_64_internal.c"
#endif
