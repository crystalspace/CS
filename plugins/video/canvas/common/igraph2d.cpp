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
#include "cs2d/common/xgraph2d.h"
#include "cs2d/common/graph2d.h"
#include "igraph2d.h"

IMPLEMENT_COMPOSITE_UNKNOWN( csGraphics2D, XGraphics2D )

STDMETHODIMP IXGraphics2D::BeginDraw()
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  return pThis->BeginDraw() ? S_OK : E_FAIL;
}

STDMETHODIMP IXGraphics2D::Close()
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  pThis->Close();
  return S_OK;
}

STDMETHODIMP IXGraphics2D::Clear(int color)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  pThis->Clear(color);
  return S_OK;
}

STDMETHODIMP IXGraphics2D::ClearAll(int color)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  pThis->ClearAll(color);
  return S_OK;
}

STDMETHODIMP IXGraphics2D::ClipLine(float& x1, float& y1, float& x2, float& y2,
  int xmin, int ymin, int xmax, int ymax)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  return pThis->ClipLine(x1, y1, x2, y2, xmin, ymin, xmax, ymax) ? S_OK : S_FALSE;
}

STDMETHODIMP IXGraphics2D::DoubleBuffer(bool Enable)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  return pThis->DoubleBuffer(Enable) ? S_OK : S_FALSE;
}

STDMETHODIMP IXGraphics2D::DrawHorizLine(int x1, int x2, int y, int color)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  pThis->DrawHorizLine(x1, x2, y, color);
  return S_OK;
}

STDMETHODIMP IXGraphics2D::DrawLine(float x1, float y1, float x2, float y2, int color)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  pThis->DrawLine(x1, y1, x2, y2, color);
  return S_OK;
}


STDMETHODIMP IXGraphics2D::DrawPixel(int x, int y, int color)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  (pThis->DrawPixel)(x, y, color);
  return S_OK;
}

STDMETHODIMP IXGraphics2D::DrawSprite(ITextureHandle *hTex, int sx, int sy, int sw, int sh, int tx, int ty, int tw, int th)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  (pThis->DrawSprite)( hTex, sx, sy, sw, sh, tx, ty, tw, th );
  return S_OK;
}

STDMETHODIMP IXGraphics2D::FinishDraw()
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  pThis->FinishDraw();
  return S_OK;
}

STDMETHODIMP IXGraphics2D::FreeArea(ImageArea* Area)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  pThis->FreeArea(Area);
  return S_OK;
}

STDMETHODIMP IXGraphics2D::GetClipRect( int& nMinX, int& nMinY, int& nMaxX, int& nMaxY )
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  pThis->GetClipRect(nMinX, nMinY, nMaxX, nMaxY);
  return S_OK;
}

STDMETHODIMP IXGraphics2D::GetDoubleBufferState(bool& State)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  State = pThis->DoubleBuffer();
  return S_OK;
}

STDMETHODIMP IXGraphics2D::GetFontID(int& id)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  id = pThis->Font;
  return S_OK;
}

STDMETHODIMP IXGraphics2D::GetPixelAt(int x, int y, unsigned char** pPixel)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  *pPixel = (pThis->GetPixelAt)(x, y);
  return S_OK;
}

STDMETHODIMP IXGraphics2D::GetStringError(HRESULT hRes, char* szErrorString)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  pThis->GetStringError(hRes, szErrorString);
  return S_OK;
}

STDMETHODIMP IXGraphics2D::Initialize ()
{
  METHOD_PROLOGUE (csGraphics2D, XGraphics2D);
  pThis->Initialize ();
  return S_OK;
}

STDMETHODIMP IXGraphics2D::Open(char* Title)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  if (!pThis->Open(Title))
    return E_FAIL;
  return S_OK;
}

STDMETHODIMP IXGraphics2D::Print(csRect* pArea)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  pThis->Print(pArea);
  return S_OK;
}

STDMETHODIMP IXGraphics2D::RestoreArea(ImageArea* Area, bool bFree)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  pThis->RestoreArea(Area, bFree);
  return S_OK;
}

STDMETHODIMP IXGraphics2D::SaveArea(ImageArea** Area, int x, int y, int w, int h)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  pThis->SaveArea(*Area, x, y, w, h);
  return S_OK;
}

STDMETHODIMP IXGraphics2D::SetClipRect(int nMinX, int nMinY, int nMaxX, int nMaxY)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  pThis->SetClipRect(nMinX, nMinY, nMaxX, nMaxY);
  return S_OK;
}

STDMETHODIMP IXGraphics2D::SetFontID(int id)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  pThis->Font = id;
  return S_OK;
}

STDMETHODIMP IXGraphics2D::SetMousePosition (int x, int y)
{
  METHOD_PROLOGUE (csGraphics2D, XGraphics2D);
  return pThis->SetMousePosition (x, y) ? S_OK : E_FAIL;
}

STDMETHODIMP IXGraphics2D::SetMouseCursor (csMouseCursorID Shape, ITextureHandle *bmp_handle)
{
  METHOD_PROLOGUE (csGraphics2D, XGraphics2D);
  return pThis->SetMouseCursor (Shape, bmp_handle) ? S_OK : E_FAIL;
}

STDMETHODIMP IXGraphics2D::SetRGB( int i, int r, int g, int b )
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  pThis->SetRGB(i, r, g, b);
  return S_OK;
}

STDMETHODIMP IXGraphics2D::Write(int x, int y, int fg, int bg, char* str)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  pThis->Write(x, y, fg, bg, str);
  return S_OK;
}

STDMETHODIMP IXGraphics2D::WriteChar(int x, int y, int fg, int bg, char c)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  (pThis->WriteChar)(x, y, fg, bg, c);
  return S_OK;
}

STDMETHODIMP IXGraphics2D::PerformExtension( char* args)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphics2D );
  return pThis->PerformExtension (args) ? S_OK : E_FAIL;
}

    

// IXGraphics2D implementation 

IMPLEMENT_COMPOSITE_UNKNOWN( csGraphics2D, XGraphicsInfo )

STDMETHODIMP IXGraphicsInfo::GetFontID(int& nID)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphicsInfo );
  nID = pThis->Font;
  return S_OK;
}

STDMETHODIMP IXGraphicsInfo::GetHeight(int& nHeight)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphicsInfo );
  nHeight = pThis->Height;
  return S_OK;
}

STDMETHODIMP IXGraphicsInfo::GetIsFullScreen(bool& retval)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphicsInfo );
  retval = pThis->FullScreen;
  return S_OK;
}

STDMETHODIMP IXGraphicsInfo::GetNumPalEntries(int& retval)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphicsInfo );
  retval = pThis->GetNumPalEntries();
  return S_OK;
}

STDMETHODIMP IXGraphicsInfo::GetPage(int& retval)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphicsInfo );
  retval = pThis->GetPage();
  return S_OK;
}

STDMETHODIMP IXGraphicsInfo::GetPixelBytes(int& retval)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphicsInfo );
  retval = pThis->GetPixelBytes();
  return S_OK;
}

STDMETHODIMP IXGraphicsInfo::GetPixelFormat(csPixelFormat* PixelFormat)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphicsInfo );
  *PixelFormat = *pThis->GetPixelFormat();
  return S_OK;
}

STDMETHODIMP IXGraphicsInfo::GetStringError(HRESULT hRes, char* szErrorString)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphicsInfo );
  (void) pThis;
  GetStringError(hRes, szErrorString);
  return S_OK;
}

STDMETHODIMP IXGraphicsInfo::GetWidth(int& retval)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphicsInfo );
  retval = pThis->Width;
  return S_OK;
}

STDMETHODIMP IXGraphicsInfo::GetPalette(RGBpaletteEntry** pPalette)
{
  METHOD_PROLOGUE( csGraphics2D, XGraphicsInfo );
  (void) pThis;
  *pPalette = csGraphics2D::Palette;
  return S_OK;
}

STDMETHODIMP IXGraphicsInfo::GetTextWidth (int &Width, int Font, char *text)
{
  METHOD_PROLOGUE (csGraphics2D, XGraphicsInfo);
  Width = pThis->GetTextWidth (Font, text);
  return S_OK;
}

STDMETHODIMP IXGraphicsInfo::GetTextHeight (int &Height, int Font)
{
  METHOD_PROLOGUE (csGraphics2D, XGraphicsInfo);
  Height = pThis->GetTextHeight (Font);
  return S_OK;
}
