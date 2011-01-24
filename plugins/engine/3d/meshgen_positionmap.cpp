/*
    Copyright (C) 2005-2006 by Jorrit Tyberghein

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
#include "meshgen_positionmap.h"

#include "csgeom/box.h"

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{
  static int AreaCompare(csVector4 const& r, csVector4 const& k)
  {
    float a = (r.z - r.x) * (r.w - r.y);
    float b = (k.z - k.x) * (k.w - k.y);
    return (a < b) ? 1 : (a - b) > 0.01 ? -1 : 0;
  }

  PositionMap::PositionMap(const csBox2& box)
  {
    posGen.Initialize();
    freeAreas.Push(csVector4(box.MinX(), box.MinY(), box.MaxX(), box.MaxY()));
  }

  bool PositionMap::GetRandomPosition(float& xpos, float& zpos, float& radius, float& minRadius)
  {
    // Do a quick search.
    for(size_t i=0; i<freeAreas.GetSize(); ++i)
    {
      csVector4 freeArea = freeAreas[i];

      if(freeArea.z - freeArea.x < radius*2 ||
	freeArea.w - freeArea.y < radius*2)
      {
	continue;
      }

      float xAvail = freeArea.x + radius * 0.5f;
      float yAvail = freeArea.y + radius * 0.5f;
      float zAvail = freeArea.z - radius * 0.5f;
      float wAvail = freeArea.w - radius * 0.5f;

      xpos = freeArea.x + radius + (zAvail - xAvail) * posGen.Get();
      zpos = freeArea.y + radius + (wAvail - yAvail) * posGen.Get();

      freeAreas.DeleteIndex(i);

      if(minRadius*2 <= (zpos-radius) - freeArea.y)
	freeAreas.InsertSorted(csVector4(freeArea.x, freeArea.y, xpos+radius, zpos-radius), AreaCompare);

      if(minRadius*2 <= freeArea.z - (xpos+radius))
	freeAreas.InsertSorted(csVector4(xpos+radius, freeArea.y, freeArea.z, zpos+radius), AreaCompare);

      if(minRadius*2 <= freeArea.w - (zpos+radius))
	freeAreas.InsertSorted(csVector4(xpos-radius, zpos+radius, freeArea.z, freeArea.w), AreaCompare);

      if(minRadius*2 <= (xpos-radius) - freeArea.x)
	freeAreas.InsertSorted(csVector4(freeArea.x, zpos-radius, xpos-radius, freeArea.w), AreaCompare);

      return true;
    }

    // No space found.. we could do a more thorough search if this case happens often.
    return false;
  }

}
CS_PLUGIN_NAMESPACE_END(Engine)
