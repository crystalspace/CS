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
#include <assert.h>
#include "sysdef.h"
#include "qint.h"
#include "cssys/system.h"
#include "csws/csgfxppl.h"
#include "igraph2d.h"
#include "igraph3d.h"

csGraphicsPipeline::csGraphicsPipeline ()
{
  MaxPage = 0;
  memset (SyncArea, 0, sizeof (SyncArea));
  ClearColor = -1;
  pipelen = 0;
}

csGraphicsPipeline::~csGraphicsPipeline ()
{
  Desync ();
}

void csGraphicsPipeline::Box (int xmin, int ymin, int xmax, int ymax, int color)
{
  csPipeEntry *pe = AllocOp (pipeopBOX);
  if (!pe) return;
  pe->Box.xmin = xmin;
  pe->Box.ymin = ymin;
  pe->Box.xmax = xmax;
  pe->Box.ymax = ymax;
  pe->Box.color = color;
}

void csGraphicsPipeline::Line (float x1, float y1, float x2, float y2, int color)
{
  csPipeEntry *pe = AllocOp (pipeopLINE);
  if (!pe) return;
  pe->Line.x1 = x1;
  pe->Line.y1 = y1;
  pe->Line.x2 = x2;
  pe->Line.y2 = y2;
  pe->Line.color = color;
}

void csGraphicsPipeline::Pixel (int x, int y, int color)
{
  csPipeEntry *pe = AllocOp (pipeopPIXEL);
  if (!pe) return;
  pe->Pixel.x = x;
  pe->Pixel.y = y;
  pe->Pixel.color = color;
}

void csGraphicsPipeline::Text (int x, int y, int fg, int bg, int font, const char *s)
{
  csPipeEntry *pe = AllocOp (pipeopTEXT);
  if (!pe) return;
  pe->Text.x = x;
  pe->Text.y = y;
  pe->Text.fg = fg;
  pe->Text.bg = bg;
  pe->Text.font = font;
  CHK (pe->Text.string = new char[strlen (s) + 1]);
  strcpy (pe->Text.string, s);
}

void csGraphicsPipeline::Sprite2D (csSprite2D *s2d, int x, int y, int w, int h)
{
  csPipeEntry *pe = AllocOp (pipeopSPR2D);
  if (!pe) return;
  pe->Spr2D.s2d = s2d;
  pe->Spr2D.x = x;
  pe->Spr2D.y = y;
  pe->Spr2D.w = w;
  pe->Spr2D.h = h;
}

void csGraphicsPipeline::SaveArea (ImageArea **Area, int x, int y, int w, int h)
{
  csPipeEntry *pe = AllocOp (pipeopSAVAREA);
  if (!pe) return;
  pe->SavArea.Area = Area;
  pe->SavArea.x = x;
  pe->SavArea.y = y;
  pe->SavArea.w = w;
  pe->SavArea.h = h;
}

void csGraphicsPipeline::RestoreArea (ImageArea *Area, bool Free)
{
  csPipeEntry *pe = AllocOp (pipeopRESAREA);
  if (!pe) return;
  pe->ResArea.Area = Area;
  pe->ResArea.Free = Free;
}

void csGraphicsPipeline::FreeArea (ImageArea *Area)
{
  System->piG2D->FreeArea (Area);
}

void csGraphicsPipeline::Clear (int color)
{
  ClearColor = color;
  ClearPage = -1;
}

void csGraphicsPipeline::SetClipRect (int xmin, int ymin, int xmax, int ymax)
{
  csPipeEntry *pe = AllocOp (pipeopSETCLIP);
  if (!pe) return;
  pe->ClipRect.xmin = xmin;
  pe->ClipRect.xmax = xmax;
  pe->ClipRect.ymin = ymin;
  pe->ClipRect.ymax = ymax;
}

void csGraphicsPipeline::RestoreClipRect ()
{
  AllocOp (pipeopRESCLIP);
}

void csGraphicsPipeline::Polygon3D (G3DPolygonDPFX &poly, UInt mode)
{
  csPipeEntry *pe = AllocOp (pipeopPOLY3D);
  if (!pe) return;
  pe->Poly3D.poly = poly;
  pe->Poly3D.mode = mode;
}

// Theoretically Update() could contain a kind of optimizer which would
// minimize redraw operations, but as we for now don't care about speed
// it doesn't. Its complex anyway.
void csGraphicsPipeline::Flush (int iCurPage)
{
  CurPage = iCurPage;
  if (CurPage > MaxPage)
    MaxPage = CurPage;

  int old_font;
  int old_xmin, old_ymin, old_xmax, old_ymax;
  int clip_xmin, clip_ymin, clip_xmax, clip_ymax;
  System->piGI->GetFontID (old_font);
  System->piG2D->GetClipRect (old_xmin, old_ymin, old_xmax, old_ymax);
  clip_xmin = old_xmin; clip_xmax = old_xmax;
  clip_ymin = old_ymin; clip_ymax = old_ymax;

#define INCLUDE_MIN_POINT(x, y)                         \
  {                                                     \
    int _x = x; if (_x < clip_xmin) _x = clip_xmin;     \
    int _y = y; if (_y < clip_ymin) _y = clip_ymin;     \
    if (xmin > _x) xmin = _x;                           \
    if (ymin > _y) ymin = _y;                           \
  }
#define INCLUDE_MAX_POINT(x, y)                         \
  {                                                     \
    int _x = x; if (_x > clip_xmax) _x = clip_xmax;     \
    int _y = y; if (_y > clip_ymax) _y = clip_ymax;     \
    if (xmax < _x) xmax = _x;                           \
    if (ymax < _y) ymax = _y;                           \
  }
#define INCLUDE_POINT(x, y)                             \
  INCLUDE_MIN_POINT (x, y);                             \
  INCLUDE_MAX_POINT (x + 1, y + 1);

  // First, synchronize image with other vide pages, if any
  RefreshRect.Set (INT_MAX, INT_MAX, INT_MIN, INT_MIN);
  Sync (CurPage, RefreshRect.xmin, RefreshRect.ymin, RefreshRect.xmax,
    RefreshRect.ymax);
  // Now paint everything user requested
  int xmin = INT_MAX, xmax = INT_MIN, ymin = INT_MAX, ymax = INT_MIN;
  for (int i = 0; i < pipelen; i++)
  {
    register csPipeEntry *op = &pipeline[i];
    switch (op->Op)
    {
      case pipeopNOP:
        break;
      case pipeopBOX:
      {
        System->piG2D->DrawBox (op->Box.xmin, op->Box.ymin,
          op->Box.xmax - op->Box.xmin, op->Box.ymax - op->Box.ymin,
          op->Box.color);
        INCLUDE_MIN_POINT (op->Box.xmin, op->Box.ymin);
        INCLUDE_MAX_POINT (op->Box.xmax, op->Box.ymax);
        break;
      }
      case pipeopLINE:
        System->piG2D->DrawLine (op->Line.x1, op->Line.y1,
          op->Line.x2, op->Line.y2, op->Line.color);
        INCLUDE_POINT (QRound (op->Line.x1), QRound (op->Line.y1));
        INCLUDE_POINT (QRound (op->Line.x2), QRound (op->Line.y2));
        break;
      case pipeopPIXEL:
        System->piG2D->DrawPixel (op->Pixel.x, op->Pixel.y, op->Pixel.color);
        INCLUDE_POINT (op->Pixel.x, op->Pixel.y);
        break;
      case pipeopTEXT:
      {
        System->piG2D->SetFontID (op->Text.font);
        System->piG2D->Write (op->Text.x, op->Text.y,
         op->Text.fg, op->Text.bg, op->Text.string);
        int tmpx = op->Text.x + TextWidth (op->Text.string, op->Text.font);
        int tmpy = op->Text.y + TextHeight (op->Text.font);
        INCLUDE_MIN_POINT (op->Text.x, op->Text.y);
        INCLUDE_MAX_POINT (tmpx, tmpy);
        CHK (delete[] op->Text.string);
        break;
      }
      case pipeopSPR2D:
        op->Spr2D.s2d->Draw (System->piG2D, op->Spr2D.x, op->Spr2D.y, op->Spr2D.w, op->Spr2D.h);
        INCLUDE_MIN_POINT (op->Spr2D.x, op->Spr2D.y);
        INCLUDE_MAX_POINT (op->Spr2D.x + op->Spr2D.w, op->Spr2D.y + op->Spr2D.h);
        break;
      case pipeopSAVAREA:
        System->piG2D->SaveArea (op->SavArea.Area, op->SavArea.x,
          op->SavArea.y, op->SavArea.w, op->SavArea.h);
        break;
      case pipeopRESAREA:
        INCLUDE_MIN_POINT (op->ResArea.Area->x, op->ResArea.Area->y);
        INCLUDE_MAX_POINT (op->ResArea.Area->x + op->ResArea.Area->w,
          op->ResArea.Area->y + op->ResArea.Area->h);
        System->piG2D->RestoreArea (op->ResArea.Area, op->ResArea.Free);
        break;
      case pipeopSETCLIP:
        System->piG2D->SetClipRect (clip_xmin = op->ClipRect.xmin,
          clip_ymin = op->ClipRect.ymin, clip_xmax = op->ClipRect.xmax,
          clip_ymax = op->ClipRect.ymax);
        break;
      case pipeopRESCLIP:
        System->piG2D->SetClipRect (clip_xmin = old_xmin, clip_ymin = old_ymin,
          clip_xmax = old_xmax, clip_ymax = old_ymax);
        break;
      case pipeopPOLY3D:
        if (System->piG3D->BeginDraw (CSDRAW_3DGRAPHICS) != S_OK)
          return;
        System->piG3D->StartPolygonFX (op->Poly3D.poly.txt_handle, op->Poly3D.mode);
        System->piG3D->DrawPolygonFX (op->Poly3D.poly);
        System->piG3D->FinishPolygonFX ();
        System->piG3D->FinishDraw ();
        System->piG3D->BeginDraw (CSDRAW_2DGRAPHICS);
        break;
      default:
        assert ("unknown graphics pipeline operation in Pipeline::Flush()");
    } /* endswitch */
  } /* endfor */
  pipelen = 0;
  System->piG2D->SetClipRect (old_xmin, old_ymin, old_xmax, old_ymax);
  System->piG2D->SetFontID (old_font);

  if (xmin < 0) xmin = 0; if (xmax > System->FrameWidth)  xmax = System->FrameWidth;
  if (ymin < 0) ymin = 0; if (ymax > System->FrameHeight) ymax = System->FrameHeight;
  SyncRect [CurPage].xmin = xmin; SyncRect [CurPage].xmax = xmax;
  SyncRect [CurPage].ymin = ymin; SyncRect [CurPage].ymax = ymax;
  if (xmin < RefreshRect.xmin) RefreshRect.xmin = xmin;
  if (xmax > RefreshRect.xmax) RefreshRect.xmax = xmax;
  if (ymin < RefreshRect.ymin) RefreshRect.ymin = ymin;
  if (ymax > RefreshRect.ymax) RefreshRect.ymax = ymax;

  if (SyncArea [CurPage])
    System->piG2D->FreeArea (SyncArea [CurPage]);
  bool dblbuff;
  System->piG2D->GetDoubleBufferState (dblbuff);
  if (dblbuff
   && (xmin < xmax) && (ymin < ymax))
    System->piG2D->SaveArea (&SyncArea [CurPage], xmin, ymin, xmax - xmin, ymax - ymin);
  else
    SyncArea [CurPage] = NULL;
}

void csGraphicsPipeline::Desync ()
{
  int i;

  // Discard all backtracking rectangles
  for (i = 0; i < MAX_SYNC_PAGES; i++)
    if (SyncArea [i])
    {
      System->piG2D->FreeArea (SyncArea [i]);
      SyncArea [i] = NULL;
    } /* endif */

  // Discard all 'restore area' operations
  for (i = 0; i < pipelen; i++)
    if (pipeline[i].Op == pipeopRESAREA)
    {
      System->piG2D->FreeArea (pipeline[i].ResArea.Area);
      pipeline[i].Op = pipeopNOP;
    }
}

// This procedure propagates all changes made to other pages onto the
// current page.
void csGraphicsPipeline::Sync (int CurPage, int &xmin, int &ymin,
  int &xmax, int &ymax)
{
#undef INCLUDE_MIN_POINT
#define INCLUDE_MIN_POINT(x, y)                         \
  {                                                     \
    if (xmin > x) xmin = x;                             \
    if (ymin > y) ymin = y;                             \
  }
#undef INCLUDE_MAX_POINT
#define INCLUDE_MAX_POINT(x, y)                         \
  {                                                     \
    if (xmax < x) xmax = x;                             \
    if (ymax < y) ymax = y;                             \
  }

  if (ClearColor >= 0)
  {
    if (ClearPage == -1)
    {
      ClearPage = CurPage;
      Desync ();
    } else if (ClearPage == CurPage)
    {
      ClearColor = -1;
      goto sync;
    } /* endif */
    System->piG2D->Clear (ClearColor);
    INCLUDE_MIN_POINT (0, 0);
    INCLUDE_MAX_POINT (System->FrameWidth, System->FrameHeight);
  } /* endif */

sync:
  int Page = CurPage - 1;
  Page = CurPage - 1;
  while (1)
  {
    if (Page < 0)
      Page = MaxPage;
    if (Page == CurPage)
      break;

    if (SyncArea [Page])
    {
      INCLUDE_MIN_POINT (SyncArea [Page]->x, SyncArea [Page]->y);
      INCLUDE_MAX_POINT (SyncArea [Page]->x + SyncArea [Page]->w,
        SyncArea [Page]->y + SyncArea [Page]->h);
      System->piG2D->RestoreArea (SyncArea [Page], false);
    }

    Page--;
  } /* endwhile */
}

int csGraphicsPipeline::TextWidth (const char *text, int Font)
{
  int Width;
  System->piGI->GetTextWidth (Width, Font, text);
  return Width;
}

int csGraphicsPipeline::TextHeight (int Font)
{
  int Height;
  System->piGI->GetTextHeight (Height, Font);
  return Height;
}

bool csGraphicsPipeline::BeginDraw ()
{
  return SUCCEEDED (System->piG3D->BeginDraw (CSDRAW_2DGRAPHICS));
}

void csGraphicsPipeline::FinishDraw ()
{
  if (!RefreshRect.IsEmpty ())
    System->piG3D->Print (&RefreshRect);
  System->piG3D->FinishDraw ();
}
