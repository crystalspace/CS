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

#include "ivideo/graph3d.h"
#include "ivideo/rendermesh.h"

struct iRenderBuffer;
struct iMaterialWrapper;
struct iShaderVariableContext;

class csReversibleTransform;

/**\file
 * Animated mesh interface files
 */

namespace CS
{
namespace Mesh
{

struct iAnimatedMeshFactory;
struct iAnimatedMeshSubMeshFactory;
struct iAnimatedMesh;
struct iAnimatedMeshSubMesh;
struct iAnimatedMeshMorphTarget;

/**
 * Represent a single influence of a bone on a single vertex
 */
struct csAnimatedMeshBoneInfluence
{
  /// The id of the bone
  BoneID bone;

  /**
   * The relative influence of the bone. Does technically not have to be
   * normalized, but you usually want it to be.
   */
  float influenceWeight;
};

/**
 * Factory for sockets attached to iAnimatedMesh's
 */
struct iAnimatedMeshSocketFactory : public virtual iBase
{
public:
  SCF_INTERFACE(CS::Mesh::iAnimatedMeshSocketFactory, 2, 0, 0);

  /**
   * Get the name of the socket
   */
  virtual const char* GetName () const = 0;

  /**
   * Set the name of the socket
   */
  virtual void SetName (const char* name) = 0;
  
  /**
   * Get the 'bone to socket transform' of the socket
   */
  virtual const csReversibleTransform& GetTransform () const = 0;

  /**
   * Set the 'bone to socket transform' of the socket
   */
  virtual void SetTransform (csReversibleTransform& transform) = 0;
  
  /**
   * Get the ID of the bone associated with the socket
   */
  virtual BoneID GetBone () const = 0;

  /**
   * Set the bone associated with the socket
   */
  virtual void SetBone (BoneID boneID) = 0;

  /**
   * Get the associated animated mesh factory
   */
  virtual iAnimatedMeshFactory* GetFactory () = 0;
};

/**
 * Sockets attached to animated meshes. Sockets are designed to 
 * attach external objects to iAnimatedMesh's.
 */
struct iAnimatedMeshSocket : public virtual iBase
{
public:
  SCF_INTERFACE(CS::Mesh::iAnimatedMeshSocket, 1, 0, 0);

  /**
   * Get the name of the socket
   */
  virtual const char* GetName () const = 0;

  /**
   * Get the factory of this socket
   */
  virtual iAnimatedMeshSocketFactory* GetFactory () = 0;

  /**
   * Get the 'bone to socket transform' of the socket
   */
  virtual const csReversibleTransform& GetTransform () const = 0;

  /**
   * Set the 'bone to socket transform' of the socket
   */
  virtual void SetTransform (csReversibleTransform& tf) = 0;

  /**
   * Get the full transform of the socket, in world coordinate
   */
  virtual const csReversibleTransform GetFullTransform () const = 0;

  /**
   * Get the ID of the bone associated with the socket
   */
  virtual BoneID GetBone () const = 0;

  /**
   * Get the associated animated mesh
   */
  virtual iAnimatedMesh* GetMesh () const = 0;

  /**
   * Get the scene node associated with the socket
   */
  virtual iSceneNode* GetSceneNode () const = 0;

  /**
   * Set the scene node associated with the socket
   */
  virtual void SetSceneNode (iSceneNode* sn) = 0;
};


/**\addtogroup meshplugins
 * @{ */

/**\name Animated mesh
 * @{ */

/**
 * State of an animated mesh object factory
 */
struct iAnimatedMeshFactory : public virtual iBase
{
  SCF_INTERFACE(CS::Mesh::iAnimatedMeshFactory, 2, 2, 0);

  /**\name SubMesh handling
   * @{ */

  /**
   * Create a new submesh.
   * This creates a submesh that use the normal per-vertex bone influence mappings.
   * The newly created submesh will use all bones.
   * \param indices Index buffer to use for the newly created submesh.
   */
  virtual iAnimatedMeshSubMeshFactory* CreateSubMesh (iRenderBuffer* indices,
    const char* name, bool visible) = 0;

  /**
   * Create a new submesh.
   * This creates a submesh which have several 'triangle sets<->bone' mapping pairs.
   * Such a submesh is useful when you want to limit the number of bones per
   * batch rendered.
   * \param indices Array of index buffers to use per part
   * \param boneIndices Array of indices of bones to use for bone mappings
   */
  virtual iAnimatedMeshSubMeshFactory* CreateSubMesh (
    const csArray<iRenderBuffer*>& indices, 
    const csArray<csArray<unsigned int> >& boneIndices,
    const char* name, bool visible) = 0;

  /**
   * Get a submesh by index.
   */
  virtual iAnimatedMeshSubMeshFactory* GetSubMesh (size_t index) const = 0;

  /**
   * Find a submesh index by name, returns (size_t)-1 if not found.
   */
  virtual size_t FindSubMesh (const char* name) const = 0;

  /**
   * Get the total number of submeshes.
   */
  virtual size_t GetSubMeshCount () const = 0;

  /**
   * Remove a submesh from this factory.
   */
  virtual void DeleteSubMesh (iAnimatedMeshSubMeshFactory* mesh) = 0;

  /** @} */

  /**\name Vertex data definition and handling
   * @{ */

  /**
   * Get the number of vertices in the mesh
   */
  virtual uint GetVertexCount () const = 0;

  /**
   * Get a pointer to the buffer specifying the vertices.
   * The buffer has at least as many entries as specified by the vertex count.
   * You must call Invalidate() after modifying it.
   */
  virtual iRenderBuffer* GetVertices () = 0;

  /**
   * Set the render buffer to use for the vertices.
   * The buffer must contain at least three components per elements and its
   * length will specify the number of vertices within the mesh.
   * \returns false if the buffer doesn't follow required specifications
   */
  virtual bool SetVertices (iRenderBuffer* renderBuffer) = 0;

  /**
   * Get a pointer to the buffer specifying the texture coordinates.
   * The buffer hass at least as many entries as specified by the vertex count.
   * You must call Invalidate() after modifying it.
   */
  virtual iRenderBuffer* GetTexCoords () = 0;

  /**
   * Set the render buffer to use for the texture coordinates.
   * It must hold at least as many elements as the vertex buffer.
   * \returns false if the buffer doesn't follow required specifications
   */
  virtual bool SetTexCoords (iRenderBuffer* renderBuffer) = 0;

  /**
   * Get a pointer to the buffer specifying the vertex normals.
   * The buffer has at least as many entries as specified by the vertex count.
   * You must call Invalidate() after modifying it.
   */
  virtual iRenderBuffer* GetNormals () = 0;

  /**
   * Set the render buffer to use for the normals.   
   * It must hold at least as many elements as the vertex buffer.
   * \returns false if the buffer doesn't follow required specifications
   */
  virtual bool SetNormals (iRenderBuffer* renderBuffer) = 0;

  /**
   * Get a pointer to the buffer specifying the vertex tangents.
   * The buffer has at least as many entries as specified by the vertex count.
   * You must call Invalidate() after modifying it.
   */
  virtual iRenderBuffer* GetTangents () = 0;

  /**
   * Set the render buffer to use for the tangents.   
   * It must hold at least as many elements as the vertex buffer.
   * \returns false if the buffer doesn't follow required specifications
   */
  virtual bool SetTangents (iRenderBuffer* renderBuffer) = 0;

  /**
   * Get a pointer to the buffer specifying the vertex binormals.
   * The buffer hass at least as many entries as specified by the vertex count.
   * You must call Invalidate() after modifying it.
   */
  virtual iRenderBuffer* GetBinormals () = 0;

  /**
   * Set the render buffer to use for the binormals.   
   * It must hold at least as many elements as the vertex buffer.
   * \returns false if the buffer doesn't follow required specifications
   */
  virtual bool SetBinormals (iRenderBuffer* renderBuffer) = 0;

  /**
   * Get a pointer to the buffer specifying the vertex color.
   * The buffer hass at least as many entries as specified by the vertex count.
   * You must call Invalidate() after modifying it.
   */
  virtual iRenderBuffer* GetColors () = 0;

  /**
   * Set the render buffer to use for the vertex color.   
   * It must hold at least as many elements as the vertex buffer.
   * \returns false if the buffer doesn't follow required specifications
   */
  virtual bool SetColors (iRenderBuffer* renderBuffer) = 0;

  /**
   * Update the mesh after modifying its geometry
   */
  virtual void Invalidate () = 0;

  /** @} */

  /**\name Bone interface and influence
  * @{ */

  /**
   * Set the skeleton factory to associate with the mesh factory.
   * When a mesh is instanced it will by default get a skeleton from this
   * skeleton factory.
   */
  virtual void SetSkeletonFactory (iSkeletonFactory2* skeletonFactory) = 0;

  /**
   * Get the skeleton factory associated with the mesh factory.
   */
  virtual iSkeletonFactory2* GetSkeletonFactory () const = 0;

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
   * You must call Invalidate() after modifying it.
   */
  virtual csAnimatedMeshBoneInfluence* GetBoneInfluences () = 0;

  /** @} */

  /**\name Morph targets
   * @{ */

  /**
   * Create a new morph target.
   * \param name Identifier of the morph target. Can be 0 or non-unique, but 
   *   setting a unique name usually helps with finding a morph target later
   *   on.
   */
  virtual iAnimatedMeshMorphTarget* CreateMorphTarget (const char* name) = 0;

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

  /**
   * Find the index of the morph target with the given name (or (uint)~0 if no
   * target with that name exists).
   */
  virtual uint FindMorphTarget (const char* name) const = 0;

  /** @} */

  /**\name Socket
  * @{ */

  /**
   * Create a new socket
   * \param bone ID of the bone to connect the socket
   * \param transform Initial transform
   * \param name Name of the socket, optional
   */
  virtual void CreateSocket (BoneID bone, 
    const csReversibleTransform& transform, const char* name) = 0;

  /**
   * Get the number of sockets in this factory
   */
  virtual size_t GetSocketCount () const = 0;

  /**
   * Get a specific socket instance
   */
  virtual iAnimatedMeshSocketFactory* GetSocket (size_t index) const = 0;

 /**
  * Find the index of the socket with the given name (or (uint)~0 if no
  * socket with that name exists).
  */
  virtual uint FindSocket (const char* name) const = 0;
  /** @} */
};

/**
 * Sub mesh (part) of an animated mesh factory. It can be used to apply
 * various materials and rendering parameters on sub-parts of the animated mesh.
 */
struct iAnimatedMeshSubMeshFactory : public virtual iBase
{
  SCF_INTERFACE(CS::Mesh::iAnimatedMeshSubMeshFactory, 1, 2, 1);

  /**
   * Get the index buffer for this submesh. Defines a triangle list.
   */
  virtual iRenderBuffer* GetIndices (size_t set) = 0;

  /**
   * Get the number of index sets 
   */
  virtual uint GetIndexSetCount () const = 0;

  /**
   * Get the bone indices used by the given index set
   */
  virtual const csArray<unsigned int>& GetBoneIndices (size_t set) = 0;

  /**
   * Get the material of this submesh
   */
  virtual iMaterialWrapper* GetMaterial () const = 0;

  /**
   * Set the material of this submesh, or 0 to use default.
   */
  virtual void SetMaterial (iMaterialWrapper* material) = 0;

  /**
   * Get the name of this submesh.
   */
  virtual const char* GetName () const = 0;

  /**
   * Set whether or not the submesh has to be rendered by default.
   */
  virtual void SetRendering (bool doRender) = 0;

  /**
   * Get whether or not the submesh has to be rendered by default.
   */
  virtual bool IsRendering () const = 0;

  /**
   * Set the render priority of this submesh.
   */
  virtual void SetRenderPriority (CS::Graphics::RenderPriority rp) = 0;

  /**
   * Get the render priority of this submesh.
   */
  virtual CS::Graphics::RenderPriority GetRenderPriority () const = 0;

  /**
   * Set the Z-buf drawing mode of this submesh.
   */
  virtual void SetZBufMode (csZBufMode mode) = 0;

  /**
   * Get the Z-buf drawing mode of this submesh.
   */
  virtual csZBufMode GetZBufMode () const = 0;
};

/**
 * State and setting for an instance of an animated mesh
 */
struct iAnimatedMesh : public virtual iBase
{
  SCF_INTERFACE(CS::Mesh::iAnimatedMesh, 1, 0, 0);

  /**
   * Set the skeleton to use for this mesh.
   * The skeleton must have at least enough bones for all references made
   * to it in the vertex influences.
   * \param skeleton
   */
  virtual void SetSkeleton (iSkeleton2* skeleton) = 0;

  /**
   * Get the skeleton to use for this mesh.
   */
  virtual iSkeleton2* GetSkeleton () const = 0;

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

  /**\name Socket
  * @{ */

  /**
   * Get the number of sockets in factory
   */
  virtual size_t GetSocketCount () const = 0;

  /**
   * Get a specific socket instance
   */
  virtual iAnimatedMeshSocket* GetSocket (size_t index) const = 0;
  /** @} */
};

/**
 * Sub mesh (part) of an animated mesh. It can be used to apply
 * various materials and rendering parameters on sub-parts of the animated mesh.
 */
struct iAnimatedMeshSubMesh : public virtual iBase
{
  SCF_INTERFACE(CS::Mesh::iAnimatedMeshSubMesh, 1, 2, 0);

  /**
   * Get the factory of this submesh
   */
  virtual iAnimatedMeshSubMeshFactory* GetFactorySubMesh () = 0;

  /**
   * Set whether or not this submesh has to be rendered.
   */
  virtual void SetRendering (bool doRender) = 0;

  /**
   * Get whether or not this submesh has to be rendered.
   */
  virtual bool IsRendering () const = 0;

  /**
   * Get a shader variable context for this submesh.
   */
  virtual iShaderVariableContext* GetShaderVariableContext (size_t buffer) const = 0;

 /**
  * Get the material of this submesh.
  */
  virtual iMaterialWrapper* GetMaterial () const = 0;

 /**
  * Set the material of this submesh, or 0 to use the material of the factory.
  */
  virtual void SetMaterial (iMaterialWrapper* material) = 0;
};

/**
 * A morph target. It can be used to deform by morphing the vertices of an
 * animated mesh.
 */
struct iAnimatedMeshMorphTarget : public virtual iBase
{
  SCF_INTERFACE(CS::Mesh::iAnimatedMeshMorphTarget, 2, 0, 0);

  /**
   * Set the render buffer to use for the vertex offsets.   
   * It must hold at least as many elements as the vertex buffer of the owning
   * mesh object.
   * \returns false if the buffer doesn't follow required specifications
   */
  virtual bool SetVertexOffsets (iRenderBuffer* renderBuffer) = 0;

  /**
   * Get the buffer of the vertex offsets
   * Remember to call Invalidate() after changing this data.
   */
  virtual iRenderBuffer* GetVertexOffsets () = 0;

  /**
   * Update the morph target after some changes to its vertex offsets
   */
  virtual void Invalidate () = 0;

  /// Get the name of this morph target
  virtual const char* GetName () const = 0;
};


/** @} */

/** @} */

} // namespace Mesh
} // namespace CS

CS_DEPRECATED_METHOD_MSG("Use CS::Mesh::iAnimatedMeshSubMeshFactory instead")
typedef CS::Mesh::iAnimatedMeshSubMeshFactory iAnimatedMeshFactorySubMesh;

#endif // __CS_IMESH_ANIMESH_H__

