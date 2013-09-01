typedef uint8_t reprint_reg_t;
typedef uint16_t reprint_uint_t;

#undef RP_CFG_FEATURES
#define RP_CFG_FEATURES  0x0


#undef RP_CFG_Q_FEATURES
#define RP_CFG_Q_FEATURES \
	RP_CFG_Q_FEATURES_BITFIELDS


#undef RP_CFG_Q_RADIX
#define RP_CFG_Q_RADIX (\
	RP_CFG_Q_RADIX_16 \
	| RP_CFG_Q_RADIX_10 \
	| RP_CFG_Q_RADIX_2)


#undef RP_CFG_Q_INT_SIZE_MASK
#define RP_CFG_Q_INT_SIZE_MASK ( \
	RP_CFG_Q_INT_SIZE_8 \
	| RP_CFG_Q_INT_SIZE_16)


#undef RP_CFG_Q_FLOAT_SIZE_MASK
#define RP_CFG_Q_FLOAT_SIZE_MASK 0x00


#undef  RP_CFG_P_FEATURES
#define RP_CFG_P_FEATURES 0x0


#define RP_size_t    "A"
#define RP_ptrdiff_t "A"
#define RP_uint_t    "A"
