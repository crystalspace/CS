/*
    Copyright (C) 2011 by Frank Richter

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

#ifndef __CS_MESHGEN_DENSITYFACTORMAP_H__
#define __CS_MESHGEN_DENSITYFACTORMAP_H__

#include "csgeom/matrix4.h"
#include "csutil/customallocated.h"
#include "csutil/ref.h"
#include "csutil/refcount.h"

struct iBase;
struct iImage;

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{
  /**
   * A map of mesh placement densities, used by the mesh generator.
   */
  class DensityFactorMap : public CS::Utility::FastRefCount<DensityFactorMap>
  {
  private:
    csRef<iBase> mapDataKeeper;
    const uint8* mapPtr;
    int width;
    int height;
    
    CS::Math::Matrix4 world2map;
  public:
    DensityFactorMap();
    
    bool IsValid() const { return mapPtr != 0; }
    float GetDensity (const csVector3& worldCoord) const;
    int GetWidth () const { return width; }
    int GetHeight () const { return height; }
    
    void SetImage (iImage* image);
    void SetWorldToMapTransform (const CS::Math::Matrix4& tf);
    const CS::Math::Matrix4& GetWorldToMapTransform () const { return world2map; }
  };
}
CS_PLUGIN_NAMESPACE_END(Engine)

#endif // __CS_MESHGEN_DENSITYFACTORMAP_H__
