/*
  Crystal Space Windowing System: 2D sprites
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

#include "sysdef.h"
#include "csengine/2d/csspr2d.h"

csSprite2D::csSprite2D (csTextureHandle *hTexture, int x, int y, int w, int h)
{
  hTex = hTexture;
  SetTextureRectangle (x, y, w, h);
}

csSprite2D::~csSprite2D ()
{
}

void csSprite2D::Draw (IGraphics2D* g2d, int sx, int sy, int sw, int sh)
{
  if (hTex)
    g2d->DrawSprite(hTex->GetTextureHandle (), sx, sy, sw, sh, tx, ty, tw, th);
}
