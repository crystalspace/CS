/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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
#include <stdlib.h>
#include "sysdef.h"
#include "cs2d/common/graph2d.h"
#include "qint.h"
#include "isystem.h"
#include "igraph3d.h"
#include "itexture.h"

// Static csGraphics2D variables
int csGraphics2D::ClipX1, csGraphics2D::ClipX2;
int csGraphics2D::ClipY1, csGraphics2D::ClipY2;
int csGraphics2D::Font;
csPixelFormat csGraphics2D::pfmt;
unsigned char *csGraphics2D::Memory = NULL;
int *csGraphics2D::LineAddress;
bool csGraphics2D::FullScreen;
int csGraphics2D::Width, csGraphics2D::Height, csGraphics2D::Depth;
RGBpaletteEntry csGraphics2D::Palette[256];
bool csGraphics2D::PaletteAlloc[256];
ISystem* csGraphics2D::system = NULL;

// "Virtual functions" through pointers in csGraphics2D
void (*csGraphics2D::DrawPixel) (int x, int y, int color);
void (*csGraphics2D::WriteChar) (int x, int y, int fg, int bg, char c);
void (*csGraphics2D::DrawSprite) (ITextureHandle *hTex, int sx, int sy, int sw, int sh, int tx, int ty, int tw, int th);
unsigned char* (*csGraphics2D::GetPixelAt) (int x, int y);

#if defined(PROC_INTEL) && !defined(NO_ASSEMBLER)
#if defined(COMP_VC)
#  include "cs3d/software/i386/drline.h"
#endif
#endif

BEGIN_INTERFACE_TABLE (csGraphics2D)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX (IGraphics2D, XGraphics2D)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX (IGraphicsInfo, XGraphicsInfo)
END_INTERFACE_TABLE ()

IMPLEMENT_UNKNOWN (csGraphics2D)

csGraphics2D::csGraphics2D (ISystem* piSystem)
{
  ASSERT (piSystem != NULL);
  system = piSystem;
}

void csGraphics2D::Initialize ()
{
  // Get the system parameters
  system->GetWidthSetting (Width);
  system->GetHeightSetting (Height);
  system->GetDepthSetting (Depth);
  system->GetFullScreenSetting (FullScreen);

  Font = 0;
  SetClipRect (0, 0, Width, Height);
  pfmt.PalEntries = 256;
  pfmt.PixelBytes = 1;
  // Initialize pointers to default drawing methods
  DrawPixel = DrawPixel8;
  WriteChar = WriteChar8;
  GetPixelAt = GetPixelAt8;
  DrawSprite = DrawSprite8;
  // Mark all slots in palette as free
  for (int i = 0; i < 256; i++)
  {
    PaletteAlloc[i] = false;
    Palette[i].red = 0;
    Palette[i].green = 0;
    Palette[i].blue = 0;
  }

}

csGraphics2D::~csGraphics2D ()
{
  Close ();
}

bool csGraphics2D::Open (char *Title)
{
  (void)Title;

  // Allocate buffer for address of each scan line to avoid multuplication
  CHK (LineAddress = new int [Height]);
  if (LineAddress == NULL) return false;

  // Initialize scanline address array
  int i,addr,bpl = Width * pfmt.PixelBytes;
  for (i = 0, addr = 0; i < Height; i++, addr += bpl)
    LineAddress[i] = addr;

  return true;
}

void csGraphics2D::Close ()
{
  if (LineAddress) { CHK (delete [] LineAddress); LineAddress = NULL; }
}

void csGraphics2D::complete_pixel_format ()
{
  long s;
  pfmt.RedShift = 0;   s = pfmt.RedMask;   while (s && !(s&1)) { pfmt.RedShift++; s >>= 1; }
  pfmt.GreenShift = 0; s = pfmt.GreenMask; while (s && !(s&1)) { pfmt.GreenShift++; s >>= 1; }
  pfmt.BlueShift = 0;  s = pfmt.BlueMask;  while (s && !(s&1)) { pfmt.BlueShift++; s >>= 1; }
  pfmt.RedBits = 0;    s = pfmt.RedMask >> pfmt.RedShift;     while (s) { pfmt.RedBits++; s >>= 1; }
  pfmt.GreenBits = 0;  s = pfmt.GreenMask >> pfmt.GreenShift; while (s) { pfmt.GreenBits++; s >>= 1; }
  pfmt.BlueBits = 0;   s = pfmt.BlueMask >> pfmt.BlueShift;   while (s) { pfmt.BlueBits++; s >>= 1; }
}

bool csGraphics2D::BeginDraw ()
{
  // no operation
  return true;
}

void csGraphics2D::FinishDraw ()
{
  // no operation
}

void csGraphics2D::Clear(int color)
{
  DrawBox (0, 0, Width, Height, color);
}

bool csGraphics2D::DoubleBuffer (bool Enable)
{
  return !Enable;
}

bool csGraphics2D::DoubleBuffer ()
{
  return false;
}

int csGraphics2D::GetPage ()
{
  return 0;
}

void csGraphics2D::ClearAll (int color)
{
  int CurPage = GetPage ();
  do
  {
    if (!BeginDraw ())
      break;
    Clear (color);
    FinishDraw ();
    Print ();
  } while (GetPage () != CurPage);
}

void csGraphics2D::DrawPixel8 (int x, int y, int color)
{
  if ((x >= ClipX1) && (x < ClipX2) && (y >= ClipY1) && (y < ClipY2))
    *(GetPixelAt (x, y)) = color;
}

void csGraphics2D::DrawPixel16 (int x, int y, int color)
{
  if ((x >= ClipX1) && (x < ClipX2) && (y >= ClipY1) && (y < ClipY2))
    *(short *)(GetPixelAt (x, y)) = color;
}

void csGraphics2D::DrawPixel32 (int x, int y, int color)
{
  if ((x >= ClipX1) && (x < ClipX2) && (y >= ClipY1) && (y < ClipY2))
    *(long *)(GetPixelAt (x, y)) = color;
}

#ifndef NO_DRAWLINE
void csGraphics2D::DrawLine (float x1, float y1, float x2, float y2, int color)
{
  if (ClipLine (x1, y1, x2, y2, ClipX1, ClipY1, ClipX2, ClipY2))
    return;

  int fx1 = QInt (x1), fx2 = QInt (x2),
      fy1 = QInt (y1), fy2 = QInt (y2);
  if (abs (fx2 - fx1) > abs (fy2 - fy1))
  {
    // Transform floating-point format to 16.16 fixed-point
    int fy1 = QInt16 (y1), fy2 = QInt16 (y2);

    if (fx1 > fx2)
    {
      int tmp = fx1; fx1 = fx2; fx2 = tmp;
      tmp = fy1; fy1 = fy2; fy2 = tmp;
    }

    // delta Y can be negative
    int deltay = (fy2 - fy1) / (fx2 - fx1);
    for (int x = fx1, y = fy1; x <= fx2; x++)
    {
      // Assumption: Y cannot be negative (after clipping)
      DrawPixel (x, y >> 16, color);
      y += deltay;
    }
  }
  else if (fy2 != fy1)
  {
    // Transform floating-point format to 16.16 fixed-point
    int fx1 = QInt16 (x1), fx2 = QInt16 (x2);

    if (fy1 > fy2)
    {
      int tmp = fy1; fy1 = fy2; fy2 = tmp;
      tmp = fx1; fx1 = fx2; fx2 = tmp;
    }

    // delta X can be negative
    int deltax = int (fx2 - fx1) / int (fy2 - fy1);
    for (int x = fx1, y = fy1; y <= fy2; y++)
    {
      // Assumption: X cannot be negative (after clipping)
      DrawPixel (x >> 16, y, color);
      x += deltax;
    }
  }
  else if (fx2 - fx1)
  {
    if (fx1 > fx2) { int tmp = fx1; fx1 = fx2; fx2 = tmp; }
    int count = fx2 - fx1 + 1;
    switch (pfmt.PixelBytes)
    {
      case 1:
        memset (GetPixelAt (fx1, fy1), color, count);
        break;
      case 2:
      {
        UShort *dest = (UShort *)GetPixelAt (fx1, fy1);
        while (count--) *dest++ = color;
        break;
      }
      case 4:
      {
        ULong *dest = (ULong *)GetPixelAt (fx1, fy1);
        while (count--) *dest++ = color;
        break;
      }
    } /* endswitch */
  }
  else
    DrawPixel (fx1, fy1, color);
}
#endif

void csGraphics2D::DrawBox (int x, int y, int w, int h, int color)
{
  if ((x > ClipX2) || (y > ClipY2))
    return;
  if (x < ClipX1)
    w -= (ClipX1 - x), x = ClipX1;
  if (y < ClipY1)
    h -= (ClipY1 - y), y = ClipY1;
  if (x + w > ClipX2)
    w = ClipX2 - x;
  if (y + h > ClipY2)
    h = ClipY2 - y;
  if ((w <= 0) || (h <= 0))
    return;
  switch (pfmt.PixelBytes)
  {
    case 1:
      while (h)
      {
        memset (GetPixelAt (x, y), color, w);
        y++; h--;
      } /* endwhile */
      break;
    case 2:
      while (h)
      {
        register UShort *dest = (UShort *)GetPixelAt (x, y);
        register int count = w;
        while (count--) *dest++ = color;
        y++; h--;
      } /* endwhile */
      break;
    case 4:
      while (h)
      {
        register ULong *dest = (ULong *)GetPixelAt (x, y);
        register int count = w;
        while (count--) *dest++ = color;
        y++; h--;
      } /* endwhile */
      break;
  } /* endswitch */
}

void csGraphics2D::Write (int x, int y, int fg, int bg, char *text)
{
  for (; *text; ++text)
  {
    WriteChar(x, y, fg, bg, *text);
    if (FontList[Font].IndividualWidth)
      x += FontList[Font].IndividualWidth[(unsigned char)*text];
    else
      x += FontList[Font].Width;
  }
}

void csGraphics2D::WriteChar8 (int x, int y, int fg, int bg, char c)
{
  int charH = FontList [Font].Height;
  int charW;
  if (FontList[Font].IndividualWidth)
    charW = FontList[Font].IndividualWidth[(unsigned char)c];
  else
    charW = FontList[Font].Width;

  if ((x + charW <= ClipX1) || (x >= ClipX2) || (y + charH <= ClipY1) || (y >= ClipY2))
    return;

  register unsigned char *CharImage =
    &FontList[Font].FontBitmap[((unsigned char)c) * FontList[Font].BytesPerChar];

  if ((x < ClipX1) || (x + charW > ClipX2) || (y < ClipY1) || (y + charH > ClipY2))
  {
    // Perform full clipping
    int lX = x < ClipX1 ? ClipX1 - x : 0;
    int rX = x + charW >= ClipX2 ? ClipX2 - x : charW;
    x += lX;
    for (int i = 0; i < charH; i++, y++)
    {
      register char CharLine = (*CharImage++) << lX;
      if ((y < ClipY1) || (y >= ClipY2)) continue;
      register unsigned char *VRAM = GetPixelAt (x, y);
      for (int j = lX; j < rX; j++)
      {
        if (CharLine & 0x80)
          *VRAM++ = fg;
        else if (bg >= 0)
          *VRAM++ = bg;
        else
          VRAM++;
        CharLine <<= 1;
      } /* endfor */
    } /* endfor */
  } else
  for (int i = 0; i < charH; i++, y++)
  {
    register unsigned char *VRAM = GetPixelAt (x, y);
    register char CharLine = *CharImage++;
    if (bg < 0)
      for (int j = 0; j < charW; j++)
      {
        if (CharLine & 0x80)
          *VRAM++ = fg;
        else
          VRAM++;
        CharLine <<= 1;
      } else
      for (int j = 0; j < charW; j++)
      {
        if (CharLine & 0x80)
          *VRAM++ = fg;
        else
          *VRAM++ = bg;
        CharLine <<= 1;
      } /* endfor */
  } /* endfor */
}

void csGraphics2D::WriteChar16 (int x, int y, int fg, int bg, char c)
{
  int charH = FontList [Font].Height;
  int charW;
  if (FontList[Font].IndividualWidth)
    charW = FontList[Font].IndividualWidth[(unsigned char)c];
  else
    charW = FontList[Font].Width;

  if ((x + charW <= ClipX1) || (x >= ClipX2) || (y + charH <= ClipY1) || (y >= ClipY2))
    return;

  register unsigned char *CharImage =
    &FontList[Font].FontBitmap[((unsigned char)c) * FontList[Font].BytesPerChar];

  if ((x < ClipX1) || (x + charW > ClipX2) || (y < ClipY1) || (y + charH > ClipY2))
  {
    // Perform full clipping
    int lX = x < ClipX1 ? ClipX1 - x : 0;
    int rX = x + charW >= ClipX2 ? ClipX2 - x : charW;
    x += lX;
    for (int i = 0; i < charH; i++, y++)
    {
      register char CharLine = (*CharImage++) << lX;
      if ((y < ClipY1) || (y >= ClipY2)) continue;
      register unsigned short *VRAM = (unsigned short *)GetPixelAt (x, y);
      for (int j = lX; j < rX; j++)
      {
        if (CharLine & 0x80)
          *VRAM++ = fg;
        else if (bg >= 0)
          *VRAM++ = bg;
        else
          VRAM++;
        CharLine <<= 1;
      } /* endfor */
    } /* endfor */
  } else
  for (int i = 0; i < charH; i++, y++)
  {
    register unsigned short *VRAM = (unsigned short *)GetPixelAt (x, y);
    register char CharLine = *CharImage++;
    if (bg < 0)
      for (int j = 0; j < charW; j++)
      {
        if (CharLine & 0x80)
          *VRAM++ = fg;
        else
          VRAM++;
        CharLine <<= 1;
      } else
      for (int j = 0; j < charW; j++)
      {
        if (CharLine & 0x80)
          *VRAM++ = fg;
        else
          *VRAM++ = bg;
        CharLine <<= 1;
      } /* endfor */
  } /* endfor */
}

void csGraphics2D::WriteChar32 (int x, int y, int fg, int bg, char c)
{
  int charH = FontList [Font].Height;
  int charW;
  if (FontList[Font].IndividualWidth)
    charW = FontList[Font].IndividualWidth[(unsigned char)c];
  else
    charW = FontList[Font].Width;

  if ((x + charW <= ClipX1) || (x >= ClipX2) || (y + charH <= ClipY1) || (y >= ClipY2))
    return;

  register unsigned char *CharImage =
    &FontList[Font].FontBitmap[((unsigned char)c) * FontList[Font].BytesPerChar];

  if ((x < ClipX1) || (x + charW > ClipX2) || (y < ClipY1) || (y + charH > ClipY2))
  {
    // Perform full clipping
    int lX = x < ClipX1 ? ClipX1 - x : 0;
    int rX = x + charW >= ClipX2 ? ClipX2 - x : charW;
    x += lX;
    for (int i = 0; i < charH; i++, y++)
    {
      register char CharLine = (*CharImage++) << lX;
      if ((y < ClipY1) || (y >= ClipY2)) continue;
      register unsigned long *VRAM = (unsigned long *)GetPixelAt (x, y);
      for (int j = lX; j < rX; j++)
      {
        if (CharLine & 0x80)
          *VRAM++ = fg;
        else if (bg >= 0)
          *VRAM++ = bg;
        else
          VRAM++;
        CharLine <<= 1;
      } /* endfor */
    } /* endfor */
  } else
  for (int i = 0; i < charH; i++, y++)
  {
    register unsigned long *VRAM = (unsigned long *)GetPixelAt (x, y);
    register char CharLine = *CharImage++;
    if (bg < 0)
      for (int j = 0; j < charW; j++)
      {
        if (CharLine & 0x80)
          *VRAM++ = fg;
        else
          VRAM++;
        CharLine <<= 1;
      } else
      for (int j = 0; j < charW; j++)
      {
        if (CharLine & 0x80)
          *VRAM++ = fg;
        else
          *VRAM++ = bg;
        CharLine <<= 1;
      } /* endfor */
  } /* endfor */
}

#define DRAWSPRITE_NAME DrawSprite8
#define DRAWSPRITE_PIXTYPE UByte
#include "drawsprt.inc"

#define DRAWSPRITE_NAME DrawSprite16
#define DRAWSPRITE_PIXTYPE UShort
#include "drawsprt.inc"

#define DRAWSPRITE_NAME DrawSprite32
#define DRAWSPRITE_PIXTYPE ULong
#include "drawsprt.inc"

void csGraphics2D::SetClipRect (int xmin, int ymin, int xmax, int ymax)
{
  ClipX1 = xmin; ClipX2 = xmax;
  ClipY1 = ymin; ClipY2 = ymax;
}

void csGraphics2D::GetClipRect (int &xmin, int &ymin, int &xmax, int &ymax)
{
  xmin = ClipX1; xmax = ClipX2;
  ymin = ClipY1; ymax = ClipY2;
}

// This algorithm has been borrowed about eight years ago from a book on
// computer graphics, I believe its author was R.Wilson or something alike.
// It was first converted from ASM to Pascal, and now I converted it to C :-)
bool csGraphics2D::ClipLine (float &x1, float &y1, float &x2, float &y2,
  int xmin, int ymin, int xmax, int ymax)
{
  // Shrink clip area by one pixel
  float fxmin = xmin;
  float fxmax = xmax - 1;
  float fymin = ymin;
  float fymax = ymax - 1;

#define CLIP_LEFT   0x01
#define CLIP_TOP    0x02
#define CLIP_RIGHT  0x04
#define CLIP_BOTTOM 0x08

#define SetOutCodes(u, x, y)                \
  if (x < fxmin) u = CLIP_LEFT; else u = 0; \
  if (y < fymin) u |= CLIP_TOP;             \
  if (x > fxmax) u |= CLIP_RIGHT;           \
  if (y > fymax) u |= CLIP_BOTTOM;

#define FSWAP(a,b) { float __tmp__ = a; a = b; b = __tmp__; }
#define CSWAP(a,b) { char __tmp__ = a; a = b; b = __tmp__; }

  char ocu1,ocu2;
  SetOutCodes (ocu1, x1, y1);
  SetOutCodes (ocu2, x2, y2);

  bool Inside,Outside;
  Inside  = (ocu1 | ocu2) == 0;
  Outside = (ocu1 & ocu2) != 0;

  while ((!Outside) && (!Inside))
  {
    if (ocu1 == 0)                      // swap endpoints if necessary
    {                                   // so that (x1,y1) needs to be clipped
      FSWAP (x1, x2);
      FSWAP (y1, y2);
      CSWAP (ocu1, ocu2);
    }
    if (ocu1 & CLIP_LEFT)               // clip left
    {
      y1 = y1 + ((y2 - y1) * (fxmin - x1)) / (x2 - x1);
      x1 = fxmin;
    }
    else if (ocu1 & CLIP_TOP)           // clip above
    {
      x1 = x1 + ((x2 - x1) * (fymin - y1)) / (y2 - y1);
      y1 = fymin;
    }
    else if (ocu1 & CLIP_RIGHT)         // clip right
    {
      y1 = y1 + ((y2 - y1) * (fxmax - x1)) / (x2 - x1);
      x1 = fxmax;
    }
    else if (ocu1 & CLIP_BOTTOM)        // clip below
    {
      x1 = x1 + ((x2 - x1) * (fymax - y1)) / (y2 - y1);
      y1 = fymax;
    }
    SetOutCodes (ocu1, x1, y1);         // update for (x1,y1)
    Inside  = (ocu1 | ocu2) == 0;
    Outside = (ocu1 & ocu2) != 0;
  }
  return Outside;
}

bool csGraphics2D::SaveArea (ImageArea *&Area, int x, int y, int w, int h)
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
  {
    Area = NULL;
    return true;
  } /* endif */

  CHK (Area = new ImageArea (x, y, w, h));
  if (!Area)
    return false;
  w *= pfmt.PixelBytes;
  CHK (char *dest = Area->data = new char [w * h]);
  if (!dest)
    return false;
  for (; h > 0; y++, h--)
  {
    unsigned char *VRAM = GetPixelAt (x, y);
    memcpy (dest, VRAM, w);
    dest += w;
  } /* endfor */
  return true;
}

void csGraphics2D::RestoreArea (ImageArea *Area, bool Free)
{
  if (Area)
  {
    char *dest = Area->data;
    int x = Area->x, y = Area->y, w = Area->w, h = Area->h;
    w *= pfmt.PixelBytes;
    for (; h; y++, h--)
    {
      unsigned char *VRAM = GetPixelAt (x, y);
      memcpy (VRAM, dest, w);
      dest += w;
    } /* endfor */
    if (Free)
      FreeArea (Area);
  } /* endif */
}

void csGraphics2D::FreeArea (ImageArea *Area)
{
  if (Area)
  {
    if (Area->data)
      CHKB (delete[] Area->data);
    CHK (delete Area);
  } /* endif */
}

void csGraphics2D::SetRGB (int i, int r, int g, int b)
{
  Palette[i].red = r;
  Palette[i].green = g;
  Palette[i].blue = b;
  PaletteAlloc[i] = true;
}

unsigned char *csGraphics2D::GetPixelAt8 (int x, int y)
{
  return (Memory + (x + LineAddress[y]));
}

unsigned char *csGraphics2D::GetPixelAt16 (int x, int y)
{
  return (Memory + (x + x + LineAddress[y]));
}

unsigned char *csGraphics2D::GetPixelAt32 (int x, int y)
{
  return (Memory + ((x<<2) + LineAddress[y]));
}

bool csGraphics2D::SetMousePosition (int x, int y)
{
  (void)x; (void)y;
  return false;
}

bool csGraphics2D::SetMouseCursor (int iShape, ITextureHandle *hBitmap)
{
  (void)hBitmap;
  return (iShape == csmcArrow);
}

void csGraphics2D::GetStringError(HRESULT hRes, char* szError)
{
  (void)hRes; (void)szError;
  strcpy(szError, "Internal Error.");
}

bool csGraphics2D::PerformExtension (char* args)
{
  (void)args;
  return false;
}

void csGraphics2D::SysPrintf(int mode, char* text, ...)
{
  char buf[1024];
  va_list arg;
	
  va_start (arg, text);
  vsprintf (buf, text, arg);
  va_end (arg);
	
  system->Print(mode, buf);
}

int csGraphics2D::GetTextWidth (int Font, char *text)
{
  if (FontList [Font].IndividualWidth)
  {
    int w = 0;
    for (int i = strlen (text); i > 0; i--, text++)
      w += FontList [Font].IndividualWidth[(unsigned char)*text];
    return w;
  } else
    return (strlen (text) * FontList [Font].Width);
}

int csGraphics2D::GetTextHeight (int Font)
{
  return FontList [Font].Height;
}
