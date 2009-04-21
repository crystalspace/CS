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

#ifndef __WALKTEST_FULLSCREENFX_H__
#define __WALKTEST_FULLSCREENFX_H__

class WalkTest;

#include "csutil/cscolor.h"

/**
 * Full screen effects.
 */
class WalkTestFullScreenFX
{
private:
  WalkTest* walktest;

  bool do_fs_inter;
  float fs_inter_amount;
  float fs_inter_anim;
  float fs_inter_length;

  bool do_fs_fadeout;
  float fs_fadeout_fade;
  bool fs_fadeout_dir;

  bool do_fs_fadecol;
  float fs_fadecol_fade;
  bool fs_fadecol_dir;
  csColor fs_fadecol_color;

  bool do_fs_fadetxt;
  float fs_fadetxt_fade;
  bool fs_fadetxt_dir;
  iTextureHandle* fs_fadetxt_txt;

  bool do_fs_red;
  float fs_red_fade;
  bool fs_red_dir;
  bool do_fs_green;
  float fs_green_fade;
  bool fs_green_dir;
  bool do_fs_blue;
  float fs_blue_fade;
  bool fs_blue_dir;

  bool do_fs_whiteout;
  float fs_whiteout_fade;
  bool fs_whiteout_dir;

  bool do_fs_shadevert;
  csColor fs_shadevert_topcol;
  csColor fs_shadevert_botcol;


public:
  WalkTestFullScreenFX (WalkTest* walktest);

  void Draw2D (csTicks current_time);
  void Draw3D (csTicks current_time);

  void InterFX (const char* arg);
  void FadeOutFX ();
  void FadeColFX (const char* arg);
  void FadeTxtFX (const char* arg);
  void FadeRedFX ();
  void FadeGreenFX ();
  void FadeBlueFX ();
  void FadeWhiteFX ();
  void ShadeVertFX (const char* arg);
};

#endif // __WALKTEST_FULLSCREENFX_H__

