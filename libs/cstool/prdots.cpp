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

#include "cstool/prdots.h"

/// the implementation of the plasma texture
csProcDots::csProcDots () : csProcTexture()
{
  palsize = 0;
  palette = NULL;
  mat_w = 16;
  mat_h = 16;

  texFlags = CS_TEXTURE_3D | CS_TEXTURE_PROC | CS_TEXTURE_NOMIPMAPS | 
    CS_TEXTURE_PROC_ALONE_HINT;
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
  if (palette) delete[] palette;
  palsize = max;
  palette = new int[palsize];
  palette[0] = ptTxtMgr->FindRGB (0,0,0);
  /// fill the palette
  for (i=1; i < palsize; i++)
    palette[i] = ptTxtMgr->FindRGB (GetRandom (255), GetRandom (255), GetRandom (255));
}

void csProcDots::Animate (csTicks current_time)
{
  csTicks elapsed = 0;
  if (last_cur_time != 0)
  {
    elapsed = current_time-last_cur_time;
    if (elapsed > 2000) elapsed = 2000;
  }
  last_cur_time = current_time;

  if (!ptG3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;
  csTicks i;
  for (i = 0 ; i < elapsed / 10 ; i++)
    ptG2D->DrawPixel (GetRandom (mat_w), GetRandom (mat_h), palette[GetRandom (255)]);
  ptG3D->FinishDraw ();
  ptG3D->Print (NULL);
}

