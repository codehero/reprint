typedef uint32_t reprint_reg_t;
typedef uint64_t reprint_uint_t;

#undef RP_CFG_Q_FLOAT_SIZE_MASK
#define RP_CFG_Q_FLOAT_SIZE_MASK ( \
	RP_CFG_Q_FLOAT_SIZE_32 \
	| RP_CFG_Q_FLOAT_SIZE_64)

#define RP_size_t    "C"
#define RP_ptrdiff_t "C"
#define RP_uint_t    "C"
