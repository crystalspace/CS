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
#include "cs2d/ddraw6/g2d.h"
#include "cs2d/ddraw6/xg2d.h"
#include "cssys/win32/DirectDetection.h"


IMPLEMENT_COMPOSITE_UNKNOWN(csGraphics2DDDraw6, XDDraw6GraphicsInfo)

STDMETHODIMP IXDDraw6GraphicsInfo::Open(const char* szTitle)
{
    METHOD_PROLOGUE( csGraphics2DDDraw6, XDDraw6GraphicsInfo);
    
    pThis->Open(szTitle);
    return S_OK;
}

STDMETHODIMP IXDDraw6GraphicsInfo::Close()
{
    METHOD_PROLOGUE( csGraphics2DDDraw6, XDDraw6GraphicsInfo);
    
    pThis->Close();
    return S_OK;
}

STDMETHODIMP IXDDraw6GraphicsInfo::GetDirectDrawDriver( LPDIRECTDRAW* lplpDirectDraw )
{
    METHOD_PROLOGUE( csGraphics2DDDraw6, XDDraw6GraphicsInfo);
    
    *lplpDirectDraw = pThis->m_lpDD;
    return S_OK;
}

STDMETHODIMP IXDDraw6GraphicsInfo::GetDirectDrawDriver( LPDIRECTDRAW4* lplpDirectDraw )
{
    METHOD_PROLOGUE( csGraphics2DDDraw6, XDDraw6GraphicsInfo);
    
    *lplpDirectDraw = pThis->m_lpDD4;
    return S_OK;
}

STDMETHODIMP IXDDraw6GraphicsInfo::GetDirectDrawPrimary( LPDIRECTDRAWSURFACE4* lplpDirectDrawPrimary )
{
    METHOD_PROLOGUE( csGraphics2DDDraw6, XDDraw6GraphicsInfo);
    
    *lplpDirectDrawPrimary = pThis->m_lpddsPrimary;
    return S_OK;
}

STDMETHODIMP IXDDraw6GraphicsInfo::GetDirectDrawBackBuffer( LPDIRECTDRAWSURFACE4* lplpDirectDrawBackBuffer )
{
    METHOD_PROLOGUE( csGraphics2DDDraw6, XDDraw6GraphicsInfo);
    
    *lplpDirectDrawBackBuffer = pThis->m_lpddsBack;
    return S_OK;
}

STDMETHODIMP IXDDraw6GraphicsInfo::SetColorPalette(void)
{
    METHOD_PROLOGUE( csGraphics2DDDraw6, XDDraw6GraphicsInfo);
    
    return pThis->SetColorPalette();
}

extern DirectDetectionDevice* DirectDevice;
STDMETHODIMP IXDDraw6GraphicsInfo::GetDirectDetection(IDirectDetectionInternal** lplpDDetection )
{
    METHOD_PROLOGUE( csGraphics2DDDraw6, XDDraw6GraphicsInfo);

    *lplpDDetection = static_cast<IDirectDetectionInternal*>(DirectDevice);
    return S_OK;
}
