/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef __SYSDRIV_H__
#define __SYSDRIV_H__

// Include the system dependent driver classes.

#if defined(OS_NEXT)
#  include "cssys/next/shared/NeXTSystemDriver.h"
#elif defined(OS_UNIX) && !defined(OS_BE)
#  include "cssys/unix/unix.h"
#endif

#if defined(OS_WIN32)
#  include "cssys/win32/win32.h"
#endif

#if defined(COMP_WCC) && defined(OS_DOS)
#  include "cssys/wcc/csdoswat.h"
#endif

#if defined(COMP_GCC) && defined(OS_DOS)
#  include "cssys/djgpp/djgpp.h"
#endif

#if defined(OS_MACOS)
#  include "cssys/mac/MacKeyboard.h"
#  include "cssys/mac/MacMouse.h"
#  include "cssys/mac/MacSystem.h"
#endif

#if defined(OS_AMIGAOS)
#  include "cssys/amiga/amiga.h"
#endif

#if defined(OS_OS2)
#  include "cssys/os2/csos2.h"
#endif

#if defined(OS_BE)
#  include "cssys/be/csbe.h"
#endif

#endif //SYSDRIV_H
