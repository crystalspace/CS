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
#include "cs2d/beglide2/glidebe2d.h"

IMPLEMENT_COMPOSITE_UNKNOWN(csGraphics2DBeGlide, XGlide2xGraphicsInfo)
//STDMETHODIMP IXGlide2xGraphicsInfo::QueryInterface (REFIID riid, void** ppv) 
//{ METHOD_PROLOGUE(csGraphics2DBeGlide, XGlide2xGraphicsInfo) return pThis->QueryInterface (riid, ppv); } 
//STDMETHODIMP_(ULong) IXGlide2xGraphicsInfo::AddRef () 
//{ METHOD_PROLOGUE(csGraphics2DBeGlide, XGlide2xGraphicsInfo) return pThis->AddRef (); } 
//STDMETHODIMP_(ULong) IXGlide2xGraphicsInfo::Release () 
//{ METHOD_PROLOGUE(csGraphics2DBeGlide, XGlide2xGraphicsInfo) return pThis->Release (); } 

STDMETHODIMP IXGlide2xGraphicsInfo::Open(const char* szTitle)
{
  METHOD_PROLOGUE( csGraphics2DBeGlide, XGlide2xGraphicsInfo);
  
  pThis->Open(szTitle);
  return S_OK;
}

STDMETHODIMP IXGlide2xGraphicsInfo::Close()
{
  METHOD_PROLOGUE( csGraphics2DBeGlide, XGlide2xGraphicsInfo);
  
  pThis->Close();
  return S_OK;
}

STDMETHODIMP IXGlide2xGraphicsInfo::GetBDirectWindow(CrystGlideWindow ** win)
{
    METHOD_PROLOGUE( csGraphics2DBeGlide, XGlide2xGraphicsInfo);
    
    *win = pThis->window;
    return S_OK;
}
