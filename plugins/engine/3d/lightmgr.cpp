/*
    Copyright (C) 2004 by Jorrit Tyberghein

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
#include "plugins/engine/3d/lightmgr.h"
#include "plugins/engine/3d/meshobj.h"
#include "plugins/engine/3d/sector.h"

// ---------------------------------------------------------------------------

csLightManager::csLightManager ()
  : scfImplementationType (this)
{
}

csLightManager::~csLightManager ()
{
}

void csLightManager::GetRelevantLights (iMeshWrapper* meshObject, 
    iLightInfluenceArray* lightArray, int maxLights, uint flags)
{
  const csBox3& meshBox = meshObject->GetWorldBoundingBox ();
  iSectorList* sectors = meshObject->GetMovable ()->GetSectors ();
  if (sectors && sectors->GetCount() > 0)
  {
    GetRelevantLights (sectors->Get (0), meshBox, lightArray, maxLights, flags);
  }
}

void csLightManager::GetRelevantLights (iMeshWrapper* meshObject, 
  iLightInfluenceCallback* lightCallback, int maxLights, 
  uint flags)
{
  const csBox3& meshBox = meshObject->GetWorldBoundingBox ();
  iSectorList* sectors = meshObject->GetMovable ()->GetSectors ();
  if (sectors && sectors->GetCount() > 0)
  {
    GetRelevantLights (sectors->Get (0), meshBox, lightCallback, maxLights, flags);
  }
}


void csLightManager::GetRelevantLights (iSector* sector, 
  iLightInfluenceArray* lightArray, int maxLights, 
  uint flags)
{
  const csBox3 bigBox (csVector3 (-CS_BOUNDINGBOX_MAXVALUE),
    csVector3 (CS_BOUNDINGBOX_MAXVALUE));
  GetRelevantLights (sector, bigBox, lightArray, maxLights, flags);
}

void csLightManager::GetRelevantLights (iSector* sector, 
  iLightInfluenceCallback* lightCallback, int maxLights, 
  uint flags)
{
  const csBox3 bigBox (csVector3 (-CS_BOUNDINGBOX_MAXVALUE),
    csVector3 (CS_BOUNDINGBOX_MAXVALUE));
  GetRelevantLights (sector, bigBox, lightCallback, maxLights, flags);
}

struct IntersectInnerBBox
{
  IntersectInnerBBox (const csBox3& box)
    : testBox (box)
  {}

  bool operator() (const csSectorLightList::LightAABBTree::Node* node)
  {
    return testBox.TestIntersect (node->GetBBox ());
  }

  const csBox3& testBox;
};

struct LightCollectArray
{
  LightCollectArray (const csBox3& box, iLightInfluenceArray* lightArray)
    : testBox (box), lightArray (lightArray)
  {}

  void operator() (const csSectorLightList::LightAABBTree::Node* node)
  {
    if (!testBox.TestIntersect (node->GetBBox ()))
      return;

    for (size_t i = 0; i < node->GetObjectCount (); ++i)
    {
      csLightInfluence newInfluence;
      newInfluence.light = node->GetLeafData (i);
      lightArray->Push (newInfluence);
    }
  }

  const csBox3& testBox;
  iLightInfluenceArray* lightArray;
};

struct LightCollectCallback
{
  LightCollectCallback (const csBox3& box, iLightInfluenceCallback* lightCallback)
    : testBox (box), lightCallback (lightCallback)
  {}

  void operator() (const csSectorLightList::LightAABBTree::Node* node)
  {
    if (!testBox.TestIntersect (node->GetBBox ()))
      return;

    for (size_t i = 0; i < node->GetObjectCount (); ++i)
    {
      csLightInfluence newInfluence;
      newInfluence.light = node->GetLeafData (i);
      lightCallback->LightInfluence (newInfluence);
    }
  }

  const csBox3& testBox;
  iLightInfluenceCallback* lightCallback;
};


void csLightManager::GetRelevantLights (iSector* sector, const csBox3& boundingBox,
  iLightInfluenceArray* lightArray, int maxLights, 
  uint flags)
{
  iLightList* llist = sector->GetLights ();
  csSectorLightList* sectorLightList = static_cast<csSectorLightList*> (llist);

  // Get the primary lights from same sector
  const csSectorLightList::LightAABBTree& aabbTree = sectorLightList->GetLightAABBTree ();
  IntersectInnerBBox inner (boundingBox);
  LightCollectArray leaf (boundingBox, lightArray);
  aabbTree.Traverse (inner, leaf);

  //@@TODO: Implement cross-sector lookups
}

void csLightManager::GetRelevantLights (iSector* sector, const csBox3& boundingBox,
  iLightInfluenceCallback* lightCallback, int maxLights, 
  uint flags)
{
  iLightList* llist = sector->GetLights ();
  csSectorLightList* sectorLightList = static_cast<csSectorLightList*> (llist);

  // Get the primary lights from same sector
  const csSectorLightList::LightAABBTree& aabbTree = sectorLightList->GetLightAABBTree ();
  IntersectInnerBBox inner (boundingBox);
  LightCollectCallback leaf (boundingBox, lightCallback);
  aabbTree.Traverse (inner, leaf);
}

// ---------------------------------------------------------------------------

void csLightManager::FreeInfluenceArray (csLightInfluence* Array)
{
  cs_free (Array);
}

template<typename T>
class csDirtyAccessArrayDetach : 
  public csDirtyAccessArray<T, csArrayElementHandler<T>,
                            CS::Memory::AllocatorMalloc>
{
public:
  T* Detach()
  {
    T* p = this->GetArray();
    this->SetData (0);
    return p;
  }
};

typedef csDirtyAccessArrayDetach<csLightInfluence> LightInfluenceArray;

struct LightCollectArrayPtr
{
  LightCollectArrayPtr (const csBox3& box, LightInfluenceArray& arr, size_t max)
    : testBox (box), arr (arr), max (max)
  {}

  void operator() (const csSectorLightList::LightAABBTree::Node* node)
  {
    if (!testBox.TestIntersect (node->GetBBox ()))
      return;

    for (size_t i = 0; i < node->GetObjectCount (); ++i)
    {
      if (arr.GetSize() < max)
      {
        csLightInfluence newInfluence;
        newInfluence.light = node->GetLeafData (i);
        arr.Push (newInfluence);
      }
    }
  }

  const csBox3& testBox;
  LightInfluenceArray& arr;
  size_t max;
};


void csLightManager::GetRelevantLights (iMeshWrapper* meshObject,
                                        csLightInfluence*& lightArray, 
                                        size_t& numLights, 
                                        size_t maxLights, uint flags)
{
  const csBox3& meshBox = meshObject->GetWorldBoundingBox ();
  iSectorList* sectors = meshObject->GetMovable ()->GetSectors ();
  if (sectors && sectors->GetCount() > 0)
  {
    csLightManager::GetRelevantLights (sectors->Get (0), meshBox, lightArray,
      numLights, maxLights, flags);
  }
}

void csLightManager::GetRelevantLights (iSector* sector, 
                                        const csBox3& boundingBox, 
                                        csLightInfluence*& lightArray, 
                                        size_t& numLights, 
                                        size_t maxLights, uint flags)
{
  iLightList* llist = sector->GetLights ();
  csSectorLightList* sectorLightList = static_cast<csSectorLightList*> (llist);

  // Get the primary lights from same sector
  const csSectorLightList::LightAABBTree& aabbTree = sectorLightList->GetLightAABBTree ();
  IntersectInnerBBox inner (boundingBox);
  LightInfluenceArray tmpLightArray;
  LightCollectArrayPtr leaf (boundingBox, tmpLightArray, maxLights);
  aabbTree.Traverse (inner, leaf);

  //@@TODO: Implement cross-sector lookups
  numLights = tmpLightArray.GetSize();
  if (numLights > 0)
    lightArray = tmpLightArray.Detach();
  else
    lightArray = 0;
}
  
void csLightManager::GetRelevantLights (iSector* sector, 
                                        csLightInfluence*& lightArray, 
                                        size_t& numLights, 
                                        size_t maxLights, uint flags)
{
  const csBox3 bigBox;  
  csLightManager::GetRelevantLights (sector, bigBox, lightArray, numLights,
    maxLights, flags);
}
