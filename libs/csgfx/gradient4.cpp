/*
    Copyright (C) 2003 by Jorrit Tyberghein
              (C) 2003 by Frank Richter
              (C) 2006 by Christopher Nelson

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
#include "csgfx/gradient4.h"

csGradientShade4::csGradientShade4() :
  left (0, 0, 0), right (0, 0, 0), position (0)
{
}

csGradientShade4::csGradientShade4 (csColor4 left_color, csColor4 right_color, 
				  float pos) :
  left (left_color), right (right_color), position (pos)
{
}

csGradientShade4::csGradientShade4 (csColor4 color, float pos) :
  left (color), right (color), position (pos)
{
}

csGradient4::csGradient4()
{
}

csGradient4::csGradient4 (csColor4 first, csColor4 last)
{
  AddShade (csGradientShade4 (first, 0.0f));
  AddShade (csGradientShade4 (last, 1.0f));
}

static int ShadeCompare (csGradientShade4 const& item,
			 csGradientShade4 const& shade)
{
  if (item.position < shade.position)
    return -1;
  else if (item.position > shade.position)
    return 1;
  else
    return 0;
}

void csGradient4::AddShade (csGradientShade4 shade)
{
  shades.InsertSorted (shade, &ShadeCompare);
}

void csGradient4::Clear ()
{
  shades.DeleteAll ();
}

#define CLAMP(x) ((x<EPSILON)?0.0f:(((x-1.0f)>EPSILON)?1.0f:x))

bool csGradient4::Render (csRGBpixel* pal, size_t count,
			 float begin, float end) const
{
  if (shades.Length() == 0) return false;

  // current color
  csColor4 color = shades[0].left;
  // delta per palette item
  csColor4 delta (0, 0, 0, 0);
  // step in the gradient per pal item
  float step = (end - begin) / (float)count;
  float gradpos = begin;

  // current shade index
  size_t csi = 0;
  const csGradientShade4* currshade = 0;
  const csGradientShade4* nextshade = &shades[0];

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
    pal[i].alpha = csQint(CLAMP (color.alpha) * 255.99f);

    color += delta;
    gradpos += step;
  }

  return true;
}

