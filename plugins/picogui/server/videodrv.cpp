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
  #include <picogui.h>
  #include <pgserver/types.h>
  #include <pgserver/video.h>
}

#include "videodrv.h"
#include "csgeom/csrect.h"
#include "csgfx/memimage.h"

csRef<iGraphics2D> csPGVideoDriver::Gfx2D;

bool csPGVideoDriver::Construct (iGraphics2D *g)
{
  Gfx2D = g;
  g_error err = load_vidlib
    (RegFunc, g->GetWidth (), g->GetHeight (), g->GetPixelBytes () * 8, 0);
  return err == 0;
}

g_error csPGVideoDriver::RegFunc (vidlib *v)
{
  v->init			= Init;
  v->setmode			= SetMode;
  v->close			= Close;
  v->coord_logicalize		= CoordLogicalize;
  v->update			= Update;
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
  v->sprite_show		= SpriteShow;
  v->sprite_hide		= SpriteHide;
  v->lxres = v->xres		= Gfx2D->GetWidth ();
  v->lyres = v->yres		= Gfx2D->GetHeight ();
  return 0;
}

g_error csPGVideoDriver::Init ()
{
  return 0;
}

void csPGVideoDriver::Close ()
{
}

g_error csPGVideoDriver::SetMode (int16 x, int16 y, int16 bpp, uint32 flags)
{
  Gfx2D->Resize (x, y);
  return 0;
}

void csPGVideoDriver::CoordLogicalize (int *x, int *y)
{
  if (*x < 0) *x = 0;
  else if (*x > Gfx2D->GetWidth()) *x = Gfx2D->GetWidth ();
  if (*y < 0) *y = 0;
  else if (*y > Gfx2D->GetHeight()) *y = Gfx2D->GetHeight ();
}

void csPGVideoDriver::Update (stdbitmap *b, int16 x, int16 y, int16 w, int16 h)
{
  if (GETBMP (b)->G2D ())
    GETBMP (b)->G2D ()->Print (& csRect (x, y, x + w, y + h));
}

uint32 csPGVideoDriver::ColorPG2CS (uint32 c)
{
  return Gfx2D->FindRGB (getred (c), getgreen (c), getblue (c));
}

uint32 csPGVideoDriver::ColorCS2PG (uint32 c)
{
  //@@@: Kludge. Why is there no method in iGraphics2D to convert a color value
  //     back into R, G and B values?
  uint8 r, g, b, or, og, ob;
  Gfx2D->GetPixel (0, 0, or, og, ob);
  Gfx2D->DrawPixel (0, 0, c);
  Gfx2D->GetPixel (0, 0, r, g, b);
  Gfx2D->DrawPixel (0, 0, Gfx2D->FindRGB (or, og, ob));
  return mkcolor (r, g, b);
}

void csPGVideoDriver::Pixel (stdbitmap *b, int16 x, int16 y,
  uint32 color, int16 lgop)
{
  GETBMP (b)->G2D ()->DrawPixel (x, y, color);
}

uint32 csPGVideoDriver::GetPixel (stdbitmap *b, int16 x, int16 y)
{
  if (GETBMP (b)->G2D ())
  {
    uint8 r, g, b;
    GETBMP (b)->G2D ()->GetPixel (x, y, r, g, b);
    return GETBMP (b)->G2D ()->FindRGB (r, g, b);
  }
  else
    return 0;
}

void csPGVideoDriver::Slab (stdbitmap *b, int16 x, int16 y, int16 w,
  uint32 color, int16 lgop)
{
  if (GETBMP (b)->G2D ()) GETBMP (b)->G2D ()->DrawBox (x, y, w, 1, color);
}

void csPGVideoDriver::Bar (stdbitmap *b, int16 x, int16 y, int16 h,
  uint32 color, int16 lgop)
{
  if (GETBMP (b)->G2D ()) GETBMP (b)->G2D ()->DrawBox (x, y, 1, h, color);
}

void csPGVideoDriver::Line (stdbitmap *b, int16 x1, int16 y1, int16 x2, int16 y2,
  uint32 color, int16 lgop)
{
  if (GETBMP (b)->G2D ()) GETBMP (b)->G2D ()->DrawLine (x1, y1, x2, y2, color);
}

void csPGVideoDriver::Rect (stdbitmap *b, int16 x1, int16 y1, int16 x2, int16 y2,
  uint32 color, int16 lgop)
{
  if (GETBMP (b)->G2D ()) GETBMP (b)->G2D ()->DrawBox (x1, y1, x2 - x1, y2 - y1, color);
}

void csPGVideoDriver::Blit (stdbitmap *b, int16 x, int16 y, int16 w, int16 h,
  stdbitmap *p, int16 px, int16 py, int16 lgop)
{
  csRef<iImage> crop = GETBMP (p)->Image ()->Crop (px, py, w, h);
  crop->SetFormat (CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA);
  GETBMP (b)->G2D ()->Blit (x, y, w, h, (unsigned char *) crop->GetImageData ());
}

g_error csPGVideoDriver::New (stdbitmap **b, int16 w, int16 h, uint16 bpp)
{
  SETBMP (b, new csHwrBitmap (new csImageMemory (w, h,
    bpp <= 8 ? CS_IMGFMT_PALETTED8 : CS_IMGFMT_TRUECOLOR)));
  return 0;
}

g_error csPGVideoDriver::Load (stdbitmap **b, const uint8 *data, uint32 len)
{
  SETBMP (b, new csHwrBitmap (new csImageMemory (len, 1, (void *) data, false,
    CS_IMGFMT_ANY)));
  return 0;
}

g_error csPGVideoDriver::GetSize (stdbitmap *b, int16 *w, int16 *h)
{
  if (GETBMP (b)->G2D ())
  {
    *w = GETBMP (b)->G2D ()->GetWidth ();
    *h = GETBMP (b)->G2D ()->GetHeight ();
  }
  else
  {
    *w = GETBMP (b)->Image ()->GetWidth ();
    *h = GETBMP (b)->Image ()->GetHeight ();
  }
  return 0;
}

g_error csPGVideoDriver::GetGropRender (stdbitmap *b, groprender **g)
{
  *g = GETBMP (b)->Grop ();
  return 0;
}

g_error csPGVideoDriver::GetShareMem (stdbitmap *b, uint32 uid, pgshmbitmap *info)
{
  //@@@: Not implemented
  return 1;
}

void csPGVideoDriver::SpriteShow (sprite *s)
{
  s->visible = 1;
}

void csPGVideoDriver::SpriteHide (sprite *s)
{
  s->visible = 0;
}

void csPGVideoDriver::SpriteRedraw (sprite *s)
{
  iImage *img = GETBMP (* s->bitmap)->Image ();
  img->SetFormat (CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA);
  Gfx2D->Blit (s->x, s->y, img->GetWidth (), img->GetHeight (),
    (unsigned char *) img->GetImageData ());
}
