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
  /*g_error err = load_vidlib
    (RegFunc, g->GetWidth (), g->GetHeight (), g->GetPixelBytes () * 8, 0);*/
  /*force_external_video_driver (RegFunc, 
    g->GetWidth (), g->GetHeight (), g->GetPixelBytes () * 8, 0);*/

  // Overwrite the first entry with our driver. Tadaa.
  videodrivers->regfunc = RegFunc;

  return true; //err == 0;
}

g_error csPGVideoDriver::RegFunc (vidlib *v)
{
  setvbl_linear32 (v);
  v->init			= Init;
  v->setmode			= (g_error (__cdecl *)(__s16,__s16,__s16,__u32))SetMode;
  v->close			= Close;
  v->coord_logicalize		= CoordLogicalize;
  v->update			= Update;
  v->is_rootless		= IsRootless;
  v->color_pgtohwr		= (hwrcolor (__cdecl *)(pgcolor))ColorPG2CS;
  v->color_hwrtopg		= (pgcolor (__cdecl *)(hwrcolor))ColorCS2PG;
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
  // These are handled by the default implementation.
  /*v->sprite_show		= SpriteShow;
  v->sprite_hide		= SpriteHide;*/
  v->lxres = v->xres		= Gfx2D->GetWidth ();
  v->lyres = v->yres		= Gfx2D->GetHeight ();
  v->bpp                        = Gfx2D->GetPixelBytes ()<<3;
  v->grop_render_presetup_hook  = BeginDraw;


  return 0;
}

g_error csPGVideoDriver::Init ()
{
  // Create an intermediate canvas, since we will want to read from it.
  csRef<iGraphics2D> canvas = 
    Gfx2D->CreateOffscreenCanvas (new char[(vid->bpp>>3)*
    vid->xres*vid->yres], vid->xres, vid->yres, vid->bpp, 0);
  vid->display = (hwrbitmap)new csHwrBitmap (canvas);
  return 0;
}

void csPGVideoDriver::Close ()
{
}

int csPGVideoDriver::BeginDraw (struct divnode **div, struct gropnode ***listp,
  struct groprender *rend)
{
  Gfx2D->BeginDraw ();
  return 0;
};

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

void csPGVideoDriver::Update (hwrbitmap b, int16 x, int16 y, int16 w, int16 h)
{ 
  if (b != vid->display)
    return;

  /*csRef<iImage> shot = GETBMP (vid->display)->Image ();
  //Blit ((hwrbitmap)&csHwrBitmap (Gfx2D), x, y, w, h, vid->display, x, y, 1);
  for (int l=0; l<h; l++)
  {
    Gfx2D->Blit (x, y+l, w, 1, 
      ((unsigned char *) shot->GetImageData ())+
      (x+y*shot->GetWidth ())*
      Gfx2D->GetPixelBytes ());
  }*/
  Gfx2D->Print (& csRect (x, y, x + w, y + h));
}

hwrcolor csPGVideoDriver::ColorPG2CS (hwrcolor c)
{
  return Gfx2D->FindRGB (getred (c), getgreen (c), getblue (c));
}

hwrcolor csPGVideoDriver::ColorCS2PG (hwrcolor c)
{
  //@@@: Kludge. Why is there no method in iGraphics2D to convert a color value
  //     back into R, G and B values?
  uint8 r, g, b, olr, olg, olb;
  Gfx2D->GetPixel (0, 0, olr, olg, olb);
  Gfx2D->DrawPixel (0, 0, c);
  Gfx2D->GetPixel (0, 0, r, g, b);
  Gfx2D->DrawPixel (0, 0, Gfx2D->FindRGB (olr, olg, olb));
  return mkcolor (r, g, b);
}

void csPGVideoDriver::Pixel (hwrbitmap b, int16 x, int16 y,
  hwrcolor color, int16 lgop)
{
  //GETBMP (b)->SetPixel (x, y, color);
  int blah = GETBMP (b)->G2D ()->GetRefCount ();
  int w = GETBMP (b)->G2D ()->GetWidth ();
  int h = GETBMP (b)->G2D ()->GetHeight ();
  csRef<iGraphics2D> g2d = GETBMP (b)->G2D ();
  g2d->DrawPixel (x, y, color);
}

hwrcolor csPGVideoDriver::GetPixel (hwrbitmap bm, int16 x, int16 y)
{
  //return GETBMP (b)->GetPixel (x, y);
  //if (GETBMP (b)->G2D ())
  {
    uint8 r, g, b;
    GETBMP (bm)->G2D ()->GetPixel (x, y, r, g, b);
    return GETBMP (bm)->G2D ()->FindRGB (r, g, b);
  }
  /*else
    return 0;*/
}

void csPGVideoDriver::Slab (hwrbitmap b, int16 x, int16 y, int16 w,
  hwrcolor color, int16 lgop)
{
  if (GETBMP (b)->G2D ()) GETBMP (b)->G2D ()->DrawBox (x, y, w, 1, color);
}

void csPGVideoDriver::Bar (hwrbitmap b, int16 x, int16 y, int16 h,
  hwrcolor color, int16 lgop)
{
  if (GETBMP (b)->G2D ()) GETBMP (b)->G2D ()->DrawBox (x, y, 1, h, color);
}

void csPGVideoDriver::Line (hwrbitmap b, int16 x1, int16 y1, int16 x2, int16 y2,
  hwrcolor color, int16 lgop)
{
  if (GETBMP (b)->G2D ()) GETBMP (b)->G2D ()->DrawLine (x1, y1, x2, y2, color);
}

void csPGVideoDriver::Rect (hwrbitmap b, int16 x1, int16 y1, int16 x2, int16 y2,
  hwrcolor color, int16 lgop)
{
  if (GETBMP (b)->G2D ()) GETBMP (b)->G2D ()->DrawBox (x1, y1, x2 - x1, y2 - y1, color);
}

void csPGVideoDriver::Blit (hwrbitmap b, int16 x, int16 y, int16 w, int16 h,
  hwrbitmap p, int16 px, int16 py, int16 lgop)
{
  csRef<iImage> shot = GETBMP (p)->Image ();
  if (GETBMP (b)->G2D ()->GetPixelBytes () !=
      GETBMP (p)->G2D ()->GetPixelBytes ())
  {
    printf ("Format mismatch!\n");
    return;
  }
  /*for (int l=0; l<h; l++)
  {
    GETBMP (b)->G2D ()->Blit (x, y+l, w, 1, 
      ((unsigned char *) shot->GetImageData ())+
      (x+y*shot->GetWidth ())*
      GETBMP (b)->G2D ()->GetPixelBytes ());
  }*/

  /*csRef<iImage> shot = GETBMP (p)->Image ();
  csRef<iImage> crop = (new csImageMemory (
    shot->GetWidth (), shot->GetHeight (), shot->GetImageData (), false,
    shot->GetFormat (), shot->GetPalette ()))->Crop (px, py, w, h);*/
  //crop->SetFormat (CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA);
  //GETBMP (b)->G2D ()->Blit (x, y, w, h, (unsigned char *) crop->GetImageData ());
}

g_error csPGVideoDriver::New (hwrbitmap *b, int16 w, int16 h, uint16 bpp)
{
  /*SETBMP (b, new csHwrBitmap (new csImageMemory (w, h,
    bpp <= 8 ? CS_IMGFMT_PALETTED8 : CS_IMGFMT_TRUECOLOR)));*/

  // Leak?
  csRef<iGraphics2D> newg2d = Gfx2D->CreateOffscreenCanvas (
    new char[(bpp>>3)*w*h], w, h, bpp, 0);
  SETBMP (b, new csHwrBitmap (newg2d));
  return 0;
}

g_error csPGVideoDriver::Load (hwrbitmap *b, const uint8 *data, unsigned long len)
{
/*  SETBMP (b, new csHwrBitmap (new csImageMemory (len, 1, (void *) data, false,
    CS_IMGFMT_ANY)));*/

  // @@@ SHOULD USE NATIVE LOADERS SOMEHOW?

  struct bitformat *fmt = bitmap_formats;
  hwrbitmap bmp;

  vid->bitmap_new = def_bitmap_new;
  vid->pixel = def_pixel;
  g_error e = mkerror(PG_ERRT_BADPARAM,8);
  while (fmt->name[0]) {   /* Dummy record has empty name */
    if (fmt->detect && fmt->load && (*fmt->detect)(data,len))
    {
      e = (*fmt->load)(&bmp,data,len);
      break;
    }
    fmt++;
  }
  vid->bitmap_new = New;
  vid->pixel = Pixel;
  if (e != 0)
    return e;
  
  /*csRef<csImageFile> img = new csImageMemory (
    bmp->w, bmp->h, );

  csRef<iGraphics2D> newg2d = Gfx2D->CreateOffscreenCanvas (
    bmp->bits, bmp->w, bmp->h, bmp->bpp, 0);
  SETBMP (b, new csHwrBitmap (newg2d));*/

  def_bitmap_free (bmp);
  return mkerror(PG_ERRT_BADPARAM,8);
}

g_error csPGVideoDriver::GetSize (hwrbitmap b, int16 *w, int16 *h)
{
  //if (GETBMP (b)->G2D ())
  {
    *w = GETBMP (b)->G2D ()->GetWidth ();
    *h = GETBMP (b)->G2D ()->GetHeight ();
  }
  /*else
  {
    *w = GETBMP (b)->Image ()->GetWidth ();
    *h = GETBMP (b)->Image ()->GetHeight ();
  }*/
  return 0;
}

g_error csPGVideoDriver::GetGropRender (hwrbitmap b, groprender **g)
{
  *g = GETBMP (b)->Grop ();
  return 0;
}

g_error csPGVideoDriver::GetShareMem (hwrbitmap b, unsigned long uid, pgshmbitmap *info)
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
