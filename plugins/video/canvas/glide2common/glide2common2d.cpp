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

IMPLEMENT_IBASE (csGraphics2DGlideCommon)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics2D)
  IMPLEMENTS_INTERFACE (iGraphics2DGlide)
IMPLEMENT_IBASE_END

bool csGraphics2DGlideCommon::locked=false;

// csGraphics2DGlideCommon function
csGraphics2DGlideCommon::csGraphics2DGlideCommon (iBase *iParent) :
  csGraphics2D ()
{
  CONSTRUCT_IBASE (NULL);
//  LocalFontServer = NULL;
//  texture_cache = NULL;
  SetVRetrace( false );
}

bool csGraphics2DGlideCommon::Initialize (iSystem *pSystem)
{
  if (!csGraphics2D::Initialize (pSystem))
    return false;

  // see if we need to go fullscreen or not...
  csIniFile* config = new csIniFile("cryst.cfg");
  m_DoGlideInWindow = (!config->GetYesNo("VideoDriver","FullScreen",FALSE));
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

  bPalettized = false;
  bPaletteChanged = false;
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

//#define GR_DRAWBUFFER GR_BUFFER_FRONTBUFFER
#define GR_DRAWBUFFER GR_BUFFER_BACKBUFFER

bool csGraphics2DGlideCommon::BeginDraw(/*int Flag*/)
{
  csGraphics2D::BeginDraw ();
  if (FrameBufferLocked != 1)
    return true;

  FxBool bret;
  lfbInfo.size=sizeof(GrLfbInfo_t);
  
  glDrawMode=GR_LFB_WRITE_ONLY;

  if(locked) FinishDraw();

    
  bret=GlideLib_grLfbLock(glDrawMode|GR_LFB_IDLE,
                          GR_DRAWBUFFER,
                          GR_LFBWRITEMODE_565,
                          GR_ORIGIN_UPPER_LEFT,
                          FXFALSE,
                          &lfbInfo);
  if(bret)
    {
      Memory=(unsigned char*)lfbInfo.lfbPtr;
      if(lfbInfo.origin==GR_ORIGIN_UPPER_LEFT)
        {
          for(int i = 0; i < Height; i++)
            LineAddress [i] = i * lfbInfo.strideInBytes;
        }
      else
        {
          int omi = Height-1;
          for(int i = 0; i < Height; i++)
            LineAddress [i] = (omi--) * lfbInfo.strideInBytes;
        }
      locked=true;
    }
  return bret;

}

void csGraphics2DGlideCommon::FinishDraw ()
{
  csGraphics2D::FinishDraw ();
  if (FrameBufferLocked)
    return;

  Memory=NULL;
  for (int i = 0; i < Height; i++) LineAddress [i] = 0;
  if (locked) 
    GlideLib_grLfbUnlock(glDrawMode,GR_DRAWBUFFER);
  
  locked = false;
}


void csGraphics2DGlideCommon::DrawPixelGlide ( csGraphics2D *This, int x, int y, int color)
{
  // can't do this while framebuffer is locked...
  if (locked) return;

  GrVertex p;
  p.x=x; p.y=y;

  grConstantColorValue(color);
  grDrawPoint(&p);
}

void csGraphics2DGlideCommon::WriteCharGlide ( csGraphics2D *This, int x, int y, int fg, int bg, char c)
{
  //if (!locked)
//thisPtr->BeginDraw();
//  if (locked) thisPtr->FinishDraw();
//  thisPtr->BeginDraw();
  This->WriteChar16( This, x,y,fg,bg,c);

  // not implemented yet...
}

void csGraphics2DGlideCommon::DrawSpriteGlide ( csGraphics2D *This, iTextureHandle *hTex, int sx, int sy,
  int sw, int sh, int tx, int ty, int tw, int th)
{
 // if (!locked) thisPtr->BeginDraw();
//  if (locked) thisPtr->FinishDraw();
  //thisPtr->BeginDraw();
  This->DrawSprite16( This, hTex,sx,sy,sw,sh,tx,ty,tw,th);
  // not implemented yet...
}

unsigned char* csGraphics2DGlideCommon::GetPixelAtGlide ( csGraphics2D *This, int x, int y)
{
  // not implemented yet...
   //static FxBool bret;
//   static unsigned char ch;
   //static GrLfbInfo_t lfbInfo;

/*   if (!locked)
   {
     lfbInfo.size=sizeof(GrLfbInfo_t);


     bret=GlideLib_grLfbLock(GR_LFB_READ_ONLY|GR_LFB_IDLE,
                          GR_DRAWBUFFER,
                          GR_LFBWRITEMODE_565,
                          GR_ORIGIN_ANY,
                          FXFALSE,
                          &lfbInfo);
     locked=bret;
   }*/


   if (!locked) This->BeginDraw();

   return This->GetPixelAt16( This, x,y);
/*   if(locked)
   {
     //Memory=(unsigned char*)lfbInfo.lfbPtr;
     return (unsigned char *)thisPtr->Memory+(y*thisPtr->lfbInfo.strideInBytes+x);

     //GlideLib_grLfbUnlock(GR_LFB_READ_ONLY,GR_DRAWBUFFER);
   }

   else return NULL;*/
}

void csGraphics2DGlideCommon::Print( csRect* area ){

    // swap the buffers only to show the new frame
    GlideLib_grBufferSwap( m_bVRetrace ? 1 : 0 );
}
