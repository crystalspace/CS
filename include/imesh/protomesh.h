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

#ifndef __CS_IMESH_PROTOMESH_H__
#define __CS_IMESH_PROTOMESH_H__

#include "csutil/scf.h"

class csVector3;
class csVector2;
class csColor;

struct csTriangle;
struct iMaterialWrapper;

SCF_VERSION (iProtoFactoryState, 0, 0, 1);

/**
 * The proto mesh is a demonstration or tutorial mesh. It is
 * very simple and really unusable in games but it is a very good
 * start to make a new mesh object. It supports only the new renderer
 * but should compile with old renderer too (just not visible then).
 * <p>
 * The proto mesh supports:
 * <ul>
 * <li>Primitive geometry (8 vertices, 12 triangles, just enough for a box).
 * <li>Setting of base color and per vertex color.
 * <li>Setting of vertices, texels, and normals.
 * <li>Material per mesh object.
 * <li>Sharing geometry in the factory.
 * <li>Collision detection.
 * <li>Direct creation of render buffers.
 * <li>Delayed creation of render buffers.
 * </ul>
 * <p>
 * The general API for the proto factory. Here you define the
 * actual geometry which is shared between all proto mesh instances.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Protomesh mesh object plugin (crystalspace.mesh.object.protomesh)
 *   <li>iMeshObjectType::NewFactory()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>SCF_QUERY_INTERFACE() on iMeshFactoryWrapper::GetMeshObjectFactory()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>Protomesh Factory Loader plugin
        (crystalspace.mesh.loader.factory.protomesh)
 *   </ul>
 */
struct iProtoFactoryState : public iBase
{
  /**
   * Get the array of vertices. It is legal to modify the vertices
   * in this array. The number of vertices in this array is guaranteed
   * to be equal to 8.
   */
  virtual csVector3* GetVertices () = 0;
  /**
   * Get the array of texels. It is legal to modify the texels in this
   * array. The number of texels in this array is guaranteed to
   * be equal to 8.
   */
  virtual csVector2* GetTexels () = 0;
  /**
   * Get the array of normals. It is legal to modify the normals in this
   * array. The number of normals in this array is guaranteed to
   * be equal to 8.
   */
  virtual csVector3* GetNormals () = 0;
  /**
   * Get the array of colors. It is legal to modify the colors in this
   * array. The number of colors in this array is guaranteed to
   * be equal to 8.
   */
  virtual csColor* GetColors () = 0;
  /**
   * Get the array of triangles. It is legal to modify the triangles in this
   * array. The number of triangles in this array is guaranteed to
   * be equal to 12.
   */
  virtual csTriangle* GetTriangles () = 0;

  /**
   * After making a significant change to the vertices or triangles you
   * probably want to let this object recalculate the bounding boxes
   * and such. This function will invalidate the internal data structures
   * so that they are recomputed.
   */
  virtual void Invalidate () = 0;
};

SCF_VERSION (iProtoMeshState, 0, 0, 1);

/**
 * This interface describes the API for the proto mesh object.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Proto mesh object plugin (crystalspace.mesh.object.protomesh)
 *   <li>iMeshObjectFactory::NewInstance()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>SCF_QUERY_INTERFACE() on iMeshWrapper::GetMeshObject()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>Protomesh Loader plugin (crystalspace.mesh.loader.protomesh)
 *   </ul>
 */
struct iProtoMeshState : public iBase
{
  /// Set material of mesh.
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;
  /// Get material of mesh.
  virtual iMaterialWrapper* GetMaterialWrapper () const = 0;
  /// Set mix mode.
  virtual void SetMixMode (uint mode) = 0;
  /// Get mix mode.
  virtual uint GetMixMode () const = 0;

  /// Set the base color to use. Will be added to the colors values.
  virtual void SetColor (const csColor& col) = 0;
  /// Get the color.
  virtual const csColor& GetColor () const = 0;
};

#endif // __CS_IMESH_PROTOMESH_H__

