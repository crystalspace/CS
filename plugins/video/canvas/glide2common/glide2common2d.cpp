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
#include "glide2common2d.h"
#include "csutil/scf.h"
#include "csinput/csevent.h"
#include "csinput/csinput.h"
#if defined(OS_BE)	// dh: is this OS-dependence necessary? 
#include "cssys/be/beitf.h"
#else
#include "cssys/unix/iunix.h"
#endif
#include "csutil/inifile.h"
#ifdef GLIDE3
#include "cs3d/glide3/gllib3.h"
#else
#include "cs3d/glide2/gllib2.h"
#endif
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

  _WriteChar = WriteCharGlide;

  return true;
}

csGraphics2DGlideCommon::~csGraphics2DGlideCommon ()
{
  Close ();
  CHKB (delete [] Memory);
}

bool csGraphics2DGlideCommon::Open(const char *Title)
{
  // Open your graphic interface
  if (!csGraphics2D::Open (Title))
    return false;

  bPalettized = false;
  bPaletteChanged = false;
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
  RGBPixel pal;
  
  for(int i=0; i<256; i++)
  {
    pal = Palette[i];
    p.data[i]=0xFF<<24 | pal.red<<16 | pal.green<<8 | pal.blue;
  }
#ifdef GLIDE3
  GlideLib_grTexDownloadTable( GR_TEXTABLE_PALETTE, &p);
#else
  GlideLib_grTexDownloadTable( tmu, GR_TEXTABLE_PALETTE, &p);
#endif
}

void csGraphics2DGlideCommon::DrawLine (float x1, float y1, float x2, float y2, int color)
{
  if (locked)
    csGraphics2D::DrawLine( x1, y1, x2, y2, color );
  else
  {
    // our origin is lower left
    y1 = Height - y1;
    y2 = Height - y2;
    if ( !ClipLine( x1, y1, x2, y2, ClipX1, ClipY1, ClipX2, ClipY2 ) ){
      MyGrVertex a,b;
      a.x=x1; a.y=y1;a.oow=1;
      b.x=x2; b.y=y2;b.oow=1;

      GlideLib_grColorCombine ( GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
                                GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_NONE, FXFALSE );
      GlideLib_grAlphaBlendFunction ( GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO );
      UByte rc,gc,bc;
      DecodeRGB( color, rc, gc, bc );
      GlideLib_grConstantColorValue ( 0xff000000 | (rc << 16) | (gc<<8) | bc );

    GlideLib_grDrawLine(&a,&b);
    }
  }
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
    }else
  {
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
    GlideLib_grLfbUnlock (glDrawMode,GR_DRAWBUFFER);
  
  locked = false;
}


void csGraphics2DGlideCommon::DrawPixel (int x, int y, int color)
{
  // can't do this while framebuffer is locked...
  if (locked)
    *((UShort*)GetPixelAt( x, y )) = color;
  else
  {
    MyGrVertex p;
    p.x=x; p.y=y;
    
    GlideLib_grColorCombine ( GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
                              GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_NONE, FXFALSE );
    GlideLib_grAlphaBlendFunction( GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO );
    UByte r,g,b;
    DecodeRGB( color, r, g, b );
    GlideLib_grConstantColorValue ( 0xff000000 | (r << 16) | (g<<8) | b );
    GlideLib_grDrawPoint(&p);
  }
}

void csGraphics2DGlideCommon::WriteCharGlide ( csGraphics2D *This, int x, int y, int fg, int bg, char c)
{
  This->WriteChar16( This, x,y,fg,bg,c);
}

unsigned char* csGraphics2DGlideCommon::GetPixelAt ( int x, int y)
{
  if (!locked) BeginDraw();
  return (Memory + (x + x + LineAddress[ y ]));
}

void csGraphics2DGlideCommon::Print( csRect* area ){

  // swap the buffers only to show the new frame
  GlideLib_grBufferSwap( m_bVRetrace ? 1 : 0 );
}

csImageArea *csGraphics2DGlideCommon::SaveArea (int x, int y, int w, int h)
{
  if (x < 0)
  { w += x; x = 0; }
  if (x + w > Width)
    w = Width - x;
  if (y < 0)
  { h += y; y = 0; }
  if (y + h > Height)
    h = Height - y;
  if ((w <= 0) || (h <= 0))
    return NULL;

  CHK (csImageArea *Area = new csImageArea (x, y, w, h));
  if (!Area)
    return NULL;
  CHK (char *dest = Area->data = new char [w * h * pfmt.PixelBytes]);
  if (!dest)
  {
    delete Area;
    return NULL;
  }
  if ( GlideLib_grLfbReadRegion( GR_BUFFER_FRONTBUFFER, x, y, w, h, w*pfmt.PixelBytes, Area->data ) )
  {
    // reorder to prepare for restoring
    y = w*h;
    UShort hlp;
    UShort *vram = (UShort*)Area->data;
    for (x=0; x < y ; x+=2)
    {
      hlp = *vram;
      *vram++ = *(vram+1);
      *vram++ = hlp;
    }
  }
  else
  {
    FreeArea( Area );
    Area = NULL;
  }      
  return Area;
  
}

void csGraphics2DGlideCommon::RestoreArea (csImageArea *Area, bool Free)
{
  if (Area)
  {
    if ( !locked ){
#ifdef GLIDE3
      GlideLib_grLfbWriteRegion( GR_BUFFER_BACKBUFFER, Area->x, Area->y, 
                                 GR_LFB_SRC_FMT_565, Area->w, Area->h, 
				 FXFALSE, Area->w*pfmt.PixelBytes, Area->data );
#else
      GlideLib_grLfbWriteRegion( GR_BUFFER_BACKBUFFER, Area->x, Area->y, 
                                 GR_LFB_SRC_FMT_565, Area->w, Area->h, 
				 Area->w*pfmt.PixelBytes, Area->data );
#endif
    }else{
      int x, y, n;
      unsigned char *data = (unsigned char*)Area->data;
      x = Area->x * pfmt.PixelBytes;
      n = Area->w * pfmt.PixelBytes;
      for( y=Area->y; y < Area->h; y++ ){
        memcpy( Memory + LineAddress[ y ] + x, data, n );
	data += n;
      }
      
    }
    if (Free)
      FreeArea (Area);
  } /* endif */
}

void csGraphics2DGlideCommon::EncodeRGB ( UShort& color, UByte r, UByte g, UByte b )
{
    color= ((r >> 3) << pfmt.RedShift)   |
           ((g >> 2) << pfmt.GreenShift) |
           ((b >> 3) >> pfmt.BlueShift);
}

void csGraphics2DGlideCommon::DecodeRGB ( UShort color, UByte& r, UByte& g, UByte& b )
{
    r = ((color&pfmt.RedMask) >> pfmt.RedShift) << 3;
    g = ((color&pfmt.GreenMask) >> pfmt.GreenShift) << 2;
    b = ((color&pfmt.BlueMask) >> pfmt.BlueShift) << 3;
}

float csGraphics2DGlideCommon::GetZBuffValue (int x, int y)
{
  GrLfbInfo_t lfbInfo;
  bool succ = GlideLib_grLfbLock ( GR_LFB_READ_ONLY | GR_LFB_NOIDLE,
                          GR_BUFFER_AUXBUFFER,
                          GR_LFBWRITEMODE_ZA16,
                          GR_ORIGIN_UPPER_LEFT,
                          FXFALSE,
                          &lfbInfo);
  float val=0.0;
  if (succ)
  {
    val = *((float*)( (unsigned char*)(lfbInfo.lfbPtr) + y*lfbInfo.strideInBytes + x + x));
    GlideLib_grLfbUnlock ( GR_LFB_READ_ONLY | GR_LFB_NOIDLE, GR_BUFFER_AUXBUFFER);
  }
  else
  {
  printf( "could not lock depthbuffer\n");
  }
  return val;
}