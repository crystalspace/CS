/*
    Copyright (C) 1998 by Jorrit Tyberghein

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
#include "qint.h"
#include "scan.h"
#include "tcache.h"
#include "soft_txt.h"
#include "soft_g3d.h"
#include "isystem.h"
#include "ipolygon.h"
#include "sttest.h"

//---------------------- This routine is pixel-depth independent ---

#ifndef NO_draw_scanline_zfil

void csScan_draw_scanline_zfil (int xx, unsigned char* d,
  unsigned long* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  (void)u_div_z; (void)v_div_z; (void)inv_z; (void)d;
  ULong* lastZbuf = z_buf + xx-1;
  int dzz, izz;
  izz = QInt24 (inv_z);
  dzz = QInt24 (Scan.M);

  do
  {
    *z_buf++ = izz;
    izz += dzz;
  }
  while (z_buf <= lastZbuf);
}

#endif // NO_draw_scanline_zfil

//------------------------------------------------------------------

void csScan_Initialize ()
{
  // 64K
  CHK (Scan.filter_mul_table = new int [4 * (1 << LOG2_STEPS_X) * (1 << LOG2_STEPS_Y)]);
  // 20K
  CHK (Scan.color_565_table = new unsigned short [2 * NUM_LIGHT_INTENSITIES * 2048]);
  // 16K
  CHK (Scan.one_div_z = new unsigned int [1 << 12]);
  // ~1.5K
  CHK (Scan.exp_256 = new unsigned char [EXP_256_SIZE+3]);
  // ~1K
  CHK (Scan.exp_16 = new unsigned char [EXP_32_SIZE+3]);

  int i, j;
  int mx = 1 << LOG2_STEPS_X, my = 1 << LOG2_STEPS_Y;
  int shifter = LOG2_STEPS_X + LOG2_STEPS_Y - LOG2_NUM_LIGHT_INTENSITIES;

  for (i = 0; i < my; i++)
  {
    for (j = 0; j < mx; j++)
    {
      int _ = 4 * (i * mx + j), total;
      total =  Scan.filter_mul_table [_ + 0] = (mx - j - 1) * (my - i - 1) >> shifter;
      total += Scan.filter_mul_table [_ + 1] = (mx - j - 1) * i >> shifter;
      total += Scan.filter_mul_table [_ + 2] = j * (my - i - 1) >> shifter;
      total += Scan.filter_mul_table [_ + 3] = j * i >> shifter;

      while (total < NUM_LIGHT_INTENSITIES - 1)
      {
        Scan.filter_mul_table [_ + rand () % 4]++;
	total++;
      }

      Scan.filter_mul_table [_+0] *= 2048;	// 2048 -- number of colors that can be
      Scan.filter_mul_table [_+1] *= 2048;	// made of green and blue in 16 bit
      Scan.filter_mul_table [_+2] *= 2048;	// hicolor mode
      Scan.filter_mul_table [_+3] *= 2048;
    }
  }

  for (i = 0; i < 2048; i++)
  {
    int g = i >> 5, b = i & 31;
    for (j = 0; j < NUM_LIGHT_INTENSITIES; j++)
    {
      int w_g = 64 * j * g / 63;
      int w_b = 32 * j * b / 31;
      Scan.color_565_table [i + j * 2048] =
        ((w_g & ~(NUM_LIGHT_INTENSITIES - 1)) << 5) |
         (w_b & ~(NUM_LIGHT_INTENSITIES - 1));
    }
  }

  for (i = 1; i < (1 << 12); i++)
    Scan.one_div_z [i] = QRound (float (0x1000000) / float (i));
  Scan.one_div_z [0] = Scan.one_div_z [1];

  for (i = 0; i < EXP_256_SIZE; i++)
    Scan.exp_256 [i] = QRound (255 * exp (-float (i) / 256.));
  for (i = 0; i < EXP_32_SIZE; i++)
    Scan.exp_16 [i] = QRound (32 * exp (-float (i) / 256.)) - 1;
}

void csScan_Finalize ()
{
  CHK (delete [] Scan.exp_16);
  CHK (delete [] Scan.exp_256);
  CHK (delete [] Scan.one_div_z);
  CHK (delete [] Scan.color_565_table);
  CHK (delete [] Scan.filter_mul_table);
}

void csScan_InitDraw (csGraphics3DSoftware* g3d, IPolygonTexture* tex,
  csTextureMMSoftware* texture, csTexture* untxt)
{
  Scan.Texture = texture;
  Scan.tw = untxt->get_width ();
  Scan.th = untxt->get_height ();
  Scan.tmap = untxt->get_bitmap8 ();
  Scan.shf_w = untxt->get_w_shift ();
  Scan.and_w = untxt->get_w_mask ();
  Scan.shf_h = untxt->get_h_shift ();
  Scan.and_h = untxt->get_h_mask ();

  Scan.FlatColor = texture->get_mean_color_idx ();

  if (g3d->do_lighting)
  {
    void* td = NULL;
    tex->GetTCacheData (&td);
    if (td)
    {
      TCacheLightedTexture* tclt = (TCacheLightedTexture*)td;
      Scan.tmap2 = tclt->get_tmap8 ();
    }
    else
      Scan.tmap2 = NULL;	// Not a lighted texture.

    tex->GetFDU (Scan.fdu);
    tex->GetFDV (Scan.fdv);
  }
  else
  {
    Scan.tmap2 = NULL;
    Scan.fdu = Scan.fdv = 0;
  }
  tex->GetWidth (Scan.tw2);
  tex->GetHeight (Scan.th2);

#ifdef STUPID_TEST
  Scan.tw2fp = (Scan.tw2 << 16) - 1;
  Scan.th2fp = (Scan.th2 << 16) - 1;
#endif
  tex->GetShiftU (Scan.shf_u);

  Scan.and_h <<= Scan.shf_w;
  Scan.shf_h = 16 - Scan.shf_w;
}

void csScan_dump (csGraphics3DSoftware* pG3D)
{
  pG3D->SysPrintf (MSG_DEBUG_0, "------------------------------------------------\n");
  if (Scan.tmap2)
  {
    pG3D->SysPrintf (MSG_DEBUG_0, "Using a texture from the texture cache.\n");
    pG3D->SysPrintf (MSG_DEBUG_0, "  Width=%d, height=%d\n", Scan.tw2, Scan.th2);
    pG3D->SysPrintf (MSG_DEBUG_0, "  fdu=%f, fdv=%f\n", Scan.fdu, Scan.fdv);
  }
  pG3D->SysPrintf (MSG_DEBUG_0, "The original unlighted texture:\n");
  pG3D->SysPrintf (MSG_DEBUG_0, "  Width=%d, height=%d\n", Scan.tw, Scan.th);
  pG3D->SysPrintf (MSG_DEBUG_0, "\n");
  pG3D->SysPrintf (MSG_DEBUG_0, "shf_u=%d, shf_w=%d, shf_h=%d\n", Scan.shf_u, Scan.shf_w, Scan.shf_h);
  pG3D->SysPrintf (MSG_DEBUG_0, "and_w=%d, and_h=%d\n", Scan.and_w, Scan.and_h);
  pG3D->SysPrintf (MSG_DEBUG_0, "\n");
  pG3D->SysPrintf (MSG_DEBUG_0, "M=%f, J1=%f, K1=%f\n", Scan.M, Scan.J1, Scan.K1);
  pG3D->SysPrintf (MSG_DEBUG_0, "dM=%f, dJ1=%f, dK1=%f\n", Scan.dM, Scan.dJ1, Scan.dK1);
  pG3D->SysPrintf (MSG_DEBUG_0, "------------------------------------------------\n");
}
