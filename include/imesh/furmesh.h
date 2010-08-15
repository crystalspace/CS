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

#ifndef __FUR_INTERF_H__
#define __FUR_INTERF_H__

#include <csutil/scf.h>

#include "crystalspace.h"

/**\file
 * Fur mesh interface files
 */

namespace CS
{
namespace Mesh
{

  struct iFurPhysicsControl;
  struct iFurMeshProperties;
  struct iFurMeshFactory;
  struct iFurMeshType;
  struct iFurMesh;

/**
 * Controller that updates the iFurMesh's geometry
 */
struct iFurPhysicsControl : public virtual iBase
{
public:
  SCF_INTERFACE (CS::Mesh::iFurPhysicsControl, 1, 0, 0);

  /**
   * Set iFurMesh's initial transform
   */
  virtual void SetInitialTransform(csReversibleTransform initialTransform) = 0;

  /**
   * Set the rigid body on which the iFurMesh is attached
   */
  virtual void SetRigidBody (iRigidBody* rigidBody) = 0;

  /**
   * Set the iDynamicSystem (optional)
   */
  virtual void SetBulletDynamicSystem (CS::Physics::Bullet::iDynamicSystem* 
    bulletDynamicSystem) = 0;

  /**
   * Initialize the fur strand with the given ID
   * \param strandID unique ID for the fur strand
   * \param coordinates C/C++ array of Vector3 representing the initial 
   * positions of the control points
   * \param coordinatesCount the number of control points 
   */
  virtual void InitializeStrand (size_t strandID, csVector3* coordinates,
    size_t coordinatesCount) = 0;

  /**
   * Animate the fur strand with the given ID
   * \param strandID unique ID for the fur strand
   * \param coordinates C/C++ array of Vector3 representing the initial 
   * positions of the control points
   * \param coordinatesCount the number of control points 
   */
  virtual void AnimateStrand (size_t strandID, csVector3* coordinates, size_t
    coordinatesCount) const = 0;

  /**
   * Remove the fur strand with the given ID
   * \param strandID unique ID for the fur strand
   */
  virtual void RemoveStrand (size_t strandID) = 0;

  /**
   * Remove all fur strands
   */
  virtual void RemoveAllStrands () = 0;
};

/**
 * Store the material used for the iFurMesh.
 * Material variables can be updated each frame via the Update function.
 */
struct iFurMeshProperties : public virtual iBase
{
public:
  SCF_INTERFACE (CS::Mesh::iFurMeshProperties, 1, 0, 0);

  /**
   * Get the material used
   */
  virtual iMaterial* GetMaterial() const = 0;

  /**
   * Set the material used. Can be created externally from an XML
   */
  virtual void SetMaterial(iMaterial* material) = 0;

  /**
   * Update the material data after modifying the material variables
   */
  virtual void Invalidate() = 0;

  /**
   * Called each frame. New material variables values can be send to the shader
   */
  virtual void Update() = 0;
};

/**\addtogroup meshplugins
 * @{ */

/**\name Fur mesh
 * @{ */

/**
 * State of a fur mesh object factory
 */
struct iFurMeshFactory : public virtual iBase
{
public:
  SCF_INTERFACE (CS::Mesh::iFurMeshFactory, 1, 0, 0);

  /**
   * Set the number of vertices in the mesh. Also allocate render buffers 
   * for vertices, texture coordinates, normals, tangents and binormals
   */  
  virtual void SetVertexCount (uint n) = 0;

  /**
   * Set the number of indices in the mesh. Also allocate indices render buffer
   */  
  virtual void SetTriangleCount (uint n) = 0;

  /**
   * Get the number of vertices in the mesh.
   */  
  virtual uint GetVertexCount() const = 0;

  /**
   * Get the number of indices in the mesh.
   */  
  virtual uint GetIndexCount() const = 0;
  
  /**
   * Get a pointer to the buffer specifying the indices.
   * The buffer has at least as many entries as specified by the index count.
   */  
  virtual iRenderBuffer* GetIndices () const = 0;

  /**
   * Set the render buffer to use for the indices.
   * The buffer must contain at least three components per elements and its
   * length will specify the number of indices within the mesh.
   * \returns false if the buffer doesn't follow required specifications
   */
  virtual bool SetIndices (iRenderBuffer* renderBuffer) = 0;

  /**
   * Get a pointer to the buffer specifying the vertices.
   * The buffer has at least as many entries as specified by the vertex count.
   */  
  virtual iRenderBuffer* GetVertices () const = 0;

  /**
   * Set the render buffer to use for the vertices.
   * The buffer must contain at least three components per elements and its
   * length will specify the number of vertices within the mesh.
   * \returns false if the buffer doesn't follow required specifications
   */
  virtual bool SetVertices (iRenderBuffer* renderBuffer) = 0;

  /**
   * Get a pointer to the buffer specifying the texture coordinates.
   * The buffer has at least as many entries as specified by the vertex count.
   */  
  virtual iRenderBuffer* GetTexCoords () const = 0;

  /**
   * Set the render buffer to use for the texture coordinates.
   * It must hold at least as many elements as the vertex buffer.
   * \returns false if the buffer doesn't follow required specifications
   */
  virtual bool SetTexCoords (iRenderBuffer* renderBuffer) = 0;

  /**
   * Get a pointer to the buffer specifying the normals.
   * The buffer has at least as many entries as specified by the vertex count.
   */  
  virtual iRenderBuffer* GetNormals () const = 0;

  /**
   * Set the render buffer to use for the normals.
   * It must hold at least as many elements as the vertex buffer.
   * \returns false if the buffer doesn't follow required specifications
   */
  virtual bool SetNormals (iRenderBuffer* renderBuffer) = 0;

  /**
   * Get a pointer to the buffer specifying the tangents.
   * The buffer has at least as many entries as specified by the vertex count.
   */  
  virtual iRenderBuffer* GetTangents () const = 0;

  /**
   * Set the render buffer to use for the tangents.
   * It must hold at least as many elements as the vertex buffer.
   * \returns false if the buffer doesn't follow required specifications
   */
  virtual bool SetTangents (iRenderBuffer* renderBuffer) = 0;

  /**
   * Get a pointer to the buffer specifying the binormals.
   * The buffer has at least as many entries as specified by the vertex count.
   */  
  virtual iRenderBuffer* GetBinormals () const = 0;

  /**
   * Set the render buffer to use for the binormals.
   * It must hold at least as many elements as the vertex buffer.
   * \returns false if the buffer doesn't follow required specifications
   */
  virtual bool SetBinormals (iRenderBuffer* renderBuffer) = 0;
};

/**
 * This plugin describes a specific type of fur mesh objects.
 * All methods are inherited from iMeshObjectType
 */
struct iFurMeshType : public virtual iMeshObjectType
{
public:
  SCF_INTERFACE (CS::Mesh::iFurMeshType, 1, 0, 0);
};

/**
 * State and setting for an instance of a fur mesh
 */
struct iFurMesh : public virtual iBase  
{
  SCF_INTERFACE (CS::Mesh::iFurMesh, 1, 0, 0);

  /**
   * Generates the geometry for the current instance of a fur mesh.
   * The associated iFurMeshFactory is created by this function
   */
  virtual void GenerateGeometry (iView* view, iSector* room) = 0;

  /**
   * Set the LOD for the guide fur. 
   * Pure guide fur is updated via the associated iFurPhysicsControl
   */
  virtual void SetGuideLOD(float guideLOD) = 0; 

  /**
   * Set the LOD for the fur strands. 
   * Fur strands are the rendered geometry.
   */
  virtual void SetStrandLOD(float strandLOD) = 0;

  /**
   * Set the overall LOD. Equivalent to calling SetGuidLOD and SetStrandLOD 
   * with the same parameter.
   */
  virtual void SetLOD(float lod) = 0;

  /**
   * Set the associated iFurPhysicsControl
   */
  virtual void SetPhysicsControl (iFurPhysicsControl* physicsControl) = 0;
  
  /**
   * Start the associated iFurPhysicsControl. 
   * Pure guide furs will be synchronized with the iFurPhysicsControl every frame
   */
  virtual void StartPhysicsControl ( ) = 0;

  /**
   * Stop the associated iFurPhysicsControl. 
   * Pure guide furs will stop being synchronized with the iFurPhysicsControl
   */
  virtual void StopPhysicsControl ( ) = 0;
 
  /**
   * Set the associated iFurMeshProperties
   */
  virtual void SetFurStrandGenerator( iFurMeshProperties* furMeshProperties) = 0;

  /**
   * Get the associated iFurMeshProperties.
   * Shader variables can be obtained via the material of the iFurMeshProperties
   */
  virtual iFurMeshProperties* GetFurStrandGenerator( ) const = 0;

  /**
   * Set an iAnimatedMeshFactory corresponding to the iAnimatedMeshSubMeshFactory
   */
  virtual void SetMeshFactory ( CS::Mesh::iAnimatedMeshFactory* meshFactory ) = 0;

  /**
   * Set iAnimatedMeshSubMeshFactory on which fur will grow
   */
  virtual void SetMeshFactorySubMesh ( CS::Mesh::iAnimatedMeshSubMeshFactory* 
    meshFactorySubMesh ) = 0;

  /**
   * Set the material used for the iAnimatedMeshSubMeshFactory.
   * This material should specify data such as height map or density map for 
   * the instance of iFurMesh.
   */
  virtual void SetBaseMaterial ( iMaterial* baseMaterial ) = 0;
};


/** @} */

/** @} */

} // namespace Mesh
} // namespace CS

#endif // __FUR_INTERF_H__
