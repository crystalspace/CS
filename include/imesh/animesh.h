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
 * Animated mesh interface file
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
struct AnimatedMeshBoneInfluence
{
  /// The id of the bone
  CS::Animation::BoneID bone;

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
  virtual CS::Animation::BoneID GetBone () const = 0;

  /**
   * Set the bone associated with the socket
   */
  virtual void SetBone (CS::Animation::BoneID boneID) = 0;

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
   * Get the name of this socket
   */
  virtual const char* GetName () const = 0;

  /**
   * Get the factory of this socket
   */
  virtual iAnimatedMeshSocketFactory* GetFactory () = 0;

  /**
   * Get the 'bone to socket transform' of this socket
   */
  virtual const csReversibleTransform& GetTransform () const = 0;

  /**
   * Set the 'bone to socket transform' of this socket
   */
  virtual void SetTransform (csReversibleTransform& tf) = 0;

  /**
   * Get the full transform of this socket, in world coordinate
   */
  virtual const csReversibleTransform GetFullTransform () const = 0;

  /**
   * Get the ID of the bone associated with this socket
   */
  virtual CS::Animation::BoneID GetBone () const = 0;

  /**
   * Get the associated animated mesh
   */
  virtual iAnimatedMesh* GetMesh () const = 0;

  /**
   * Get the scene node associated with this socket
   */
  virtual iSceneNode* GetSceneNode () const = 0;

  /**
   * Set the scene node associated with this socket
   */
  virtual void SetSceneNode (iSceneNode* sn) = 0;
};


/**\addtogroup meshplugins
 * @{ */

/**\name Animated mesh
 * @{ */

/**
 * State of an animated mesh object factory.
 *
 * These meshes are animated by the skeletal animation system (see
 * CS::Animation::iSkeletonFactory) and by morphing (see CS::Mesh::iAnimatedMeshMorphTarget).
 * 
 * To improve the morphing process, mesh factories are segmented into subsets. All
 * vertices of a subset are influenced by the same morph targets (i.e. the offsets
 * corresponding to these vertices in the morph targets are non-zero). All null 
 * entries of a morph target are removed from the offset buffer. Thus, segmentation 
 * into subsets improves memory usage and computational resources since morph targets 
 * are only applied to a vertex when they contain deformations. Subset with index 0
 * regroups the vertices which are not influenced by any morph target and
 * consequently will never be morphed.
 */
struct iAnimatedMeshFactory : public virtual iBase
{
  SCF_INTERFACE(CS::Mesh::iAnimatedMeshFactory, 2, 2, 3);

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
   * Update the mesh after modifying its geometry.
   * It generates automatically a segmentation of the mesh and
   * morph targets into subsets to optimize the morphing process.
   *
   * \warning Invalidate() must be called once all morph targets
   *   have been created on the animated mesh factory.
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
  virtual void SetSkeletonFactory (CS::Animation::iSkeletonFactory* skeletonFactory) = 0;

  /**
   * Get the skeleton factory associated with the mesh factory.
   */
  virtual CS::Animation::iSkeletonFactory* GetSkeletonFactory () const = 0;

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
  virtual AnimatedMeshBoneInfluence* GetBoneInfluences () = 0;

  /** @} */

  /**\name Morph targets
   * @{ */

  /**
   * Create a new morph target.
   * \param name Identifier of the morph target. Can be 0 or non-unique, but 
   *   setting a unique name usually helps with finding a morph target later
   *   on.
   *
   * \warning You must call Invalidate() once all morph targets
   *   are created on the animated mesh factory.
   */
  virtual iAnimatedMeshMorphTarget* CreateMorphTarget (const char* name) = 0;

  /**
   * Get a specific morph target.
   */
  virtual iAnimatedMeshMorphTarget* GetMorphTarget (uint target) = 0;

  /**
   * Get number of morph targets.
   */
  virtual uint GetMorphTargetCount () const = 0;

  /**
   * Remove all morph targets.
   * You must call ClearSubsets() after clearing the morph targets.
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
  virtual void CreateSocket (CS::Animation::BoneID bone, 
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

  /**\name Vertex data definition and handling
   * @{ */
  /**
   * Compute the tangents and binormals from the current vertices, normals and texels.
   * The current content of the tangent and binormal buffers will be overwritten.
   */
  virtual void ComputeTangents () = 0;

  /** @} */

  /**\name Bounding boxes
  * @{ */

  /**
   * Set the bounding box of the given bone, in bone space. Bone bounding boxes 
   * are used to update the global bounding box of the animated mesh factory and 
   * to speed up hitbeam tests. If you don't specify a bounding box for a bone,
   * a bounding box is automatically generated: it includes all the mesh vertices 
   * which have a non zero weight for this bone.
   * You must call Invalidate() after modifying it.
   * \param bone The ID of the bone.
   * \param box The bounding box of the given bone, in bone space.
   */
  virtual void SetBoneBoundingBox (CS::Animation::BoneID bone, const csBox3& box) = 0; 

  /**
   * Get the bounding box of the bone with the given ID, in bone space.
   */
  virtual const csBox3& GetBoneBoundingBox (CS::Animation::BoneID bone) const = 0; 

  /** @} */

  /**\name Subsets
  * @{ */

  /**
   * Create a new user-defined subset and return its index.
   * The first subset (with index 0) regroups the vertices of the mesh object 
   * which are not influenced by any morph target, e.i. all corresponding 
   * offsets are null.
   */
  virtual size_t AddSubset () = 0;

  /**
   * Add a vertex to a subset. 
   * All vertices of a subset must be influenced by the same morph targets:
   * their corresponding offsets in these morph targets are non-zero.
   * \param subset The index of the subset
   * \param vertexIndex The index of the vertex to be added
   */
  virtual void AddSubsetVertex (const size_t subset, const size_t vertexIndex) = 0;

  /**
   * Get the index of a vertex in a specified subset.
   * \param subset The index of the subset
   * \param vertexIndex The index of the subset vertex
   * \returns the index of this vertex in the vertex buffer
   */
  virtual size_t GetSubsetVertex (const size_t subset, const size_t vertexIndex) const = 0;

  /**
   * Get the number of vertices belonging to a subset.
   */
  virtual size_t GetSubsetVertexCount (const size_t subset) const = 0;

  /**
   * Get the number of subsets associated with this factory 
   */
  virtual size_t GetSubsetCount () const = 0;

  /**
   * Remove all subsets from this factory and rebuild the 
   * original (unoptimized) morph targets.
   * You must call Invalidate() after clearing the subsets.
   */
  virtual void ClearSubsets () = 0;

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
 *
 * These meshes are animated by the skeletal animation system (see
 * CS::Animation::iSkeleton) and by morphing (see CS::Mesh::iAnimatedMeshMorphTarget).
 */
struct iAnimatedMesh : public virtual iBase
{
  SCF_INTERFACE(CS::Mesh::iAnimatedMesh, 1, 0, 3);

  /**
   * Set the skeleton to use for this mesh.
   * The skeleton must have at least enough bones for all references made
   * to it in the vertex influences.
   * \param skeleton
   */
  virtual void SetSkeleton (CS::Animation::iSkeleton* skeleton) = 0;

  /**
   * Get the skeleton to use for this mesh.
   */
  virtual CS::Animation::iSkeleton* GetSkeleton () const = 0;

  /**
   * Get a submesh by index.
   */
  virtual iAnimatedMeshSubMesh* GetSubMesh (size_t index) const = 0;

  /**
   * Get the total number of submeshes.
   */
  virtual size_t GetSubMeshCount () const = 0;

  /**
   * Set the weight for the blending of a given morph target
   */
  virtual void SetMorphTargetWeight (uint target, float weight) = 0;

  /**
   * Get the weight for the blending of a given morph target
   */
  virtual float GetMorphTargetWeight (uint target) const = 0;

  /**\name Socket
  * @{ */

  /**
   * Get the number of sockets in the factory
   */
  virtual size_t GetSocketCount () const = 0;

  /**
   * Get a specific socket instance
   */
  virtual iAnimatedMeshSocket* GetSocket (size_t index) const = 0;
  /** @} */

  /**
   * Convenient accessor method for the CS::Mesh::iAnimatedMeshFactory of this animesh.
   */
  virtual iAnimatedMeshFactory* GetAnimatedMeshFactory () const = 0;

  /**
   * Get the render buffer accessor of this mesh
   */
  virtual iRenderBufferAccessor* GetRenderBufferAccessor () const = 0;

  /**
   * Set the bounding box of the given bone, in bone space. Bone bounding boxes 
   * are used to update the global bounding box of the animated mesh and 
   * to speed up HitBeam tests. They should cover all vertices belonging to
   * the bone, even when the morph targets are active.
   *
   * If you don't specify a bounding box for a bone, then it will be generated
   * automatically but may not be optimized nor correct when the morph targets
   * are active.
   *
   * \param bone The ID of the bone.
   * \param box The bounding box of the given bone, in bone space.
   */
  virtual void SetBoneBoundingBox (CS::Animation::BoneID bone, const csBox3& box) = 0; 

  /**
   * Get the bounding box of the bone with the given ID, in bone space.
   */
  virtual const csBox3& GetBoneBoundingBox (CS::Animation::BoneID bone) const = 0; 

  /**
   * Unset the custom bounding box of this animated mesh. It will now be again
   * computed and updated automatically.
   * \sa iObjectModel::SetObjectBoundingBox()
   */
  virtual void UnsetObjectBoundingBox () = 0;

  /**
   * Clear the weight of all active morph targets
   */
  virtual void ClearMorphTargetWeights () = 0;
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
  SCF_INTERFACE(CS::Mesh::iAnimatedMeshMorphTarget, 2, 0, 1);

  /**
   * Set the render buffer to use for the vertex offsets.   
   * If no subset is defined on the owning mesh object, the buffer 
   * must hold as many elements as the vertex buffer of this mesh 
   * object.
   * Remember to call Invalidate() after changing this data.
   */
  virtual bool SetVertexOffsets (iRenderBuffer* renderBuffer) = 0;

  /**
   * Get the buffer of the vertex offsets.
   * Remember to call Invalidate() after changing this data.
   */
  virtual iRenderBuffer* GetVertexOffsets () = 0;

  /**
   * Update the morph target after some changes to its vertex offsets.
   */
  virtual void Invalidate () = 0;

  /**
   * Get the name of this morph target.
   */
  virtual const char* GetName () const = 0;

  /**
   * Add a subset to this morph target.
   * The morph target must have non zero offsets for all the
   * vertices of this subset.
   */
  virtual void AddSubset (const size_t subset) = 0;

  /**
   * Get a subset associated with this morph target.
   */
  virtual size_t GetSubset (const size_t index) const = 0;

  /**
   * Get the number of subsets associated with this morph target.
   */
  virtual size_t GetSubsetCount () const = 0;
};


/** @} */

/** @} */

} // namespace Mesh
} // namespace CS

CS_DEPRECATED_METHOD_MSG("Use CS::Mesh::AnimatedMeshBoneInfluence instead")
typedef CS::Mesh::AnimatedMeshBoneInfluence csAnimatedMeshBoneInfluence;
CS_DEPRECATED_METHOD_MSG("Use CS::Mesh::iAnimatedMesh instead")
typedef CS::Mesh::iAnimatedMesh iAnimatedMesh;
CS_DEPRECATED_METHOD_MSG("Use CS::Mesh::iAnimatedMeshFactory instead")
typedef CS::Mesh::iAnimatedMeshFactory iAnimatedMeshFactory;
CS_DEPRECATED_METHOD_MSG("Use CS::Mesh::iAnimatedMeshSocketFactory instead")
typedef CS::Mesh::iAnimatedMeshSocketFactory iAnimatedMeshSocketFactory;
CS_DEPRECATED_METHOD_MSG("Use CS::Mesh::iAnimatedMeshSubMesh instead")
typedef CS::Mesh::iAnimatedMeshSubMesh iAnimatedMeshSubMesh;
CS_DEPRECATED_METHOD_MSG("Use CS::Mesh::iAnimatedMeshSubMeshFactory instead")
typedef CS::Mesh::iAnimatedMeshSubMeshFactory iAnimatedMeshFactorySubMesh;
CS_DEPRECATED_METHOD_MSG("Use CS::Mesh::iAnimatedMeshMorphTarget instead")
typedef CS::Mesh::iAnimatedMeshMorphTarget iAnimatedMeshMorphTarget;

#endif // __CS_IMESH_ANIMESH_H__
