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

#include "sysdef.h"
#include "cscom/com.h"
#include "cs2d/ddraw/g2d.h"
#include "cs2d/ddraw/xg2d.h"
#include "cssys/win32/DirectDetection.h"


IMPLEMENT_COMPOSITE_UNKNOWN(csGraphics2DDDraw3, XDDraw3GraphicsInfo)

STDMETHODIMP IXDDraw3GraphicsInfo::Open(const char* szTitle)
{
    METHOD_PROLOGUE( csGraphics2DDDraw3, XDDraw3GraphicsInfo);
    
    pThis->Open(szTitle);
    return S_OK;
}

STDMETHODIMP IXDDraw3GraphicsInfo::Close()
{
    METHOD_PROLOGUE( csGraphics2DDDraw3, XDDraw3GraphicsInfo);
    
    pThis->Close();
    return S_OK;
}

STDMETHODIMP IXDDraw3GraphicsInfo::GetDirectDrawDriver( LPDIRECTDRAW* lplpDirectDraw )
{
    METHOD_PROLOGUE( csGraphics2DDDraw3, XDDraw3GraphicsInfo);
    
    *lplpDirectDraw = pThis->m_lpDD;
    return S_OK;
}

STDMETHODIMP IXDDraw3GraphicsInfo::GetDirectDrawPrimary( LPDIRECTDRAWSURFACE* lplpDirectDrawPrimary )
{
    METHOD_PROLOGUE( csGraphics2DDDraw3, XDDraw3GraphicsInfo);
    
    *lplpDirectDrawPrimary = pThis->m_lpddsPrimary;
    return S_OK;
}

STDMETHODIMP IXDDraw3GraphicsInfo::GetDirectDrawBackBuffer( LPDIRECTDRAWSURFACE* lplpDirectDrawBackBuffer )
{
    METHOD_PROLOGUE( csGraphics2DDDraw3, XDDraw3GraphicsInfo);
    
    *lplpDirectDrawBackBuffer = pThis->m_lpddsBack;
    return S_OK;
}

STDMETHODIMP IXDDraw3GraphicsInfo::SetColorPalette(void)
{
    METHOD_PROLOGUE( csGraphics2DDDraw3, XDDraw3GraphicsInfo);
    
    return pThis->SetColorPalette();
}

extern DirectDetectionDevice* DirectDevice;
STDMETHODIMP IXDDraw3GraphicsInfo::GetDirectDetection(IDirectDetectionInternal** lplpDDetection )
{
    METHOD_PROLOGUE( csGraphics2DDDraw3, XDDraw3GraphicsInfo);

    *lplpDDetection = static_cast<IDirectDetectionInternal*>(DirectDevice);
    return S_OK;
}
