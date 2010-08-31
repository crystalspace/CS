/*
  Copyright (C) 2010 Alexandru - Teodor Voicu

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __FURDATA_H__
#define __FURDATA_H__

#include <iutil/comp.h>
#include <csgeom/vector3.h>
#include <imesh/furmesh.h>
#include <csgfx/shadervarcontext.h>
#include <imesh/genmesh.h>

#include "crystalspace.h"

#include "csutil/scf_implementation.h"

#define GUIDE_HAIRS_COUNT 3

struct iObjectRegistry;

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  struct csGuideHair;
  struct csGuideHairLOD;

  struct csHairData
  {
    csVector3 *controlPoints;
    size_t controlPointsCount;
  };

  struct csGuideHairReference
  {
    size_t index;
    float distance;
  };

  struct csHairStrand : csHairData
  {
    csVector2 GetUV( const csArray<csGuideHair> &guideHairs,
      const csArray<csGuideHairLOD> &guideHairsLOD ) const;

    //Generate & Update
    void Generate( size_t controlPointsCount, const csArray<csGuideHair> &guideHairs,
      const csArray<csGuideHairLOD> &guideHairsLOD );

    void Update( const csArray<csGuideHair> &guideHairs,
      const csArray<csGuideHairLOD> &guideHairsLOD );

    csGuideHairReference guideHairsRef[GUIDE_HAIRS_COUNT];
  };

  struct csGuideHair : csHairData
  {
    void Generate(size_t controlPointsCount, float distance,
      const csVector3& pos, const csVector3& direction);

    csVector2 uv;
  };

  struct csGuideHairLOD : csGuideHair
  {
    void Generate( size_t controlPointsCount, const csArray<csGuideHair> &guideHairs,
      const csArray<csGuideHairLOD> &guideHairsLOD );

    void Update( const csArray<csGuideHair> &guideHairs,
      const csArray<csGuideHairLOD> &guideHairsLOD );

    csGuideHairReference guideHairsRef[GUIDE_HAIRS_COUNT];
    bool isActive;  //  ropes vs interpolate
  };

}
CS_PLUGIN_NAMESPACE_END(FurMesh)

#endif // __FURDATA_H__
