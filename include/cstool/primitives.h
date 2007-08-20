/*
    Copyright (C) 2005 by Jorrit Tyberghein

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

#ifndef __CS_PRIMITIVES_H__
#define __CS_PRIMITIVES_H__

/**\file
 * Primitive Mesh Generator
 */

#include "csextern.h"

#include "csgeom/vector3.h"
#include "csgeom/vector2.h"
#include "csutil/cscolor.h"
#include "csutil/dirtyaccessarray.h"

class csBox3;
class csEllipsoid;

namespace CS
{
namespace Geometry
{

/**
 * Class for controlling texture mapping on the generated meshes.
 * Implement this class and pass it to one of the csPrimitives
 * functions to create a mesh with specific texture mapping requirements.
 */
struct TextureMapper
{
  /**
   * Map a 3D coordinate and a triangle plane to 2D UV space.
   * \param point is the point in 3D space.
   * \param normal is the normal of the point that we're mapping.
   * \param idx is the index in the model.
   * \return the resulting uv mapping.
   */
  virtual csVector2 Map (const csVector3& point, const csVector3& normal,
  	size_t idx) = 0;
};

/**
 * Table texture mapper. This one just maps based on a table and
 * vertex index given at construction time.
 */
class TableTextureMapper : public TextureMapper
{
private:
  const csVector2* table;

public:
  TableTextureMapper (const csVector2* table) : table (table) { }
  virtual ~TableTextureMapper () { }
  virtual csVector2 Map (const csVector3&, const csVector3&, size_t idx)
  {
    return table[idx];
  }
};

} // namespace Geometry
} // namespace CS

/**
 * A primitive mesh generator.
 */
class CS_CRYSTALSPACE_EXPORT csPrimitives
{
public:
  static csVector2 boxTable[];
  static csVector2 quadTable[];

  enum BoxFlags
  {
    CS_PRIMBOX_INSIDE = 1,
    CS_PRIMBOX_SMOOTH = 2
  };

  /**
   * Generate a box with 24 vertices and 12 triangles so that
   * the normals of every face point in or outwards (the normals of the
   * vertices belonging to a face will point with the correct
   * normal of the face).
   * \param flags is a combination of BoxFlags enumeration values. Default
   * is CS_PRIMBOX_SMOOTH.
   * \param mapper is an optional texture mapper. If not given the default
   * TableTextureMapper is used with boxTable.
   */
  static void GenerateBox (
      const csBox3& box,
      csDirtyAccessArray<csVector3>& mesh_vertices,
      csDirtyAccessArray<csVector2>& mesh_texels,
      csDirtyAccessArray<csVector3>& mesh_normals,
      csDirtyAccessArray<csTriangle>& mesh_triangles,
      uint32 flags = CS_PRIMBOX_SMOOTH,
      CS::Geometry::TextureMapper* mapper = 0);

  /**
   * Generate quad.
   * \param mapper is an optional texture mapper. If not given the default
   * TableTextureMapper is used with quadTable.
   */
  static void GenerateQuad (
      const csVector3 &v1, const csVector3 &v2,
      const csVector3 &v3, const csVector3 &v4,
      csDirtyAccessArray<csVector3>& mesh_vertices,
      csDirtyAccessArray<csVector2>& mesh_texels,
      csDirtyAccessArray<csVector3>& mesh_normals,
      csDirtyAccessArray<csTriangle>& mesh_triangles,
      CS::Geometry::TextureMapper* mapper = 0);

  /**
   * Generate a capsule of given length and radius.
   * \param l Capsule length.
   * \param r Capsule radius.
   * \param sides Number of sides.
   * \param mapper is an optional texture mapper. If not given the
   * default capsule texture mapping will be used (currently not
   * implemented, you have to specify a mapper).
   */
  static void GenerateCapsule (float l, float r, uint sides,
      csDirtyAccessArray<csVector3>& mesh_vertices,
      csDirtyAccessArray<csVector2>& mesh_texels,
      csDirtyAccessArray<csVector3>& mesh_normals,
      csDirtyAccessArray<csTriangle>& mesh_triangles,
      CS::Geometry::TextureMapper* mapper = 0);

  /**
   * Generate a sphere with 'num' vertices on the rim.
   * \param ellips Properties of the ellipsoid to create.
   * \param num Number of vertices in the generated  mesh.
   * \param mesh_vertices Returns the generated vertices.
   * \param mesh_texels Returns the generated texture coordinates.
   * \param mesh_normals Returns the generated normals.
   * \param mesh_triangles Returns the generated triangles.
   * \param cyl_mapping if true then use cylindrical texture mapping.
   * \param toponly if true then only generate the top half of the sphere.
   * \param reversed if true then generate the sphere so it is visible
   * from the inside.
   * \param mapper is an optional texture mapper. If not given the
   * mapping as defined by the 'cyl_mapping' flag will be used.
   */
  static void GenerateSphere (const csEllipsoid& ellips, int num,
      csDirtyAccessArray<csVector3>& mesh_vertices,
      csDirtyAccessArray<csVector2>& mesh_texels,
      csDirtyAccessArray<csVector3>& mesh_normals,
      csDirtyAccessArray<csTriangle>& mesh_triangles,
      bool cyl_mapping = false,
      bool toponly = false,
      bool reversed = false,
      CS::Geometry::TextureMapper* mapper = 0);
};

/** @} */

#endif // __CS_PRIMITIVES_H__

