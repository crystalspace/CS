/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Written by Samuel Humphreys

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
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/fontserv.h"
#include "apps/perftest/ptests2d.h"

void Tester2D::Setup (iGraphics3D* g3d, PerfTest* /*perftest*/)
{
  draw = 0;
  G2D = g3d->GetDriver2D ();
  max_w = G2D->GetWidth () - 10;
  max_h = G2D->GetHeight () - 10;
  inc_h = max_h/5;
  colour[0] = G2D->FindRGB (0,0,255);
  colour[1] = G2D->FindRGB (0,150,0);
  colour[2] = G2D->FindRGB (255,0,0);
  colour[3] = G2D->FindRGB (0,0,150);
  colour[4] = G2D->FindRGB (150,150,150);
  colour[5] = G2D->FindRGB (0,255,0);
  colour[6] = G2D->FindRGB (150,0,0);
  colour[7] = G2D->FindRGB (255,255,255);
}


//-----------------------------------------------------------------------------

void LineTester2D::Draw (iGraphics3D* /*g3d*/)
{
  draw++;
  int i, j = 0;
  for (i = 10; i < max_w; i++)
  {
    int k = ++j&7;
    G2D->DrawLine (i, 10, i, inc_h, colour [k]);
    G2D->DrawLine (i, inc_h+5, i, inc_h*2, colour [k]);
    G2D->DrawLine (i, inc_h*2+5, i, inc_h*3, colour [k]);
    G2D->DrawLine (i, inc_h*3+5, i, inc_h*4, colour [k]);
    G2D->DrawLine (i, inc_h*4+5, i, max_h, colour [k]);
  }
}

Tester* LineTester2D::NextTester ()
{
  return new PixelTester ();
}

//-----------------------------------------------------------------------------

void PixelTester::Draw (iGraphics3D* /*g3d*/)
{
  draw++;
  int i, j;
  for (i = 10; i < max_w; i++)
    for (j = 10; j < max_h; j++)
      G2D->DrawPixel (i, j, colour[(i+j)&7]);
}

Tester* PixelTester::NextTester ()
{

  return new StringTester ();
}

//-----------------------------------------------------------------------------
void StringTester::Setup (iGraphics3D* g3d, PerfTest* /*perftest*/)
{
  Tester2D::Setup (g3d, 0);
  font = G2D->GetFontServer ()->LoadFont (CSFONT_SMALL);
  char tl [] = "aA1! bB2 cC3 dD4$ 5%eE fF^6 gG&7hH8*iI9(j0)Jk[K1lL]}mM@n2No+O5p> Qq<r;RSs Tt =Uu|uv;Vwxyz WXYZ Crystal Space, the open sourced graphics engine.";
  size_t tl_len = strlen (tl);
  int line_length;
  font->GetDimensions (tl, line_length, text_height);
  float ave_char_w = ((float)line_length)/tl_len;

  if (line_length > max_w)
  {
    int len_diff = line_length - max_w;
    int sub_len = (int) (len_diff/ave_char_w);
    length = tl_len - sub_len + 1;
    line = new char[length];
    strncpy (line, tl, length);
    font->GetDimensions (line, line_length, text_height);
    line [length-1] = 0;
  }
  else
  {
    float screen_len = ((float)max_w)/ave_char_w;
    length = (int)screen_len;
    line = new char[length];
    char *buf = line;
    size_t buf_len = length;
    while (buf_len > tl_len)
    {
      strncpy (buf, tl, tl_len);
      buf += tl_len;
      buf_len -= tl_len;
    }
    strncpy (buf, tl, buf_len);
    font->GetDimensions (line, line_length, text_height);
    line [length-1] = 0;
  }

  rows = (max_h - 10)/text_height;
}

void StringTester::Draw (iGraphics3D* /*g3d*/)
{
  draw++;
  int height = 10 + text_height;
  while (height < max_h)
  {
    G2D->Write (font, 10, height, colour[0], -1, line);
    height += text_height +1;
  }
}

Tester* StringTester::NextTester ()
{
  font = 0;
  delete [] line;
  return 0;
}
