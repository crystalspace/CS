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
#include "csgfx/gradient.h"

csGradientShade::csGradientShade() :
  left (0, 0, 0, 1.0f), right (0, 0, 0, 1.0f), position (0)
{
}

csGradientShade::csGradientShade (csColor4 left_color, csColor4 right_color, 
				  float pos) :
  left (left_color), right (right_color), position (pos)
{
}

csGradientShade::csGradientShade (csColor left_color, csColor right_color, 
				  float pos) :
  position (pos)
{
	left.Set(left_color.red, left_color.green, left_color.blue, 1.0f);
	right.Set(right_color.red, right_color.green, right_color.blue, 1.0f);	
}

csGradientShade::csGradientShade (csColor4 color, float pos) :
  left (color), right (color), position (pos)
{
}

csGradientShade::csGradientShade (csColor color, float pos):position(pos)
{
	left.Set(color.red, color.green, color.blue);
	right.Set(color.red, color.green, color.blue);	
}

/////////////////////// csGradient //////////////////////////

csGradient::csGradient()
{
}

csGradient::csGradient (csColor4 first, csColor4 last)
{
  AddShade (csGradientShade (first, 0.0f));
  AddShade (csGradientShade (last, 1.0f));
}

static int ShadeCompare (csGradientShade const& item,
			 csGradientShade const& shade)
{
  if (item.position < shade.position)
    return -1;
  else if (item.position > shade.position)
    return 1;
  else
    return 0;
}

void csGradient::AddShade (csGradientShade shade)
{
  shades.InsertSorted (shade, &ShadeCompare);
}

void csGradient::Clear ()
{
  shades.DeleteAll ();
}

#define CLAMP(x) ((x<EPSILON)?0.0f:(((x-1.0f)>EPSILON)?1.0f:x))

bool csGradient::Render (csRGBcolor* pal, size_t count,
			 float begin, float end) const
{

	csRGBpixel *tmp = new csRGBpixel[count];
	
	bool result = Render(tmp, count, begin, end);	
	
	if (result)
	{
		for(size_t i=0; i<count; ++i)
		{
			pal[i].Set(tmp[i].red, tmp[i].green, tmp[i].blue);			
		}	
	}	
	
	delete [] tmp;
	
	return result;
}

bool csGradient::Render (csRGBpixel* pal, size_t count,
			 float begin, float end) const
{
  if (shades.Length() == 0) return false;

  // current color
  csColor4 color = shades[0].left;
  // delta per palette item
  csColor4 delta (0, 0, 0);
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
    pal[i].alpha = csQint (CLAMP (color.alpha) * 255.99f);

    color += delta;
    gradpos += step;
  }

  return true;
}

