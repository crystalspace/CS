
#include "csutil/scf.h"
#include <ddraw.h>
#include <d3d.h>
#include <d3dcaps.h>

#ifndef __IDDETECT_H__
#define __IDDETECT_H__

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

#endif
