/*
    pixfmt.h
   
    Created by Matt Reda on Thu Jan 17 2002.

*/


#ifndef __CS_PIXFMT_H__
#define __CS_PIXFMT_H__

/**\file
 * This file defines what pixel format to use in 24 bit mode if 
 * the compiler does not pass one in    
 */

// Whether the renderer uses ARGB or ABGR should not be tied directly to
// the endianness of the platform.  But in order not to break code that
// is used to the old way of doing things, we use the following #define's
// which can be overridden for certain platforms
#define CS_24BIT_PIXEL_ARGB 0
#define CS_24BIT_PIXEL_ABGR 1

#if !defined(CS_24BIT_PIXEL_LAYOUT)
#  if defined(CS_LITTLE_ENDIAN)
#    define CS_24BIT_PIXEL_LAYOUT CS_24BIT_PIXEL_ARGB
#  else
#    define CS_24BIT_PIXEL_LAYOUT CS_24BIT_PIXEL_ABGR
#  endif
#endif


#endif // __CS_PIXFMT_H__

