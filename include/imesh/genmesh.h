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

#ifndef __CS_IMESH_GENMESH_H__
#define __CS_IMESH_GENMESH_H__

/**\file
 * General mesh object
 */ 

#include "csutil/scf.h"

struct iDocumentNode;
struct iGenMeshAnimationControl;
struct iGenMeshAnimationControlFactory;
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

SCF_VERSION (iGeneralMeshCommonState, 0, 0, 3);

/**
 * The common interface between genmesh meshes and factories.
 * This interface is usually not used alone. Generally one
 * uses iGeneralMeshState or iGeneralFactoryState.
 */
struct iGeneralMeshCommonState : public iBase
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
  virtual const csColor& GetColor () const = 0;
  /**
   * Set manual colors. If this is set then lighting will be ignored
   * and so will the color set with SetColor(). In this case you can
   * manipulate the color array manually by calling GetColors().
   */
  virtual void SetManualColors (bool m) = 0;
  /// Are manual colors enabled?
  virtual bool IsManualColors () const = 0;
  /**
   * Set shadowing. By default genmesh objects will cast shadows
   * (during the static lighting phase). You can disable this here.
   */
  virtual void SetShadowCasting (bool m) = 0;
  /// Is shadow casting enabled?
  virtual bool IsShadowCasting () const = 0;
  /**
   * Set shadow receiving on. By default this is disabled in which
   * case the genmesh object will receive all lighting information
   * dynamically but without shadows. If this is enabled then
   * the lighting system resembles more the lighting system with
   * things which static and pseudo-dynamic lighting. In this
   * case there will be shadows on the genmesh instance.
   */
  virtual void SetShadowReceiving (bool m) = 0;
  /// Is shadow receiving enabled?
  virtual bool IsShadowReceiving () const = 0;

  /**
   * Remove all submeshes added to this object
   */
  virtual void ClearSubMeshes () = 0;

  /**
   * Add a submesh to this object. A submesh is a subset of the mesh triangles
   * rendered with a certain material. When a mesh has one or more submeshes,
   * only submeshes are drawn and not original geometry. That means submeshes
   * should cover all original triangles to avoid holes in the mesh.
   * triangles is an array of indices into the factory triangle list
   * tricount is the number of triangles in "triangles"
   * material is a material to assign to the mesh
   * Note! Submeshes added to an instance of a genmesh will override
   * the submeshes from the factory (i.e. the submeshes of the factory will
   * be completely ignored as soon as the instance has submeshes).
   */
  virtual void AddSubMesh (unsigned int *triangles,
    int tricount,
    iMaterialWrapper *material) = 0;

  /**
   * Add a submesh to this object. A submesh is a subset of the mesh triangles
   * rendered with a certain material. When a mesh has one or more submeshes,
   * only submeshes are drawn and not original geometry. That means submeshes
   * should cover all original triangles to avoid holes in the mesh.
   * triangles is an array of indices into the factory triangle list
   * tricount is the number of triangles in "triangles"
   * material is a material to assign to the mesh
   * Note! Submeshes added to an instance of a genmesh will override
   * the submeshes from the factory (i.e. the submeshes of the factory will
   * be completely ignored as soon as the instance has submeshes).
   * This version overrides the parent mixmode.
   */
  virtual void AddSubMesh (unsigned int *triangles,
    int tricount,
    iMaterialWrapper *material, uint mixmode) = 0;

  /**
   * Adds an independently named render buffer.
   */
  virtual bool AddRenderBuffer (const char *name, iRenderBuffer* buffer) = 0;
  /**
   * Removes an independently named render buffer.
   */
  virtual bool RemoveRenderBuffer (const char *name) = 0;
};

SCF_VERSION (iGeneralMeshState, 0, 1, 0);

/**
 * This interface describes the API for the general mesh object.
 * 
 * Main creators of instances implementing this interface:
 * - Genmesh mesh object plugin (crystalspace.mesh.object.genmesh)
 * - iMeshObjectFactory::NewInstance()
 *   
 * Main ways to get pointers to this interface:
 * - scfQueryInterface() on iMeshWrapper::GetMeshObject()
 *   
 * Main users of this interface:
 * - Genmesh Loader plugin (crystalspace.mesh.loader.genmesh)
 *   
 */
struct iGeneralMeshState : public iGeneralMeshCommonState
{
  /**
   * Set the animation control to use for this mesh object.
   * See iGenMeshAnimationControl for more information.
   */
  virtual void SetAnimationControl (iGenMeshAnimationControl* anim_ctrl) = 0;

  /**
   * Get the current animation control for this object.
   */
  virtual iGenMeshAnimationControl* GetAnimationControl () const = 0;
};

SCF_VERSION (iGeneralFactoryState, 0, 3, 0);

struct csSphere;
/**
 * This interface describes the API for the general mesh factory.
 * iGeneralFactoryState inherits from iGeneralMeshState. All methods
 * from iGeneralMeshState as set on the factory will serve as defaults
 * for mesh objects that are created from this factory AFTER the
 * default value is set. So changing such a value on the factory will have
 * no effect on meshes already created. The material wrapper is an
 * exception to this rule. Setting that on the factory will have an
 * effect immediatelly on all mesh objects created from that factory
 * except for those mesh objects that have their own material set.
 * 
 * Main creators of instances implementing this interface:
 * - Genmesh mesh object plugin (crystalspace.mesh.object.genmesh)
 * - iMeshObjectType::NewFactory()
 *   
 * Main ways to get pointers to this interface:
 * - scfQueryInterface() on iMeshFactoryWrapper::GetMeshObjectFactory()
 *   
 * Main users of this interface:
 * - Genmesh Factory Loader plugin (crystalspace.mesh.loader.factory.genmesh)
 *   
 */
struct iGeneralFactoryState : public iGeneralMeshCommonState
{
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
  virtual csColor4* GetColors () = 0;

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

  /**
   * Automatically generate a sphere. This will set the apropriate number 
   * of vertices and generate vertices, texels, normals, and triangles. The colors
   * are not initialized here.
   */
  virtual void GenerateSphere (const csSphere& sphere, int rim_vertices) = 0;

  //virtual void GeneratePlane (const csPlane3& plane) = 0;

  /**
   * Enable back to front rendering for the triangles of this genmesh.
   * This is useful if this factory represents a transparent genmesh
   * or the material that is being used is itself transparent.
   */
  virtual void SetBack2Front (bool b2f) = 0;

  /**
   * Returns whether normals were autogenerated or manual.
   */
  virtual bool IsAutoNormals () const = 0;

  /**
   * Get the value of the back2front flag.
   */
  virtual bool IsBack2Front () const = 0;

  /**
   * Set the animation control factory to use for this factory.
   * See iGenMeshAnimationControlFactory for more information.
   */
  virtual void SetAnimationControlFactory (
  	iGenMeshAnimationControlFactory* anim_ctrl) = 0;

  /**
   * Get the current animation control factory for this factory.
   */
  virtual iGenMeshAnimationControlFactory* GetAnimationControlFactory ()
  	const = 0;
};

SCF_VERSION (iGenMeshAnimationControl, 0, 0, 1);

/**
 * Implementing this class allows the creation of classes that control
 * animation of vertex, texel, normal, and color data right before it is
 * being used. This can be used for various special effects. Note then when
 * animating vertex that it is prefered that the bounding box of the
 * object doesn't change too dramatically because this animation is
 * called AFTER visibility culling!
 * 
 * Main creators of instances implementing this interface:
 * - iGenMeshAnimationControlFactory::CreateAnimationControl()
 *   
 * Main ways to get pointers to this interface:
 * - iGeneralMeshState::GetAnimationControl()
 *   
 * Main users of this interface:
 * - Genmesh plugin (crystalspace.mesh.object.genmesh)
 *   
 */
struct iGenMeshAnimationControl : public iBase
{
  /// Returns true if this control animates vertices.
  virtual bool AnimatesVertices () const = 0;
  /// Returns true if this control animates texels.
  virtual bool AnimatesTexels () const = 0;
  /// Returns true if this control animates normals.
  virtual bool AnimatesNormals () const = 0;
  /// Returns true if this control animates colors.
  virtual bool AnimatesColors () const = 0;

  /**
   * Given the factory vertex data, return the animated data.
   * If this control doesn't animate vertices then it will return the
   * source array unchanged.
   * The 'version_id' is a number that changes whenever the input array
   * changes. The animation control can use this to optimize the animation
   * calculation by caching the animated version of the array and returning
   * that one.
   */
  virtual const csVector3* UpdateVertices (csTicks current,
  	const csVector3* verts, int num_verts, uint32 version_id) = 0;

  /**
   * Given the factory texel data, return the animated data.
   * If this control doesn't animate texels then it will return the
   * source array unchanged.
   * The 'version_id' is a number that changes whenever the input array
   * changes. The animation control can use this to optimize the animation
   * calculation by caching the animated version of the array and returning
   * that one.
   */
  virtual const csVector2* UpdateTexels (csTicks current,
  	const csVector2* texels, int num_texels, uint32 version_id) = 0;

  /**
   * Given the factory normal data, return the animated data.
   * If this control doesn't animate normals then it will return the
   * source array unchanged.
   * The 'version_id' is a number that changes whenever the input array
   * changes. The animation control can use this to optimize the animation
   * calculation by caching the animated version of the array and returning
   * that one.
   */
  virtual const csVector3* UpdateNormals (csTicks current,
  	const csVector3* normals, int num_normals, uint32 version_id) = 0;

  /**
   * Given the factory color data, return the animated data.
   * If this control doesn't animate colors then it will return the
   * source array unchanged.
   * The 'version_id' is a number that changes whenever the input array
   * changes. The animation control can use this to optimize the animation
   * calculation by caching the animated version of the array and returning
   * that one.
   */
  virtual const csColor4* UpdateColors (csTicks current,
  	const csColor4* colors, int num_colors, uint32 version_id) = 0;
};

SCF_VERSION (iGenMeshAnimationControlFactory, 0, 0, 1);

struct iDocumentNode;
/**
 * This class is a factory for creating animation controls.
 * 
 * Main creators of instances implementing this interface:
 * - iGenMeshAnimationControlType::CreateAnimationControlFactory()
 *   
 * Main ways to get pointers to this interface:
 * - iGeneralFactoryState::GetAnimationControlFactory()
 *   
 * Main users of this interface:
 * - Genmesh plugin (crystalspace.mesh.object.genmesh)
 *   
 */
struct iGenMeshAnimationControlFactory : public iBase
{
  /**
   * Create a new animation control.
   */
  virtual csPtr<iGenMeshAnimationControl> CreateAnimationControl (iMeshObject *mesh) = 0;

  /**
   * Setup this animation control from a document node.
   * Returns 0 on success or an error description on failure.
   */
  virtual const char* Load (iDocumentNode* node) = 0;

  /**
   * Save this animation control to a document node.
   * Returns 0 on success or an error description on failure.
   */
  virtual const char* Save (iDocumentNode* parent) = 0;
};

SCF_VERSION (iGenMeshAnimationControlType, 0, 0, 1);

/**
 * This class is the animation control type.
 * 
 * Main creators of instances implementing this interface:
 * - Genmesh animation control plugin (crystalspace.mesh.anim.genmesh)
 *   
 * Main ways to get pointers to this interface:
 * - csQueryPluginClass()
 * - csLoadPlugin()
 *   
 * Main users of this interface:
 * - Genmesh plugin (crystalspace.mesh.object.genmesh)
 *   
 */
struct iGenMeshAnimationControlType : public iBase
{
  /**
   * Create a new animation control factory.
   */
  virtual csPtr<iGenMeshAnimationControlFactory> CreateAnimationControlFactory
  	() = 0;

};

/** @} */

#endif // __CS_IMESH_GENMESH_H__

