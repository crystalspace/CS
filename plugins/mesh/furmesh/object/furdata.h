/*
  Copyright (C) 2010 Alexandru - Teodor Voicu
      Faculty of Automatic Control and Computer Science of the "Politehnica"
      University of Bucharest
      http://csite.cs.pub.ro/index.php/en/

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

#include "crystalspace.h"

#include "csutil/scf_implementation.h"

#define GUIDE_HAIRS_COUNT 3

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  // Data stored for a RGBA texture for the FurMesh plugin implementation 
  struct csTextureRGBA
  {
    csRef<iTextureHandle> handle;
    int width, height;
    iDataBuffer* databuf;
    uint8* data;

    csTextureRGBA ();
    csTextureRGBA (int width, int height);

    uint8 Get(int x, int y, int channel) const;
    void Set(int x, int y, int channel, uint8 value) ;

    // Read data buffer from texture
    bool Read();
    // Write data buffer to texture
    void Write();

    // Create a black texture
    bool Create(iGraphics3D* g3d);
    // Save the texture to a png file
    void SaveImage(iObjectRegistry* object_reg, const char* texname) const;
  };

  struct csGuideFur;
  struct csGuideFurLOD;

  // Store control points for fur strands
  struct csFurData
  {
    size_t GetControlPointsCount(float controlPointsLOD) const;
    csVector3 *controlPoints;

    // Instead of ~csFurData
    void Clear();

  protected:
    size_t controlPointsCount;
  };

  // Reference to a fur strand
  struct csGuideFurReference
  {
    size_t index;
    float distance;
  };

  // Pure guide fur, synchronized with iFurPhysicsControl
  struct csGuideFur : csFurData
  {
    void Generate(size_t controlPointsCount, float distance,
      const csVector3& pos, const csVector3& direction);

    csVector2 uv;
  };

  // Fur strands, geometry rendered
  struct csFurStrand : csGuideFur
  {
    // Automatically set the uv coordinates
    void SetUV( const csArray<csGuideFur> &guideFurs,
      const csArray<csGuideFurLOD> &guideFursLOD );

    void Generate( size_t controlPointsCount, const csArray<csGuideFur> 
      &guideFurs, const csArray<csGuideFurLOD> &guideFursLOD );

    void Update( const csArray<csGuideFur> &guideFurs,
      const csArray<csGuideFurLOD> &guideFursLOD, float controlPointsLOD);

    void SetGuideHairsRefs(const csTriangle& triangle, csRandomGen *rng);

    csGuideFurReference guideHairsRef[GUIDE_HAIRS_COUNT];
  };

  // Non-pure guide ropes, can be synchronized or interpolated
  struct csGuideFurLOD : csFurStrand
  {
    bool isActive;
  };

}
CS_PLUGIN_NAMESPACE_END(FurMesh)

#endif // __FURDATA_H__
