/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_CSSYS_CSOSDEFS_H__
#define __CS_CSSYS_CSOSDEFS_H__

// Include platform-specific system definitions and overrides.

#if defined(CS_PLATFORM_MACOSX)	 /* Must appear before CS_PLATFORM_UNIX */
#  include "macosx/csosdefs.h"
#elif defined(CS_PLATFORM_UNIX)
#  include "unix/csosdefs.h"
#elif defined(CS_PLATFORM_WIN32)
#  include "win32/csosdefs.h"
# else
#  error Unable to locate platform-specific "csosdefs.h" file.
#endif

#endif // __CS_CSSYS_CSOSDEFS_H__
