/*
    Copyright (C) 2000  by Samuel Humphreys 
    Based on the glide implementation by Norman Krämer
  
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
#include "cssysdef.h"
#include <GL/gl.h>
#include "ogl_txtcache.h"
#include "ogl_txtmgr.h"
#include "ogl_dyntexback.h"
#include "ogl_g3dcom.h"
#include "csgeom/polyclip.h"

IMPLEMENT_IBASE (csOpenGLDynamicBackBuffer)
  IMPLEMENTS_INTERFACE (iGraphics3D)
IMPLEMENT_IBASE_END;

#define COPY_TO_TEXTURE

csOpenGLDynamicBackBuffer::csOpenGLDynamicBackBuffer (iBase *parent) :
  csGraphics3DOGLCommon ()
{ 
  CONSTRUCT_IBASE (parent);
  tex = NULL; 
  g3d = NULL;
  System = NULL;

  rstate_bilinearmap = false;
}

csOpenGLDynamicBackBuffer::~csOpenGLDynamicBackBuffer ()
{
  // Our dummy 2d driver will get destroyed lower down in the class hierachy
  // NULL out those members which are shared with the frame buffer interfaces
  // so they don't get deleted.
  txtmgr = NULL;
  texture_cache = NULL;
  lightmap_cache = NULL;
  m_fogtexturehandle = 0; 
}

void csOpenGLDynamicBackBuffer::SetTarget (csGraphics3DOGLCommon *g3d, csTextureMMOpenGL *tex)
{ 
  this->g3d = g3d;
  this->tex = tex;
  g2d = g3d->GetDriver2D ();

  tex->GetMipMapDimensions(0, width, height);

  G2D = new csOpenGLDynamicBackBuffer2D (g2d, width, height);
  pixel_bytes = g2d->GetPixelBytes ();
  frame_height = g2d->GetHeight ();

  tex_0 = (csTextureOpenGLDynamic*) tex->get_texture (0);

  SharedInitialize (g3d);
  SharedOpen (g3d);
}

bool csOpenGLDynamicBackBuffer::BeginDraw (int DrawFlags)
{
  // Optimise: try and use DrawBox to clear screen
  bool succ = csGraphics3DOGLCommon::BeginDraw (DrawFlags);
  if (succ)
  {
    // copy the tex into upper left corner
    g3d->DrawPixmap (tex, 0, 0, width, height, 0, 0,
		     width, height);
  }
  return succ;
}

void csOpenGLDynamicBackBuffer::Print (csRect *area)
{
  // Optimise: copy over area only
  (void)area;
  glEnable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  glDisable (GL_DITHER);
  glDisable (GL_ALPHA_TEST);
  glPixelZoom (-1,0);
  csGLCacheData *tex_data = (csGLCacheData*) tex->GetCacheData();
  if (tex_data)
  {
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    // Texture is in tha cache, update texture directly.
#ifdef COPY_TO_TEXTURE
    glDeleteTextures(1, &tex_data->Handle);
    tex_data->Handle = 0;
    glGenTextures (1, &tex_data->Handle);
#endif

    glBindTexture (GL_TEXTURE_2D, tex_data->Handle);

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
		     rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
		     rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);

#ifdef COPY_TO_TEXTURE
    // We know currently dynamic textures are not mip-mapped
//      glPixelStorei (GL_UNPACK_ROW_LENGTH, g2d->GetWidth ());
    glCopyTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 
		      0, 0, width, height, 0);
#else
//      glPixelStorei (GL_PACK_ROW_LENGTH, g2d->GetWidth ());

    glCopyTexSubImage2D (GL_TEXTURE_2D, 0,
			 0, 0,                 /* texture offset */ 
			 0, 0, width, height); /* frame buffer */
#endif
  }
  else
  {
    // Not in cache.
    glReadPixels (0, height, width, height,
		  GL_RGBA, GL_UNSIGNED_BYTE, tex_0->get_image_data());
  }
}

void csOpenGLDynamicBackBuffer::DrawPixmap (iTextureHandle *hTex, int sx, 
  int sy, int sw, int sh,int tx, int ty, int tw, int th)
{
  csGraphics3DOGLCommon::DrawPixmap (hTex, sx, sy, sw, sh, tx, ty, tw, th); 
}

iGraphics3D *csOpenGLDynamicBackBuffer::CreateOffScreenRenderer 
  (iGraphics3D *parent_g3d, int width, int height, csPixelFormat *pfmt, 
   void *buffer, RGBPixel *palette, int pal_size)
{ 
  (void) parent_g3d;
  (void) width;
  (void) height;
  (void) pfmt;
  (void) buffer;
  (void) palette;
  (void) pal_size;

  return NULL;
}

//---------------------------------------------------------------------------
// Dummy iGraphics2D
//---------------------------------------------------------------------------

IMPLEMENT_IBASE (csOpenGLDynamicBackBuffer2D)
  IMPLEMENTS_INTERFACE (iGraphics2D)
IMPLEMENT_IBASE_END;

csOpenGLDynamicBackBuffer2D::csOpenGLDynamicBackBuffer2D 
    (iGraphics2D *ig2d, int iwidth, int iheight)
{  
  CONSTRUCT_IBASE (NULL);
  g2d = ig2d; 
  width = iwidth;
  height = iheight; 
  frame_height = g2d->GetHeight (); 
  font = g2d->GetFontID ();
  g2d->IncRef (); 
}

csOpenGLDynamicBackBuffer2D::~csOpenGLDynamicBackBuffer2D ()
{ 
  g2d->DecRef (); 
}

void csOpenGLDynamicBackBuffer2D::SetFontID (int FontID)
{
  font = FontID;
  g2d->SetFontID (FontID); 
}

void csOpenGLDynamicBackBuffer2D::DrawLine (float x1, float y1, 
					    float x2, float y2, int color)
{ 
  g2d->DrawLine (x1, frame_height - y1, x2, frame_height - y2, color); 
}

void csOpenGLDynamicBackBuffer2D::DrawBox (int x, int y, int w, int h, 
					   int color)
{
    g2d->DrawBox (x, frame_height - y, w, h, color); 
}

void csOpenGLDynamicBackBuffer2D::DrawPixel (int x, int y, int color)
{ 
  g2d->DrawPixel (x, frame_height - y, color); 
}

unsigned char *csOpenGLDynamicBackBuffer2D::GetPixelAt (int x, int y)
{ 
  return g2d->GetPixelAt (x, frame_height - y); 
}

csImageArea *csOpenGLDynamicBackBuffer2D::SaveArea (int x, int y, int w, int h)
{ 
  return g2d->SaveArea (x, frame_height - y, w, h); 
}

void csOpenGLDynamicBackBuffer2D::Write (int x, int y, int fg, int bg, 
					 const char *str)
{ 
  g2d->Write (x, frame_height - y - g2d->GetTextHeight (font), fg, bg, str); 
}


void csOpenGLDynamicBackBuffer2D::WriteChar (int x, int y, 
					     int fg, int bg, char c)
{
  g2d->WriteChar (x, frame_height - y - g2d->GetTextHeight (font), fg, bg, c); 
}


int csOpenGLDynamicBackBuffer2D::GetWidth ()
{ 
  return width; 
}

int csOpenGLDynamicBackBuffer2D::GetHeight ()
{ 
  return height; 
}

void csOpenGLDynamicBackBuffer2D::GetPixel (int x, int y, 
					    UByte &oR, UByte &oG, UByte &oB)
{ 
  g2d->GetPixel (x, frame_height - y, oR, oG, oB); 
}
