/*
    Copyright (C) 1999,2000 by Eric Sunshine <sunshine@sunshineco.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CSOSDEFS_H__
#define __CSOSDEFS_H__

// The 2D graphics driver used by software renderer on this platform
#define CS_SOFTWARE_2D_DRIVER_BEOS "crystalspace.graphics2d.be"

#define CS_SOFTWARE_2D_DRIVER CS_SOFTWARE_2D_DRIVER_BEOS

// The 2D graphics driver used by OpenGL renderer
#define CS_OPENGL_2D_DRIVER "crystalspace.graphics2d.glbe"

// BeOS param.h unconditionally defines MAX and MIN, so we must remove the
// definitions set up by csdefs.h in order to avoid redifinition warnings.
#undef MIN
#undef MAX
#include <sys/param.h>
#if !defined(MIN)
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#if !defined(MAX)
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

// SCF symbol export facility.
#undef CS_EXPORTED_FUNCTION
#define CS_EXPORTED_FUNCTION extern "C" __declspec(dllexport)

#if defined (CS_SYSDEF_PROVIDE_DIR)
#  define __NEED_GENERIC_ISDIR
#endif

#if defined(CS_SYSDEF_PROVIDE_SOCKETS)
#  include <socket.h>
#  define CS_CLOSESOCKET closesocket
#  define CS_USE_FAKE_SOCKLEN_TYPE
#endif

#if defined(CS_SYSDEF_PROVIDE_SELECT)
#  include <socket.h>
#  undef CS_SYSDEF_PROVIDE_SELECT
#endif

#include <ByteOrder.h>
#if B_HOST_IS_LENDIAN
#  define CS_LITTLE_ENDIAN
#elif B_HOST_IS_BENDIAN
#  define CS_BIG_ENDIAN
#else
#  error "Please define a suitable CS_XXX_ENDIAN macro in be/csosdefs.h!"
#endif

#endif // __CSOSDEFS_H__
