/*
    Copyright (C) 2003 by Jorrit Tyberghein
              (C) 2003 by Frank Richter

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

#include "csqint.h"
#include "csgfx/gradient.h"

csGradientShade::csGradientShade() :
  left (0, 0, 0), right (0, 0, 0), position (0)
{
}

csGradientShade::csGradientShade (csColor left_color, csColor right_color, 
				  float pos) :
  left (left_color), right (right_color), position (pos)
{
}

csGradientShade::csGradientShade (csColor color, float pos) :
  left (color), right (color), position (pos)
{
}

csGradient::csGradient()
{
}

csGradient::csGradient (csColor first, csColor last)
{
  AddShade (csGradientShade (first, 0.0f));
  AddShade (csGradientShade (last, 1.0f));
}

void csGradient::AddShade (csGradientShade shade)
{
  size_t lo = 0, hi = shades.Length();
  size_t mid = 0;
  while (lo < hi)
  {
    mid = (lo + hi) / 2;
    if (shades[mid].position < shade.position)
    {
      lo = mid + 1;
    }
    else
    {
      hi = mid;
    }
  }
  shades.Insert (lo, shade);
}

void csGradient::Clear ()
{
  shades.DeleteAll ();
}

#define CLAMP(x) ((x<EPSILON)?0.0f:(((x-1.0f)>EPSILON)?1.0f:x))

bool csGradient::Render (csRGBcolor* pal, size_t count,
			 float begin, float end) const
{
  if (shades.Length() == 0) return false;

  // current color
  csColor color = shades[0].left;
  // delta per palette item
  csColor delta (0, 0, 0);
  // step in the gradient per pal item
  float step = (end - begin) / (float)count;
  float gradpos = begin;

  // current shade index
  size_t csi = 0;
  const csGradientShade* currshade = 0;
  const csGradientShade* nextshade = &shades[0];

  for (size_t i = 0; i < count; i++)
  {
    while (csi < shades.Length() && 
      (gradpos >= nextshade->position))
    {
      currshade = nextshade;
      csi++;
      if (csi < shades.Length())
      {
	nextshade = &shades[csi];
      }
      color = (step > EPSILON)? currshade->right : currshade->left;
      delta = (((step > EPSILON)? nextshade->left : nextshade->right) - color);
      float diff = (nextshade->position - currshade->position);

      if (ABS (diff) > EPSILON)
      {
	 color += 
	  (delta * ((gradpos - currshade->position) / diff));

        delta *= (step / diff);
      }
    }

    pal[i].red = csQint (CLAMP (color.red) * 255.99f);
    pal[i].green = csQint (CLAMP (color.green) * 255.99f);
    pal[i].blue = csQint (CLAMP (color.blue) * 255.99f);

    color += delta;
    gradpos += step;
  }

  return true;
}

