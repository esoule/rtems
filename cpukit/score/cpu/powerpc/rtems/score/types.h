/*  types.h
 *
 *  This include file contains type definitions pertaining to the PowerPC
 *  processor family.
 *
 *  Author:	Andrew Bray <andy@i-cubed.co.uk>
 *
 *  COPYRIGHT (c) 1995 by i-cubed ltd.
 *
 *  To anyone who acknowledges that this file is provided "AS IS"
 *  without any express or implied warranty:
 *      permission to use, copy, modify, and distribute this file
 *      for any purpose is hereby granted without fee, provided that
 *      the above copyright notice and this notice appears in all
 *      copies, and that the name of i-cubed limited not be used in
 *      advertising or publicity pertaining to distribution of the
 *      software without specific, written prior permission.
 *      i-cubed limited makes no representations about the suitability
 *      of this software for any purpose.
 *
 *  Derived from c/src/exec/cpu/no_cpu/no_cputypes.h:
 *
 *  COPYRIGHT (c) 1989-1997.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may in
 *  the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  $Id$
 */

#ifndef __PPC_TYPES_h
#define __PPC_TYPES_h

#ifndef ASM
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  This section defines the basic types for this processor.
 */

typedef uint8_t     unsigned8;      /* unsigned 8-bit  integer */
typedef uint16_t    unsigned16;     /* unsigned 16-bit integer */
typedef uint32_t    unsigned32;     /* unsigned 32-bit integer */
typedef uint64_t    unsigned64; /* unsigned 64-bit integer */

typedef unsigned32     Priority_Bit_map_control;

typedef int8_t      signed8;      /* 8-bit  signed integer */
typedef int16_t     signed16;     /* 16-bit signed integer */
typedef int32_t     signed32;     /* 32-bit signed integer */
typedef int64_t     signed64;     /* 64 bit signed integer */

typedef unsigned32 boolean;     /* Boolean value   */

typedef float          single_precision;     /* single precision float */
typedef double         double_precision;     /* double precision float */

typedef void ppc_isr;

#ifdef __cplusplus
}
#endif

#endif  /* !ASM */

#endif
/* end of include file */
