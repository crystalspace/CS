/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __IMESH_GENMESH_H__
#define __IMESH_GENMESH_H__

#include "csutil/scf.h"

class csVector3;
class csVector2;
class csColor;
class csBox3;
struct csTriangle;

struct iMaterialWrapper;

SCF_VERSION (iGeneralMeshState, 0, 0, 1);

/**
 * This interface describes the API for the general mesh object.
 */
struct iGeneralMeshState : public iBase
{
  /// Set material of mesh.
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;
  /// Get material of mesh.
  virtual iMaterialWrapper* GetMaterialWrapper () const = 0;
  /// Set mix mode.
  virtual void SetMixMode (uint mode) = 0;
  /// Get mix mode.
  virtual uint GetMixMode () const = 0;

  /// Set lighting.
  virtual void SetLighting (bool l) = 0;
  /// Is lighting enabled.
  virtual bool IsLighting () const = 0;
  /// Set the color to use. Will be added to the lighting values.
  virtual void SetColor (const csColor& col) = 0;
  /// Get the color.
  virtual csColor GetColor () const = 0;
  /**
   * Set manual colors. If this is set then lighting will be ignored
   * and so will the color set with SetColor(). In this case you can
   * manipulate the color array manually by calling GetColors().
   */
  virtual void SetManualColors (bool m) = 0;
  /// Are manual colors enabled?
  virtual bool IsManualColors () const = 0;
};

SCF_VERSION (iGeneralFactoryState, 0, 0, 2);

/**
 * This interface describes the API for the general mesh factory.
 */
struct iGeneralFactoryState : public iBase
{
  /// Set material of factory.
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;
  /// Get material of factory.
  virtual iMaterialWrapper* GetMaterialWrapper () const = 0;

  /// Set the number of vertices to use for this mesh.
  virtual void SetVertexCount (int n) = 0;
  /// Get the number of vertices for this mesh.
  virtual int GetVertexCount () const = 0;
  /**
   * Get the array of vertices. It is legal to modify the vertices
   * in this array. The number of vertices in this array will be
   * equal to the number of vertices set.
   */
  virtual csVector3* GetVertices () = 0;
  /**
   * Get the array of texels. It is legal to modify the texels in this
   * array. The number of texels in this array will be equal to
   * the number of vertices set.
   */
  virtual csVector2* GetTexels () = 0;
  /**
   * Get the array of normals. It is legal to modify the normals in this
   * array. The number of normals in this array will be equal to the
   * number of vertices set. Note that modifying the normals is only
   * useful when manual colors are not enabled and lighting is enabled
   * because the normals are used for lighting.
   */
  virtual csVector3* GetNormals () = 0;

  /// Set the number of triangles to use for this mesh.
  virtual void SetTriangleCount (int n) = 0;
  /// Get the number of triangles for this mesh.
  virtual int GetTriangleCount () const = 0;
  /**
   * Get the array of triangles. It is legal to modify the triangles in this
   * array. The number of triangles in this array will be equal to
   * the number of triangles set.
   */
  virtual csTriangle* GetTriangles () = 0;
  /**
   * Get the array of colors. It is legal to modify the colors in this
   * array. The number of colors in this array will be equal to the
   * number of vertices set. Note that modifying the colors will not do
   * a lot if manual colors is not enabled (SetManualColors).
   */
  virtual csColor* GetColors () = 0;

  /**
   * After making a significant change to the vertices or triangles you
   * probably want to let this object recalculate the bounding boxes
   * and such. This function will invalidate the internal data structures
   * so that they are recomputed.
   */
  virtual void Invalidate () = 0;

  /**
   * Automatically calculate normals based on the current mesh.
   */
  virtual void CalculateNormals () = 0;

  /**
   * Automatically generate a box. This will set the number of vertices
   * to eight and generate vertices, texels, and triangles. The colors
   * and normals are not initialized here.
   */
  virtual void GenerateBox (const csBox3& box) = 0;
};

#endif

