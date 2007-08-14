/*
    Copyright (C) 2007 by Jorrit Tyberghein

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

#ifndef __CS_GENMESHPRIM_H__
#define __CS_GENMESHPRIM_H__

/**\file
 * Primitive Mesh Generator for GenMesh.
 */

#include "csextern.h"

#include "cstool/primitives.h"

struct iGeneralFactoryState;

/**
 * A primitive mesh generator for genmesh.
 */
class CS_CRYSTALSPACE_EXPORT csGeneralMeshPrimitives
{
public:
  /**
   * Generate a box with 24 vertices and 12 triangles so that
   * the normals of every face point inwards or outwards (the normals of the
   * vertices belonging to a face will point with the correct
   * normal of the face).
   * \param append if true then append the vertices and triangles
   * to the geometry already in the factory.
   * \param flags is a combination of csPrimitives::BoxFlags enumeration
   * values. Default is CS_PRIMBOX_SMOOTH.
   */
  static void GenerateBox (
      iGeneralFactoryState* factory, bool append,
      const csBox3& box,
      uint32 flags = csPrimitives::CS_PRIMBOX_SMOOTH);

  /**
   * Generate quad.
   * \param append if true then append the vertices and triangles
   * to the geometry already in the factory.
   */
  static void GenerateQuad (
      iGeneralFactoryState* factory, bool append,
      const csVector3 &v1, const csVector3 &v2,
      const csVector3 &v3, const csVector3 &v4);

  /**
   * Generate a capsule of given length and radius.
   * \param append if true then append the vertices and triangles
   * to the geometry already in the factory.
   * \param l Capsule length.
   * \param r Capsule radius.
   * \param sides Number of sides.
   */
  static void GenerateCapsule (
      iGeneralFactoryState* factory, bool append,
      float l, float r, uint sides);

  /**
   * Generate a sphere with 'num' vertices on the rim.
   * \param append if true then append the vertices and triangles
   * to the geometry already in the factory.
   * \param ellips Properties of the ellipsoid to create.
   * \param num Number of vertices in the generated  mesh.
   * \param cyl_mapping if true then use cylindrical texture mapping.
   * \param toponly if true then only generate the top half of the sphere.
   * \param reversed if true then generate the sphere so it is visible
   * from the inside.
   */
  static void GenerateSphere (
      iGeneralFactoryState* factory, bool append,
      const csEllipsoid& ellips, int num,
      bool cyl_mapping = false,
      bool toponly = false,
      bool reversed = false);
};

/** @} */

#endif // __CS_GENMESHPRIM_H__

