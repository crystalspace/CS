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
#include "ivideo/txtmgr.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "csutil/cscolor.h"
#include "csutil/util.h"
#include "iutil/objreg.h"

#include "cstool/prdots.h"

/// the implementation of the dots texture
csProcDots::csProcDots () : csProcTexture()
{
  palsize = 0;
  palette = 0;
  mat_w = 16;
  mat_h = 16;
  elapsed = 0;

  texFlags = CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS;
  state = 0;
}

csProcDots::~csProcDots ()
{
  delete[] palette;
}

bool csProcDots::PrepareAnim ()
{
  if (anim_prepared) return true;
  if (!csProcTexture::PrepareAnim ()) return false;
  MakePalette (256);
  return true;
}

void csProcDots::MakePalette (int max)
{
  int i;
  delete[] palette;
  palsize = max;
  palette = new int[palsize];
  palette[0] = g2d->FindRGB (0,0,0);
  /// fill the palette
  for (i=1 ; i < palsize ; i++)
    palette[i] = g2d->FindRGB (GetRandom (255),
    	GetRandom (255), GetRandom (255));
}

void csProcDots::Animate (csTicks current_time)
{
  if (last_cur_time != 0)
  {
    elapsed += current_time-last_cur_time;
  }
  last_cur_time = current_time;

#if 0
  if (elapsed < 1000) return;
  elapsed = elapsed % 1000;
  g3d->SetRenderTarget (tex->GetTextureHandle (), true);
  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS)) return;
  g3d->SetClipper (0, 0);
  iTextureManager* txtmgr = g3d->GetTextureManager ();
  switch (state)
  {
    case 0:
      g2d->DrawBox (0, 0, 16, 16, g2d->FindRGB (0, 128, 128));
      break;
    case 1:
      g2d->DrawPixel (8, 8, g2d->FindRGB (255, 0, 0));
      break;
    case 2:
      g2d->DrawPixel (7, 8, g2d->FindRGB (0, 255, 0));
      break;
    case 3:
      g2d->DrawPixel (9, 8, g2d->FindRGB (0, 0, 255));
      break;
    case 4:
      g2d->DrawPixel (8, 7, g2d->FindRGB (255, 0, 255));
      break;
    case 5:
      g2d->DrawPixel (8, 9, g2d->FindRGB (255, 255, 0));
      break;
  }
  state++;
  if (state == 6) state = 0;

#elif 0
  g3d->SetRenderTarget (tex->GetTextureHandle ());
  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS)) return;
  g3d->SetClipper (0, 0);
  iTextureManager* txtmgr = g3d->GetTextureManager ();
  int i, j;
  int offs = 0;
  for (i = -1+offs ; i < 17+offs ; i++)
    for (j = -1+offs ; j < 17+offs ; j++)
      g2d->DrawPixel (i, j, g2d->FindRGB (0, 128, 128));
  for (i = 0+offs ; i < 16+offs ; i++)
  {
    g2d->DrawPixel (i, 2+offs, g2d->FindRGB (64, 0, 0));
    g2d->DrawPixel (i, 0+offs, g2d->FindRGB (255, 0, 0));
    g2d->DrawPixel (i, 15+offs, g2d->FindRGB (0, 255, 0));
    g2d->DrawPixel (0+offs, i, g2d->FindRGB (0, 0, 255));
    g2d->DrawPixel (2+offs, i, g2d->FindRGB (0, 0, 64));
    g2d->DrawPixel (15+offs, i, g2d->FindRGB (255, 255, 0));

    g2d->DrawPixel (i, -1+offs, g2d->FindRGB (255, 255, 255));
    g2d->DrawPixel (i, 16+offs, g2d->FindRGB (255, 255, 255));
    g2d->DrawPixel (-1+offs, i, g2d->FindRGB (255, 255, 255));
    g2d->DrawPixel (16+offs, i, g2d->FindRGB (255, 255, 255));
  }
#elif 1
  if (elapsed < 100) return;
  if (elapsed > 2000) elapsed = 2000;

  g3d->SetRenderTarget (tex->GetTextureHandle (), true);
  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS)) return;
  csTicks i;
  for (i = 0 ; i < (elapsed / 50) ; i++)
    g2d->DrawPixel (GetRandom (mat_w), GetRandom (mat_h),
    	palette[GetRandom (255)]);
  elapsed = elapsed % 50;	// Keep remainder for more accuracy.
#endif
  g3d->FinishDraw ();
}

