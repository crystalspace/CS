/*
    Crystal Space Windowing System: background class
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>

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

#include "cssysdef.h"
#include "csws/csapp.h"
#include "csws/csbackgr.h"

csBackground::csBackground () : tex (0)
{
  Free ();
}

csBackground::~csBackground ()
{
  type = csbgNone;
}

void csBackground::SetTexture (iTextureHandle *iTex)
{
  if (tex)
    tex->DecRef ();
  tex = iTex;
  if (tex)
  {
    tex->IncRef ();
    type = csbgTextured;
  }
  else
    type = csbgNone;
}

void csBackground::Draw (csComponent &This, int x, int y, int w, int h,
  int xorg, int yorg, uint8 iAlpha)
{
  switch (type)
  {
    case csbgTextured:
      This.Texture (tex, x, y, w, h, xorg, yorg, iAlpha);
      break;
    case csbgNone:
    case csbgColor:
      if (!iAlpha)
      {
        This.Box (x, y, x + w, y + h, color);
        break;
      }
    case csbgGradient:
    {
      G3DPolygonDPFX poly;
      poly.use_fog = false;
      poly.num = 4;
      poly.mat_handle = 0;
	  int i;
      for (i = 0; i < 4; i++)
      {
        poly.vertices [i].x = (i == 0 || i == 3) ? x : x + w;
        poly.vertices [i].y = (i == 0 || i == 1) ? y : y + h;
        poly.z [i] = 0;
        poly.colors [i].red = colors [i].red  * (1 / 255.0f);
        poly.colors [i].green = colors [i].green * (1 / 255.0f);
        poly.colors [i].blue = colors [i].blue  * (1 / 255.0f);
      }
      poly.flat_color_r = poly.flat_color_g = poly.flat_color_b = 255;

      This.app->SetZbufferMode (CS_ZBUF_NONE);
      This.Polygon3D (poly, ((type == csbgGradient) ? 0 : CS_FX_FLAT) |
        (iAlpha ? (CS_FX_ALPHA | iAlpha) : CS_FX_COPY));
      break;
    }
  }
}
