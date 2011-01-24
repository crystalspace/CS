/*
    Copyright (C) 2005-2006 by Jorrit Tyberghein
	      (C) 2011 by Frank Richter

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

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{
  PositionMap::PositionMap (const float* minRadii, size_t numMinRadii,
			    const csBox2& box)
  {
    posGen.Initialize();
    
    buckets.SetCapacity (numMinRadii);
    for (size_t b = 0; b < numMinRadii; b++)
    {
      Bucket bucket (minRadii[b]*2);
      buckets.InsertSorted (bucket);
    }
    
    InsertNewArea (box);
  }

  bool PositionMap::GetRandomPosition (const float radius, float& xpos, float& zpos,
				       AreaID& area)
  {
    /* Collect candidates from areas from all buckets with a minRadius that
       is larger or equal to radius */
    size_t numAreas = 0;
    for (size_t b = 0; b < buckets.GetSize(); b++)
    {
      if (buckets[b].minSide < radius*2) break;
      numAreas += buckets[b].freeAreas.GetSize();
    }
    // No space found.
    if (numAreas == 0) return false;
    
    /* Pick the area.
       Note it's guaranteed to be at least as large as radius on it's smaller
       side. */
    csBox2 freeArea;
    size_t areaIndex = size_t (numAreas * posGen.Get ());
    for (size_t b = 0; b < buckets.GetSize(); b++)
    {
      if (areaIndex < buckets[b].freeAreas.GetSize())
      {
	freeArea = buckets[b].freeAreas[areaIndex];
	area.first = b;
	area.second = areaIndex;
	break;
      }
      else
	areaIndex -= buckets[b].freeAreas.GetSize();
    }
    
    CS_ASSERT ((freeArea.MaxX() - freeArea.MinX() >= radius*2) 
	       && (freeArea.MaxY() - freeArea.MinY() >= radius*2));

    float xAvail = freeArea.MinX() + radius * 0.5f;
    float yAvail = freeArea.MinY() + radius * 0.5f;
    float zAvail = freeArea.MaxX() - radius * 0.5f;
    float wAvail = freeArea.MaxY() - radius * 0.5f;

    xpos = freeArea.MinX() + radius + (zAvail - xAvail) * posGen.Get();
    zpos = freeArea.MinY() + radius + (wAvail - yAvail) * posGen.Get();

    return true;
  }

  void PositionMap::MarkAreaUsed (const AreaID& area, 
				  const float radius, const float xpos, const float zpos)
  {
    size_t b = area.first;
    size_t areaIndex = area.second;
    csBox2 freeArea (buckets[b].freeAreas[areaIndex]);
    
    buckets[b].freeAreas.DeleteIndexFast (areaIndex);
    
    InsertNewArea (csBox2 (freeArea.MinX(), freeArea.MinY(), xpos+radius, zpos-radius));
    InsertNewArea (csBox2 (xpos+radius, freeArea.MinY(), freeArea.MaxX(), zpos+radius));
    InsertNewArea (csBox2 (xpos-radius, zpos+radius, freeArea.MaxX(), freeArea.MaxY()));
    InsertNewArea (csBox2 (freeArea.MinX(), zpos-radius, xpos-radius, freeArea.MaxY()));
  }

  void PositionMap::InsertNewArea (const csBox2& area)
  {
    float minSide = csMin (area.MaxX() - area.MinX(),
			   area.MaxY() - area.MinY());

    /* Insert area into bucket with the largest radius smaller or equal
       to minSide */
    for (size_t b = 0; b < buckets.GetSize(); b++)
    {
      if (minSide >= buckets[b].minSide)
      {
	buckets[b].freeAreas.Push (area);
	break;
      }
    }
  }
}
CS_PLUGIN_NAMESPACE_END(Engine)
