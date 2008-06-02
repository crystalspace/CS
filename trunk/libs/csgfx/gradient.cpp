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
#include "csgeom/math.h"

/////////////////////// csGradient //////////////////////////

csGradient::csGradient() : scfImplementationType (this)
{
}

csGradient::csGradient (csColor4 first, csColor4 last) : 
  scfImplementationType (this)
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

void csGradient::AddShade (const csGradientShade& shade)
{
  shades.InsertSorted (shade, &ShadeCompare);
}

void csGradient::AddShade (const csColor4& color, float position)
{
  shades.InsertSorted (csGradientShade (color, position), &ShadeCompare);
}

void csGradient::AddShade (const csColor4& left, const csColor4& right, 
  float position)
{
  shades.InsertSorted (csGradientShade (left, right, position), &ShadeCompare);
}

void csGradient::Clear ()
{
  shades.DeleteAll ();
}

bool csGradient::Render (csRGBcolor* pal, size_t count,
			 float begin, float end) const
{
  csRGBpixel *tmp = new csRGBpixel[count];
	
  bool result = Render(tmp, count, begin, end);	
	
  if (result)
  {
    for(size_t i=0; i<count; ++i)
    {
      pal[i].Set (tmp[i].red, tmp[i].green, tmp[i].blue);			
    }	
  }	

  delete [] tmp;
	
  return result;
}

bool csGradient::Render (csRGBpixel* pal, size_t count,
			 float begin, float end) const
{
  if (shades.GetSize () == 0) return false;

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
    while (csi < shades.GetSize () && 
      (gradpos >= nextshade->position))
    {
      currshade = nextshade;
      csi++;
      if (csi < shades.GetSize ())
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

    pal[i].red = csQint (csClamp (color.red, 1.0f, 0.0f) * 255.99f);
    pal[i].green = csQint (csClamp (color.green, 1.0f, 0.0f) * 255.99f);
    pal[i].blue = csQint (csClamp (color.blue, 1.0f, 0.0f) * 255.99f);
    pal[i].alpha = csQint (csClamp (color.alpha, 1.0f, 0.0f) * 255.99f);

    color += delta;
    gradpos += step;
  }

  return true;
}

csPtr<iGradientShades> csGradient::GetShades ()
{
  csRef<iGradientShades> shadesArray;
  shadesArray.AttachNew (new scfGradientShadesArray (this));
  return csPtr<iGradientShades> (shadesArray);
}
