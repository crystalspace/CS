#ifndef TYPES_H
#define TYPES_H

//----------------
// If your compiler complains about 'true', 'false', and 'bool' it
// may be an older C++ compiler which doesn't understand these constructs.
// In that case, define -DNO_BOOL_TYPE in the makefile or here.
//----------------
//#define NO_BOOL_TYPE
#ifdef NO_BOOL_TYPE
    typedef int bool;
#   undef true
#   define true 1

#   undef false
#   define false 0
#endif /*NO_BOOL_TYPE*/

//----------------
// Make sure the following six types are correct for your system.
//----------------
typedef unsigned char UByte;	// Unsigned 8 bit value
typedef signed char SByte;	// Signed 8 bit value
typedef unsigned short UShort;	// Unsigned 16 bit value
typedef signed short SShort;	// Signed 16 bit value
typedef unsigned long ULong;	// Unsigned 32 bit value
typedef signed long SLong;	// Signed 32 bit value
typedef unsigned int UInt;	// Unsigned int value (16..32 bit)
typedef signed int SInt; 	// Signed int value (16..32 bit)

typedef unsigned long CS_ID;    // Used for uniquely generated id numbers 

#endif
