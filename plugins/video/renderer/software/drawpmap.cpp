/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein

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
#include "sft3dcom.h"
#include "soft_txt.h"

#define DRAWSPRITE_NAME DrawPixmap8
#define DRAWSPRITE_PIXTYPE UByte
#include "drawsprt.inc"

#define DRAWSPRITE_NAME DrawPixmap16
#define DRAWSPRITE_PIXTYPE UShort
#include "drawsprt.inc"

#define DRAWSPRITE_NAME DrawPixmap32
#define DRAWSPRITE_PIXTYPE ULong
#include "drawsprt.inc"

void csGraphics3DSoftwareCommon::DrawPixmap (iTextureHandle *hTex,
  int sx, int sy, int sw, int sh,
  int tx, int ty, int tw, int th)
{
  switch (pfmt.PixelBytes)
  {
    case 1:  DrawPixmap8 (G2D, hTex, sx, sy, sw, sh, tx, ty, tw, th); break;
    case 2:  DrawPixmap16(G2D, hTex, sx, sy, sw, sh, tx, ty, tw, th); break;
    default: DrawPixmap32(G2D, hTex, sx, sy, sw, sh, tx, ty, tw, th); break;
  }
}
