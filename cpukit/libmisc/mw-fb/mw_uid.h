/*
/////////////////////////////////////////////////////////////////////////////
// $Header$
//
// Copyright (c) 2000 - Rosimildo da Silva
//  
// MODULE DESCRIPTION: 
// This module defines the interface for input devices used by MicroWindows
// in an embedded system environment.
//
// MODIFICATION/HISTORY:
//
// $Log$
//
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef _MW_UID_H
#define _MW_UID_H

#ifdef	__cplusplus
extern "C" {
#endif

/* 0x41XX  -- IOCLT functions for the Micro Input Devices commands */ 
#define MW_UID_REGISTER_DEVICE     0x4100
#define MW_UID_UNREGISTER_DEVICE   0x4101


/* devices supported by MicroWindows */
enum MW_INPUT_DEVICE_TYPE 
{
    MV_UID_INVALID   = 0,
    MV_UID_REL_POS   = 1,   /* mouse        */
    MV_UID_ABS_POS   = 2,   /* touch-screen */
    MV_UID_KBD       = 3,   /* keyboard     */
    MV_UID_TIMER     = 4    /* timer -- not used */
};


/* matching MicroWindows */
#define MV_BUTTON_RIGHT                 0x01
#define MV_BUTTON_CENTER                0x02
#define MV_BUTTON_LEFT                  0x04

/* modifiers of the keyboard type devices */
#define MV_KEY_MODIFIER_SHIFT_DOWN      0x10
#define MV_KEY_MODIFIER_ALT_DOWN        0x20

/* indication of the LEDS */
#define MV_KEY_MODIFIER_CAPS_ON         0x04
#define MV_KEY_MODIFIER_NUN_LOCK_ON     0x02
#define MV_KEY_SCROLL_LOCK_ON           0x01

/* keyboard modes -- default ASCII     */
#define MV_KEY_MODE_ASCII               0x01
/* 
 * This mode one event is sent when a key is pressed,
 * and another one is send when a key is released.
 */
#define MV_KEY_MODE_SCANCODE            0x00


/* these defines match with the linux keyboard range
   for ioctls functions for the keyboard interface.
   0x4BXX --- keyboard related functions
 */
#define MV_KDGKBMODE	0x4B44   /* gets current keyboard mode */
#define MV_KDSKBMODE	0x4B45   /* sets current keyboard mode */

/*
 * Message generated by input devices controlled by MicroWindows. 
 */
struct MW_UID_MESSAGE
{
  enum MW_INPUT_DEVICE_TYPE type;  /* device type */
  union
  {
     /* fired when keyboard events are raised */
     struct kbd_t {
        unsigned short code;        /* keycode or scancode        */
        unsigned char  modifiers;   /* key modifiers              */
        unsigned char  mode;        /* current Kbd mode           */
    } kbd;

    /* fired when position events are raised, mouse, touch screen, etc */
    struct pos_t {
        unsigned short btns; /* indicates which buttons are pressed */
        short x;             /* x location */
        short y;             /* y location */
        short z;             /* z location, 0 for 2D */
    } pos;

    /* fired by a timer device periodically */
    struct timer_t {
        unsigned long  frt;   /* free running timer */
        unsigned long  seq;   /* sequence number */
    } tmr;

  } m;
};


/*
 * API for creating/closing/accessing the message queue used by the micro
 * input device interface. All functions in this interface returns a 
 * zero ( 0 ) on success. One exception for that is the "read" routine
 * that returns the number of bytes read. Negaive numbers indicate errors
 *
 * The implementation of the message queue for RTEMS uses a POSIX message
 * queue interface. It should be very portable among systems with a POSIX
 * support.
 */

/* creates the message queue that holds events from the input devices */
extern int uid_open_queue( const char *q_name, int flags, size_t max_msgs );

/* closes message queue */
extern int uid_close_queue( void );

/* 
 * reads a message from the queue. It waits up to the specified 
 * timeout in mili-seconds.
 */
extern int uid_read_message( struct MW_UID_MESSAGE *m, unsigned long timeout );

/* write a message to the queue */
extern int uid_write_message( struct MW_UID_MESSAGE *m );


/* register device to insert data to the queue */
extern int uid_register_device( int fd, const char *q_name );

/* unregister device to stop adding messages to the queue */
extern int uid_unregister_device( int fd );

/* set the keyboard */
extern int uid_set_kbd_mode( int fd, int mode, int *old_mode );

#ifdef	__cplusplus
}
#endif

#endif /* _MW_UID_H */