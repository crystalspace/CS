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
#include "ogl_proctexback.h"
#include "ogl_g3dcom.h"
#include "csgeom/polyclip.h"

IMPLEMENT_IBASE (csOpenGLProcBackBuffer)
  IMPLEMENTS_INTERFACE (iGraphics3D)
IMPLEMENT_IBASE_END;

extern void csglBindTexture (GLenum target, GLuint handle);

csOpenGLProcBackBuffer::csOpenGLProcBackBuffer (iBase *parent) :
  csGraphics3DOGLCommon ()
{ 
  CONSTRUCT_IBASE (parent);
  tex = NULL; 
  g3d = NULL;
  System = NULL;

  rstate_bilinearmap = false;
}

csOpenGLProcBackBuffer::~csOpenGLProcBackBuffer ()
{
  // Our dummy 2d driver will get destroyed lower down in the class hierachy
  // NULL out those members which are shared with the frame buffer interfaces
  // so they don't get deleted.
  txtmgr = NULL;
  texture_cache = NULL;
  lightmap_cache = NULL;
  m_fogtexturehandle = 0;
}

void csOpenGLProcBackBuffer::Prepare (csGraphics3DOGLCommon *g3d, 
			     csTextureMMOpenGL *tex, csPixelFormat *ipfmt)
{ 
  memcpy (&pfmt, ipfmt, sizeof(csPixelFormat));
  this->g3d = g3d;
  this->tex = tex;
  g2d = g3d->GetDriver2D ();

  tex->GetMipMapDimensions(0, width, height);

  G2D = new csOpenGLProcBackBuffer2D (g2d, width, height);
  pixel_bytes = g2d->GetPixelBytes ();
  frame_height = g2d->GetHeight ();
  frame_width = g2d->GetWidth ();

  tex_0 = (csTextureProcOpenGL*) tex->get_texture (0);

  SharedInitialize (g3d);
  SharedOpen (g3d);
}

bool csOpenGLProcBackBuffer::BeginDraw (int DrawFlags)
{
  // Optimise: try and use DrawBox to clear screen
  // if 2D graphics is not locked, lock it
  if ((DrawFlags & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
      && (!(DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))))
  {
    if (!G2D->BeginDraw ())
      return false;

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glOrtho (0., (GLdouble) width, (GLdouble) height, 0., -1.0, 10.0);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    glColor3f (1., 0., 0.);
    glClearColor (0., 0., 0., 0.);
    dbg_current_polygon = 0;
  }

  if (DrawFlags & CSDRAW_CLEARZBUFFER)
  {
    if (DrawFlags & CSDRAW_CLEARSCREEN)
      glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    else
      glClear (GL_DEPTH_BUFFER_BIT);
  }
  else if (DrawFlags & CSDRAW_CLEARSCREEN)
    G2D->Clear (0);

  DrawMode = DrawFlags;

  end_draw_poly ();

  // copy the tex into lower left corner
  g3d->DrawPixmap (tex, 0, 0, width, height, 0, 0,
		   width, height);
  glViewport(0,0,width,height);

  return true;
}

void csOpenGLProcBackBuffer::FinishDraw ()
{
  csGraphics3DOGLCommon::FinishDraw ();
  glViewport (0,0,frame_width, frame_height);
}

void csOpenGLProcBackBuffer::Print (csRect *area)
{
  // Optimise: copy over area only
  (void)area;
  glEnable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  glDisable (GL_DITHER);
  glDisable (GL_ALPHA_TEST);

  csGLCacheData *tex_data = (csGLCacheData*) tex->GetCacheData();

  if (tex_data)
  {
    // Currently Mesa3.x has bugs which affects the voodoo2
    // Copying the framebuffer to make a new texture will work with the voodoo2
    // as of Mesa CVS 6/6/2000. Unfortunately this change has broken the 
    // glCopyTexSubImage command, so I cant test this with just updating
    // the rectangle as it is intended to be.
    glDeleteTextures (1, &tex_data->Handle);
    tex_data->Handle = 0;
    glGenTextures (1, &tex_data->Handle);
    // Texture is in tha cache, update texture directly. 
    csglBindTexture (GL_TEXTURE_2D, tex_data->Handle);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
		     rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
		     rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);

    glCopyTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 0,0,width,height,0); 
//      glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
  }
  else
  {
    // Not in cache.
#ifdef GL_VERSION_1_2
    if (pfmt.PixelBytes == 2)
	{
	  glReadPixels (0,0, width, height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
			tex_0->get_image_data());
	}
    else
#endif
      glReadPixels (0, 0, width, height,
		    GL_RGB, GL_UNSIGNED_BYTE, tex_0->get_image_data());
  }
}

void csOpenGLProcBackBuffer::DrawPixmap (iTextureHandle *hTex, int sx, 
  int sy, int sw, int sh,int tx, int ty, int tw, int th)
{
  csGraphics3DOGLCommon::DrawPixmap (hTex, sx, sy, sw, sh, tx, ty, tw, th); 
}

float csOpenGLProcBackBuffer::GetZBuffValue (int x, int y)
{
  GLfloat zvalue;
  glReadPixels (x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zvalue);
  // 0.090909=1/11, that is 1 divided by total depth delta set by
  // glOrtho. Where 0.090834 comes from, I don't know
  return (0.090834 / (zvalue - (0.090909)));
}
//---------------------------------------------------------------------------
// Dummy iGraphics2D
//---------------------------------------------------------------------------

IMPLEMENT_IBASE (csOpenGLProcBackBuffer2D)
  IMPLEMENTS_INTERFACE (iGraphics2D)
IMPLEMENT_IBASE_END;

csOpenGLProcBackBuffer2D::csOpenGLProcBackBuffer2D 
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

csOpenGLProcBackBuffer2D::~csOpenGLProcBackBuffer2D ()
{ 
  g2d->DecRef (); 
}

void csOpenGLProcBackBuffer2D::SetFontID (int FontID)
{
  font = FontID;
  g2d->SetFontID (FontID); 
}

void csOpenGLProcBackBuffer2D::DrawLine (float x1, float y1, 
					    float x2, float y2, int color)
{ 
  g2d->DrawLine (x1, height - y1, x2, height - y2, color); 
}

void csOpenGLProcBackBuffer2D::DrawBox (int x, int y, int w, int h, 
					   int color)
{
    g2d->DrawBox (x, height - y, w, h, color); 
}

void csOpenGLProcBackBuffer2D::DrawPixel (int x, int y, int color)
{ 
  g2d->DrawPixel (x, height - y, color); 
}

unsigned char *csOpenGLProcBackBuffer2D::GetPixelAt (int x, int y)
{ 
  return g2d->GetPixelAt (x, height - y); 
}

csImageArea *csOpenGLProcBackBuffer2D::SaveArea (int x, int y, int w, int h)
{ 
  return g2d->SaveArea (x, height - y, w, h); 
}

void csOpenGLProcBackBuffer2D::Write (int x, int y, int fg, int bg, 
					 const char *str)
{ 
  g2d->Write (x, height - y - g2d->GetTextHeight (font), fg, bg, str); 
}


void csOpenGLProcBackBuffer2D::WriteChar (int x, int y, 
					     int fg, int bg, char c)
{
  g2d->WriteChar (x, height - y - g2d->GetTextHeight (font), fg, bg, c); 
}


int csOpenGLProcBackBuffer2D::GetWidth ()
{ 
  return width; 
}

int csOpenGLProcBackBuffer2D::GetHeight ()
{ 
  return height; 
}

void csOpenGLProcBackBuffer2D::GetPixel (int x, int y, 
					    UByte &oR, UByte &oG, UByte &oB)
{ 
  g2d->GetPixel (x, height - y, oR, oG, oB); 
}
