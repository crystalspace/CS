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
// Texture-related interfaces.

#include "sysdef.h"
#include "cs3d/direct3d5/d3d_txtmgr.h"
#include "itexture.h"

// ITextureHandle interface.

IMPLEMENT_COMPOSITE_UNKNOWN (csTextureMM, TextureHandle)

IMPLEMENT_GET_PROPERTY (GetTransparent, istransp, bool, csTextureMM, TextureHandle)
IMPLEMENT_GET_PROPERTY (GetMeanColor, mean_idx, int, csTextureMM, TextureHandle)
IMPLEMENT_GET_PROPERTY (GetNumberOfColors, usage->get_num_colors(), int, csTextureMM, TextureHandle)
 
STDMETHODIMP ITextureHandle::SetTransparent (int red, int green, int blue) 
{ 
  METHOD_PROLOGUE (csTextureMM, TextureHandle)
  pThis->set_transparent (red, green, blue);
  return S_OK; 
}

STDMETHODIMP ITextureHandle::GetMipMapDimensions (int mipmap, int& w, int& h) 
{ 
  METHOD_PROLOGUE (csTextureMM, TextureHandle)
  csTexture* txt = pThis->get_texture (mipmap);
  w = txt->get_width ();
  h = txt->get_height ();
  return S_OK; 
}

STDMETHODIMP ITextureHandle::GetBitmapDimensions (int& w, int& h) 
{ 
  METHOD_PROLOGUE (csTextureMM, TextureHandle)
  csTexture* txt = pThis->get_texture (-1);
  w = txt->get_width ();
  h = txt->get_height ();
  return S_OK; 
}

STDMETHODIMP ITextureHandle::GetBitmapData (void** bmdata)
{ 
  METHOD_PROLOGUE (csTextureMM, TextureHandle)
  csTexture* txt = pThis->get_texture (-1);
  *bmdata = (void*)(txt->get_bitmap8 ());
  return S_OK; 
}

STDMETHODIMP ITextureHandle::GetMeanColor (float& r, float& g, float& b)
{ 
  METHOD_PROLOGUE (csTextureMM, TextureHandle)
  r = pThis->mean_color.red;
  g = pThis->mean_color.green;
  b = pThis->mean_color.blue;
  return S_OK; 
}

