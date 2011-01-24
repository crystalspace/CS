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
#include "plugins/engine/3d/light.h"
#include "plugins/engine/3d/lightmgr.h"
#include "plugins/engine/3d/meshobj.h"
#include "plugins/engine/3d/sector.h"

#include "csgeom/sphere.h"

using namespace CS_PLUGIN_NAMESPACE_NAME(Engine);

static csLightInfluence MakeInfluence (csLight* light,
                                       const csBox3& box,
                                       const csVector3& lightCenter)
{
  csLightInfluence l;
  l.light = light;
  if (light != 0)
  {
    l.type = light->csLight::GetType();
    l.flags = light->csLight::GetFlags();
    l.dynamicType = light->csLight::GetDynamicType();
    
    float distAttn;
    if (box.In (lightCenter))
      distAttn = 1.0f;
    else
    {
      distAttn = csMin (light->GetBrightnessAtDistance (
        sqrt (box.SquaredPosDist (lightCenter))), 1.0f);
    }
    l.perceivedIntensity = light->GetColor().Luminance() * distAttn;
  }
  return l;
}

// ---------------------------------------------------------------------------

csLightManager::csLightManager ()
  : scfImplementationType (this), tempInfluencesUsed (false)
{
}

csLightManager::~csLightManager ()
{
}

void csLightManager::GetRelevantLights (iMeshWrapper* meshObject, 
    iLightInfluenceArray* lightArray, int maxLights, uint flags)
{
  const csBox3& meshBox = meshObject->GetMeshObject()
    ->GetObjectModel()->GetObjectBoundingBox();
  csReversibleTransform objectToWorld =
    meshObject->GetMovable()->GetFullTransform();
  iSectorList* sectors = meshObject->GetMovable ()->GetSectors ();
  if (sectors && sectors->GetCount() > 0)
  {
    GetRelevantLights (sectors->Get (0), meshBox, lightArray, maxLights,
      &objectToWorld, flags);
  }
}

void csLightManager::GetRelevantLights (iMeshWrapper* meshObject, 
  iLightInfluenceCallback* lightCallback, int maxLights, 
  uint flags)
{
  const csBox3& meshBox = meshObject->GetMeshObject()
    ->GetObjectModel()->GetObjectBoundingBox();
  csReversibleTransform objectToWorld =
    meshObject->GetMovable()->GetFullTransform();
  iSectorList* sectors = meshObject->GetMovable ()->GetSectors ();
  if (sectors && sectors->GetCount() > 0)
  {
    GetRelevantLights (sectors->Get (0), meshBox, lightCallback, maxLights,
      &objectToWorld, flags);
  }
}


void csLightManager::GetRelevantLights (iSector* sector, 
  iLightInfluenceArray* lightArray, int maxLights, 
  uint flags)
{
  const csBox3 bigBox (csVector3 (-CS_BOUNDINGBOX_MAXVALUE),
    csVector3 (CS_BOUNDINGBOX_MAXVALUE));
  GetRelevantLights (sector, bigBox, lightArray, maxLights, 0, flags);
}

void csLightManager::GetRelevantLights (iSector* sector, 
  iLightInfluenceCallback* lightCallback, int maxLights, 
  uint flags)
{
  const csBox3 bigBox (csVector3 (-CS_BOUNDINGBOX_MAXVALUE),
    csVector3 (CS_BOUNDINGBOX_MAXVALUE));
  GetRelevantLights (sector, bigBox, lightCallback, maxLights, 0, flags);
}

struct BoxSpaceIdentity
{
  csSphere FromWorld (const csSphere& sphere) const { return sphere; }
  csBox3 ToWorld (const csBox3& box) const { return box; }
};

struct BoxSpaceTransform
{
  BoxSpaceTransform (const csReversibleTransform& tf) : tf (tf) {}

  csSphere FromWorld (const csSphere& sphere) const
  { return tf.Other2This (sphere); }
  csBox3 ToWorld (const csBox3& box) const
  { return tf.This2Other (box); }
protected:
  const csReversibleTransform& tf;
};

struct IntersectInnerBBoxAndLightFilter
{
  IntersectInnerBBoxAndLightFilter (const csBox3& box, uint lightFilter)
    : testBox (box), lightFilter (lightFilter)
  {}

  bool operator() (const csSectorLightList::LightAABBTree::Node* node)
  {
    return ((node->lightTypes & lightFilter) != 0)
      && testBox.TestIntersect (node->GetBBox ());
  }

  const csBox3& testBox;
  uint lightFilter;
};

template<typename BoxSpace>
struct LightCollectArray
{
  LightCollectArray (const BoxSpace& boxSpace,
    const csBox3& box, const csBox3& boxWorld, 
    uint lightFilter,
    iLightInfluenceArray* lightArray)
    : boxSpace (boxSpace), testBox (box), testBoxWorld (boxWorld),
      lightFilter (lightFilter), lightArray (lightArray)
  {}

  bool operator() (const csSectorLightList::LightAABBTree::Node* node)
  {
    if ((node->lightTypes & lightFilter) == 0)
      return true;
  
    if (!testBoxWorld.TestIntersect (node->GetBBox ()))
      return true;

    for (size_t i = 0; i < node->GetObjectCount (); ++i)
    {
      csLight* light = node->GetLeafData (i);
      
      csSphere lightSphere (light->GetMovable()->GetFullPosition(),
        light->GetCutoffDistance());
      lightSphere = boxSpace.FromWorld (lightSphere);
      if (!csIntersect3::BoxSphere (testBox, lightSphere.GetCenter(),
          lightSphere.GetRadius()*lightSphere.GetRadius()))
        continue;
      
      csLightInfluence newInfluence = MakeInfluence (light,
        testBox, lightSphere.GetCenter());
      if ((lightFilter
          & LightExtraAABBNodeData::GetLightType (newInfluence.dynamicType)) == 0)
        continue;
      lightArray->Push (newInfluence);
    }
      
    return true;
  }

  const BoxSpace& boxSpace;
  const csBox3& testBox;
  const csBox3& testBoxWorld;
  uint lightFilter;
  iLightInfluenceArray* lightArray;
};

template<typename BoxSpace>
struct LightCollectCallback
{
  LightCollectCallback (const BoxSpace& boxSpace,
    const csBox3& box, const csBox3& boxWorld,
    uint lightFilter,
    iLightInfluenceCallback* lightCallback)
    : boxSpace (boxSpace), testBox (box), testBoxWorld (boxWorld),
      lightFilter (lightFilter), lightCallback (lightCallback)
  {}

  bool operator() (const csSectorLightList::LightAABBTree::Node* node)
  {
    if ((node->lightTypes & lightFilter) == 0)
      return true;
  
    if (!testBoxWorld.TestIntersect (node->GetBBox ()))
      return true;

    for (size_t i = 0; i < node->GetObjectCount (); ++i)
    {
      csLight* light = node->GetLeafData (i);
      
      csSphere lightSphere (light->GetMovable()->GetFullPosition(),
        light->GetCutoffDistance());
      lightSphere = boxSpace.FromWorld (lightSphere);
      if (!csIntersect3::BoxSphere (testBox, lightSphere.GetCenter(),
          lightSphere.GetRadius()*lightSphere.GetRadius()))
        continue;
      
      csLightInfluence newInfluence = MakeInfluence (light,
        testBox, lightSphere.GetCenter());
      if ((lightFilter
          & LightExtraAABBNodeData::GetLightType (newInfluence.dynamicType)) == 0)
        continue;
      lightCallback->LightInfluence (newInfluence);
    }
    return true;
  }

  const BoxSpace& boxSpace;
  const csBox3& testBox;
  const csBox3& testBoxWorld;
  uint lightFilter;
  iLightInfluenceCallback* lightCallback;
};


void csLightManager::GetRelevantLights (iSector* sector, const csBox3& boundingBox,
  iLightInfluenceArray* lightArray, int maxLights, 
  const csReversibleTransform* bboxToWorld, uint flags)
{
  iLightList* llist = sector->GetLights ();
  csSectorLightList* sectorLightList = static_cast<csSectorLightList*> (llist);
  
  uint lightFilter = 0;
  if (flags & CS_LIGHTQUERY_GET_TYPE_STATIC)
    lightFilter |= LightExtraAABBNodeData::ltStatic;
  if (flags & CS_LIGHTQUERY_GET_TYPE_DYNAMIC)
    lightFilter |= LightExtraAABBNodeData::ltDynamic;

  // Get the primary lights from same sector
  const csSectorLightList::LightAABBTree& aabbTree = sectorLightList->GetLightAABBTree ();
  if (!bboxToWorld || bboxToWorld->IsIdentity())
  {
    BoxSpaceIdentity boxSpace;
    IntersectInnerBBoxAndLightFilter inner (boundingBox, lightFilter);
    LightCollectArray<BoxSpaceIdentity> leaf (boxSpace, boundingBox, 
      boundingBox, lightFilter, lightArray);
    aabbTree.Traverse (inner, leaf);
  }
  else
  {
    BoxSpaceTransform boxSpace (*bboxToWorld);
    csBox3 boxWorld (boxSpace.ToWorld (boundingBox));
    IntersectInnerBBoxAndLightFilter inner (boxWorld, lightFilter);
    LightCollectArray<BoxSpaceTransform> leaf (boxSpace, boundingBox, 
      boxWorld, lightFilter, lightArray);
    aabbTree.Traverse (inner, leaf);
  } 

  //@@TODO: Implement cross-sector lookups
}

void csLightManager::GetRelevantLights (iSector* sector, const csBox3& boundingBox,
  iLightInfluenceCallback* lightCallback, int maxLights, 
  const csReversibleTransform* bboxToWorld, uint flags)
{
  iLightList* llist = sector->GetLights ();
  csSectorLightList* sectorLightList = static_cast<csSectorLightList*> (llist);

  uint lightFilter = 0;
  if (flags & CS_LIGHTQUERY_GET_TYPE_STATIC)
    lightFilter |= LightExtraAABBNodeData::ltStatic;
  if (flags & CS_LIGHTQUERY_GET_TYPE_DYNAMIC)
    lightFilter |= LightExtraAABBNodeData::ltDynamic;

  // Get the primary lights from same sector
  const csSectorLightList::LightAABBTree& aabbTree = sectorLightList->GetLightAABBTree ();
  if (!bboxToWorld || bboxToWorld->IsIdentity())
  {
    BoxSpaceIdentity boxSpace;
    IntersectInnerBBoxAndLightFilter inner (boundingBox, lightFilter);
    LightCollectCallback<BoxSpaceIdentity> leaf (boxSpace, boundingBox, 
      boundingBox, lightFilter, lightCallback);
    aabbTree.Traverse (inner, leaf);
  }
  else
  {
    BoxSpaceTransform boxSpace (*bboxToWorld);
    csBox3 boxWorld (boxSpace.ToWorld (boundingBox));
    IntersectInnerBBoxAndLightFilter inner (boxWorld, lightFilter);
    LightCollectCallback<BoxSpaceTransform> leaf (boxSpace, boundingBox, 
      boxWorld, lightFilter, lightCallback);
    aabbTree.Traverse (inner, leaf);
  } 

}

// ---------------------------------------------------------------------------

void csLightManager::FreeInfluenceArray (csLightInfluence* Array)
{
  if (Array == 0) return;

  if (tempInfluencesUsed && (Array == tempInfluences.GetArray()))
  {
    tempInfluences.Empty();
    tempInfluencesUsed = false;
  }
  else
    cs_free (Array);
}

typedef csDirtyAccessArray<csLightInfluence> LightInfluenceArray;

template<typename ArrayType, typename BoxSpace>
struct LightCollectArrayPtr
{
  LightCollectArrayPtr (const BoxSpace& boxSpace,
    const csBox3& box, const csBox3& boxWorld,
    uint lightFilter,
    ArrayType& arr, size_t max)
    : boxSpace (boxSpace), testBox (box), testBoxWorld (boxWorld),
      lightFilter (lightFilter), arr (arr), max (max)
  {}

  bool operator() (const csSectorLightList::LightAABBTree::Node* node)
  {
    if ((node->lightTypes & lightFilter) == 0)
      return true;
  
    if (!testBoxWorld.TestIntersect (node->GetBBox ()))
      return true;

    for (size_t i = 0; i < node->GetObjectCount (); ++i)
    {
      csLight* light = node->GetLeafData (i);
      
      csSphere lightSphere (light->GetMovable()->GetFullPosition(),
        light->GetCutoffDistance());
      lightSphere = boxSpace.FromWorld (lightSphere);
      if (!csIntersect3::BoxSphere (testBox, lightSphere.GetCenter(),
          lightSphere.GetRadius()*lightSphere.GetRadius()))
        continue;
      
      if (arr.GetSize() < max)
      {
        csLightInfluence newInfluence = MakeInfluence (light,
          testBox, lightSphere.GetCenter());
	if ((lightFilter
	    & LightExtraAABBNodeData::GetLightType (newInfluence.dynamicType)) == 0)
	  continue;
        arr.Push (newInfluence);
      }
      else
        return false;
    }
    return true;
  }

  const BoxSpace& boxSpace;
  const csBox3& testBox;
  const csBox3& testBoxWorld;
  uint lightFilter;
  ArrayType& arr;
  size_t max;
};


void csLightManager::GetRelevantLights (iMeshWrapper* meshObject,
                                        csLightInfluence*& lightArray, 
                                        size_t& numLights, 
                                        size_t maxLights, uint flags)
{
  const csBox3& meshBox = meshObject->GetMeshObject()
    ->GetObjectModel()->GetObjectBoundingBox();
  csReversibleTransform objectToWorld =
    meshObject->GetMovable()->GetFullTransform();
  iSectorList* sectors = meshObject->GetMovable ()->GetSectors ();
  if (sectors && sectors->GetCount() > 0)
  {
    csLightManager::GetRelevantLights (sectors->Get (0), meshBox, lightArray,
      numLights, maxLights, &objectToWorld, flags);
  }
}

template<typename BoxSpace>
void csLightManager::GetRelevantLightsWorker (const BoxSpace& boxSpace,
                                              iSector* sector, 
                                              const csBox3& boundingBox,
                                              csLightInfluence*& lightArray, 
                                              size_t& numLights,
                                              size_t maxLights,
                                              uint flags)
{
  iLightList* llist = sector->GetLights ();
  csSectorLightList* sectorLightList = static_cast<csSectorLightList*> (llist);

  uint lightFilter = 0;
  if (flags & CS_LIGHTQUERY_GET_TYPE_STATIC)
    lightFilter |= LightExtraAABBNodeData::ltStatic;
  if (flags & CS_LIGHTQUERY_GET_TYPE_DYNAMIC)
    lightFilter |= LightExtraAABBNodeData::ltDynamic;

  // Get the primary lights from same sector
  //@@TODO: Implement cross-sector lookups
  const csSectorLightList::LightAABBTree& aabbTree = sectorLightList->GetLightAABBTree ();
  csBox3 boxWorld (boxSpace.ToWorld (boundingBox));
  IntersectInnerBBoxAndLightFilter inner (boxWorld, lightFilter);
  if (!tempInfluencesUsed)
  {
    tempInfluencesUsed = true;
    LightCollectArrayPtr<TempInfluences, BoxSpace> leaf (boxSpace,
      boundingBox, boxWorld, lightFilter, tempInfluences, maxLights);
    aabbTree.Traverse (inner, leaf);
    
    numLights = tempInfluences.GetSize();
    if (numLights > 0)
      lightArray = tempInfluences.GetArray();
    else
    {
      lightArray = 0;
      tempInfluencesUsed = false;
    }
  }
  else
  {
    LightInfluenceArray tmpLightArray;
    LightCollectArrayPtr<LightInfluenceArray, BoxSpace> leaf (boxSpace,
      boundingBox, boxWorld, lightFilter, tmpLightArray, maxLights);
    aabbTree.Traverse (inner, leaf);
    
    numLights = tmpLightArray.GetSize();
    if (numLights > 0)
      lightArray = tmpLightArray.Detach();
    else
      lightArray = 0;
  }
}

void csLightManager::GetRelevantLights (iSector* sector, 
                                        const csBox3& boundingBox, 
                                        csLightInfluence*& lightArray, 
                                        size_t& numLights, 
                                        size_t maxLights,
                                        const csReversibleTransform* bboxToWorld,
                                        uint flags)
{
  if (!bboxToWorld || bboxToWorld->IsIdentity())
  {
    BoxSpaceIdentity boxspace;
    GetRelevantLightsWorker (boxspace, sector, boundingBox, lightArray, numLights,
      maxLights, flags);
  }
  else
  {
    BoxSpaceTransform boxspace (*bboxToWorld);
    GetRelevantLightsWorker (boxspace, sector, boundingBox, lightArray, numLights,
      maxLights, flags);
  } 
}
  
void csLightManager::GetRelevantLights (iSector* sector, 
                                        csLightInfluence*& lightArray, 
                                        size_t& numLights, 
                                        size_t maxLights, uint flags)
{
  const csBox3 bigBox;  
  csLightManager::GetRelevantLights (sector, bigBox, lightArray, numLights,
    maxLights, 0, flags);
}

static int SortInfluenceByIntensity (const void* a, const void* b)
{
  float d = reinterpret_cast<const csLightInfluence*>(a)->perceivedIntensity
    - reinterpret_cast<const csLightInfluence*>(b)->perceivedIntensity;
  if (d < 0)
    return 1;
  else if (d > 0)
    return -1;
  else
    return 0;
}

void csLightManager::GetRelevantLightsSorted (iSector* sector,
                                              const csBox3& boundingBox,
                                              csLightInfluence*& lightArray, 
                                              size_t& numLights,
                                              size_t maxLights,
                                              const csReversibleTransform* bboxToWorld,
                                              uint flags)
{
  // Get all lights,
  csLightManager::GetRelevantLights (sector, boundingBox, lightArray, numLights,
    (size_t)~0, bboxToWorld, flags);
  // sort,
  qsort (lightArray, numLights, sizeof (csLightInfluence),
    SortInfluenceByIntensity);
  // return only first numLights lights
  numLights = csMin (numLights, maxLights);
}
