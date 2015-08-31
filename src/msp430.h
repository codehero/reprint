/* msp430.h
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


typedef uint16_t reprint_reg_t;
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

#undef RP_CFG_SPECIFIER_USER
