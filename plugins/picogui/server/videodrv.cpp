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

csRef<iGraphics2D> csPGVideoDriver::Gfx2D;
csRef<iImageIO> csPGVideoDriver::ImageIO;

bool csPGVideoDriver::Construct (iGraphics2D *g, iImageIO *i)
{
  Gfx2D = g;
  ImageIO = i;
  videodrivers->regfunc = RegFunc;

  return true;
}

g_error csPGVideoDriver::RegFunc (vidlib *v)
{
  setvbl_linear32 (v);
  v->init			= Init;
  v->setmode			= SetMode;
  v->close			= Close;
  v->coord_logicalize		= CoordLogicalize;
  // v->update			= Update;
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
  v->lxres = v->xres		= Gfx2D->GetWidth ();
  v->lyres = v->yres		= Gfx2D->GetHeight ();
  v->bpp                        = Gfx2D->GetPixelBytes () * 8;
  v->grop_render_presetup_hook  = BeginDraw;

  return 0;
}

g_error csPGVideoDriver::Init ()
{
  // Create an intermediate canvas, since we will want to read from it.
  // @@@ What's responsible for deleting data? /Anders Stenberg
  /*void *data = new char[(vid->bpp>>3)*vid->xres*vid->yres];
  memset (data, 0, (vid->bpp>>3)*vid->xres*vid->yres);
  // csRef<iGraphics2D> canvas = 
  //   Gfx2D->CreateOffscreenCanvas (data, vid->xres, vid->yres, vid->bpp, 0);
  */
  vid->display = (hwrbitmap)new csHwrBitmap (Gfx2D);
  vid->display->bits = 0;
  return 0;
}

void csPGVideoDriver::Close ()
{
  ImageIO = 0;
  Gfx2D = 0;
}

int csPGVideoDriver::BeginDraw (struct divnode **div, struct gropnode ***listp,
  struct groprender *rend)
{
  return 0;
};

// Unsigned long for flags due to MSVC/posix typedef differences
// Works in gcc too, right? Otherwise someone on posix fix it :)
// - Anders Stenberg
g_error csPGVideoDriver::SetMode (int16 x, int16 y, int16 bpp, 
                                  __u32 flags)
{
  delete vid->display->bits;
  vid->display->bits = new u8[vid->xres*vid->yres*(vid->bpp>>3)];
  vid->display->pitch = vid->xres*(vid->bpp>>3);

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

void csPGVideoDriver::Update (hwrbitmap b, int16 x, int16 y, int16 w, int16 h)
{ 
  if (b != vid->display) return;

  Gfx2D->BeginDraw ();
  /*Blit ((hwrbitmap)&csHwrBitmap (Gfx2D), x, y, w, h, 
        vid->display, x, y, PG_LGOP_NONE);*/
  for (int l=0; l<h; l++)
    Gfx2D->Blit (x, y+l, w, 1, vid->display->bits+
      x*(vid->bpp>>3)+
      (y+l)*vid->display->pitch);
  Gfx2D->Print (& csRect (x, y, x + w, y + h));
  Gfx2D->FinishDraw ();
}

hwrcolor csPGVideoDriver::ColorPG2CS (hwrcolor c)
{
  return Gfx2D->FindRGB (getred (c), getgreen (c), getblue (c));
}

hwrcolor csPGVideoDriver::ColorCS2PG (hwrcolor c)
{
  //@@@: Kludge. Why is there no method in iGraphics2D to convert a color value
  //     back into R, G and B values?
  uint8 r, g, b, oldr, oldg, oldb;
  Gfx2D->GetPixel (0, 0, oldr, oldg, oldb);
  Gfx2D->DrawPixel (0, 0, c);
  Gfx2D->GetPixel (0, 0, r, g, b);
  Gfx2D->DrawPixel (0, 0, Gfx2D->FindRGB (oldr, oldg, oldb));
  return mkcolor (r, g, b);
}

void csPGVideoDriver::Pixel (hwrbitmap b, int16 x, int16 y,
  hwrcolor color, int16 lgop)
{
  hwrcolor newcol = color;
  hwrcolor oldcol;
  // These logical operations will be slow. Just to make it work for now.
  switch (lgop)
  {
  case PG_LGOP_OR:
    oldcol = GetPixel (b, x, y);
    newcol = oldcol | color;
    break;
  case PG_LGOP_AND:
    oldcol = GetPixel (b, x, y);
    newcol = oldcol & color;
    break;
  case PG_LGOP_XOR:
    oldcol = GetPixel (b, x, y);
    newcol = oldcol ^ color;
    break;
  case PG_LGOP_INVERT:
    newcol = ~color;
    break;
  case PG_LGOP_INVERT_OR:
    oldcol = GetPixel (b, x, y);
    newcol = oldcol | ~color;
    break;
  case PG_LGOP_INVERT_AND:
    oldcol = GetPixel (b, x, y);
    newcol = oldcol & ~color;
    break;
  case PG_LGOP_INVERT_XOR:
    oldcol = GetPixel (b, x, y);
    newcol = oldcol ^ ~color;
    break;
  case PG_LGOP_ADD:
    oldcol = GetPixel (b, x, y);
    newcol = (min (getred(oldcol)+getred(color),255)<<16)+
             (min (getgreen(oldcol)+getgreen(color),255)<<8)+
             (min (getblue(oldcol)+getblue(color),255));
    break;
  case PG_LGOP_SUBTRACT:
    oldcol = GetPixel (b, x, y);
    newcol = (max (getred(oldcol)-getred(color),0)<<16)+
             (max (getgreen(oldcol)-getgreen(color),0)<<8)+
             (max (getblue(oldcol)-getblue(color),0));
    break;
  case PG_LGOP_MULTIPLY:
    oldcol = GetPixel (b, x, y);
    newcol = ((min (getred(oldcol)*getred(color),65535)<<8)&0xFF0000)+
             ((min (getgreen(oldcol)*getgreen(color),65535))&0xFF00)+
             (min (getblue(oldcol)*getblue(color),65535)>>8);
    break;
  case PG_LGOP_ALPHA:
    newcol = ((min (getred(oldcol)*(255-getalpha(color))+
                    getred(color)*getalpha(color),65535)<<8)&0xFF0000)+
             ((min (getgreen(oldcol)*(255-getalpha(color))+
                    getgreen(color)*getalpha(color),65535))&0xFF00)+
             (min (getblue(oldcol)*(255-getalpha(color))+
                   getblue(color)*getalpha(color),65535)>>8);
    break;
  }
  GETBMP (b)->G2D ()->DrawPixel (x, y, newcol);
}

hwrcolor csPGVideoDriver::GetPixel (hwrbitmap b, int16 x, int16 y)
{
  uint8 red, green, blue;
  GETBMP (b)->G2D ()->GetPixel (x, y, red, green, blue);
  return GETBMP (b)->G2D ()->FindRGB (red, green, blue);
}

void csPGVideoDriver::Slab (hwrbitmap b, int16 x, int16 y, int16 w,
  hwrcolor color, int16 lgop)
{
  if (lgop != PG_LGOP_NONE)
    def_slab (b, x, y, w, color, lgop);
  GETBMP (b)->G2D ()->DrawBox (x, y, w, 1, color);
}

void csPGVideoDriver::Bar (hwrbitmap b, int16 x, int16 y, int16 h,
  hwrcolor color, int16 lgop)
{
  if (lgop != PG_LGOP_NONE)
    def_bar (b, x, y, h, color, lgop);
  GETBMP (b)->G2D ()->DrawBox (x, y, 1, h, color);
}

void csPGVideoDriver::Line (hwrbitmap b, int16 x1, int16 y1, int16 x2, int16 y2,
  hwrcolor color, int16 lgop)
{
  if (lgop != PG_LGOP_NONE)
    def_line (b, x1, y1, x2, y2, color, lgop);
  GETBMP (b)->G2D ()->DrawLine (x1, y1, x2, y2, color);
}

void csPGVideoDriver::Rect (hwrbitmap b, int16 x1, int16 y1, int16 x2, int16 y2,
  hwrcolor color, int16 lgop)
{
  if (lgop != PG_LGOP_NONE)
    def_rect (b, x1, y1, x2, y2, color, lgop);
  GETBMP (b)->G2D ()->DrawBox (x1, y1, x2 - x1, y2 - y1, color);
}

void csPGVideoDriver::Blit (hwrbitmap b, int16 x, int16 y, int16 w, int16 h,
  hwrbitmap p, int16 px, int16 py, int16 lgop)
{
  csImageArea* srcarea = GETBMP (p)->G2D ()->SaveArea (px, py, w, h);

  int srcformat, dstformat;
  switch (GETBMP (p)->G2D ()->GetPixelBytes ())
  {
  case 1:
    srcformat = CS_IMGFMT_PALETTED8;
    break;
  case 4:
    srcformat = CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA;
    break;
  }

  switch (GETBMP (b)->G2D ()->GetPixelBytes ())
  {
  case 1:
    dstformat = CS_IMGFMT_PALETTED8;
    break;
  case 4:
    dstformat = CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA;
    break;
  }

  csRef<iImage> crop = new csImageMemory (
    w, h, srcarea->data, false, srcformat, GETBMP (p)->G2D ()->GetPalette ());
  crop->SetFormat (dstformat);
  unsigned char* srcdata = (unsigned char*)crop->GetImageData ();

  if (lgop != PG_LGOP_NONE)
  {
    csImageArea* dstarea = GETBMP (b)->G2D ()->SaveArea (x, y, w, h);
    unsigned char* s = srcdata;
    unsigned char* d = (unsigned char*)dstarea->data;
    uint32 l = w*h*GETBMP (b)->G2D ()->GetPixelBytes ();
    switch (lgop)
    {
    case PG_LGOP_OR:
      for (int i=0; i<l; i++)
        *(s++) |= *(d++);
      break;
    case PG_LGOP_AND:
      for (int i=0; i<l; i++)
        *(s++) &= *(d++);
      break;
    case PG_LGOP_XOR:
      for (int i=0; i<l; i++)
        *(s++) ^= *(d++);
      break;
    default:
      def_blit (b, x, y, w, h, p, px, py, lgop);
      GETBMP (b)->G2D ()->FreeArea (dstarea);
      GETBMP (p)->G2D ()->FreeArea (srcarea);
      return;
    }
    GETBMP (b)->G2D ()->FreeArea (dstarea);
  }

  GETBMP (b)->G2D ()->Blit (x, y, w, h, (unsigned char *) srcdata);
  GETBMP (p)->G2D ()->FreeArea (srcarea);
}

g_error csPGVideoDriver::New (hwrbitmap *b, int16 w, int16 h, uint16 bpp)
{
  char * nchar = new char[(bpp / 8) * w * h];
  csRef<iGraphics2D> newg2d = Gfx2D->CreateOffscreenCanvas (nchar, w, h, bpp, 0);
  SETBMP (b, new csHwrBitmap (newg2d));
  return 0;
}

g_error csPGVideoDriver::Load (hwrbitmap *b, const uint8 *data, __u32 len)
{

  int format;
  switch (vid->bpp)
  {
    case 8:
      format = CS_IMGFMT_PALETTED8;
      break;
    case 32:
      format = CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA;
      break;
  }

  uint8 *tmp = new uint8[len];
  memcpy (tmp, data, len);
  csRef<iImage> img = ImageIO->Load (tmp, len, format);
  delete tmp;
#if 1
  img->SetFormat (format);
  def_bitmap_new (b, img->GetWidth (), img->GetHeight (), vid->bpp);
  memcpy ((*b)->bits, img->GetImageData (), 
    img->GetWidth ()*img->GetHeight ()*((*b)->bpp>>3));
#else

/*  SETBMP (b, new csHwrBitmap (new csImageMemory (len, 1, (void *) data, false,
    CS_IMGFMT_ANY)));*/

  /*csRef<csImageFile> img = new csImageMemory (
    bmp->w, bmp->h, );*/

  tmp = new uint8[(vid->bpp>>3)*img->GetWidth ()*img->GetHeight ()];
  csRef<iGraphics2D> newg2d = Gfx2D->CreateOffscreenCanvas (
    tmp, img->GetWidth (), img->GetHeight (), vid->bpp, 0);
  newg2d->Blit (0, 0, img->GetWidth (), img->GetHeight (), 
    (unsigned char*)img->GetImageData ());
  SETBMP (b, new csHwrBitmap (newg2d));
#endif

  return 0;
}

g_error csPGVideoDriver::GetSize (hwrbitmap b, int16 *w, int16 *h)
{
  *w = GETBMP (b)->G2D ()->GetWidth ();
  *h = GETBMP (b)->G2D ()->GetHeight ();
  return 0;
}

g_error csPGVideoDriver::GetGropRender (hwrbitmap b, groprender **g)
{
  *g = GETBMP (b)->Grop ();
  return 0;
}

g_error csPGVideoDriver::GetShareMem (hwrbitmap b, __u32 uid, pgshmbitmap *info)
{
#if 0
  if (GETBMP (b)->G2D ())
  {
    return 1;
  }
  else
  {
    int fmt = GETBMP (b)->Image ()->GetFormat ();
    int alpha = fmt & CS_IMGFMT_ALPHA ? PG_BITFORMAT_ALPHA : 0;
    if (fmt & CS_IMGFMT_TRUECOLOR)
    {
      info->format = htonl (PG_BITFORMAT_TRUECOLOR | alpha);
      info->bpp = htons (32);

      info->red_mask   = htonl (0xff000000);
      info->green_mask = htonl (0x00ff0000);
      info->blue_mask  = htonl (0x0000ff00);
      info->alpha_mask = htonl (0x000000ff);

      info->red_shift   = htonl (24);
      info->green_shift = htonl (16);
      info->blue_shift  = htonl (8);
      info->alpha_shift = htonl (0);

      info->red_length = info->green_length = info->blue_length =
        info->alpha_length = htonl (8);
    }
    else if (fmt & CS_IMGFMT_PALETTED8)
    {
      info->format = htonl (PG_BITFORMAT_INDEXED | alpha);
      info->bpp = htons (8);

      info->palette = htonl (GETBMP (b)->Image ()->GetPalette ()->/*TODO*/);
    }
    else return 1;

    info->width = htons (GETBMP (b)->Image ()->GetWidth ());
    info->height = htons (GETBMP (b)->Image ()->GetHeight ());

    int len = GETBMP (b)->Image ()->GetSize ();
    uint8 *mem;
    int32 id, key;
    g_error err = os_shm_alloc (& mem, len, & id, & key, 0);
    if (err) return err;

    GETBMP (b)->ShmID (id);
    info->shm_key = htonl (key);
    info->shm_length = htonl (len);
    memcpy (mem, GETBMP (b)->Image ()->GetImageData (), len);
  }
#endif
  return 0;
}

void csPGVideoDriver::SpriteRedraw (sprite *s)
{
  csRef<iImage> img = GETBMP (* s->bitmap)->G2D ()->ScreenShot ();
  img->SetFormat (CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA);
  Gfx2D->Blit (s->x, s->y, img->GetWidth (), img->GetHeight (),
    (unsigned char *) img->GetImageData ());
}
