/*  or1ktypes.h
 *
 *  This include file contains type definitions pertaining to the Opencores
 *  or1k processor family.
 *
 *  COPYRIGHT (c) 1989-1999.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  This file adapted from no_cpu example of the RTEMS distribution.
 *  The body has been modified for the Opencores Or1k implementation by
 *  Chris Ziomkowski. <chris@asics.ws>
 */

#ifndef __OR1K_TYPES_h
#define __OR1K_TYPES_h

#ifndef ASM

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  This section defines the basic types for this processor.
 */

typedef unsigned char  unsigned8;      /* unsigned 8-bit  integer */
typedef unsigned short unsigned16;     /* unsigned 16-bit integer */
typedef unsigned int   unsigned32;     /* unsigned 32-bit integer */
typedef unsigned long long unsigned64; /* unsigned 64-bit integer */

typedef unsigned16     Priority_Bit_map_control;

typedef signed char      signed8;      /* 8-bit  signed integer */
typedef signed short     signed16;     /* 16-bit signed integer */
typedef signed int       signed32;     /* 32-bit signed integer */
typedef signed long long signed64;     /* 64 bit signed integer */

typedef unsigned32 boolean;     /* Boolean value   */

typedef float          single_precision;     /* single precision float */
typedef double         double_precision;     /* double precision float */

typedef void no_cpu_isr;
typedef void ( *no_cpu_isr_entry )( void );

/* 
 * Turns out the that the disable POSIX option doesn't work
 * too well. Specifically, the libc library still wants to know
 * these POSIX values. We'll go ahead and include them here
 * until such time as someone like OAR who is familiar with this
 * can figure out what should really be done.
 */

#define NAME_MAX 255
#define LINK_MAX 8

#ifdef __cplusplus
}
#endif

#endif  /* !ASM */

#endif
/* end of include file */