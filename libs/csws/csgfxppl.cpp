/*
    Crystal Space Windowing System: Graphics Pipeline class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#include <limits.h>
#include "cssysdef.h"
#include "qint.h"
#include "csws/csgfxppl.h"
#include "csutil/util.h"
#include "isystem.h"
#include "igraph2d.h"
#include "igraph3d.h"

#define INCLUDE_MIN_POINT(x, y)						\
{									\
  register int _x = x; if (_x < ClipRect.xmin) _x = ClipRect.xmin;	\
  register int _y = y; if (_y < ClipRect.ymin) _y = ClipRect.ymin;	\
  if (RefreshRect.xmin > _x) RefreshRect.xmin = _x;			\
  if (RefreshRect.ymin > _y) RefreshRect.ymin = _y;			\
}
#define	INCLUDE_MAX_POINT(x, y)						\
{									\
  register int _x = x; if (_x > ClipRect.xmax) _x = ClipRect.xmax;	\
  register int _y = y; if (_y > ClipRect.ymax) _y = ClipRect.ymax;	\
  if (RefreshRect.xmax < _x) RefreshRect.xmax = _x;			\
  if (RefreshRect.ymax < _y) RefreshRect.ymax = _y;			\
}
#define	INCLUDE_POINT(x, y)						\
{									\
  INCLUDE_MIN_POINT (x, y);						\
  INCLUDE_MAX_POINT (x + 1, y + 1);					\
}

csGraphicsPipeline::csGraphicsPipeline (iSystem *System)
{
  MaxPage = 0;
  DrawMode = 0;
  memset (SyncArea, 0, sizeof (SyncArea));
  G3D = QUERY_PLUGIN (System, iGraphics3D);
  G2D = QUERY_PLUGIN (System, iGraphics2D);
  if (G2D)
  {
    FrameWidth = G2D->GetWidth ();
    FrameHeight = G2D->GetHeight ();
  }
  RefreshRect.Set (INT_MAX, INT_MAX, INT_MIN, INT_MIN);
}

csGraphicsPipeline::~csGraphicsPipeline ()
{
  Desync ();
  if (G2D) G2D->DecRef ();
  if (G3D) G3D->DecRef ();
}

bool csGraphicsPipeline::ClipLine (float &x1, float &y1, float &x2, float &y2,
  int ClipX1, int ClipY1, int ClipX2, int ClipY2)
{
  return G2D->ClipLine (x1, y1, x2, y2, ClipX1, ClipY1, ClipX2, ClipY2);
}

void csGraphicsPipeline::GetPixel (int x, int y, UByte &oR, UByte &oG, UByte &oB)
{
  if (!BeginDraw (CSDRAW_2DGRAPHICS))
  {
    oR = oG = oB = 0;
    return;
  }
  G2D->GetPixel (x, y, oR, oG, oB);
}

bool csGraphicsPipeline::SwitchMouseCursor (csMouseCursorID Shape)
{
  return G2D->SetMouseCursor (Shape);
}

void csGraphicsPipeline::Box (int xmin, int ymin, int xmax, int ymax, int color)
{
  if (!BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  INCLUDE_MIN_POINT (xmin, ymin);
  INCLUDE_MAX_POINT (xmax, ymax);
  G2D->DrawBox (xmin, ymin, xmax - xmin, ymax - ymin, color);
}

void csGraphicsPipeline::Line (float x1, float y1, float x2, float y2, int color)
{
  if (!BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  INCLUDE_POINT (QRound (x1), QRound (y1));
  INCLUDE_POINT (QRound (x2), QRound (y2));
  G2D->DrawLine (x1, y1, x2, y2, color);
}

void csGraphicsPipeline::Pixel (int x, int y, int color)
{
  if (!BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  INCLUDE_POINT (x, y);
  G2D->DrawPixel (x, y, color);
}

void csGraphicsPipeline::Text (int x, int y, int fg, int bg, int font, int fontsize, const char *s)
{
  if (!BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  int tmpx = x + TextWidth (s, font, fontsize);
  int tmpy = y + TextHeight (font, fontsize);
  INCLUDE_MIN_POINT (x, y);
  INCLUDE_MAX_POINT (tmpx, tmpy);

  G2D->SetFontID (font);
  G2D->SetFontSize (fontsize);
  G2D->Write (x, y, fg, bg, s);
}

void csGraphicsPipeline::Sprite2D (csPixmap *s2d, int x, int y, int w, int h)
{
  if (!BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  INCLUDE_MIN_POINT (x, y);
  INCLUDE_MAX_POINT (x + w, y + h);
  s2d->Draw (G3D, x, y, w, h);
}

void csGraphicsPipeline::SaveArea (csImageArea **Area, int x, int y, int w, int h)
{
  if (!BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  *Area = G2D->SaveArea (x, y, w, h);
}

void csGraphicsPipeline::RestoreArea (csImageArea *Area, bool Free)
{
  if (!BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  INCLUDE_MIN_POINT (Area->x, Area->y);
  INCLUDE_MAX_POINT (Area->x + Area->w, Area->y + Area->h);
  G2D->RestoreArea (Area, Free);
}

void csGraphicsPipeline::FreeArea (csImageArea *Area)
{
  G2D->FreeArea (Area);
}

void csGraphicsPipeline::Clear (int color)
{
  if (!BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  INCLUDE_MIN_POINT (0, 0);
  INCLUDE_MAX_POINT (FrameWidth, FrameHeight);
  G2D->Clear (color);
}

void csGraphicsPipeline::SetClipRect (int xmin, int ymin, int xmax, int ymax)
{
  if (!BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  G2D->SetClipRect (ClipRect.xmin = xmin, ClipRect.ymin = ymin,
    ClipRect.xmax = xmax, ClipRect.ymax = ymax);
}

void csGraphicsPipeline::RestoreClipRect ()
{
  if (!BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  G2D->SetClipRect (OrigClip.xmin, OrigClip.ymin, OrigClip.xmax, OrigClip.ymax);
  ClipRect = OrigClip;
}

void csGraphicsPipeline::Polygon3D (G3DPolygonDPFX &poly, UInt mode)
{
  if (!BeginDraw (CSDRAW_3DGRAPHICS))
    return;

  for (int i = 0; i < poly.num; i++)
    INCLUDE_POINT (QRound (poly.vertices [i].sx), QRound (poly.vertices [i].sy));

  G3D->StartPolygonFX (poly.txt_handle, mode);
  G3D->DrawPolygonFX (poly);
  G3D->FinishPolygonFX ();
}

int csGraphicsPipeline::TextWidth (const char *text, int Font, int FontSize)
{
  G2D->SetFontID (Font);
  G2D->SetFontSize (FontSize);
  return text ? G2D->GetTextWidth (Font, text) : 0;
}

int csGraphicsPipeline::TextHeight (int Font, int FontSize)
{
  G2D->SetFontID (Font);
  G2D->SetFontSize (FontSize);
  return G2D->GetTextHeight (Font);
}

void csGraphicsPipeline::StartFrame (int iCurPage)
{
  CurPage = iCurPage;
  if (CurPage > MaxPage)
    MaxPage = CurPage;

  // First, synchronize image with other video pages, if any
  Sync (CurPage, RefreshRect.xmin, RefreshRect.ymin, RefreshRect.xmax,
    RefreshRect.ymax);
}

void csGraphicsPipeline::FinishFrame ()
{
  if (SyncArea [CurPage])
  {
    G2D->FreeArea (SyncArea [CurPage]);
    SyncArea [CurPage] = NULL;
  }

  // to avoid fiddling too much with floating-point coords, we just enlarge
  // the min/max bounding box in both directions by one pixel.
  if (!RefreshRect.IsEmpty ())
  {
    RefreshRect.xmin--; RefreshRect.xmax++; RefreshRect.ymin--; RefreshRect.ymax++;

    if (RefreshRect.xmin < 0) RefreshRect.xmin = 0;
    if (RefreshRect.ymin < 0) RefreshRect.ymin = 0;
    if (RefreshRect.xmax > FrameWidth) RefreshRect.xmax = FrameWidth;
    if (RefreshRect.ymax > FrameHeight) RefreshRect.ymax = FrameHeight;

    if (G2D->GetDoubleBufferState ())
      SyncArea [CurPage] = G2D->SaveArea (RefreshRect.xmin, RefreshRect.ymin,
        RefreshRect.Width (), RefreshRect.Height ());
  }
}

// This procedure propagates all changes made to previous pages
// to the current page.
void csGraphicsPipeline::Sync (int CurPage, int &xmin, int &ymin,
  int &xmax, int &ymax)
{
#define INCLUDE_MIN_SYNC(x, y)						\
  {									\
    if (xmin > x) xmin = x;						\
    if (ymin > y) ymin = y;						\
  }
#define INCLUDE_MAX_SYNC(x, y)						\
  {									\
    if (xmax < x) xmax = x;						\
    if (ymax < y) ymax = y;						\
  }

  int Page = CurPage + 1;
  while (1)
  {
    if (Page > MaxPage)
      Page = 0;
    if (Page == CurPage)
      break;

    if (SyncArea [Page])
    {
      INCLUDE_MIN_SYNC (SyncArea [Page]->x, SyncArea [Page]->y);
      INCLUDE_MAX_SYNC (SyncArea [Page]->x + SyncArea [Page]->w,
        SyncArea [Page]->y + SyncArea [Page]->h);
      G2D->RestoreArea (SyncArea [Page], false);
    }

    Page++;
  } /* endwhile */
}

void csGraphicsPipeline::Desync ()
{
  int i;

  // Discard all backtracking rectangles
  for (i = 0; i < MAX_SYNC_PAGES; i++)
    if (SyncArea [i])
    {
      G2D->FreeArea (SyncArea [i]);
      SyncArea [i] = NULL;
    } /* endif */
}

bool csGraphicsPipeline::BeginDrawImp (int iMode)
{
  if (DrawMode != 0)
  {
    // Restore clip rectangle, font id and font size
    G2D->SetClipRect (OrigClip.xmin, OrigClip.ymin, OrigClip.xmax, OrigClip.ymax);
    G2D->SetFontID (OrigFont);
    G2D->SetFontSize (OrigFontSize);
    G3D->FinishDraw ();
  }
  if (G3D->BeginDraw (DrawMode = iMode))
  {
    G2D->GetClipRect (OrigClip.xmin, OrigClip.ymin, OrigClip.xmax, OrigClip.ymax);
    ClipRect = OrigClip;
    OrigFont = G2D->GetFontID ();
    OrigFontSize = G2D->GetFontSize ();
    return true;
  }
  DrawMode = 0;
  return false;
}

void csGraphicsPipeline::FinishDraw ()
{
  // Restore clip rectangle, font id and font size
  G2D->SetClipRect (OrigClip.xmin, OrigClip.ymin, OrigClip.xmax, OrigClip.ymax);
  G2D->SetFontID (OrigFont);
  G2D->SetFontSize (OrigFontSize);

#if 0 // debug
if (!RefreshRect.IsEmpty ())
{
G2D->DrawLine (RefreshRect.xmin, RefreshRect.ymin, RefreshRect.xmax-1, RefreshRect.ymin, -1);
G2D->DrawLine (RefreshRect.xmin, RefreshRect.ymax-1, RefreshRect.xmax-1, RefreshRect.ymax-1, -1);
G2D->DrawLine (RefreshRect.xmin, RefreshRect.ymin, RefreshRect.xmin, RefreshRect.ymax-1, -1);
G2D->DrawLine (RefreshRect.xmax-1, RefreshRect.ymin, RefreshRect.xmax-1, RefreshRect.ymax-1, -1);
}
#endif

  G3D->FinishDraw ();

  if (!RefreshRect.IsEmpty ())
    G3D->Print (&RefreshRect);
  DrawMode = 0;

  // Clear refresh rectangle now
  RefreshRect.Set (INT_MAX, INT_MAX, INT_MIN, INT_MIN);
}
