/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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
#include "cssysdef.h"
#include "glide2common2d.h"
#include "csutil/scf.h"
#include "cssys/csinput.h"
#include "video/renderer/glide/gllib.h"
#include "csutil/csrect.h"
#include "isys/isystem.h"
#include "ivideo/itexture.h"
#include "iengine/itexture.h"
#include "glidcurs.h"

IMPLEMENT_IBASE (csGraphics2DGlideCommon)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics2D)
  IMPLEMENTS_INTERFACE (iGraphics2DGlide)
IMPLEMENT_IBASE_END

bool csGraphics2DGlideCommon::locked = false;

// csGraphics2DGlideCommon function
csGraphics2DGlideCommon::csGraphics2DGlideCommon (iBase *iParent) :
  csGraphics2D ()
{
  (void)iParent;
  CONSTRUCT_IBASE (NULL);
  SetVRetrace( false );
  cursorBmp=mcBack=NULL;
  nCursor=nCurCursor=mcCols=mcRows=0;
  mx=my=mxold=myold=-1;
  writtenArea.MakeEmpty();
  mouseRect.MakeEmpty();
  m_drawbuffer = GR_BUFFER_BACKBUFFER;
  PrepareCursors (mouseshapes);
}

bool csGraphics2DGlideCommon::Initialize (iSystem *pSystem)
{
  if (!csGraphics2D::Initialize (pSystem))
    return false;

  int w, h;
  pSystem->GetSettings (w, h, Depth, m_DoGlideInWindow);
  m_DoGlideInWindow = !m_DoGlideInWindow;
  Depth = 16;

  _WriteString = WriteStringGlide;

  return true;
}

csGraphics2DGlideCommon::~csGraphics2DGlideCommon ()
{
  Close ();
  if (cursorBmp) delete cursorBmp;
  if (mcBack) delete mcBack;
}

bool csGraphics2DGlideCommon::Open(const char *Title)
{
  if (!csGraphics2D::Open (Title))
    return false;

  bPalettized = false;
  bPaletteChanged = false;
  return true;
}

void csGraphics2DGlideCommon::Close()
{
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
  csRGBpixel pal;
  
  for(int i=0; i<256; i++)
  {
    pal = Palette[i];
    p.data[i]=0xFF<<24 | pal.red<<16 | pal.green<<8 | pal.blue;
  }
#ifdef GLIDE3
  (void)tmu;
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
//#define GR_DRAWBUFFER GR_BUFFER_BACKBUFFER

bool csGraphics2DGlideCommon::BeginDraw()
{
  csGraphics2D::BeginDraw ();
  if (FrameBufferLocked != 1)
    return true;

  FxBool bret;
  lfbInfo.size=sizeof(GrLfbInfo_t);
  
  glDrawMode=GR_LFB_WRITE_ONLY;

  if(locked) FinishDraw();

  bret=GlideLib_grLfbLock(glDrawMode|GR_LFB_IDLE,
                          m_drawbuffer,
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
  printf("could not lock mem\n");
  }
  return bret;

}

void csGraphics2DGlideCommon::FinishDraw ()
{
  if (nCurCursor>-1) 
    DrawCursor();
  csGraphics2D::FinishDraw ();
  if (FrameBufferLocked)
    return;
  Memory=NULL;
  for (int i = 0; i < Height; i++) LineAddress [i] = 0;
  if (locked) 
    GlideLib_grLfbUnlock (glDrawMode,m_drawbuffer);
  
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

void csGraphics2DGlideCommon::WriteStringGlide (csGraphics2D *This, iFont *font, int x, int y, int fg, int bg, const char *c)
{
  This->WriteString16 (This, font, x,y,fg,bg,c);
}

unsigned char* csGraphics2DGlideCommon::GetPixelAt (int x, int y)
{
  if (!locked) BeginDraw();
  return (Memory + (x + x + LineAddress[ y ]));
}

void csGraphics2DGlideCommon::Print (csRect* area)
{
  if (!GetDoubleBufferState())
    if ( area != NULL ) 
      writtenArea.Set( *area );
    else
      writtenArea.MakeEmpty();
  else
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

  csImageArea *Area = new csImageArea (x, y, w, h);
  if (!Area)
    return NULL;
  char *dest = Area->data = new char [w * h * pfmt.PixelBytes];
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
      GlideLib_grLfbWriteRegion( m_drawbuffer, Area->x, Area->y, 
                                 GR_LFB_SRC_FMT_565, Area->w, Area->h, 
				 FXFALSE, Area->w*pfmt.PixelBytes, Area->data );
#else
      GlideLib_grLfbWriteRegion( m_drawbuffer, Area->x, Area->y, 
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
  }
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
    FxU32 *ptr;
    FxU16 wfloat;

    ptr = (FxU32*)lfbInfo.lfbPtr;
    ptr += (y*lfbInfo.strideInBytes + x + x)>>2;
    if ( x&1 ) 
      wfloat = *ptr&0xffff;
    else
      wfloat = *ptr>>16;
    
    int xx = (wfloat << 11) + (127 << 23);
    memcpy( &val, &xx, 4 );
    GlideLib_grLfbUnlock ( GR_LFB_READ_ONLY | GR_LFB_NOIDLE, GR_BUFFER_AUXBUFFER);
  }
  else
  {
  printf( "could not lock depthbuffer\n");
  }
  return val;
}

bool csGraphics2DGlideCommon::DoubleBuffer (bool Enable)
{
  while (locked) FinishDraw ();
  m_drawbuffer = (Enable ? GR_BUFFER_BACKBUFFER : GR_BUFFER_FRONTBUFFER);
  GlideLib_grRenderBuffer (m_drawbuffer);
/*
  if (!bSwitchDoubleBuffer && (Enable != (m_drawbuffer == GR_BUFFER_BACKBUFFER)))
  {
    bSwitchDoubleBuffer = true;
  }
  else // this is called again before the buffering switched
  if (bSwitchDoubleBuffer && (Enable == (m_drawbuffer == GR_BUFFER_BACKBUFFER)))
    bSwitchDoubleBuffer = false;
    */
  return true;
}

int csGraphics2DGlideCommon::PrepareCursors (char **shapes)
{
  // read xpm desc
  int w, h, numcolors, chars_per_pixel, i, x, y, off;
  int bw, bh, row, col;
  sscanf( (const char*)shapes[0], "%d %d %d %d", &w, &h, &numcolors, &chars_per_pixel );
  
  char *pc = new char[chars_per_pixel+1]; // character sequence representing one pixel
  pc[chars_per_pixel] = '\0';
  char *pattern1 = new char[chars_per_pixel+14];
  char *pattern2 = new char[chars_per_pixel+10];
  char pattern3[4];
  sprintf( pattern1, "%%%dc c #%%6c", chars_per_pixel );
  sprintf( pattern2, "%%%dc %%d %%d", chars_per_pixel );
  sprintf( pattern3, "%%%dc", chars_per_pixel );
  char hexrgb[6];
  
  // first count the cursors
  nCursor= (w>h ? w/h : h/w);
  bw = w / (w>h?nCursor:1);
  bh = h / (w>h?1:nCursor);
  
  off = nCursor + 1 + numcolors;
  // prepare the bitmap we send to the card
  if (cursorBmp) delete cursorBmp;
  if (mcBack) delete mcBack;
  cursorBmp = new UShort[ w*h + 3*nCursor];
  mcBack = new UShort[ bw*bh ];

#define HEX2DEC(d) ( (d)>'a' ? (d)-87 : (d)>'A' ? (d)-55 : (d)-30 )
#define HEXX(a,b) HEX2DEC(a) << 4 | HEX2DEC(b)

  // read in cursor data
  int nC;
  for (nC=0; nC<nCursor; nC++)
  {
    UByte r, g, b;
    
    // hot spot
    sscanf( (const char*)shapes[1+nC], pattern2, pc, &x, &y);
    cursorBmp[ 3*nC +1] = x;
    cursorBmp[ 3*nC +2] = y;
    
    // keycolordef
    i=1;
    while (i<=numcolors){
      if ( strstr(shapes[i+nCursor],pc) == shapes[i+nCursor] ) break;
      i++;
    }
    sscanf( shapes[i+nCursor], pattern1, pc, hexrgb);
    r = HEXX(hexrgb[0],hexrgb[1]); g = HEXX(hexrgb[2],hexrgb[3]); b = HEXX(hexrgb[4],hexrgb[5]);
    cursorBmp[ 3*nC ]= ((r >> 3) << 11) |
                       ((g >> 2) <<  5) |
                       ((b >> 3) >>  0);
      
    // cursorbitmap
    for(row=0; row<bh; row++)
      for(col=0; col<bw; col++){
        if ( w>h )
          sscanf( &shapes[ off+row ][ nC*bw*chars_per_pixel + col*chars_per_pixel ], pattern3, pc );
	else  
          sscanf( &shapes[ nC*bh + row +off ][ col*chars_per_pixel ], pattern3, pc );
        i=1;
        while (i<=numcolors){
          if ( strstr(shapes[i+nCursor],pc) == shapes[i+nCursor] ) break;
          i++;
        }
        sscanf( shapes[i+nCursor], pattern1, pc, hexrgb);
        r = HEXX(hexrgb[0],hexrgb[1]); g = HEXX(hexrgb[2],hexrgb[3]); b = HEXX(hexrgb[4],hexrgb[5]);
        cursorBmp[ 3*nCursor + nC*bw*bw + row*bw + col ]= ((r >> 3) << 11) |
                                                          ((g >> 2) <<  5) |
                                                          ((b >> 3) >>  0);
      }
  }

#undef HEX2DEC(d)
#undef HEXX(a,b)

  delete [] pc;
  delete pattern1;
  delete pattern2;
  
  mcRows = bh;
  mcCols = bw;
  return nCursor;
}

void csGraphics2DGlideCommon::DrawCursor ()
{
  short hx, hy, sx, sy;
  int keycolor, color;
  int row, col;
  csRect r1;
  
  // read hotspotvalues for curent cursor
  hx = cursorBmp [ nCurCursor*3 +1 ];
  hy = cursorBmp [ nCurCursor*3 +2 ];
  
  // first restore background where cursor was last
  r1.Set(mouseRect);
  r1.Subtract( writtenArea );

  if (!mouseRect.IsEmpty() && (!writtenArea.IsEmpty() || mxold != mx || myold != my ))
  {
    for (sy=0,row=mouseRect.ymin; row<mouseRect.ymax; row++, sy++)
      for (sx=0,col=mouseRect.xmin; col<mouseRect.xmax; col++, sx++ )
        if ( !writtenArea.Contains (col, row) ) 
	  DrawPixel (col, row, mcBack[sy*mcCols+sx] );
  
  // remember what we are overdrawing
  mouseRect.Set( MAX(0,mx-hx), MAX(0,my-hy), MIN(Width-1,mx-hx+mcCols), MIN(Height-1,my-hy+mcRows) );

  for (row=mouseRect.ymin; row<mouseRect.ymax; row++)
  {
    UShort *p = (UShort*)GetPixelAt( mouseRect.xmin, row );
    memcpy( mcBack + (row-mouseRect.ymin)*mcCols, p, sizeof(UShort)*(mouseRect.Width()) );
  }
  }else if (mouseRect.IsEmpty()) 
  {
  // remember what we are overdrawing
  mouseRect.Set( MAX(0,mx-hx), MAX(0,my-hy), MIN(Width-1,mx-hx+mcCols), MIN(Height-1,my-hy+mcRows) );

  for (row=mouseRect.ymin; row<mouseRect.ymax; row++){
    UShort *p = (UShort*)GetPixelAt( mouseRect.xmin, row );
    memcpy( mcBack + (row-mouseRect.ymin)*mcCols, p, sizeof(UShort)*(mouseRect.Width()) );
  }
  }
  writtenArea.MakeEmpty();
  mxold=mx; myold=my;
  
  // read keycolor
  keycolor = cursorBmp [ nCurCursor*3 ];
  sy = my - hy;
  for (row=0; row<mcRows && sy<Height; row++, sy++)
  {
    sx = mx - hx;
    for (col=0; col<mcCols && sx<Width; col++, sx++)
    {
      color = cursorBmp [ nCursor*3 + nCurCursor*mcRows*mcCols + row*mcCols + col ];
      if ( sx >= 0 && sy >= 0 && color != keycolor )
      {
        DrawPixel( sx, sy, color );
      }
    }
  }
}
