/*
    Copyright (C) 2005 by Jorrit Tyberghein
	                  Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_DIRECTX_GUIDS_H__
#define __CS_CSPLUGINCOMMON_DIRECTX_GUIDS_H__

// Must appear first after cssysdef.h

#include "csextern.h"
#include <windows.h>

#ifdef CS_BUILD_SHARED_LIBS
// Modify DEFINE_GUID macro to do DLL importing
#undef DEFINE_GUID
#undef GUID_EXT
#define GUID_EXT 	extern CS_CSPLUGINCOMMON_DX_EXPORT

#ifdef INITGUID
#define DEFINE_GUID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) GUID_EXT const GUID n = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#else
#define DEFINE_GUID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) GUID_EXT const GUID n
#endif

#endif // CS_BUILD_SHARED_LIBS

#endif // __CS_CSPLUGINCOMMON_DIRECTX_GUIDS_H__
