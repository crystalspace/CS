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
#include "cs2d/winglide3/g2d.h"
#include "cs2d/winglide3/xg2d.h"


IMPLEMENT_COMPOSITE_UNKNOWN(csGraphics2DGlide3x, XGlide3xGraphicsInfo)

STDMETHODIMP IXGlide3xGraphicsInfo::Open(const char* szTitle)
{
  METHOD_PROLOGUE( csGraphics2DGlide3x, XGlide3xGraphicsInfo);
  
  pThis->Open(szTitle);
  return S_OK;
}

STDMETHODIMP IXGlide3xGraphicsInfo::Close()
{
  METHOD_PROLOGUE( csGraphics2DGlide3x, XGlide3xGraphicsInfo);
  
  pThis->Close();
  return S_OK;
}

#if defined(OS_WIN32)
STDMETHODIMP IXGlide3xGraphicsInfo::GethWnd(HWND * hWnd)
{
    METHOD_PROLOGUE( csGraphics2DGlide3x, XGlide3xGraphicsInfo);
    
    *hWnd = pThis->m_hWnd;
    return S_OK;
}
#endif
