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

#include <stdarg.h>
#include "sysdef.h"
#include "cs2d/openglcommon/glcommon2d.h"
#include "cs3d/opengl/ogl_txtmgr.h"
#include "cscom/com.h"
#include "csinput/csevent.h"
#include "csinput/csinput.h"
#include "cssys/unix/iunix.h"
#include "csgeom/csrect.h"
#include "isystem.h"
#include "itexture.h"

csGraphics2DOpenGLFontServer *csGraphics2DGLCommon::LocalFontServer = NULL;
OpenGLTextureCache *csGraphics2DGLCommon::texture_cache = NULL;

// csGraphics2DGLCommon function
csGraphics2DGLCommon::csGraphics2DGLCommon (ISystem* piSystem) :
  csGraphics2D (piSystem)
{
  System = piSystem;
}

void csGraphics2DGLCommon::Initialize ()
{
  csGraphics2D::Initialize ();

  DrawPixel = DrawPixelGL;
  WriteChar = WriteCharGL;
  GetPixelAt = GetPixelAtGL;
  DrawSprite = DrawSpriteGL;
}


csGraphics2DGLCommon::~csGraphics2DGLCommon ()
{
  // Destroy your graphic interface
  Close ();
}

bool csGraphics2DGLCommon::Open(const char *Title)
{
  if (glGetString (GL_RENDERER))
    CsPrintf (MSG_INITIALIZATION, "Renderer %s ", glGetString(GL_RENDERER) );
  if (glGetString (GL_VERSION))
    CsPrintf (MSG_INITIALIZATION, "Version %s", glGetString(GL_VERSION));
  CsPrintf (MSG_INITIALIZATION, "\n");

  // Open your graphic interface
  if (!csGraphics2D::Open (Title))
    return false;

  // load font 'server'
  if (LocalFontServer == NULL)
  {
       CsPrintf(MSG_INITIALIZATION,"Loading fonts...");
       LocalFontServer = new csGraphics2DOpenGLFontServer(&FontList[0]);
       for (int fontindex=1; 
       		fontindex < 8;
		fontindex++)
       {
       	   CsPrintf(MSG_INITIALIZATION,"%d...",fontindex);
	   LocalFontServer->AddFont(FontList[fontindex]);
       }
       CsPrintf(MSG_INITIALIZATION,"\n");
  }

  // make our own local texture cache for 2D sprites
  if (texture_cache == NULL)
  {
    CHK (texture_cache = new OpenGLTextureCache(1<<24,24));
  }

  Clear (0);
  return true;
}

void csGraphics2DGLCommon::Close(void)
{
  // Close your graphic interface
  csGraphics2D::Close ();
//  CHK (delete [] Memory);
  CHK (delete LocalFontServer);
  LocalFontServer = NULL;
  CHK (delete texture_cache);
  texture_cache = NULL;
}

void csGraphics2DGLCommon::Clear(int color)
{
  switch (pfmt.PixelBytes)
  {
  case 1: // paletted colors
    glClearColor(Palette[color].red,
    		Palette[color].green,
		Palette[color].blue,0.);
    break;
  case 2: // 16bit color
  case 4: // truecolor
    glClearColor( ( (color & pfmt.RedMask) >> pfmt.RedShift )     / (float)pfmt.RedBits,
               ( (color & pfmt.GreenMask) >> pfmt.GreenShift ) / (float)pfmt.GreenBits,
               ( (color & pfmt.BlueMask) >> pfmt.BlueShift )   / (float)pfmt.BlueBits,
	       0. );
    break;
  }
  glClear(GL_COLOR_BUFFER_BIT);
}


void csGraphics2DGLCommon::SetRGB(int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
}


void csGraphics2DGLCommon::setGLColorfromint(int color)
{
  switch (pfmt.PixelBytes)
  {
  case 1: // paletted colors
    glColor3i(Palette[color].red,
    		Palette[color].green,
		Palette[color].blue);
    break;
  case 2: // 16bit color
  case 4: // truecolor
    glColor3f( ( (color & pfmt.RedMask) >> pfmt.RedShift )     / (float)pfmt.RedBits,
               ( (color & pfmt.GreenMask) >> pfmt.GreenShift ) / (float)pfmt.GreenBits,
               ( (color & pfmt.BlueMask) >> pfmt.BlueShift )   / (float)pfmt.BlueBits);
    break;
  }
}

void csGraphics2DGLCommon::DrawLine (int x1, int y1, int x2, int y2, int color)
{
  // prepare for 2D drawing--so we need no fancy GL effects!
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  setGLColorfromint(color);

  glBegin (GL_LINES);
  glVertex2i (x1, Height-y1-1);
  glVertex2i (x2, Height-y2-1);
  glEnd ();
}

void csGraphics2DGLCommon::DrawBox (int x, int y, int w, int h, int color)
{
  // prepare for 2D drawing--so we need no fancy GL effects!
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  setGLColorfromint(color);

  glBegin (GL_QUADS);
  glVertex2i (x, Height - y - 1);
  glVertex2i (x + w - 1, Height - y - 1);
  glVertex2i (x + w - 1, Height - (y + h - 1) - 1);
  glVertex2i (x, Height - (y + h - 1) - 1);
  glEnd ();
}

void csGraphics2DGLCommon::DrawPixelGL (int x, int y, int color)
{
  // prepare for 2D drawing--so we need no fancy GL effects!
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  setGLColorfromint(color);

  glBegin (GL_POINTS);
  glVertex2i (x, Height-y-1);
  glEnd ();
}

void csGraphics2DGLCommon::WriteCharGL (int x, int y, int fg, int /*bg*/, char c)
{
  // prepare for 2D drawing--so we need no fancy GL effects!
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  
  setGLColorfromint(fg);

  // in fact the WriteCharacter() method properly shifts over
  // the current modelview transform on each call, so that characters
  // are drawn left-to-write.  But we bypass that because we know the
  // exact x,y location of each letter.  We manipulate the transform
  // directly, so any shift in WriteCharacter() is effectively ignored
  // due to the Push/PopMatrix calls

  glPushMatrix();
  glTranslatef (x, Height-y-FontList[Font].Height,0.0);

  LocalFontServer->WriteCharacter(c,Font);
  glPopMatrix();
}

void csGraphics2DGLCommon::DrawSpriteGL (ITextureHandle *hTex, int sx, int sy,
  int sw, int sh, int tx, int ty, int tw, int th)
{
  texture_cache->Add (hTex);

  // cache the texture if we haven't already.
  csTextureMMOpenGL* txt_mm = (csTextureMMOpenGL*)GetcsTextureMMFromITextureHandle (hTex);

  HighColorCache_Data *cachedata;
  cachedata = txt_mm->get_hicolorcache ();
  GLuint texturehandle = *( (GLuint *) (cachedata->pData) );

  // as we are drawing in 2D, we disable some of the commonly used features
  // for fancy 3D drawing
  glShadeModel(GL_FLAT);
  glDisable (GL_DEPTH_TEST);

  // if the texture has transparent bits, we have to tweak the
  // OpenGL blend mode so that it handles the transparent pixels correctly
  if (txt_mm->get_transparent())
  {
    glEnable (GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  }
  else
    glDisable (GL_BLEND);

  glEnable(GL_TEXTURE_2D);
  glColor4f(1.,1.,1.,1.);
  glBindTexture(GL_TEXTURE_2D,texturehandle);
  
  int bitmapwidth=0, bitmapheight=0;
  hTex->GetBitmapDimensions(bitmapwidth,bitmapheight);

  // convert texture coords given above to normalized (0-1.0) texture coordinates
  float ntx1,nty1,ntx2,nty2;
  ntx1 = tx/bitmapwidth;
  ntx2 = (tx+tw)/bitmapwidth;
  nty1 = ty/bitmapheight;
  nty2 = (ty+th)/bitmapheight;

  // draw the bitmap - we could use GL_QUADS, but why?
  glBegin(GL_TRIANGLE_FAN);
  glTexCoord2f(ntx1,nty1);
  glVertex2i(sx,Height-sy-1);
  glTexCoord2f(ntx2,nty1);
  glVertex2i(sx+sw,Height-sy-1);
  glTexCoord2f(ntx2,nty2);
  glVertex2i(sx+sw,Height-sy-sh-1);
  glTexCoord2f(ntx1,nty2);
  glVertex2i(sx,Height-sy-sh-1);
  glEnd();
}

unsigned char* csGraphics2DGLCommon::GetPixelAtGL (int /*x*/, int /*y*/)
{
  return NULL;
}

// Used to printf through system driver
void csGraphics2DGLCommon::CsPrintf (int msgtype, const char *format, ...)
{
  va_list arg;
  char buf[256];

  va_start (arg, format);
  vsprintf (buf, format, arg);
  va_end (arg);

  System->Print (msgtype, buf);
}

