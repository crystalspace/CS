/*
  Copyright (C) 2007 by Frank Richter

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

#ifndef __RAYDEBUG_H__
#define __RAYDEBUG_H__

#include "raytracer.h"

namespace lighter
{
  struct HitPoint;
  class Light;
  class Object;
  class Sector;

  class RayDebugHelper
  {
  public:
    struct LightObjectPair
    {
      csRef<Light> light;
      csRef<Object> obj;

      LightObjectPair (Light* light, Object* obj);
    };
  private:
    csRegExpMatcher* reMatcher;

    struct RayAndHits
    {
      csVector3 rayStart;
      csVector3 rayEnd;

      csArray<float, csArrayElementHandler<float>,
        CS::Memory::AllocatorMalloc,
        csArrayCapacityLinear<csArrayThresholdFixed<1> > > hits;
    };
    struct ObjectData
    {
      typedef csHash<RayAndHits, size_t> RayHash;
      RayHash rays;
    };
    struct SectorData
    {
      typedef csHash<ObjectData, LightObjectPair> ObjectDataHash;
      ObjectDataHash objectData;
    };
    csHash<SectorData, csRef<Sector> > allData;

    RayAndHits& GetHits (Light* light, Object* obj, const Ray &ray);

    void RealRegisterHit (Light* light, Object* obj,
      const Ray &ray, const HitPoint &hit);
    void RealRegisterUnhit (Light* light, Object* obj, const Ray &ray);
  public:
    RayDebugHelper ();
    ~RayDebugHelper ();

    bool IsEnabled() const { return reMatcher != 0; }
    
    void SetFilterExpression (const csString& expr);
    bool EnableForMesh (const char* name);

    inline void RegisterHit (Light* light, Object* obj, const Ray &ray,
                             const HitPoint &hit)
    {
      if (!IsEnabled()) return;
      RealRegisterHit (light, obj, ray, hit);
    }
    inline void RegisterUnhit (Light* light, Object* obj, const Ray &ray)
    {
      if (!IsEnabled()) return;
      RealRegisterUnhit (light, obj, ray);
    }

    void AppendMeshFactories (Sector* sector, iDocumentNode* node, 
      Statistics::Progress& progress);
    void AppendMeshObjects (Sector* sector, iDocumentNode* node, 
      Statistics::Progress& progress);

    void FreeInfo (Sector* sector);
  };
} // namespace lighter

template<>
class csHashComputer<lighter::RayDebugHelper::LightObjectPair> : 
  public csHashComputerStruct<lighter::RayDebugHelper::LightObjectPair> {};

template<>
class csComparator<lighter::RayDebugHelper::LightObjectPair> : 
  public csComparatorStruct<lighter::RayDebugHelper::LightObjectPair> {};

#endif // __RAYDEBUG_H__
