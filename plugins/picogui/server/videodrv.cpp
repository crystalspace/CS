/*
    Crystal Space Video Interface for PicoGUI
    (C) 2003 Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

extern "C"
{
  #include <picogui/types.h>
  #include <pgserver/common.h>
  #include <pgserver/init.h>
  #include <pgserver/types.h>
  #include <pgserver/video.h>
}

#include "csgeom/csrect.h"
#include "csgfx/memimage.h"
#include "igraphic/imageio.h"
#include "videodrv.h"

#include <stdlib.h>
#if 0
#include <sys/shm.h>
#endif

csRef<iGraphics3D> csPGVideoDriver::Gfx3D;
csRef<iImageIO> csPGVideoDriver::ImageIO;

bool csPGVideoDriver::Construct (iGraphics3D *g, iImageIO *i)
{
  Gfx3D = g;
  ImageIO = i;
  videodrivers->regfunc = RegFunc;

  return true;
}

g_error csPGVideoDriver::RegFunc (vidlib *v)
{
  setvbl_default (v);
  v->init			= Init;
  v->setmode			= SetMode;
  v->close			= Close;
  //v->coord_logicalize		= CoordLogicalize;
  //v->update			= Update;
  v->is_rootless		= IsRootless;
  v->color_pgtohwr		= ColorPG2CS;
  v->color_hwrtopg		= ColorCS2PG;
  v->pixel			= Pixel;
  v->getpixel			= GetPixel;
  v->slab			= Slab;
  v->bar			= Bar;
  v->line			= Line;
  v->rect			= Rect;
  v->blit			= Blit;
  v->bitmap_load		= Load; 
  v->bitmap_new			= New;
  v->bitmap_free		= Free;
  v->bitmap_getsize		= GetSize;
  v->bitmap_get_groprender	= GetGropRender;
  v->bitmap_getshm		= GetShareMem;
  v->sprite_show                = SpriteShow;
  v->sprite_hide                = SpriteHide;
  v->sprite_update              = SpriteRedraw;
  v->sprite_protectarea         = SpriteProtect;
  v->lxres = v->xres		= Gfx3D->GetDriver2D ()->GetWidth ();
  v->lyres = v->yres		= Gfx3D->GetDriver2D ()->GetHeight ();
  v->bpp                        = Gfx3D->GetDriver2D ()->GetPixelBytes () * 8;
  //v->grop_render_presetup_hook  = BeginDraw;

  return 0;
}

g_error csPGVideoDriver::Init ()
{
  return success;
}

void csPGVideoDriver::Close ()
{
}

g_error csPGVideoDriver::SetMode (int16 x, int16 y, int16 bpp, __u32 flags) 
{
  return 0;
}

int csPGVideoDriver::BeginDraw (divnode **, gropnode ***, groprender *)
{
  return 0;
}

g_error csPGVideoDriver::New (hwrbitmap* b, int16 w, int16 h, uint16 bpp)
{
  csHwrBitmap** csbmp = (csHwrBitmap**)b;
  hwrbitmap picobmp;
  def_bitmap_new(&picobmp, w, h, 32); 
  *csbmp = new csHwrBitmap (picobmp, Gfx3D);
  return success;
}

g_error csPGVideoDriver::Load (hwrbitmap *b, const uint8 *data, __u32 len)
{

  uint8 *tmp = new uint8[len];
  memcpy (tmp, data, len);
  csRef<iImage> img = ImageIO->Load (tmp, len, 
    CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA);
  delete tmp;

  hwrbitmap picobmp;

  def_bitmap_new (&picobmp, img->GetWidth (), img->GetHeight (), 32);
  memcpy (picobmp->bits, img->GetImageData (), 
    img->GetWidth ()*img->GetHeight ()*4);

  csHwrBitmap** csbmp = (csHwrBitmap**)b;
  *csbmp = new csHwrBitmap (picobmp, Gfx3D);

  return success;
}

void csPGVideoDriver::Free (hwrbitmap b)
{
  csHwrBitmap* csbmp = (csHwrBitmap*)b;
  delete csbmp;
}

g_error csPGVideoDriver::GetSize (hwrbitmap b, int16 *w, int16 *h)
{
  hwrbitmap picobmp = ((csHwrBitmap*)b)->GetPicoBitmap ();
  if (picobmp)
  {
    *w = picobmp->w;
    *h = picobmp->h;
  } else {
    *w = Gfx3D->GetDriver2D ()->GetWidth ();
    *h = Gfx3D->GetDriver2D ()->GetHeight ();
  }
  return success;
}

void csPGVideoDriver::Pixel (hwrbitmap b, int16 x, int16 y, 
                             hwrcolor color, int16 logop)
{
  hwrbitmap picobmp = ((csHwrBitmap*)b)->GetPicoBitmap ();
  if (picobmp)
  {
    linear32_pixel (picobmp, x, y, color, logop);
    ((csHwrBitmap*)b)->MarkAsDirty ();
} else {
    // @@@ Should support logop somehow
    Gfx3D->GetDriver2D ()->DrawPixel (x, y, color);
  }
}

hwrcolor csPGVideoDriver::GetPixel (hwrbitmap b, int16 x, int16 y)
{
  hwrbitmap picobmp = ((csHwrBitmap*)b)->GetPicoBitmap ();
  if (picobmp)
  {
    linear32_getpixel (picobmp, x, y);
    ((csHwrBitmap*)b)->MarkAsDirty ();
  } else {
    // @@@ Isn't supported, returning magenta to indicate something's wrong.
    return 0xFF00FF;
  }
}

void csPGVideoDriver::Rect (hwrbitmap b, int16 x1, int16 y1, 
                           int16 x2, int16 y2, hwrcolor color, int16 logop)
{
  hwrbitmap picobmp = ((csHwrBitmap*)b)->GetPicoBitmap ();
  if (picobmp)
  {
    linear32_rect (picobmp, x1, y1, x2, y2, color, logop);
    ((csHwrBitmap*)b)->MarkAsDirty ();
  } else {
    // @@@ Should support logop somehow
    Gfx3D->GetDriver2D ()->DrawBox (x1, y1, x2-x1, y2-y1, color);
  }
}

void csPGVideoDriver::Slab (hwrbitmap b, int16 x, int16 y, 
                           int16 w, hwrcolor color, int16 logop)
{
  hwrbitmap picobmp = ((csHwrBitmap*)b)->GetPicoBitmap ();
  if (picobmp)
  {
    linear32_slab (picobmp, x, y, w, color, logop);
    ((csHwrBitmap*)b)->MarkAsDirty ();
  } else {
    // @@@ Should support logop somehow
    Gfx3D->GetDriver2D ()->DrawBox (x, y, w, 1, color);
  }
}

void csPGVideoDriver::Bar (hwrbitmap b, int16 x, int16 y, 
                           int16 h, hwrcolor color, int16 logop)
{
  hwrbitmap picobmp = ((csHwrBitmap*)b)->GetPicoBitmap ();
  if (picobmp)
  {
    linear32_bar (picobmp, x, y, h, color, logop);
    ((csHwrBitmap*)b)->MarkAsDirty ();
  } else {
    // @@@ Should support logop somehow
    Gfx3D->GetDriver2D ()->DrawBox (x, y, 1, h, color);
  }
}

void csPGVideoDriver::Line (hwrbitmap b, int16 x1, int16 y1, 
                           int16 x2, int16 y2, hwrcolor color, int16 logop)
{
  hwrbitmap picobmp = ((csHwrBitmap*)b)->GetPicoBitmap ();
  if (picobmp)
  {
    // @@@ Something is wrong with linear32_line. Borked headers?
    def_line (picobmp, x1, y1, x2, y2, color, logop);
    ((csHwrBitmap*)b)->MarkAsDirty ();
  } else {
    // @@@ Should support logop somehow
    Gfx3D->GetDriver2D ()->DrawLine (x1, y1, x2, y2, color);
  }
}

void csPGVideoDriver::Blit (hwrbitmap db, int16 dx, int16 dy, 
                            int16 w, int16 h, 
                            hwrbitmap sb, int16 sx, int16 sy, 
                            int16 logop)
{
  hwrbitmap srcpicobmp = ((csHwrBitmap*)sb)->GetPicoBitmap ();
  hwrbitmap dstpicobmp = ((csHwrBitmap*)db)->GetPicoBitmap ();
  if (srcpicobmp && dstpicobmp)
  {
    linear32_blit (dstpicobmp, dx, dy, w, h, srcpicobmp, sx, sy, logop);
    ((csHwrBitmap*)db)->MarkAsDirty ();
  } else if (srcpicobmp && !dstpicobmp) { // pico->cs
    // @@@ Should support logop somehow
    // Not sure if there is a better way to draw parts of a pixmap
    // than to use DrawTiled
    ((csHwrBitmap*)sb)->GetCSBitmap ()->DrawTiled (
      Gfx3D, dx, dy, w, h, dx-sx, dy-sy);
  }
  // cs->pico is not supported
}

g_error csPGVideoDriver::GetShareMem (hwrbitmap, __u32, pgshmbitmap *)
{
  return 0;
}

g_error csPGVideoDriver::GetGropRender (hwrbitmap b, groprender **grop)
{
  def_bitmap_get_groprender (((csHwrBitmap*)b)->GetPicoBitmap (), grop);
}

pgcolor csPGVideoDriver::ColorCS2PG (hwrcolor color)
{
  return color;
}

hwrcolor csPGVideoDriver::ColorPG2CS (pgcolor color)
{
  return color;
}

void csPGVideoDriver::SpriteShow (sprite *spr)
{
  Blit (vid->display, spr->x, spr->y, spr->w, spr->h, *spr->bitmap,
    0, 0, spr->lgop);
}

void csPGVideoDriver::SpriteHide (sprite *)
{
  // Not needed
}

void csPGVideoDriver::SpriteRedraw (sprite *)
{
  // Not needed
}

void csPGVideoDriver::SpriteProtect (pgquad*, sprite *)
{
  // Not needed
}
