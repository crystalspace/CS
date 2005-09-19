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

namespace cspluginSoft3d
{

#define DP_NAME DrawPixmap16_555
#define DP_PIXTYPE uint16
#define DP_PIXFORM_R5G5B5
#include "drawsprt.inc"

#define DP_NAME DrawPixmap16_565
#define DP_PIXTYPE uint16
#define DP_PIXFORM_R5G6B5
#include "drawsprt.inc"

#define DP_NAME DrawPixmap32
#define DP_PIXTYPE uint32
#define DP_PIXFORM_R8G8B8
#include "drawsprt.inc"

void csSoftwareGraphics3DCommon::DrawPixmap (iTextureHandle *hTex,
  int sx, int sy, int sw, int sh,
  int tx, int ty, int tw, int th, uint8 Alpha)
{
  if (pfmt.PixelBytes == 2)
  {
    if (pfmt.GreenBits == 5)
      DrawPixmap16_555 (G2D, hTex, sx, sy, sw, sh, tx, ty, tw, th, Alpha);
    else
      DrawPixmap16_565 (G2D, hTex, sx, sy, sw, sh, tx, ty, tw, th, Alpha);
  }
  else
    DrawPixmap32 (G2D, hTex, sx, sy, sw, sh, tx, ty, tw, th, Alpha);
}

} // namespace cspluginSoft3d
