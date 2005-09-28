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

/**\file
 * 3D Cal3d (skeletal animation) sprite mesh object
 */ 

#include "csutil/scf.h"
#include "imesh/sprite3d.h"

/**\addtogroup meshplugins
 * @{ */

struct iMaterialWrapper;
struct iMeshObject;
struct iMeshWrapper;
struct iMeshObjectFactory;
struct iRenderView;
struct iShaderVariableContext;
struct iVFS;

class csColor;
class csRandomGen;
class csReversibleTransform;
class csString;

class CalModel;
class CalCoreModel;

SCF_VERSION (iSpriteCal3DSocket, 0, 0, 2);

/**
 * A socket for specifying where sprites can plug into other sprites.
 */
struct iSpriteCal3DSocket : public iSpriteSocket
{
  /// Set the index of the submesh for the socket.
  virtual void SetSubmeshIndex (int subm_index) = 0;
  /// Get the index of the submesh for the socket.
  virtual int GetSubmeshIndex () const = 0;

  /// Set the index of the mesh for the socket.
  virtual void SetMeshIndex (int m_index) = 0;
  /// Get the index of the mesh for the socket.
  virtual int GetMeshIndex () const = 0;

  /// Set the transform of the main mesh
  virtual void SetTransform (const csReversibleTransform & trans) = 0;
  /// Get the transform of the main mesh
  virtual csReversibleTransform GetTransform () const = 0;

  /**
   * Get a count of the secondary attached meshes (this doesn't include the
   * primary mesh)
   */
  virtual size_t GetSecondaryCount () const = 0;
  /// Get the attached secondary mesh at the given index
  virtual iMeshWrapper * GetSecondaryMesh (size_t index) = 0;
  /// Get the transform of the attached secondary mesh at the given index
  virtual csReversibleTransform GetSecondaryTransform (size_t index) = 0;
  /// Set the transform of the attached secondary mesh at the given index
  virtual void SetSecondaryTransform (size_t index, csReversibleTransform) = 0;
  /// Attach a secondary mesh
  virtual size_t AttachSecondary (iMeshWrapper*, csReversibleTransform) = 0;
  /// Detach a secondary mesh by name
  virtual void DetachSecondary (const char* mesh_name) = 0;
  /// Detach a secondary mesh by index
  virtual void DetachSecondary (size_t index) = 0;
  /// Finds the index of the given attached secondary mesh
  virtual size_t FindSecondary (const char* mesh_name) = 0;
};


SCF_VERSION (iSpriteCal3DFactoryState, 0, 0, 3);
struct CalAnimationCallback;

/**
 * This interface describes the API for the 3D sprite factory mesh object.
 */
struct iSpriteCal3DFactoryState : public iBase
{
  /// Initialize internal Cal3d data structures.
  virtual bool Create(const char *name) = 0;

  /**
   * This prints the message if any cal3d function is unsuccessful.
   * There is no way I can see to retrieve the string and use cs report 
   * with it.
   */
  virtual void ReportLastError () = 0;

  /**
   * This function sets the flags, which the factory sets in CalLoader
   * when loading new models.
   */
  virtual void SetLoadFlags(int flags) = 0;

  /**
   * This sets the path to which other filenames will be appended before
   * loading.
   */
  virtual void SetBasePath(const char *path) = 0;

  /**
   * This loads the supplied file as the skeleton data for the sprite.
   */
  virtual bool LoadCoreSkeleton(iVFS *vfs,const char *filename) = 0;

  /**
   * This function resizes all instances of this factory permanently.
   * Factor=1 means no change.
   */
  virtual void RescaleFactory(float factor) = 0;

  /**
   * This loads the supplied file as one animation action for the sprite.
   * \param vfs The ref to the vfs plugin used when loading the anim file
   * \param filename The VFS path to the anim file.
   * \param name The animation's name.
   * \param type The type of anim this file represents.
   * \param base_velocity On movement type anims, this represents the native
   *   traversal speed of the model implied by this animation.
   *   For example, a "walk" anim might specify 2m/sec.
   * \param min_velocity  On movement type anims, this represents the minimum
   *   velocity for which this animation should be considered
   *   or used.  The anim will be blended with other anims to achieve the
   *   desired exact velocity.
   * \param max_velocity  Same thing for max velocity for this anim to be
   *   blended in.
   * \param min_interval  When the anim of type "idle" is playing, the model
   *   will randomly choose override actions to play every so often to enhance
   *   the realism of the idle.  (Thus a "standing" creature might shift his
   *   feet or scratch his nose every 30 seconds or so.)  This param is the
   *   minimum time between these overrides.
   * \param max_interval  Max interval between these override idle actions.
   *   The model will randomly choose a time between min and max.
   * \param idle_pct For anims of type action, if the model is idling it will
   *   randomly choose among these based on the idle_pct weights specified
   *   here.  This param should total 100 across all anims for the model if
   *   used.
   * \param lock This specifies whether the animation is to be locked on last
   *   frame or not. If not locked, the action will return to the base keyframe
   *   when complete.  If locked, the action will stay in the final keyframe
   *   position until cleared.  (This is usually for anims like "death".)
   */
  virtual int LoadCoreAnimation(
	iVFS *vfs,
	const char *filename,
	const char *name,
	int type,
	float base_velocity,
	float min_velocity,
	float max_velocity,
        int min_interval,
        int max_interval,
        int idle_pct,
        bool lock) = 0;

  /**
   * This loads a submesh which will attach to this skeleton.
   * filename is the native system filename of the mesh file.
   * name is the logical name which will be used by the mesh object to
   * attach and detach this mesh.
   * attach should be true if this mesh should be part of the mesh object
   * after it is first created, or false if it will be optionally added
   * later.
   * defmat is the material which should be used when the object is created,
   * if any.
   */
  virtual int LoadCoreMesh(iVFS *vfs,const char *filename,
  	const char *name,bool attach,iMaterialWrapper *defmat) = 0;

  /**
   * This adds a mesh as a morph target of another mesh.
   *
   * \param vfs The VFS object where `filename' resides.
   * \param mesh_index The index of the mesh we are going to add a morph
   *   target to.
   * \param filename The name of the file of the mesh of the morph tarrget.
   * \param name The name of the morph target.
   *
   * \return The index of the morph target.
   */
  virtual int LoadCoreMorphTarget(iVFS *vfs, int mesh_index,
  	const char *filename, const char *name) = 0;

  /**
   * This adds a new morph animation.
   *
   * \param name The name of morph animation.
   * \return The index of the morph animation.
   */
  virtual int AddMorphAnimation(const char *name) = 0;
  
  /**
   * This adds a mesh and one of its morph target to the given morph animation.
   *
   * \param morphanimation_index The index of the morph animation.
   * \param mesh_name The name of the mesh.
   * \param morphtarget_name The name of the morph target of the mesh.
   *
   * \return True if successfull.
   */
  virtual bool AddMorphTarget(int morphanimation_index,
		              const char *mesh_name,
			      const char *morphtarget_name) = 0;
  
  /**
   * This jams a CS material into a cal3d material struct.
   * Don't try this at home!
   */
  virtual bool AddCoreMaterial(iMaterialWrapper *mat) = 0;

  /// Does internal cal3d prep work for doing fast bbox calcs later.
  virtual void CalculateAllBoneBoundingBoxes() = 0;

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
   * \param mesh_id The id of the mesh.
   *
   * \return The number of morph targets of a mesh.
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
   * Returns the name of the default material that should go on this mesh.
   */
  virtual const char* GetDefaultMaterial( const char* meshName ) = 0;
  
  
  /**
   * Returns the xml name of the morph animation at a certain index in the
   * array.
   */
  virtual const char *GetMorphAnimationName(int idx) = 0;

  /**
   * Returns the index of the specified morph animation name,
   * or -1 if not found.
   */
  virtual int  FindMorphAnimationName(const char *meshName) = 0;

  /**
   * Returns whether the mesh is a default mesh or not.
   */
  virtual bool IsMeshDefault(int idx) = 0;

  /// Create and add a new socket to the sprite.
  virtual iSpriteCal3DSocket* AddSocket () = 0;
  /// find a named socket into the sprite.
  virtual iSpriteCal3DSocket* FindSocket (const char * name) const = 0;
  /// find a socked based on the sprite attached to it.
  virtual iSpriteCal3DSocket* FindSocket (iMeshWrapper *mesh) const = 0;
  /// Query the number of sockets.
  virtual int GetSocketCount () const = 0;
  /// Query the socket number f.
  virtual iSpriteCal3DSocket* GetSocket (int f) const = 0;

  /** This gives you access to the internal Cal3d Core Model class
   *  which sprcal3dfact wraps.  If you use it directly, you run
   *  the risk of making sprcal3dfact and CalCoreModel get out of sync.
   *  Use carefully!
   */
  virtual CalCoreModel *GetCal3DCoreModel() = 0;

  /**
   * This function will attach a callback to the Core Anim, to be called
   * whenever the min_interval passes and this animation is active.
   */
  virtual bool RegisterAnimCallback(const char *anim,
  	CalAnimationCallback *callback, float min_interval) = 0;

  /**
   * This function should be called to remove callbacks when the meshfact is 
   * destroyed.
   */
  virtual bool RemoveAnimCallback(const char *anim,
  	CalAnimationCallback *callback) = 0;

};

SCF_VERSION (iAnimTimeUpdateHandler, 0, 0, 1); 

/**
 * By default, csSpriteCal3DMeshObject::Advance() updates the model's via
 * CalModel::update() with the elapsed time since the last advancement.  If
 * this simplistic approach is insufficient for your case, you can override the
 * default behavior by providing your own implementation of the
 * iAnimTimeUpdateHandler interface and registering it with
 * iSpriteCal3DState::SetAnimTimeUpdateHandler().
 */
struct iAnimTimeUpdateHandler : public iBase
{
  /**
   * Given the elapsed time, update the position of the model. By default,
   * csSpriteCal3DMeshObject::Advance() updates the model's via
   * CalModel::update(), but you can override this simplistic approach by
   * implementing your own UpdatePosition() method.
   */
  virtual void UpdatePosition (float delta, CalModel*) = 0;
};

struct csSpriteCal3DActiveAnim
{
  int index;
  float weight;
};

SCF_VERSION (iSpriteCal3DState, 1, 0, 0);

/**
 * This interface describes the API for changing the Cal3D sprite 
 * mesh object's animations playing and other current traits.
 */
struct iSpriteCal3DState : public iBase
{
  /**\name Animation management
   * @{ */

  /// List of current animation types, used for introspection mostly.
  enum
  {
    C3D_ANIM_TYPE_NONE,
    C3D_ANIM_TYPE_IDLE,
    C3D_ANIM_TYPE_TRAVEL,
    C3D_ANIM_TYPE_CYCLE,
    C3D_ANIM_TYPE_STYLE_CYCLE,
    C3D_ANIM_TYPE_ACTION
  };

  /// Returns the number of animations currently loaded for the core model.
  virtual int GetAnimCount() = 0;

  /**
   * Returns the name, from the xml file, of the indexed anim, or 0
   * if out of bounds.
   */
  virtual const char *GetAnimName(int idx) = 0;

  /// Returns the type from the enum above, as specified in the XML.
  virtual int  GetAnimType(int idx) = 0;

  /// Find the index of the named animation. Returns -1 if not found.
  virtual int FindAnim(const char* name) = 0;

  /// This resets all currently blended animations and stops the sprite.
  virtual void ClearAllAnims() = 0;

  /**
   * This clears the active anims for this sprite and sets it to use only the
   * specified anim, where the anim is specified by name.
   */
  virtual bool SetAnimCycle(const char *name, float weight) = 0;

  /**
   * This clears the active anims for this sprite and sets it to use only the
   * specified anim, where the anim is specified by index.
   */
  virtual bool SetAnimCycle(int idx, float weight) = 0;

  /**
   * This adds the specified animation to the ones already being blended by
   * cal3d.
   * The weight value is dependent on other weights used, and is only relative.
   * The delay is the period in seconds over which the blended weight will be 
   * interpolated from 0 to "weight" value.  A cal3d anim cycle, by definition,
   * is a looping animation (see SetAnimAction for non-looping anims).
   */
  virtual bool AddAnimCycle(const char *name, float weight, float delay) = 0;

  /**
   * Uses the specified index directly to add the anim cycle.
   */
  virtual bool AddAnimCycle(int idx, float weight, float delay) = 0;

  /**
   * This removes the animation at index `idx' from the current blend set over
   * the period of time specifed by "delay" parm in seconds.
   */
  virtual bool ClearAnimCycle (int idx, float delay) = 0;

  /**
   * This removes the named animation from the current blend set over the
   * period of time specifed by "delay" parm in seconds. Returns true if the
   * named animation was found; else false.
   */
  virtual bool ClearAnimCycle (const char *name, float delay) = 0;

  /**
   * Returns the count of currently playing animation cycles.  This should
   * be used to allocate the buffer required by GetActiveAnims below.  
   */
  virtual size_t GetActiveAnimCount() = 0;

  /**
   * Fills the supplied buffer with the information to reconstruct the exact
   * animation mix currently playing in the model.  It does NOT include
   * any non-repeating actions.  Those must be handled separately, due to
   * the timing issues.
   * \param buffer Buffer receiving information about active animations. 
   * \param max_length Maximum number of entries that fit into \a buffer.
   * \return Whether the buffer was successfully filled.
   */
  virtual bool GetActiveAnims(csSpriteCal3DActiveAnim* buffer, 
    size_t max_length) = 0;

  /**
   * Uses the supplied buffer (created by GetActiveAnims) to recreate an
   * exact mix of animation cycles and weights.
   */
  virtual void SetActiveAnims(const csSpriteCal3DActiveAnim* buffer,
  	size_t anim_count) = 0;

  /**
   * This adds a non-looping animation to the blend set for the cal3d Mixer.
   * This animation will play one time overlaid on top of the other currently
   * active animations.  delayIn and delayOut allow you to fade in and fade
   * out the action for smoothness of response.
   */
  virtual bool SetAnimAction(const char *name, float delayIn,
                             float delayOut) = 0;

  /**
   * This adds a non-looping animation to the blend set for the cal3d Mixer.
   * This animation will play one time overlaid on top of the other currently
   * active animations.  delayIn and delayOut allow you to fade in and fade
   * out the action for smoothness of response.
   */
  virtual bool SetAnimAction(int idx, float delayIn,
                             float delayOut) = 0;

  /**
   * This function searches all actions specified as type TRAVEL for actions 
   * with a base velocity matching "vel", and blends them into a travel action.
   * If no animation is found with a base animation that exactly matches "vel" 
   * It selects animations with an appropriate velocity range and blends them 
   * to create the travel animation.  The calling program
   * is still responsible for actually moving the sprite.
   */
  virtual bool SetVelocity(float vel,csRandomGen *rng=0) = 0;

  /**
   * This function sets the name to use when SetVelocity(0) is called.
   */
  virtual void SetDefaultIdleAnim(const char *name) = 0;
  /** @} */

  /**\name LOD
   * @{ */
  /**
   * This function sets the Level of Detail used by the sprite.  This is used
   * to reduce the polygon count and simplify the scene for the renderer.
   */
  virtual void SetLOD(float lod) = 0;
  /** @} */

  /**\name Mesh attaching
   * @{ */
  /**
   * This attaches a mesh with the specified name (from xml) to the instance of
   * the model.  
   */
  virtual bool AttachCoreMesh(const char *meshname) = 0;

  /**
   * This detaches a mesh with the specified name (from xml) to the instance of
   * the model.
   * \remarks Note that changes made to the mesh's shader variable context will
   *  get lost.
   */
  virtual bool DetachCoreMesh(const char *meshname) = 0;

  /**
   * This attaches a mesh with the specified calCoreModel id (as returned by
   * iSpriteCal3DFactoryState::FindMeshName() to the instance of the model.  
   * It is expected this function is only called by the mesh object
   * itself under normal circumstances.  Callers should normally refer to
   * meshes by name to prevent behavior changes when xml order is updated.
   * iMatWrap is the iMaterialWrapper to be used in rendering.
   */
  virtual bool AttachCoreMesh(int mesh_id, iMaterialWrapper* iMatWrap = 0) = 0;
  /**
   * This detaches a mesh with the specified calCoreModel id to the instance of
   * the model.  It is expected this function is only called by the mesh object
   * itself under normal circumstances.  Callers should normally refer to
   * meshes by name to prevent behavior changes when xml order is updated.
   */
  virtual bool DetachCoreMesh(int mesh_id) = 0;
  /** @} */

  /**\name Morph targets
   * @{ */
  /**
   * Blends the morph target.
   *
   * \param morph_animation_id The id of the morph animation we want to blend.
   * \param weight The weight of the morph target.
   * \param delay The delay untill the full weight is reached.
   *
   * \return False if something went wrong.
   */
  virtual bool BlendMorphTarget(int morph_animation_id, float weight,
  	float delay) = 0;

  /**
   * Clears the morph target.
   *
   * \param morph_animation_id The id of the morph animation we want to clear.
   * \param delay The delay untill the morph target is cleared.
   *
   * \return False if something went wrong.
   */
  virtual bool ClearMorphTarget(int morph_animation_id, float delay) = 0;
  /** @} */

  /**\name Sockets
   * @{ */
  /// find a socked based on the sprite attached to it.
  virtual iSpriteCal3DSocket* FindSocket (iMeshWrapper *mesh) const = 0;

  /// find a named socket into the sprite.
  virtual iSpriteCal3DSocket* FindSocket (const char* name) const = 0;
  /** @} */

  /// Change the material on a named submesh.  Returns true if successful.
  virtual bool SetMaterial(const char *mesh_name,iMaterialWrapper *mat) = 0;

  /**\name Time
   * @{ */
  /// Set the animation time adjustment factor.  1=normal speed.
  virtual void SetTimeFactor(float timeFactor) = 0;

  /// Return the current time factor of the model.
  virtual float GetTimeFactor() = 0;

  /// Return current animation time.
  virtual float GetAnimationTime() = 0;

  /// Return whole animation duration.
  virtual float GetAnimationDuration() = 0;

  /// Set current animation time.
  virtual void SetAnimationTime(float animationTime) = 0;

  /**
   * This gives you ability to update the internal Cal3d model directly rather
   * than relying upon the default behavior which merely invokes
   * CalModel::update().  You may need to do this, for example, when you want
   * to move Cal3d skeleton from your own code (to implement rag-doll physics,
   * for instance).
   */
  virtual void SetAnimTimeUpdateHandler(iAnimTimeUpdateHandler*) = 0;
  /** @} */

  /// Set user data in the model, for access from the callback later, mostly.
  virtual void SetUserData(void *data) = 0;
  
  /**
   * Get the shader variable context for the attached mesh identified by
   * \a name.
   * \return Shader variable context for the attached mesh \a name, 0 if the
   *   mesh is not attached.
   */
  virtual iShaderVariableContext* GetCoreMeshShaderVarContext (
    const char* meshName) = 0;

  /**\name Direct Cal3d model manipulation
   * You can get access to the internal Cal3d Model class which sprcal3d
   * wraps.  
   * \warning If you use it directly, you run the risk of making sprcal3d and
   * CalModel get out of sync.  Use carefully!
   * @{ */

  /// Gives access to the internal Cal3d Model instance
  virtual CalModel *GetCal3DModel() = 0;
  /** @} */
};

/** @} */

#endif// __CS_IMESH_SPRITECAL3D_H__
