#ifndef __NeXT_osdefs_h
#define __NeXT_osdefs_h
//=============================================================================
//
//	Copyright (C)1999 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// osdefs.h
//
//	Platform-specific interface to common functionality.
//
// *NOTE* This file is included by system/sysdef.h and must not be renamed.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// The 2D graphics driver used by the software renderer on this platform.
//-----------------------------------------------------------------------------
#undef  SOFTWARE_2D_DRIVER
#define SOFTWARE_2D_DRIVER "crystalspace.graphics2d.next"


//-----------------------------------------------------------------------------
// NeXT does not supply strdup() so fake one up.
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>

static inline char* strdup( char const* s )
    {
    if (s == 0) s = "";
    char* p = (char*)malloc( strlen(s) + 1 );
    strcpy( p, s );
    return p;
    }


//-----------------------------------------------------------------------------
// Pull in definitions for getwd(), ntohl(), htonl(), etc.
//-----------------------------------------------------------------------------
#if defined(SYSDEF_GETCWD) || defined(SYSDEF_SOCKETS)
#include <libc.h>
#endif


//-----------------------------------------------------------------------------
// NeXT does not supply getcwd() so fake one up using getwd().
//-----------------------------------------------------------------------------
#if defined(SYSDEF_GETCWD)
#undef SYSDEF_GETCWD

#include <sys/param.h>

static inline char* getcwd( char* p, size_t size )
    {
    char s[ MAXPATHLEN ];
    char* r = getwd(s);
    if (r != 0)
	{
        strncpy( p, r, size - 1 );
	p[ size - 1 ] = '\0';
        r = p;
	}
    return r;
    }

#endif // SYSDEF_GETCWD


//-----------------------------------------------------------------------------
// NeXT does not properly support Posix 'dirent', so fake it with 'direct'.
//-----------------------------------------------------------------------------
#ifdef SYSDEF_DIR

#ifdef _POSIX_SOURCE
#  undef _POSIX_SOURCE
#  include <sys/dir.h>
#  define _POSIX_SOURCE
#else
#  include <sys/dir.h>
#endif
#include <sys/dirent.h>	// Just so it gets included *before* #define below.
#define dirent direct

#endif // SYSDEF_DIR


//-----------------------------------------------------------------------------
// NeXT uses built-in alloca().
//-----------------------------------------------------------------------------
#ifdef SYSDEF_ALLOCA
#undef SYSDEF_ALLOCA
#define	alloca(x) __builtin_alloca(x)
#endif // SYSDEF_ALLOCA

#endif // __NeXT_osdefs_h
