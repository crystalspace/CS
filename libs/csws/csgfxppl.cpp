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
#include "csqint.h"
#include "csws/csgfxppl.h"
#include "csws/csmouse.h"
#include "csutil/util.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "iutil/plugin.h"

CS_LEAKGUARD_IMPLEMENT (csGraphicsPipeline);

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

csGraphicsPipeline::~csGraphicsPipeline ()
{
  Desync ();
}

void csGraphicsPipeline::Initialize (iObjectRegistry *object_reg)
{
  MaxPage = 0;
  DrawMode = 0;
  memset (SyncArea, 0, sizeof (SyncArea));
  G3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (G3D)
  {
    G2D = G3D->GetDriver2D ();
  }
  else
  {
    G2D = CS_QUERY_REGISTRY (object_reg, iGraphics2D);
  }
  RefreshRect.Set (INT_MAX, INT_MAX, INT_MIN, INT_MIN);
  CanvasResize ();
}

void csGraphicsPipeline::CanvasResize ()
{
  if (G2D)
  {
    FrameWidth = G2D->GetWidth ();
    FrameHeight = G2D->GetHeight ();
  }
}

bool csGraphicsPipeline::ClipLine (float &x1, float &y1, float &x2, float &y2,
  int ClipX1, int ClipY1, int ClipX2, int ClipY2)
{
  return G2D->ClipLine (x1, y1, x2, y2, ClipX1, ClipY1, ClipX2, ClipY2);
}

void csGraphicsPipeline::GetPixel (int x, int y, uint8 &oR, uint8 &oG, uint8 &oB)
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

  INCLUDE_POINT (csQint (x1), csQint (y1));
  INCLUDE_POINT (csQint (x2), csQint (y2));
  G2D->DrawLine (x1, y1, x2, y2, color);
}

void csGraphicsPipeline::Pixel (int x, int y, int color)
{
  if (!BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  INCLUDE_POINT (x, y);
  G2D->DrawPixel (x, y, color);
}

void csGraphicsPipeline::Text (int x, int y, int fg, int bg, iFont *font,
  const char *s)
{
  if (!BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  int fh, fw;
  font->GetDimensions (s, fw, fh);
  INCLUDE_MIN_POINT (x, y);
  INCLUDE_MAX_POINT (x + fw, y + fh);

  G2D->Write (font, x, y, fg, bg, s);
}

void csGraphicsPipeline::Pixmap (csPixmap *s2d, int x, int y, int w, int h,
  uint8 Alpha)
{
  if (!BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  INCLUDE_MIN_POINT (x, y);
  INCLUDE_MAX_POINT (x + w, y + h);
  s2d->DrawScaled (G3D, x, y, w, h, Alpha);
}

void csGraphicsPipeline::TiledPixmap (csPixmap *s2d, int x, int y, int w, int h,
  int orgx, int orgy, uint8 Alpha)
{
  if (!BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  INCLUDE_MIN_POINT (x, y);
  INCLUDE_MAX_POINT (x + w, y + h);
  s2d->DrawTiled (G3D, x, y, w, h, orgx, orgy, Alpha);
}

void csGraphicsPipeline::Texture (iTextureHandle *hTex, int sx, int sy,
  int sw, int sh, int tx, int ty, int tw, int th, uint8 Alpha)
{
  if (!BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  INCLUDE_MIN_POINT (sx, sy);
  INCLUDE_MAX_POINT (sx + sw, sy + sh);
  G3D->DrawPixmap (hTex, sx, sy, sw, sh, tx, ty, tw, th, Alpha);
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

void csGraphicsPipeline::Polygon3D (G3DPolygonDPFX &poly, uint mode)
{
  if (!BeginDraw (CSDRAW_3DGRAPHICS))
    return;

  int i;
  for (i = 0; i < poly.num; i++)
  {
    int x = csQint (poly.vertices [i].x);
    int y = FrameHeight - 1 - csQint (poly.vertices [i].y);
    INCLUDE_MIN_POINT (x, y);
    INCLUDE_MAX_POINT (x + 1, y + 1);
  }

  poly.mixmode = mode;
  G3D->DrawPolygonFX (poly);
}

void csGraphicsPipeline::ClearZbuffer (int x1, int y1, int x2, int y2)
{
  if (!BeginDraw (CSDRAW_3DGRAPHICS))
    return;

  SetZbufferMode (CS_ZBUF_FILLONLY);

  G3DPolygonDP poly;
  memset (&poly, 0, sizeof (poly));
  poly.num = 4;
  y1 = FrameHeight - y1;
  y2 = FrameHeight - y2;
  poly.vertices [0].x = x1;
  poly.vertices [0].y = y1;
  poly.vertices [1].x = x1;
  poly.vertices [1].y = y2;
  poly.vertices [2].x = x2;
  poly.vertices [2].y = y2;
  poly.vertices [3].x = x2;
  poly.vertices [3].y = y1;
  // Set plane normal to be perpendicular to OZ and very far away
  poly.normal.Set (0, 0, -1.0f, 1e35f);
  G3D->DrawPolygon (poly);
}

void csGraphicsPipeline::Invalidate (csRect &rect)
{
  if (rect.IsEmpty ())
    return;
  INCLUDE_MIN_POINT (rect.xmin, rect.ymin);
  INCLUDE_MAX_POINT (rect.xmax, rect.ymax);
}

void csGraphicsPipeline::StartFrame (csMouse *Mouse)
{
  DontCacheFrame = false;
  CurPage = G2D->GetPage ();
  if (CurPage > MaxPage)
    MaxPage = CurPage;

  // Clear refresh rectangle
  RefreshRect.Set (INT_MAX, INT_MAX, -INT_MAX, -INT_MAX);

  // Restore previous background under mouse associated with current page
  Mouse->Undraw (CurPage);

  // First, synchronize image with other video pages, if any
  PageCarry.Set (RefreshRect);
  Sync (CurPage, PageCarry.xmin, PageCarry.ymin, PageCarry.xmax, PageCarry.ymax);

  // Clear refresh rectangle again
  RefreshRect.Set (INT_MAX, INT_MAX, -INT_MAX, -INT_MAX);
}

void csGraphicsPipeline::FinishFrame (csMouse *Mouse)
{
  if (SyncArea [CurPage])
  {
    G2D->FreeArea (SyncArea [CurPage]);
    SyncArea [CurPage] = 0;
  }

  if (!RefreshRect.IsEmpty ())
  {
    // to avoid fiddling too much with floating-point coords, we just enlarge
    // the min/max bounding box in both directions by one pixel.
    RefreshRect.xmin--; RefreshRect.xmax++;
    RefreshRect.ymin--; RefreshRect.ymax++;

    if (RefreshRect.xmin < 0) RefreshRect.xmin = 0;
    if (RefreshRect.ymin < 0) RefreshRect.ymin = 0;
    if (RefreshRect.xmax > FrameWidth) RefreshRect.xmax = FrameWidth;
    if (RefreshRect.ymax > FrameHeight) RefreshRect.ymax = FrameHeight;

    if (G2D->GetDoubleBufferState () && !DontCacheFrame)
      SyncArea [CurPage] = G2D->SaveArea (RefreshRect.xmin, RefreshRect.ymin,
        RefreshRect.Width (), RefreshRect.Height ());
  }

  // Save background under mouse and draw mouse cursor
  Mouse->Draw (CurPage);
  // Switch the backbuffer
  FinishDraw ();
}

void csGraphicsPipeline::FinishDraw ()
{
  FinishDrawImp ();

  PageCarry.Union (RefreshRect);
  if (!PageCarry.IsEmpty ())
    G3D->Print (&PageCarry);
  DrawMode = 0;
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

  if (!BeginDraw (CSDRAW_2DGRAPHICS))
    return;

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
      SyncArea [i] = 0;
    } /* endif */
}

bool csGraphicsPipeline::BeginDrawImp (int iMode)
{
  int clearz = (DrawMode & CSDRAW_CLEARZBUFFER);

  if (DrawMode != 0)
    FinishDrawImp ();

  if (G3D->BeginDraw ((DrawMode = iMode) | clearz))
  {
    G2D->GetClipRect (OrigClip.xmin, OrigClip.ymin, OrigClip.xmax, OrigClip.ymax);
    ClipRect = OrigClip;
    return true;
  }

  // Failed to start drawing
  DrawMode = 0;
  return false;
}

void csGraphicsPipeline::FinishDrawImp ()
{
  // Restore clip rectangle, font id and font size
  G2D->SetClipRect (OrigClip.xmin, OrigClip.ymin, OrigClip.xmax, OrigClip.ymax);
  G3D->FinishDraw ();
}
