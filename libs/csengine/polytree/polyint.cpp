/*
    Copyright (C) 2000 by Jorrit Tyberghein

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
#include "csengine/polyint.h"

csPolygonIntArray::csPolygonIntArray ()
{
  polygons = NULL;
  num = max = 0;
}

csPolygonIntArray::~csPolygonIntArray ()
{
  delete [] polygons;
}

void csPolygonIntArray::AddPolygon (csPolygonInt* poly)
{
  if (!polygons)
  {
    max = 2;
    polygons = new csPolygonInt* [max];
  }

  if (num >= max)
  {
    csPolygonInt** pp = new csPolygonInt* [max+3];
    memcpy (pp, polygons, sizeof (csPolygonInt*)*max);
    max += 3;
    delete [] polygons;
    polygons = pp;
  }

  polygons[num] = poly;
  num++;
}

