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

#include <math.h>
#include "sysdef.h"
#include "qint.h"
#include "cs3d/software/tables.h"

csTables tables;

csTables::~csTables ()
{
  CHK (delete [] exp_256);
  CHK (delete [] one_div_z);
  CHK (delete [] color_565_table);
  CHK (delete [] filter_mul_table);
}

void csTables::Initialize ()
{
  // 64K
  CHK (filter_mul_table = new int [4 * (1 << LOG2_STEPS_X) * (1 << LOG2_STEPS_Y)]);
  // 20K
  CHK (color_565_table = new unsigned short [2 * NUM_LIGHT_INTENSITIES * 2048]);
  // 16K
  CHK (one_div_z = new unsigned int [1 << 12]);
  // ~1.5K
  CHK (exp_256 = new unsigned char [EXP_256_SIZE]);

  int i, j;
  int mx = 1 << LOG2_STEPS_X, my = 1 << LOG2_STEPS_Y;
  int shifter = LOG2_STEPS_X + LOG2_STEPS_Y - LOG2_NUM_LIGHT_INTENSITIES;

  for (i = 0; i < my; i++)
  {
    for (j = 0; j < mx; j++)
    {
      int _ = 4 * (i * mx + j), total;
      total =  filter_mul_table [_ + 0] = (mx - j - 1) * (my - i - 1) >> shifter;
      total += filter_mul_table [_ + 1] = (mx - j - 1) * i >> shifter;
      total += filter_mul_table [_ + 2] = j * (my - i - 1) >> shifter;
      total += filter_mul_table [_ + 3] = j * i >> shifter;

      while (total < NUM_LIGHT_INTENSITIES - 1)
      {
        filter_mul_table [_ + rand () % 4]++;
	total++;
      }

      filter_mul_table [_+0] *= 2048;	// 2048 -- number of colors that can be
      filter_mul_table [_+1] *= 2048;	// made of green and blue in 16 bit
      filter_mul_table [_+2] *= 2048;	// hicolor mode
      filter_mul_table [_+3] *= 2048;
    }
  }

  for (i = 0; i < 2048; i++)
  {
    int g = i >> 5, b = i & 31;
    for (j = 0; j < NUM_LIGHT_INTENSITIES; j++)
    {
      int w_g = 64 * j * g / 63;
      int w_b = 32 * j * b / 31;
      color_565_table [i + j * 2048] =
        ((w_g & ~(NUM_LIGHT_INTENSITIES - 1)) << 5) |
         (w_b & ~(NUM_LIGHT_INTENSITIES - 1));
    }
  }

  for (i = 1; i < (1 << 12); i++)
    one_div_z [i] = QRound (float (0x1000000) / float (i));
  one_div_z [0] = one_div_z [1];

  for (i = 0; i < EXP_256_SIZE; i++)
    exp_256 [i] = QRound (255 * exp (-float (i) / 256.));
}
