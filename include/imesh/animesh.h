/*
  Copyright (C) 2008 by Marten Svanfeldt

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

#ifndef __CS_IMESH_ANIMESH_H__
#define __CS_IMESH_ANIMESH_H__

#include "csutil/scf_interface.h"

#include "imesh/skeleton2.h"

struct iRenderBuffer;

struct iAnimatedMeshFactory;
struct iAnimatedMeshFactorySubMesh;
struct iAnimatedMesh;
struct iAnimatedMeshSubMesh;
struct iAnimatedMeshMorphTarget;

/**\file
 * Animated mesh interface files
 */

/**
 * 
 */
struct csAnimatedMeshBoneInfluence
{
  ///
  BoneID bone;

  ///
  float influenceWeight;
};




/**\addtogroup meshplugins
 * @{ */

/**\name Animated mesh
 * @{ */

/**
 * State of animated mesh object factory
 */
struct iAnimatedMeshFactory : public virtual iBase
{
  SCF_INTERFACE(iAnimatedMeshFactory, 1, 0, 0);

  /**\name SubMesh handling
   * @{ */

  /**
   * Create a new submesh.
   * \param indices Index buffer to use for the newly created submesh.
   */
  virtual iAnimatedMeshFactorySubMesh* CreateSubMesh (iRenderBuffer* indices) = 0;

  /**
   * Get a submesh by index.
   */
  virtual iAnimatedMeshFactorySubMesh* GetSubMesh (size_t index) const = 0;

  /**
   * Get the total number of submeshes.
   */
  virtual size_t GetSubMeshCount () const = 0;

  /**
   * Remove a submesh from factory.
   */
  virtual void DeleteSubMesh (iAnimatedMeshFactorySubMesh* mesh) = 0;

  /** @} */

  /**\name Vertex data definition and handling
   * @{ */

  /**
   * Set the number of vertices in the mesh
   */
  virtual void SetVertexCount (uint count) = 0;

  /**
   * Get the number of vertices in the mesh
   */
  virtual uint GetVertexCount () const = 0;

  /**
   * Get a pointer to the buffer specifying vertices.
   * The buffer is at least as many entries as specified by the vertex count.
   * You must call Invalidate() after modifying it.
   */
  virtual iRenderBuffer* GetVertices () = 0;

  /**
   * Get a pointer to the buffer specifying vertices.
   * The buffer is at least as many entries as specified by the vertex count.
   * You must call Invalidate() after modifying it.
   */
  virtual iRenderBuffer* GetTexCoords () = 0;

  /**
   * Get a pointer to the buffer specifying vertex normals.
   * The buffer is at least as many entries as specified by the vertex count.
   * You must call Invalidate() after modifying it.
   */
  virtual iRenderBuffer* GetNormals () = 0;

  /**
   * Get a pointer to the buffer specifying vertex tangents.
   * The buffer is at least as many entries as specified by the vertex count.
   * You must call Invalidate() after modifying it.
   */
  virtual iRenderBuffer* GetTangents () = 0;

  /**
   * Get a pointer to the buffer specifying vertex binormals.
   * The buffer is at least as many entries as specified by the vertex count.
   * You must call Invalidate() after modifying it.
   */
  virtual iRenderBuffer* GetBinormals () = 0;

  /**
   * Get a pointer to the buffer specifying vertex color.
   * The buffer is at least as many entries as specified by the vertex count.
   * You must call Invalidate() after modifying it.
   */
  virtual iRenderBuffer* GetColors () = 0;

  /**
   * Update the mesh after modifying its geometry
   */
  virtual void Invalidate () = 0;

  /** @} */

  /**\name Bone interface and influence
   * @{ */

  /**
   * Set the requested number of bone influences per vertex.
   * The mesh might not support as many and/or round it, so check the real
   * amount with GetBoneInfluencesPerVertex ().
   */
  virtual void SetBoneInfluencesPerVertex (uint num) = 0;

  /**
   * Get the number of bone influences per vertex.
   */
  virtual uint GetBoneInfluencesPerVertex () const = 0;

  /**
   * Get the bone influences.
   */
  virtual csAnimatedMeshBoneInfluence* GetBoneInfluences () = 0;

  /** @} */

  /**
   * Create a new morph target
   */
  virtual iAnimatedMeshMorphTarget* CreateMorphTarget () = 0;

  /**
   * Get a specific morph target
   */
  virtual iAnimatedMeshMorphTarget* GetMorphTarget (uint target) = 0;

  /**
   * Get number of morph targets
   */
  virtual uint GetMorphTargetCount () const = 0;

  /**
   * Remove all morph targets
   */
  virtual void ClearMorphTargets () = 0;
};

/**
 * Sub mesh (part) of an animated mesh factory
 */
struct iAnimatedMeshFactorySubMesh : public virtual iBase
{
  SCF_INTERFACE(iAnimatedMeshFactorySubMesh, 1, 0, 0);

  /**
   * Get the index buffer for this submesh. Defines a triangle list.
   */
  virtual iRenderBuffer* GetIndices () const = 0;

  /**
   * Get the number of triangles 
   */
  virtual uint GetTriangleCount () const = 0;

  
};

/**
 * State and setting for an instance of an animated mesh
 */
struct iAnimatedMesh : public virtual iBase
{
  SCF_INTERFACE(iAnimatedMesh, 1, 0, 0);

  /**
   * Set the skeleton to use for this mesh.
   * The skeleton must have at least enough bones for all references made
   * to it in the vertex influences.
   * \param skeleton
   */
  virtual void SetSkeleton (iSkeleton2* skeleton) = 0;

  /**
   * Get a submesh by index.
   */
  virtual iAnimatedMeshSubMesh* GetSubMesh (size_t index) const = 0;

  /**
   * Get the total number of submeshes.
   */
  virtual size_t GetSubMeshCount () const = 0;

  /**
   * Set the weight for blending of a given morph target
   */
  virtual void SetMorphTargetWeight (uint target, float weight) = 0;

  /**
   * Get the weight for blending of a given morph target
   */
  virtual float GetMorphTargetWeight (uint target) const = 0;
};

/**
 * Sub mesh (part) of an animated mesh
 */
struct iAnimatedMeshSubMesh : public virtual iBase
{
  SCF_INTERFACE(iAnimatedMeshSubMesh, 1, 0, 0);

  /**
   * Get the factory submesh
   */
  virtual iAnimatedMeshFactorySubMesh* GetFactorySubMesh () = 0;

  /**
   * Set current rendering state for this submesh
   */
  virtual void SetRendering (bool doRender) = 0;

  /**
   * Get current rendering state for this submesh
   */
  virtual bool IsRendering () const = 0;  
};

/**
 * A morph target
 */
struct iAnimatedMeshMorphTarget : public virtual iBase
{
  SCF_INTERFACE(iAnimatedMeshMorphTarget, 1, 0, 0);

  /**
   * Set the number of vertices affected by this morph target
   */
  virtual void SetVertexCount (uint count) = 0;

  /**
   * Get the number of vertices affected by this morph target
   */
  virtual uint GetVertexCount () const = 0;

  /**
   * Get the buffer of vertex indices affects
   * Remember to call Invalidate() after changing this data.
   */
  virtual iRenderBuffer* GetIndices () = 0;

  /**
   * Get the buffer of vertex offsets
   * Remember to call Invalidate() after changing this data.
   */
  virtual iRenderBuffer* GetVertexOffsets () = 0;

  /**
   * Update target after changes to its index or vertex offsets
   */
  virtual void Invalidate () = 0;
};


/** @} */

/** @} */


#endif // __CS_IMESH_ANIMESH_H__

