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

//---------------------------------------------------------------------------

csTables tables;

csTables::~csTables ()
{
  CHK (delete[] sin_tab);
  CHK (delete[] cos_tab);
  CHK (delete[] acos_tab);
  CHK (delete[] asin_tab);
  CHK (delete[] cos_asin_tab);
  CHK (delete[] exp_table);
  CHK (delete[] mul_table);
  CHK (delete[] exp_table_2_real);
  CHK (delete[] another_mul_table);
  CHK (delete[] color_565_table);
}

void csTables::Initialize ()
{
  CHK (sin_tab = new float[360*2+1]);
  CHK (cos_tab = new float[360*2+1]);
  CHK (acos_tab = new int[512*2+1]);
  CHK (asin_tab = new int[512*2+1]);
  CHK (cos_asin_tab = new long[512*2+1]);
  CHK (exp_table = new UByte[256*256]);
  CHK (mul_table = new int[256*513]);
  CHK (exp_table_2_real = new int[256*256+20]); exp_table_2 = exp_table_2_real+20;
  CHK (another_mul_table = new int[4*(1<<LOG2_STEPS_X)*(1<<LOG2_STEPS_Y)]);
  CHK (color_565_table = new unsigned short[2*NUM_LIGHT_INTENSITIES*2048]);

  int i, j;
  for (i = -360 ; i <= 360 ; i++)
  {
    sin_tab[i+360] = sin (2.*M_PI*(float)i / 360.);
    cos_tab[i+360] = cos (2.*M_PI*(float)i / 360.);
  }
  for (i = -512 ; i <= 512 ; i++)
  {
    asin_tab[i+512] = QInt (360. * asin (((float)i) / 512.) / (2.*M_PI));
    acos_tab[i+512] = QInt (360. * acos (((float)i) / 512.) / (2.*M_PI));
    cos_asin_tab[i+512] = QInt (16.*256.*cos_tab [ asin_tab [ i+512 ] + 360 ]);
  }
  for (i = 0 ; i < 65535 ; i++)
  {
    float dist_x_density = ((float)i)/256.;
    exp_table[i] = (UByte)((1-exp (-dist_x_density))*255);
    exp_table_2[i] = exp_table[i]*513;
  }
  for (i = -20 ; i < 0 ; i++)
    exp_table_2[i] = exp_table_2[0];

  for(i=0;i<256;i++)
    for(j=0;j<513;j++)
      mul_table[i*513+j]=i*(j-256)/256;

  int mx=1<<LOG2_STEPS_X,my=1<<LOG2_STEPS_Y;
  int shifter=LOG2_STEPS_X+LOG2_STEPS_Y-LOG2_NUM_LIGHT_INTENSITIES;

  for(i=0;i<my;i++)
  {
    for(j=0;j<mx;j++)
    {
      int _=4*(i*mx+j),total=0;
      total=another_mul_table[_+0]=(mx-j-1)*(my-i-1)>>shifter;
      total+=another_mul_table[_+1]=(mx-j-1)*i>>shifter;
      total+=another_mul_table[_+2]=j*(my-i-1)>>shifter;
      total+=another_mul_table[_+3]=j*i>>shifter;

      while(total<NUM_LIGHT_INTENSITIES-1)
      {
        another_mul_table[_+rand()%4]++;
	total++;
      }

      another_mul_table[_+0]*=2048;	// 2048 -- number of colors that can be
      another_mul_table[_+1]*=2048;	// made of green and blue in 16 bit
      another_mul_table[_+2]*=2048;	// hicolor mode
      another_mul_table[_+3]*=2048;
    }
  }

  for(i=0;i<2048;i++)
  {
    int g=i>>5,b=i&31;
    for(j=0;j<NUM_LIGHT_INTENSITIES;j++)
    {
      int w_g=64*j*g/63;
      int w_b=32*j*b/31;
      color_565_table[i+j*2048]=((w_g&~(NUM_LIGHT_INTENSITIES-1))<<5)|(w_b&~(NUM_LIGHT_INTENSITIES-1));
    }
  }
}

//---------------------------------------------------------------------------
