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
#include "cs2d/glide2common/glide2common2d.h"
#include "cs3d/glide2/gl_txtmgr.h"
#include "csutil/scf.h"
#include "csinput/csevent.h"
#include "csinput/csinput.h"
#if defined(OS_BE)	// dh: is this OS-dependence necessary? 
#include "cssys/be/beitf.h"
#else
#include "cssys/unix/iunix.h"
#endif
#include "csutil/inifile.h"
#include "cs3d/glide2/glidelib.h"
#include "csutil/csrect.h"
#include "isystem.h"
#include "itexture.h"

IMPLEMENT_IBASE (csGraphics2DGlide2x)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics2D)
  IMPLEMENTS_INTERFACE (iGraphics2DGlide)
IMPLEMENT_IBASE_END

// csGraphics2DGlideCommon function
csGraphics2DGlideCommon::csGraphics2DGlideCommon (iBase *iParent) :
  csGraphics2D ()
{
  CONSTRUCT_IBASE (iParent);
  LocalFontServer = NULL;
  texture_cache = NULL;
  locked = false;
}

bool csGraphics2DGlideCommon::Initialize (iSystem *pSystem)
{
  if (!csGraphics2D::Initialize (pSystem))
    return false;

  // see if we need to go fullscreen or not...
  csIniFile* config = new csIniFile("cryst.cfg");
  m_DoGlideInWindow = (!config->GetYesNo("VideoDriver","FULL_SCREEN",FALSE));
  CHK (delete config);
  
  Depth = 16;

  _DrawPixel = DrawPixelGlide;
  _WriteChar = WriteCharGlide;
  _GetPixelAt = GetPixelAtGlide;
  _DrawSprite = DrawSpriteGlide;

  return true;
}

csGraphics2DGlideCommon::~csGraphics2DGlideCommon ()
{
  // Destroy your graphic interface
  Close ();
  CHKB (delete [] Memory);
//  CHKB (delete [] LocalFontServer);
//  LocalFontServer = NULL;
//  CHKB (delete [] texture_cache);
//  texture_cache = NULL;
}

bool csGraphics2DGlideCommon::Open(const char *Title)
{
  // Open your graphic interface
  if (!csGraphics2D::Open (Title))
    return false;
/*
  // load font 'server'
  if (LocalFontServer == NULL)
  {
       CsPrintf(MSG_INITIALIZATION,"loading fonts...");
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

  Clear (0);*/  // to be implemented
  return true;
}

void csGraphics2DGlideCommon::Close(void)
{
  // Close your graphic interface
  csGraphics2D::Close ();
}

void csGraphics2DGlideCommon::SetRGB(int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
  bPaletteChanged = true;
  SetTMUPalette(0);
}

void csGraphics2DGlideCommon::SetTMUPalette(int tmu)
{
  GuTexPalette p;
  RGBpaletteEntry pal;
  
  for(int i=0; i<256; i++)
  {
    pal = Palette[i];
    p.data[i]=0xFF<<24 | pal.red<<16 | pal.green<<8 | pal.blue;
  }
  
  GlideLib_grTexDownloadTable(tmu, GR_TEXTABLE_PALETTE, &p);		
}

void csGraphics2DGlideCommon::DrawLine (int x1, int y1, int x2, int y2, int color)
{
  // can't do this while framebuffer is locked...
  if (locked) return;
 
  GrVertex a,b;
  a.x=x1; a.y=y1;
  b.x=x2; b.y=y2;

  grConstantColorValue(color);
  grDrawLine(&a,&b);
}

void csGraphics2DGlideCommon::DrawPixel (csGraphics2D *This, int x, int y, int color)
{
   // can't do this while framebuffer is locked...
  if (locked) return;

  GrVertex p;
  p.x=x; p.y=y;

  grConstantColorValue(color);
  grDrawPoint(&p);
}

void csGraphics2DGlideCommon::WriteChar (csGraphics2D *This, int x, int y, int fg, int bg, char c)
{
  // not implemented yet...
}

void csGraphics2DGlideCommon::DrawSprite (csGraphics2D *This, iTextureHandle *hTex, int sx, int sy,
  int sw, int sh, int tx, int ty, int tw, int th)
{
  // not implemented yet...
}

unsigned char* csGraphics2DGlideCommon::GetPixelAt (csGraphics2D *This, int /*x*/, int /*y*/)
{
  return NULL;
}
