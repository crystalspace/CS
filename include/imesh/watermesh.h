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

#ifndef __CS_IMESH_WATERMESH_H__
#define __CS_IMESH_WATERMESH_H__

/**\file
 * Tutorial mesh object
 */ 

#include "csutil/scf.h"
#include "csutil/scf_implementation.h"

/**\addtogroup meshplugins
 * @{ */

class csVector3;
class csVector2;
class csColor;

struct csTriangle;
struct iMaterialWrapper;

/**
 * The proto mesh is a demonstration or tutorial mesh. It is
 * very simple and really unusable in games but it is a very good
 * start to make a new mesh object. 
 * 
 * The proto mesh supports:
 * - Primitive geometry (8 vertices, 12 triangles, just enough for a box).
 * - Setting of base color and per vertex color.
 * - Setting of vertices, texels, and normals.
 * - Material per mesh object.
 * - Sharing geometry in the factory.
 * - Collision detection.
 * - Direct creation of render buffers.
 * - Delayed creation of render buffers.
 * 
 * The general API for the proto factory. Here you define the
 * actual geometry which is shared between all proto mesh instances.
 * 
 * Main creators of instances implementing this interface:
 * - Protomesh mesh object plugin (crystalspace.mesh.object.protomesh)
 * - iMeshObjectType::NewFactory()
 *   
 * Main ways to get pointers to this interface:
 * - scfQueryInterface() on iMeshFactoryWrapper::GetMeshObjectFactory()
 *   
 * Main users of this interface:
 * - Protomesh Factory Loader plugin
        (crystalspace.mesh.loader.factory.protomesh)
 *   
 */
struct iWaterFactoryState : public virtual iBase
{
  SCF_INTERFACE (iWaterFactoryState, 2, 0, 0);

  virtual csVector3* GetVertices () = 0;
  virtual csVector2* GetTexels () = 0;
  virtual csVector3* GetNormals () = 0;
  virtual csColor* GetColors () = 0;
  virtual csTriangle* GetTriangles () = 0;

  /**
   * After making a significant change to the vertices or triangles you
   * probably want to let this object recalculate the bounding boxes
   * and such. This function will invalidate the internal data structures
   * so that they are recomputed.
   */
  virtual void Invalidate () = 0;

  virtual void SetLength(uint length) = 0;
  virtual uint GetLength() = 0;

  virtual void SetWidth(uint width) = 0;
  virtual uint GetWidth() = 0;

  virtual void SetGranularity(uint granularity) = 0;
  virtual uint GetGranularity() = 0;

  virtual void SetMurkiness(float murk) = 0;
  virtual float GetMurkiness() = 0;

  //Size must be a power of two.
  //virtual csRef<iTextureWrapper> MakeFresnelTex(int size);
};

/**
 * This interface describes the API for the proto mesh object.
 * 
 * Main creators of instances implementing this interface:
 * - Proto mesh object plugin (crystalspace.mesh.object.protomesh)
 * - iMeshObjectFactory::NewInstance()
 *   
 * Main ways to get pointers to this interface:
 * - scfQueryInterface() on iMeshWrapper::GetMeshObject()
 *   
 * Main users of this interface:
 * - Protomesh Loader plugin (crystalspace.mesh.loader.protomesh)
 *   
 */
struct iWaterMeshState : public virtual iBase
{
  SCF_INTERFACE (iWaterMeshState, 1, 0, 0);

  virtual void SetNormalMap(iTextureWrapper *map) = 0;
  virtual iTextureWrapper *GetNormalMap() = 0;
};

/** @} */

#endif // __CS_IMESH_PROTOMESH_H__