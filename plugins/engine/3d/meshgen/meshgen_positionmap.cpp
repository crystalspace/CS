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

#include "csutil/blockallocator.h"

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{
  typedef csBlockAllocator<csBox2> BoxAllocator;
  CS_IMPLEMENT_STATIC_VAR(GetBoxAlloc, BoxAllocator, )

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
  
  PositionMap::~PositionMap ()
  {
    for (size_t b = 0; b < buckets.GetSize(); b++)
    {
      Bucket& bucket = buckets[b];
      for (size_t a = 0; a < bucket.freeAreas.GetSize(); a++)
      {
	GetBoxAlloc()->Free (bucket.freeAreas[a].box);
      }
    }
  }

  bool PositionMap::GetRandomPosition (const float radius, float& xpos, float& zpos,
				       AreaID& area)
  {
    /* Collect candidates from areas from all buckets with a minRadius that
       is larger or equal to radius */
    float totalArea = 0;
    for (size_t b = 0; b < buckets.GetSize(); b++)
    {
      const Bucket& bucket = buckets[b];
      if (bucket.minSide < radius*2) break;
      if (bucket.freeAreas.GetSize() == 0) continue;
      totalArea += bucket.freeAreas[0].area;
    }
    // No space found.
    if (totalArea < SMALL_EPSILON) return false;
    
    /* Pick the area.
       Note it's guaranteed to be at least as large as radius on it's smaller
       side. */
    csBox2 freeArea;
    float theArea = totalArea * posGen.Get ();
    for (size_t b = 0; b < buckets.GetSize(); b++)
    {
      const Bucket& bucket = buckets[b];
      if (bucket.freeAreas.GetSize() == 0) continue;
      float bucketArea = bucket.freeAreas[0].area;
      if (theArea < bucketArea)
      {
	size_t areaIndex = FindBox (bucket, theArea);
	freeArea = *(bucket.freeAreas[areaIndex].box);
	area.first = b;
	area.second = areaIndex;
	break;
      }
      else
	theArea -= bucketArea;
    }
    
    CS_ASSERT ((freeArea.MaxX() - freeArea.MinX() >= radius*2) 
	       && (freeArea.MaxY() - freeArea.MinY() >= radius*2));

    float xAvail = freeArea.MinX() + radius;
    float yAvail = freeArea.MinY() + radius;
    float zAvail = freeArea.MaxX() - radius;
    float wAvail = freeArea.MaxY() - radius;

    xpos = freeArea.MinX() + radius + (zAvail - xAvail) * posGen.Get();
    zpos = freeArea.MinY() + radius + (wAvail - yAvail) * posGen.Get();

    return true;
  }

  void PositionMap::MarkAreaUsed (const AreaID& area, 
				  const float radius, const float xpos, const float zpos)
  {
    // Don't bother with very small radii
    if (radius < SMALL_EPSILON)
      return;
    
    size_t b = area.first;
    size_t areaIndex = area.second;
    Bucket& bucket = buckets[b];
    Bucket::Area freeArea (buckets[b].freeAreas[areaIndex]);
    csBox2 box (*(freeArea.box));
    
    Bucket::Area replaceArea = bucket.freeAreas.Pop ();
    BubbleAreaIncrease (bucket, bucket.freeAreas.GetSize(), -replaceArea.area);
    if (bucket.freeAreas.GetSize() > areaIndex)
    {
      BubbleAreaIncrease (bucket, areaIndex, replaceArea.area - freeArea.area);
      bucket.freeAreas[areaIndex] = replaceArea;
    }
    GetBoxAlloc()->Free (freeArea.box);
    
    InsertNewArea (csBox2 (box.MinX(), box.MinY(), xpos+radius, zpos-radius));
    InsertNewArea (csBox2 (xpos+radius, box.MinY(), box.MaxX(), zpos+radius));
    InsertNewArea (csBox2 (xpos-radius, zpos+radius, box.MaxX(), box.MaxY()));
    InsertNewArea (csBox2 (box.MinX(), zpos-radius, xpos-radius, box.MaxY()));
  }

  void PositionMap::InsertNewArea (const csBox2& areaBox)
  {
    float side1 = areaBox.MaxX() - areaBox.MinX();
    float side2 = areaBox.MaxY() - areaBox.MinY();
    float minSide = csMin (side1, side2);

    /* Insert area into bucket with the largest radius smaller or equal
       to minSide */
    for (size_t b = 0; b < buckets.GetSize(); b++)
    {
      Bucket& bucket = buckets[b];
      if (minSide >= bucket.minSide)
      {
	Bucket::Area newArea;
	newArea.area = side1*side2;
	newArea.box = GetBoxAlloc()->Alloc (areaBox);
	size_t index = bucket.freeAreas.Push (newArea);
	if (index > 0)
	{
	  size_t parent = (index-1) / 2;
	  if (bucket.freeAreas[parent].box)
	  {
	    // 'Parent' node is a leaf, turn it into a node
	    bucket.freeAreas.Push (bucket.freeAreas[parent]);
	    bucket.freeAreas[parent].box = nullptr;
	  }
	}
	BubbleAreaIncrease (bucket, index, newArea.area);
	break;
      }
    }
  }
  
  void PositionMap::BubbleAreaIncrease (Bucket& bucket, size_t index, float amount)
  {
    if (index == 0) return;

    do
    {
      index = ((index-1) / 2);
      bucket.freeAreas[index].area += amount;
    } while (index > 0);
  }
  
  size_t PositionMap::FindBox (const Bucket& bucket, float pos)
  {
    size_t index = ~0;
    size_t nextIndex = 0;
    while (nextIndex < bucket.freeAreas.GetSize())
    {
      index = nextIndex;
      float nodeArea = bucket.freeAreas[index].area;
      if (pos < nodeArea)
      {
	nextIndex = index*2+1; // 'left' child
      }
      else
      {
	nextIndex = index+1; // step to 'right' child
	pos -= nodeArea;
      }
    }
    return index;
  }
  
}
CS_PLUGIN_NAMESPACE_END(Engine)
