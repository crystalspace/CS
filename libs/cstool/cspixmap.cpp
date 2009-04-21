/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#include "cstool/cspixmap.h"

#include "ivideo/texture.h"

csSimplePixmap::csSimplePixmap (iTextureHandle *hTexture)
{
  if (hTexture != 0)
  {
    int w,h;
    hTex = hTexture;
    //hTex->GetMipMapDimensions (0,w,h);
    hTex->GetOriginalDimensions (w,h);
    SetTextureRectangle (0, 0, w, h);
  }
}

csSimplePixmap::csSimplePixmap (iTextureHandle *hTexture, int x, int y, int w, int h)
{
  if (hTexture != 0)
  {
    hTex = hTexture;
    SetTextureRectangle (x, y, w, h);
  }
}

csSimplePixmap::~csSimplePixmap ()
{
}

void csSimplePixmap::DrawScaled (iGraphics3D* g3d, int sx, int sy, int sw,
    int sh, uint8 Alpha)
{
  if (hTex != 0) g3d->DrawPixmap (hTex, sx, sy, sw, sh, tx, ty, tw, th, Alpha);
}

void csSimplePixmap::DrawTiled (iGraphics3D* g3d, int sx, int sy,
  int sw, int sh, int orgx, int orgy, uint8 Alpha)
{
  if (hTex != 0) g3d->DrawPixmap
    (hTex, sx, sy, sw, sh, sx - orgx, sy - orgy, sw, sh, Alpha);
}

int csSimplePixmap::Width () { return tw; }
int csSimplePixmap::Height () { return th; }
iTextureHandle *csSimplePixmap::GetTextureHandle() {return hTex;}

void csSimplePixmap::SetTextureHandle (iTextureHandle *hTexture)
{
  hTex = hTexture;
}
