/*  rtems/posix/key.inl
 *
 *  This include file contains the macro implementation of the private 
 *  inlined routines for POSIX key's.
 *
 *  COPYRIGHT (c) 1989-1999.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  $Id$
 */
 
#ifndef __RTEMS_POSIX_KEY_inl
#define __RTEMS_POSIX_KEY_inl
 
/*PAGE
 *
 *  _POSIX_Keys_Allocate
 */
 
#define _POSIX_Keys_Allocate() \
  (POSIX_Keys_Control *) _Objects_Allocate( &_POSIX_Keys_Information )
 
/*PAGE
 *
 *  _POSIX_Keys_Free
 */
 
#define _POSIX_Keys_Free( _the_key ) \
  _Objects_Free( &_POSIX_Keys_Information, &(_the_key)->Object );
 
/*PAGE
 *
 *  _POSIX_Keys_Get
 */
 
#define _POSIX_Keys_Get( _id, _location ) \
  (POSIX_Keys_Control *) \
    _Objects_Get( &_POSIX_Keys_Information, (_id), (_location) )
 
/*PAGE
 *
 *  _POSIX_Keys_Is_null
 */
 
#define _POSIX_Keys_Is_null( _the_key ) \
  (!(_the_key))

#endif
/*  end of include file */
