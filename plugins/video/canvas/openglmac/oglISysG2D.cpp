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
#include "cs2d/openglmac/oglg2d.h"
#include "cs2d/mac/xsysg2d.h"


IMPLEMENT_COMPOSITE_UNKNOWN (csGraphics2DOpenGL, XMacGraphicsInfo)

STDMETHODIMP IXMacGraphicsInfo::Open( char* szTitle)
{
  METHOD_PROLOGUE( csGraphics2DOpenGL, XMacGraphicsInfo);

  pThis->Open(szTitle);
  return S_OK;
}

STDMETHODIMP IXMacGraphicsInfo::Close()
{
  METHOD_PROLOGUE( csGraphics2DOpenGL, XMacGraphicsInfo);

  pThis->Close();
  return S_OK;
}

STDMETHODIMP IXMacGraphicsInfo::ActivateWindow( WindowPtr theWindow, bool active )
{
    METHOD_PROLOGUE( csGraphics2DOpenGL, XMacGraphicsInfo);
    
	pThis->ActivateWindow( theWindow, active );
    return S_OK;
}

STDMETHODIMP IXMacGraphicsInfo::UpdateWindow( WindowPtr theWindow, bool *updated )
{
    METHOD_PROLOGUE( csGraphics2DOpenGL, XMacGraphicsInfo);
    
	pThis->UpdateWindow( theWindow, updated );
    return S_OK;
}

STDMETHODIMP IXMacGraphicsInfo::PointInWindow( Point *thePoint, bool *inWindow )
{
    METHOD_PROLOGUE( csGraphics2DOpenGL, XMacGraphicsInfo);
    
    pThis->PointInWindow( thePoint, inWindow );
    return S_OK;
}

STDMETHODIMP IXMacGraphicsInfo::SetColorPalette(void)
{
  METHOD_PROLOGUE( csGraphics2DOpenGL, XMacGraphicsInfo);

  pThis->SetColorPalette();
  return S_OK;
}

STDMETHODIMP IXMacGraphicsInfo::DoesDriverNeedEvent( bool *isEnabled )
{
  METHOD_PROLOGUE( csGraphics2DOpenGL, XMacGraphicsInfo);

  pThis->DoesDriverNeedEvent( isEnabled );

  return S_OK;
}

STDMETHODIMP IXMacGraphicsInfo::WindowChanged( void )
{
    METHOD_PROLOGUE( csGraphics2DOpenGL, XMacGraphicsInfo);
    
    pThis->WindowChanged();
    return S_OK;
}

STDMETHODIMP IXMacGraphicsInfo::HandleEvent( EventRecord *inEvent, bool *outEventWasProcessed )
{
    METHOD_PROLOGUE( csGraphics2DOpenGL, XMacGraphicsInfo);
    
    pThis->HandleEvent( inEvent, outEventWasProcessed );
    return S_OK;
}
