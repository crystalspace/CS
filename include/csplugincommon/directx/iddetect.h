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

#ifndef __CS_IDDETECT_H__
#define __CS_IDDETECT_H__

#include "csutil/scf.h"
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <d3dcaps.h>

extern const IID IID_IDirectDetectionInternal;

/**
 * This interface is only used for communication between Direct3DRender.dll and Driver2D.dll.
 * It is not a true COM interface (not derived from iBase), and should not be exposed by the API.
 */
interface IDirectDetectionInternal
{
    ///
    STDMETHOD_(LPD3DDEVICEDESC, GetDesc3D)() = 0;
    ///
    STDMETHOD_(LPGUID, GetGuid3D)() = 0;
    ///
    STDMETHOD_(bool, GetMipmap)() = 0;
    ///
    STDMETHOD_(bool, GetAlphaBlend)() = 0;
    ///
    STDMETHOD_(int, GetAlphaBlendType)() = 0;
    ///
    STDMETHOD_(bool, GetAlphaBlendHalo)() = 0;
    ///
    STDMETHOD_(bool, GetHardware)() = 0;
};

#endif // __CS_IDDETECT_H__
