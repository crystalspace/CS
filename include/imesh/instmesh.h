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

#ifndef __CS_IMESH_INSTMESH_H__
#define __CS_IMESH_INSTMESH_H__

/**\file
 * Instancing mesh object
 */ 

#include "csutil/scf.h"

struct iDocumentNode;
struct iMaterialWrapper;
struct iRenderBuffer;
struct iMeshObject;

/**\addtogroup meshplugins
 * @{ */

class csBox3;
class csColor;
class csColor4;
struct csTriangle;
class csVector2;
class csVector3;

/**
 * The common interface between instancing meshes and factories.
 * This interface is usually not used alone. Generally one
 * uses iInstancingMeshState or iInstancingFactoryState.
 */
struct iInstancingMeshCommonState : public virtual iBase
{
  SCF_INTERFACE (iInstancingMeshCommonState, 1, 0, 0);
  
  /// Set lighting.
  virtual void SetLighting (bool l) = 0;
  /// Is lighting enabled.
  virtual bool IsLighting () const = 0;
  /**
   * Set manual colors. If this is set then lighting will be ignored
   * and so will the color set with SetColor(). In this case you can
   * manipulate the color array manually by calling GetColors().
   */
  virtual void SetManualColors (bool m) = 0;
  /// Are manual colors enabled?
  virtual bool IsManualColors () const = 0;
  /**
   * Set shadowing. By default instancing objects will cast shadows
   * (during the static lighting phase). You can disable this here.
   */
  virtual void SetShadowCasting (bool m) = 0;
  /// Is shadow casting enabled?
  virtual bool IsShadowCasting () const = 0;
  /**
   * Set shadow receiving on. By default this is disabled in which
   * case the instancing object will receive all lighting information
   * dynamically but without shadows. If this is enabled then
   * the lighting system resembles more the lighting system with
   * things which static and pseudo-dynamic lighting. In this
   * case there will be shadows on the instancing instance.
   */
  virtual void SetShadowReceiving (bool m) = 0;
  /// Is shadow receiving enabled?
  virtual bool IsShadowReceiving () const = 0;
};

/**
 * This interface describes the API for the instancing mesh object.
 * 
 * Main creators of instances implementing this interface:
 * - Instmesh mesh object plugin (crystalspace.mesh.object.instmesh)
 * - iMeshObjectFactory::NewInstance()
 *   
 * Main ways to get pointers to this interface:
 * - scfQueryInterface() on iMeshWrapper::GetMeshObject()
 *   
 * Main users of this interface:
 * - Instmesh Loader plugin (crystalspace.mesh.loader.instmesh)
 *   
 */
struct iInstancingMeshState : public virtual iInstancingMeshCommonState
{
  SCF_INTERFACE (iInstancingMeshState, 1, 0, 0);
  
  /**
   * Add an instance. Returns an ID to identify that instance.
   */
  virtual size_t AddInstance (const csReversibleTransform& trans) = 0;

  /**
   * Remove an instance.
   */
  virtual void RemoveInstance (size_t id) = 0;

  /**
   * Remove all instances.
   */
  virtual void RemoveAllInstances () = 0;

  /**
   * Move an instance.
   */
  virtual void MoveInstance (size_t id,
      const csReversibleTransform& trans) = 0;

  /**
   * Get instance transform.
   */
  virtual const csReversibleTransform& GetInstanceTransform (size_t id) = 0;
};

class csSphere;

/**
 * This interface describes the API for the instancing mesh factory.
 * iInstancingFactoryState inherits from iInstancingMeshCommonState. All methods
 * from iInstancingMeshCommonState as set on the factory will serve as defaults
 * for mesh objects that are created from this factory AFTER the
 * default value is set. So changing such a value on the factory will have
 * no effect on meshes already created. The material wrapper is an
 * exception to this rule. Setting that on the factory will have an
 * effect immediatelly on all mesh objects created from that factory
 * except for those mesh objects that have their own material set.
 * 
 * Main creators of instances implementing this interface:
 * - Instmesh mesh object plugin (crystalspace.mesh.object.instmesh)
 * - iMeshObjectType::NewFactory()
 *   
 * Main ways to get pointers to this interface:
 * - scfQueryInterface() on iMeshFactoryWrapper::GetMeshObjectFactory()
 *   
 * Main users of this interface:
 * - Instmesh Factory Loader plugin (crystalspace.mesh.loader.factory.instmesh)
 *   
 */
struct iInstancingFactoryState : public virtual iInstancingMeshCommonState
{
  SCF_INTERFACE (iInstancingFactoryState, 1, 0, 0);
  
  /// Set the color to use. Will be added to the lighting values.
  virtual void SetColor (const csColor& col) = 0;
  /// Get the color.
  virtual const csColor& GetColor () const = 0;

  /**
   * Add a vertex.
   */
  virtual void AddVertex (const csVector3& v,
      const csVector2& uv, const csVector3& normal,
      const csColor4& color) = 0;

  /// Get the number of vertices for this mesh.
  virtual size_t GetVertexCount () const = 0;
  /**
   * Get the array of vertices.
   */
  virtual const csVector3* GetVertices () = 0;
  /**
   * Get the array of texels.
   */
  virtual const csVector2* GetTexels () = 0;
  /**
   * Get the array of normals.
   */
  virtual const csVector3* GetNormals () = 0;
  /**
   * Get the array of colors.
   */
  virtual const csColor4* GetColors () = 0;

  /**
   * Add a triangle.
   */
  virtual void AddTriangle (const csTriangle& tri) = 0;

  /// Get the number of triangles for this mesh.
  virtual size_t GetTriangleCount () const = 0;
  /**
   * Get the array of triangles.
   */
  virtual const csTriangle* GetTriangles () = 0;

  /**
   * Automatically calculate normals based on the current mesh.
   * /param compress if true (default) then calculate the normals
   * based on compressed vertices.
   */
  virtual void CalculateNormals (bool compress = true) = 0;

  /**
   * Compress the vertex table. This should be called after setting
   * up the geometry.
   */
  virtual void Compress () = 0;

  /**
   * Automatically generate a qad. 
   */
  virtual void GenerateQuad (const csVector3& v1, const csVector3& v2, 
    const csVector3& v3, const csVector3& v4) = 0;

  /**
   * Automatically generate a box. This will set the number of vertices
   * to eight and generate vertices, texels, and triangles. The colors
   * and normals are not initialized here.
   */
  virtual void GenerateBox (const csBox3& box) = 0;

  /**
   * Automatically generate a sphere. This will set the apropriate number 
   * of vertices and generate vertices, texels, normals, and triangles.
   * The colors are not initialized here.
   * \param sphere Properties of the ellipsoid to generate.
   * \param rim_vertices Number of vertices on a rim.
   * \param cyl_mapping if true then use cylindrical texture mapping.
   * \param toponly if true then only generate the top half of the sphere.
   * \param reversed if true then generate the sphere so it is visible
   * from the inside.
   */
  virtual void GenerateSphere (const csEllipsoid& sphere, int rim_vertices,
      bool cyl_mapping = false,
      bool toponly = false,
      bool reversed = false) = 0;

  /**
   * Returns whether normals were autogenerated or manual.
   */
  virtual bool IsAutoNormals () const = 0;
};

/** @} */

#endif // __CS_IMESH_INSTMESH_H__

