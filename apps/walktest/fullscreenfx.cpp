/*
    Copyright (C) 2008 by Jorrit Tyberghein

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
#include "csutil/scanstr.h"
#include "cstool/csfxscr.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "ivideo/material.h"

#include "walktest.h"
#include "fullscreenfx.h"

WalkTestFullScreenFX::WalkTestFullScreenFX (WalkTest* walktest)
  : walktest (walktest)
{
  do_fs_inter = false;
  do_fs_shadevert = false;
  do_fs_whiteout = false;
  do_fs_blue = false;
  do_fs_red = false;
  do_fs_green = false;
  do_fs_fadetxt = false;
  do_fs_fadecol = false;
  do_fs_fadeout = false;
}

void WalkTestFullScreenFX::Draw2D (csTicks current_time)
{
  if (do_fs_inter)
  {
    csfxInterference (walktest->myG2D, fs_inter_amount, fs_inter_anim,
    	fs_inter_length);
    fs_inter_anim = fmod (fabs (float (current_time)/3000.0), 1.);
  }
}

void WalkTestFullScreenFX::Draw3D (csTicks current_time)
{
  if (do_fs_fadeout)
  {
    csfxFadeOut (walktest->myG3D, fs_fadeout_fade);
    float t3 = fabs (float (current_time)/3000.0);
    fs_fadeout_fade = fmod (t3, 1.0f);
    fs_fadeout_dir = fmod (t3, 2.0f) >= 1;
    if (!fs_fadeout_dir) fs_fadeout_fade = 1-fs_fadeout_fade;
  }
  if (do_fs_fadecol)
  {
    csfxFadeToColor (walktest->myG3D, fs_fadecol_fade, fs_fadecol_color);
    float t3 = fabs (float (current_time)/3000.0);
    fs_fadecol_fade = fmod (t3, 1.0f);
    fs_fadecol_dir = fmod (t3, 2.0f) >= 1;
    if (!fs_fadecol_dir) fs_fadecol_fade = 1-fs_fadecol_fade;
  }
  if (do_fs_fadetxt)
  {
    csfxFadeTo (walktest->myG3D, fs_fadetxt_txt, fs_fadetxt_fade);
    float t3 = fabs (float (current_time)/3000.0);
    fs_fadetxt_fade = fmod (t3, 1.0f);
    fs_fadetxt_dir = fmod (t3, 2.0f) >= 1;
    if (!fs_fadetxt_dir) fs_fadetxt_fade = 1-fs_fadetxt_fade;
  }
  if (do_fs_red)
  {
    csfxRedScreen (walktest->myG3D, fs_red_fade);
    float t3 = fabs (float (current_time)/3000.0);
    fs_red_fade = fmod (t3, 1.0f);
    fs_red_dir = fmod (t3, 2.0f) >= 1;
    if (!fs_red_dir) fs_red_fade = 1-fs_red_fade;
  }
  if (do_fs_green)
  {
    csfxGreenScreen (walktest->myG3D, fs_green_fade);
    float t3 = fabs (float (current_time)/3000.0);
    fs_green_fade = fmod (t3, 1.0f);
    fs_green_dir = fmod (t3, 2.0f) >= 1;
    if (!fs_green_dir) fs_green_fade = 1-fs_green_fade;
  }
  if (do_fs_blue)
  {
    csfxBlueScreen (walktest->myG3D, fs_blue_fade);
    float t3 = fabs (float (current_time)/3000.0);
    fs_blue_fade = fmod (t3, 1.0f);
    fs_blue_dir = fmod (t3, 2.0f) >= 1;
    if (!fs_blue_dir) fs_blue_fade = 1-fs_blue_fade;
  }
  if (do_fs_whiteout)
  {
    csfxWhiteOut (walktest->myG3D, fs_whiteout_fade);
    float t3 = fabs (float (current_time)/3000.0);
    fs_whiteout_fade = fmod (t3, 1.0f);
    fs_whiteout_dir = fmod (t3, 2.0f) >= 1;
    if (!fs_whiteout_dir) fs_whiteout_fade = 1-fs_whiteout_fade;
  }
  if (do_fs_shadevert)
  {
    csfxShadeVert (walktest->myG3D, fs_shadevert_topcol, fs_shadevert_botcol,
    	CS_FX_ADD);
  }
}

void WalkTestFullScreenFX::InterFX (const char* arg)
{
  do_fs_inter = !do_fs_inter;
  if (do_fs_inter)
  {
    fs_inter_amount = 0.3f;
    fs_inter_length = 30;
    fs_inter_anim = 0;
    if (arg)
      csScanStr (arg, "%f,%f", &fs_inter_amount, &fs_inter_length);
  }
}

void WalkTestFullScreenFX::FadeOutFX ()
{
  do_fs_fadeout = !do_fs_fadeout;
  if (do_fs_fadeout)
  {
    fs_fadeout_fade = 0;
    fs_fadeout_dir = true;
  }
}

void WalkTestFullScreenFX::FadeColFX (const char* arg)
{
  do_fs_fadecol = !do_fs_fadecol;
  if (do_fs_fadecol)
  {
    fs_fadecol_fade = 0;
    fs_fadecol_dir = true;
    float r = 1, g = 0, b = 0;
    if (arg) csScanStr (arg, "%f,%f,%f", &r, &g, &b);
    fs_fadecol_color.Set (r, g, b);
  }
}

void WalkTestFullScreenFX::FadeTxtFX (const char* arg)
{
  do_fs_fadetxt = !do_fs_fadetxt;
  if (do_fs_fadetxt)
  {
    fs_fadetxt_fade = 0;
    fs_fadetxt_dir = true;
    char buf[255];
    *buf = 0;
    if (arg) csScanStr (arg, "%s", buf);
    iMaterialWrapper* mat = walktest->Engine->GetMaterialList ()->FindByName (buf);
    if (mat)
    {
      fs_fadetxt_txt = mat->GetMaterial()->GetTexture ();
    }
    else
    {
      walktest->Report (CS_REPORTER_SEVERITY_NOTIFY,
		"Can't find material!");
      do_fs_fadetxt = false;
    }
  }
}

void WalkTestFullScreenFX::FadeRedFX ()
{
  do_fs_red = !do_fs_red;
  if (do_fs_red)
  {
    fs_red_fade = 0;
    fs_red_dir = true;
  }
}

void WalkTestFullScreenFX::FadeGreenFX ()
{
  do_fs_green = !do_fs_green;
  if (do_fs_green)
  {
    fs_green_fade = 0;
    fs_green_dir = true;
  }
}

void WalkTestFullScreenFX::FadeBlueFX ()
{
  do_fs_blue = !do_fs_blue;
  if (do_fs_blue)
  {
    fs_blue_fade = 0;
    fs_blue_dir = true;
  }
}

void WalkTestFullScreenFX::FadeWhiteFX ()
{
  do_fs_whiteout = !do_fs_whiteout;
  if (do_fs_whiteout)
  {
    fs_whiteout_fade = 0;
    fs_whiteout_dir = true;
  }
}

void WalkTestFullScreenFX::ShadeVertFX (const char* arg)
{
  do_fs_shadevert = !do_fs_shadevert;
  if (do_fs_shadevert)
  {
    float tr = 1, tg = 0, tb = 0, br = 0, bg = 0, bb = 1;
    if (arg) csScanStr (arg, "%f,%f,%f,%f,%f,%f",
      	&tr, &tg, &tb, &br, &bg, &bb);
    fs_shadevert_topcol.Set (tr, tg, tb);
    fs_shadevert_botcol.Set (br, bg, bb);
  }
}

