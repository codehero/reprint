/* arch.h
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

#ifndef __REPRINT_ARCH_H__
#define __REPRINT_ARCH_H__

/* Formatting features. */
#define RP_CFG_FEATURES_RIGHT_ALIGN 0x2
#define RP_CFG_FEATURES_FIELD_WIDTH 0x1

#define RP_CFG_FEATURES  0x3


/* Quantititative input features.  */
#define RP_CFG_Q_FEATURES_PFX_SEL   0x8
#define RP_CFG_Q_FEATURES_BITFIELDS 0x4
#define RP_CFG_Q_FEATURES_EXPO_FORM 0x2
#define RP_CFG_Q_FEATURES_ROUNDING  0x1

#define RP_CFG_Q_FEATURES           0xF


/* Quantititative radix output support. */
/* Support all radices by default. */
#define RP_CFG_Q_RADIX_16   0x8
#define RP_CFG_Q_RADIX_10   0x4
#define RP_CFG_Q_RADIX_8    0x2
#define RP_CFG_Q_RADIX_2    0x1

#define RP_CFG_Q_RADIX      0xF 


/* Quantitive Integer sizes supported. */
#define RP_CFG_Q_INT_SIZE_8   0x01
#define RP_CFG_Q_INT_SIZE_16  0x02
#define RP_CFG_Q_INT_SIZE_32  0x04
#define RP_CFG_Q_INT_SIZE_64  0x08
#define RP_CFG_Q_INT_SIZE_128 0x10

#define RP_CFG_Q_INT_SIZE_MASK 0x0F


/* Quantitive Float sizes supported. */
#define RP_CFG_Q_FLOAT_SIZE_16  0x01
#define RP_CFG_Q_FLOAT_SIZE_32  0x02
#define RP_CFG_Q_FLOAT_SIZE_64  0x04
#define RP_CFG_Q_FLOAT_SIZE_128 0x08
#define RP_CFG_Q_DEC_FLOAT_SIZE_32  0x10
#define RP_CFG_Q_DEC_FLOAT_SIZE_64  0x20
#define RP_CFG_Q_DEC_FLOAT_SIZE_128 0x40

#define RP_CFG_Q_FLOAT_SIZE_MASK 0x00

#define RP_CFG_Q_ARB_PREC  0x80


/* String support. */


/* Character features. */


/* Pointer features.  */
#define RP_CFG_P_FEATURES_BASE2  0x2
#define RP_CFG_P_FEATURES_MANIP  0x1

#define RP_CFG_P_FEATURES        0x3


/* Determines arch specific include. */

#ifdef ARCH_MSP430
#include "msp430.h"
#endif

#ifdef ARCH_X86_64
#include "x86_64.h"
#endif

#endif
