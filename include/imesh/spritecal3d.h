/*
    Copyright (C) 2003 by Keith Fulton

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

#ifndef __CS_IMESH_SPRITECAL3D_H__
#define __CS_IMESH_SPRITECAL3D_H__

#include "csutil/scf.h"
#include "csutil/garray.h"
#include "csutil/cscolor.h"
#include "ivideo/graph3d.h"

class csColor;
struct iMaterialWrapper;
struct iSkeleton;
struct iSkeletonState;
struct iMeshObject;
struct iMeshWrapper;
struct iMeshObjectFactory;
struct iRenderView;
struct iRenderView;



SCF_VERSION (iSpriteCal3DFactoryState, 0, 0, 3);

/**
 * This interface describes the API for the 3D sprite factory mesh object.
 */
struct iSpriteCal3DFactoryState : public iBase
{
  /// Initialize internal Cal3d data structures.
  virtual bool Create(const char *name) = 0;

  /** This prints the message if any cal3d function is unsuccessful.
   * There is no way I can see to retrieve the string and use cs report 
   * with it.
   */
  virtual void ReportLastError () = 0;

  /**
   * This sets the path to which other filenames will be appended before
   * loading.
   */
  virtual void SetBasePath(const char *path) = 0;

  /**
   * This sets the scale factor of the sprite. 1 = as-is size
   */
  virtual void SetRenderScale(float scale) = 0;

  /**
   * This loads the supplied file as the skeleton data for the sprite.
   */
  virtual bool LoadCoreSkeleton(const char *filename) = 0;

  /**
   * This loads the supplied file as one animation action for the sprite.
   */
  virtual int  LoadCoreAnimation(const char *filename,
				 const char *name,
				 int type,
				 float base_velocity,
				 float min_velocity,
				 float max_velocity) = 0;

  /**
   * This loads a submesh which will attach to this skeleton.
   * filename is the native system filename of the mesh file.
   * name is the logical name which will be used by the mesh object to
   * attach and detach this mesh.
   * attach should be true if this mesh should be part of the mesh object
   * after it is first created, or false if it will be optionally added
   * later.
   * defmat is the material which should be used when the object is created, if any.
   */
  virtual int LoadCoreMesh(const char *filename,const char *name,bool attach,iMaterialWrapper *defmat) = 0;

  /**
   * This adds a mesh as a morph target of another mesh.
   *
   * @param mesh_index The index of the mesh we are going to add a morph target to.
   * @param filename The name of the file of the mesh of the morph tarrget.
   * @param name The name of the morph target.
   *
   * @return The index of the morph target.
   */
  virtual int LoadCoreMorphTarget(int mesh_index,const char *filename,const char *name) = 0;
  
  /**
   * This adds a new morph animation.
   *
   * @param name The name of morph animation.
   *
   * @return The index of the morph animation.
   */
  virtual int AddMorphAnimation(const char *name) = 0;
  
  /**
   * This adds a mesh and one of it's morph target to the given morph animation.
   *
   * @param morphanimation_index The index of the morph animation.
   * @param mesh_index The index of the mesh.
   * @param morphtarget_index The index of the morph target of the mesh.
   *
   * @return True if successfull.
   */
  virtual bool AddMorphTarget(int morphanimation_index,
		              const char *mesh_name, const char *morphtarget_name) = 0;
  
  /**
   * This jams a CS material into a cal3d material struct.
   * Don't try this at home!
   */
  virtual bool AddCoreMaterial(iMaterialWrapper *mat) = 0;

  /**
   * Cal3d requires extra initialization once all materials are loaded.
   * The loader calls this at the appropriate time automatically.
   */
  virtual void BindMaterials() = 0;

  /**
   * Returns the count of all the meshes available for attachment
   * to the core model.
   */
  virtual int  GetMeshCount() = 0;

  /**
   * Returns the count of all Morph animations of this core model.
   */
  virtual int GetMorphAnimationCount() = 0;
  
  /**
   * Returns the number of morph targets of a mesh.
   *
   * @parm mesh_id The id of the mesh.
   *
   * @return The number of morph targets of a mesh.
   *         -1 if something went wrong.
   */
  virtual int GetMorphTargetCount(int mesh_id) = 0;

  /**
   * Returns the xml name of the mesh at a certain index in the array.
   */
  virtual const char *GetMeshName(int idx) = 0;

  /**
   * Returns the index of the specified mesh name, or -1 if not found.
   */
  virtual int  FindMeshName(const char *meshName) = 0;

  /**
   * Returns the xml name of the morph animation at a certain index in the array.
   */
  virtual const char *GetMorphAnimationName(int idx) = 0;

  /**
   * Returns the index of the specified morph animation name, or -1 if not found.
   */
  virtual int  FindMorphAnimationName(const char *meshName) = 0;

  /**
   * Returns whether the mesh is a default mesh or not.
   */
  virtual bool IsMeshDefault(int idx) = 0;
};

SCF_VERSION (iSpriteCal3DState, 0, 0, 1);


/**
 * This interface describes the API for changing the Cal3D sprite 
 * mesh object's animations playing and other current traits.
 */
struct iSpriteCal3DState : public iBase
{
  /// List of current animation types, used for introspection mostly.
  enum
  {
    C3D_ANIM_TYPE_NONE,
    C3D_ANIM_TYPE_TRAVEL,
    C3D_ANIM_TYPE_CYCLE,
    C3D_ANIM_TYPE_STYLE_CYCLE,
    C3D_ANIM_TYPE_ACTION
  };

  /// Returns the number of animations currently loaded for the core model.
  virtual int GetAnimCount() = 0;

  /// Returns the name, from the xml file, of the indexed anim, or NULL if out of bounds.
  virtual const char *GetAnimName(int idx) = 0;

  /// Returns the type from the enum above, as specified in the XML.
  virtual int  GetAnimType(int idx) = 0;

  /// This resets all currently blended animations and stops the sprite.
  virtual void ClearAllAnims() = 0;

  /// This clears the active anims for this sprite and sets it to use only the specified anim. 
  virtual bool SetAnimCycle(const char *name, float weight) = 0;

  /**
   * This adds the specified animation to the ones already being blended by cal3d.
   * The weight value is dependent on other weights used, and is only relative.
   * The delay is the period in seconds over which the blended weight will be 
   * interpolated from 0 to "weight" value.  A cal3d anim cycle, by definition,
   * is a looping animation (see SetAnimAction for non-looping anims).
   */
  virtual bool AddAnimCycle(const char *name, float weight, float delay) = 0;

  /**
   * This removes the specified anim from the current blend set over the period
   * of time specifed by "delay" parm in seconds.
   */
  virtual bool ClearAnimCycle(const char *name, float delay) = 0;

  /**
   * This adds a non-looping animation to the blend set for the cal3d Mixer.
   * This animation will play one time overlaid on top of the other currently
   * active animations.  delayIn and delayOut allow you to fade in and fade
   * out the action for smoothness of response.
   */
  virtual bool SetAnimAction(const char *name, float delayIn, float delayOut) = 0;

  /**
   * This function searches all actions specified as type TRAVEL, and uses their
   * preferred velocities to create a set of blended animations which will equate
   * in velocity to the specified parm "vel".  The calling program is still
   * responsible for actually moving the sprite.
   */
  virtual bool SetVelocity(float vel) = 0;

  /**
   * This function sets the Level of Detail used by the sprite.  This is used to 
   * reduce the polygon count and simplify the scene for the renderer.
   */
  virtual void SetLOD(float lod) = 0;

  /**
   * This attaches a mesh with the specified name (from xml) to the instance of
   * the model.  
   */
  virtual bool AttachCoreMesh(const char *meshname) = 0;

  /**
   * This attaches a mesh with the specified calCoreModel id to the instance of
   * the model.  It is expected this function is only called by the mesh object
   * itself under normal circumstances.  Callers should normally refer to meshes
   * by name to prevent behavior changes when xml order is updated.
   * iMatWrap is the iMaterialWrapper to be used in rendering.
   */
  virtual bool AttachCoreMesh(int mesh_id,int iMatWrap) = 0;

  /**
   * This detaches a mesh with the specified name (from xml) to the instance of
   * the model.
   */
  virtual bool DetachCoreMesh(const char *meshname) = 0;

  /**
   * This detaches a mesh with the specified calCoreModel id to the instance of
   * the model.  It is expected this function is only called by the mesh object
   * itself under normal circumstances.  Callers should normally refer to meshes
   * by name to prevent behavior changes when xml order is updated.
   */
  virtual bool DetachCoreMesh(int mesh_id) = 0;

  /**
   * Blends the morph target.
   *
   * @parm mesh_id The id of the morph animation we want to blend.
   * @parm weight The weight of the morph target.
   * @parm delay The delay untill the full weight is reached.
   *
   * @return False if something went wrong.
   */
  virtual bool BlendMorphTarget(int morph_animation_id, float weight, float delay) = 0;

  /**
   * Clears the morph target.
   *
   * @parm mesh_id The id of the morph animation we want to clear.
   * @parm delay The delay untill the morph target is cleared.
   *
   * @return False if something went wrong.
   */
  virtual bool ClearMorphTarget(int morph_animation_id, float delay) = 0;

};

#endif// __CS_IMESH_SPRITECAL3D_H__
