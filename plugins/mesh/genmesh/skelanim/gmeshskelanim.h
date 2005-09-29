/*
    Copyright (C) 2004 by Jorrit Tyberghein
                          Hristo Hristov
                          Boyan Hristov
                          Vladimir Ivanov
                          Simeon Ivanov

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

#ifndef __CS_GENMESHSKELANIM_H__
#define __CS_GENMESHSKELANIM_H__

#include "csgeom/quaterni.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"
#include "csutil/array.h"
#include "csutil/cscolor.h"
#include "csutil/csstring.h"
#include "csutil/flags.h"
#include "csutil/hash.h"
#include "csutil/parray.h"
#include "csutil/refarr.h"
#include "csutil/strhash.h"
#include "csutil/stringarray.h"
#include "imesh/genmesh.h"
#include "imesh/gmeshskel.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/virtclk.h"
#include "imesh/object.h"
#include "ivaria/dynamics.h"

class csGenmeshSkelAnimationControl;
class csGenmeshSkelAnimationControlFactory;
class csGenmeshSkelAnimationControlType;
class csSkelAnimControlRunnable;
struct iEvent;

#define SKEL_ANIMATION_ALWAYS_UPDATE       0x00000001
#define SKEL_ANIMATION_ALWAYS_UPDATE_BONES     0x00000002
#define SKEL_ANIMATION_ALWAYS_UPDATE_VERTICES   0x00000004

/**
 * Vertex information for a bone.
 */
struct sac_vertex_data
{
  int idx;
  float weight;
  float col_weight;
  csVector3 pos;
};

/**
 * Bone information for a vertex.
 */
struct sac_bone_data
{
  int idx;
  int v_idx;
};

/**
 * This is a bone of vertices. Every bone has
 * a set of vertices (with weights), children, and a transform
 * relative to a parent. Script operations operate on bones of vertices.
 */
class csSkelBone : public iGenMeshSkeletonBone
{
private:
  char* name;
  csArray<sac_vertex_data> vertices;
  csSkelBone* parent;
  csRefArray<csSkelBone> bones;
  csReversibleTransform next_transform;
  csReversibleTransform transform;
  csReversibleTransform full_transform;
  csReversibleTransform offset_body_transform;
  csRef<iGenMeshSkeletonBoneUpdateCallback> cb;
  csBoneTransformMode bone_mode;
  iRigidBody *rigid_body;
  csGenmeshSkelAnimationControl *anim_control;
  csQuaternion rot_quat;

public:
  csReversibleTransform & GetOffsetTransform() { return offset_body_transform; }
  csQuaternion & GetQuaternion () { return rot_quat; };
  void CopyFrom (csSkelBone *other);
  void AddVertex (int idx, float weight, float col_weight);
  csArray<sac_vertex_data>& GetVertexData () { return vertices; }
  void AddBone (csRef<csSkelBone> & bone) { bones.Push (bone); }
  csRefArray<csSkelBone>& GetBones () { return bones; }

  void SetParent (csSkelBone* p) { parent = p; }
  void UpdateBones ();
  void UpdateBones (csSkelBone* parent_bone);

  void UpdateRotation();
  void UpdatePosition();
  void FireCallback() 
  { if (cb) cb->UpdateTransform(this, next_transform); }

  //------------------------------------------------------------------------

  csSkelBone (csGenmeshSkelAnimationControl *animation_control);
  virtual ~csSkelBone ();

  SCF_DECLARE_IBASE;
  virtual const char* GetName () const { return name; }
  virtual void SetName (const char* name) {csSkelBone::name = csStrNew (name); }
  virtual csReversibleTransform &GetTransform () { return transform; }
  virtual void SetTransform (const csReversibleTransform &transform) 
  {
    csSkelBone::transform = transform;
    rot_quat = csQuaternion(transform.GetO2T());
  }
  virtual csReversibleTransform &GetFullTransform () { return full_transform; }
  virtual iGenMeshSkeletonBone* GetParent () { return parent; }
  virtual void GetSkinBox (csBox3 &box, csVector3 &center);
  virtual void SetMode (csBoneTransformMode mode);
  virtual csBoneTransformMode GetMode () { return bone_mode; }
  virtual void SetRigidBody (iRigidBody *r_body,
  	csReversibleTransform & offset_transform) 
  {
    rigid_body = r_body;
    offset_body_transform = offset_transform;
  }
  virtual iRigidBody *GetRigidBody () { return rigid_body; }
  virtual int GetChildrenCount () { return (int)bones.Length () ;}
  virtual iGenMeshSkeletonBone *GetChild (int i) { return bones[i]; }
  virtual iGenMeshSkeletonBone *FindChild (const char *name);
  virtual void SetUpdateCallback (iGenMeshSkeletonBoneUpdateCallback *callback) 
  { cb = callback; }
  virtual iGenMeshSkeletonBoneUpdateCallback *GetUpdateCallback () 
  { return cb; };
};

class csSkelBoneDefaultUpdateCallback :
	public iGenMeshSkeletonBoneUpdateCallback
{
public:
  SCF_DECLARE_IBASE;
  csSkelBoneDefaultUpdateCallback()
  {
    SCF_CONSTRUCT_IBASE(0);
  }
  
  virtual ~csSkelBoneDefaultUpdateCallback()
  {
    SCF_DESTRUCT_IBASE();
  }

  virtual void UpdateTransform(iGenMeshSkeletonBone *bone,
  	const csReversibleTransform & transform)
  {
    bone->GetTransform().SetO2T(transform.GetO2T());
    bone->GetTransform().SetOrigin(transform.GetOrigin());
  }
};

/**
 * Transform modes for every bone
 */
enum sac_transform_mode
{
  AC_TRANSFORM_ABSOLUTE = 0,
  AC_TRANSFORM_RELATIVE
};

/**
 * Possible opcodes for instructions in a script.
 */
enum sac_opcode
{
  AC_MOVE = 0,
  AC_ROT
};

/**
 * A script instruction with the parameter data for that instruction.
 * Every script has an array of these.
 */
struct sac_instruction
{
  sac_opcode opcode;
  sac_transform_mode tr_mode;
  size_t bone_id;
  union
  {
    struct
    {
      float posx;
      float posy;
      float posz;
    } movement;
    struct
    {
      float quat_r;
      float quat_x;
      float quat_y;
      float quat_z;
    } rotate;
  };
};


/**
 * Key frame data
 */
struct sac_frame
{
  csArray<sac_instruction> instructions;
  csTicks duration;
  int repeat_times;
  bool active;
};

/**
 * A static compiled script. This is basically a series of instructions.
 * No state information is kept here. Look at csAnimControlRunnable for that.
 */
class csSkelAnimControlScript
{
private:
  char* name;
  csArray<sac_instruction> instructions;
  csTicks time, forced_duration;
  csArray<sac_frame> frames;
  bool loop;
  int loop_times;

public:
  csSkelAnimControlScript (const char* name);
  ~csSkelAnimControlScript () { delete[] name; }

  void SetForcedDuration(csTicks new_duration)
  {
    forced_duration = new_duration;
  }

  csTicks GetForcedDuration()
  {
    return forced_duration;
  }

  sac_instruction& AddInstruction (sac_frame &frame, sac_opcode opcode);
  sac_frame& AddFrame (csTicks duration);
  const sac_instruction& GetInstruction (size_t idx) const
  {
    return instructions[idx];
  }
  const csArray<sac_instruction>& GetInstructions () const
  {
    return instructions;
  }

  csArray<sac_frame>& GetFrames ()
  {
    return frames;
  }

  const char* GetName () const { return name; }
  csTicks& GetTime () { return time; }
  void SetLoop (bool loop) { csSkelAnimControlScript::loop = loop; }
  bool GetLoop () { return loop; }
  void SetLoopTimes (bool loop_times) 
  { csSkelAnimControlScript::loop_times = loop_times; }
  int GetLoopTimes () { return loop_times; }
};

struct bone_transform_data
{
  csQuaternion quat;
  csVector3 axis;
};

/**
 * A running rotate operation.
 */
struct sac_rotate_execution
{
  csSkelBone* bone;
  bone_transform_data* bone_rotation;
  csQuaternion quat;
  csQuaternion curr_quat;
  csTicks elapsed_ticks;
};

/**
 * A running movement operation.
 */

struct sac_move_execution
{
  csSkelBone* bone;
  bone_transform_data* bone_position;
  csVector3 delta_per_tick;
  csVector3 final_position;
  csTicks elapsed_ticks;
};

/**
 * The runtime state information for a running script. This class does
 * the actual operations in the script in a time based fashion.
 */

class csSkelAnimControlRunnable : public iGenMeshSkeletonScript
{
public:
  typedef csHash<bone_transform_data*, csPtrKey<csSkelBone> > 
    TransformHash;
private:
  csSkelAnimControlScript* script;
  csGenmeshSkelAnimationControl* anim_control;
  size_t current_instruction;
  int current_frame;
  float morph_factor;
  float time_factor;
  csQuaternion zero_quat;
  int loop_times;

  csArray<sac_move_execution> relative_moves;
  csArray<sac_move_execution> absolute_moves;
  csArray<sac_rotate_execution> absolute_rotates;
  csArray<sac_rotate_execution> relative_rotates;

  struct runnable_frame
  {
    bool active;
    int repeat_times;
  };

  csArray<runnable_frame> runnable_frames;

  struct del
  {
    csTicks current;
    csTicks final;
    csTicks diff;
  } delay;

  bool parse_key_frame;

  csTicks current_ticks;

  TransformHash rotations;
  TransformHash positions;

  void release_tranform_data(TransformHash&);
  
  void ParseFrame(const sac_frame & frame);
  sac_frame & NextFrame();

public:
  // Return true if one of the bone transforms was actually modified.
  // 'stop' will be set to true if the runnable needs to end.
  
  bool Do (csTicks elapsed, bool& stop, csTicks & left);

  bone_transform_data *GetBoneRotation(csSkelBone *bone);
  TransformHash& GetRotations() { return rotations; };

  bone_transform_data *GetBonePosition(csSkelBone *bone);
  TransformHash& GetPositions() { return positions; };

  //------------------------------------------

  csSkelAnimControlRunnable (csSkelAnimControlScript* script,
    csGenmeshSkelAnimationControl* anim_control);
  virtual ~csSkelAnimControlRunnable ();

  SCF_DECLARE_IBASE;
  virtual const char *GetName () { return script ? script->GetName () : 0; }
  virtual float GetFactor () { return morph_factor; }
  virtual void SetFactor (float factor) { morph_factor = factor; }
  virtual size_t GetTime ()
  {
    return (size_t) (script->GetTime ()*time_factor);
  }
  virtual void SetTime (size_t time) 
  { 
    if (time && script->GetTime ())
    {
      time_factor = (float)time/(float)script->GetTime (); 
    }
  }
  virtual float GetSpeed () { return time_factor; }
  virtual void SetSpeed (float speed) 
  { 
    if (speed > 0)
    {
      time_factor = speed;
    }
  }
};

/**
 * Recalculate face normals method
 */

enum CalcNormsMethod
{
  CALC_NORMS_NONE = 0,
  CALC_NORMS_FAST,
  CALC_NORMS_ACCURATE
};

/**
 * Genmesh animation control.
 */

class csGenmeshSkelAnimationControl :
  public iGenMeshAnimationControl,
  public iGenMeshSkeletonControlState
{
private:
  iObjectRegistry* object_reg;

  csRef<csGenmeshSkelAnimationControlFactory> factory;

  csRef<iGeneralFactoryState> genmesh_fact_state;

  csRefArray<csSkelAnimControlRunnable> running_scripts;

  int num_animated_verts;
  csVector3* animated_verts;
  csVector3* transformed_verts;
  csColor4* animated_colors;

  struct face_data
  {
    size_t face_idx;
    csTicks time;
  };

  struct vt_con_tri
  {
    csArray<face_data> tri_indices;
    csTicks time;
  };
  csArray<vt_con_tri> vts_con_tris;

  int num_animated_vert_norms;
  csVector3* animated_vert_norms;
  int num_animated_face_norms;
  csVector3* animated_face_norms;
  bool* updated_face_norms;

  csTicks last_update_time;
  uint32 last_version_id;
  csTicks elapsed;

  CalcNormsMethod calc_norms_method;

  // Work tables.
  static csArray<csReversibleTransform> bone_transforms;
  static csArray<csColor4> bone_colors;
  csRefArray<csSkelBone> bones;
  csArray<size_t> parent_bones;

  // Copied from the factory.
  bool animates_vertices;
  bool animates_texels;
  bool animates_colors;
  bool animates_normals;

  // Set to true if the animated version of that array needs updating.
  bool dirty_vertices;
  bool dirty_texels;
  bool dirty_colors;
  bool dirty_normals;
  
  bool bones_updated;
  bool vertices_updated;

  bool force_bone_update;

  // Update the arrays to have correct size. If a realloc was
  // needed then last_version_id will be forced to ~0.
  void UpdateArrays (int num_verts);
  void UpdateVertNormArrays (int num_norms);

  // Update animation state. Set the 'dirty_XXX' flags to true if
  // the arrays need updating too.
  //void UpdateAnimation (csTicks current, int num_verts, uint32 version_id);
  bool vertices_mapped;
  void TransformVerticesToBones (const csVector3* verts, int num_verts);
  void UpdateAnimatedVertices (csTicks current, const csVector3* verts,
  	int num_verts);
  void UpdateBones ();

public:
  bool UpdateAnimation (csTicks current);

  csRefArray<csSkelBone>& GetBones () { return bones; }
  csArray<size_t>& GetParentBones () { return parent_bones; }
  csRefArray<csSkelAnimControlRunnable> & GetRunningScripts ()
  {
    return running_scripts;
  }

  void SetForceUpdate(bool force_update)
  {
    force_bone_update = force_update;
  }

  void SetCalcNormsMethod(CalcNormsMethod cnm)
  {
    calc_norms_method = cnm;
  }

  /// Constructor.
  csGenmeshSkelAnimationControl (csGenmeshSkelAnimationControlFactory* fact, 
    iMeshObject *mesh, iObjectRegistry* object_reg);
  /// Destructor.
  virtual ~csGenmeshSkelAnimationControl ();

  SCF_DECLARE_IBASE;

  // --- For iGenMeshAnimationControl --------------------------------
  virtual bool AnimatesVertices () const { return animates_vertices; }
  virtual bool AnimatesTexels () const { return animates_texels; }
  virtual bool AnimatesNormals () const { return animates_normals; }
  virtual bool AnimatesColors () const { return animates_colors; }
  virtual const csVector3* UpdateVertices (csTicks current,
    const csVector3* verts, int num_verts, uint32 version_id);
  virtual const csVector2* UpdateTexels (csTicks current,
    const csVector2* texels, int num_texels, uint32 version_id);
  virtual const csVector3* UpdateNormals (csTicks current,
    const csVector3* normals, int num_normals, uint32 version_id);
  virtual const csColor4* UpdateColors (csTicks current,
    const csColor4* colors, int num_colors, uint32 version_id);

  // --- For iGenMeshSkeletonControlState
  virtual int GetBonesCount () { return (int)bones.Length (); }
  virtual iGenMeshSkeletonBone *GetBone (int i) { return bones[i]; }
  virtual iGenMeshSkeletonBone *FindBone (const char *name);

  virtual iGenMeshSkeletonScript* Execute (const char *scriptname);
  virtual size_t GetScriptsCount ();
  virtual iGenMeshSkeletonScript* GetScript (size_t i);
  virtual iGenMeshSkeletonScript* FindScript (const char *scriptname);
  virtual void StopAll ();
  virtual void Stop (const char* scriptname);
  virtual void Stop (iGenMeshSkeletonScript *script);
  virtual iGenMeshSkeletonControlFactory *GetFactory() 
  { 
    return (iGenMeshSkeletonControlFactory *)
            (csGenmeshSkelAnimationControlFactory*)factory; 
  }
  
  virtual int GetAnimatedVerticesCount()
  { return num_animated_verts; }

  virtual csVector3 *GetAnimatedVertices()
  { return animated_verts; }

  virtual csVector3 *GetAnimatedFaceNormals()
  {
    return animated_face_norms; 
  }

  virtual int GetAnimatedFaceNormalsCount()
  {
    return num_animated_face_norms; 
  }

  virtual csVector3 *GetAnimatedVertNormals()
  {
    return animated_vert_norms; 
  }

  virtual int GetAnimatedVertNormalsCount()
  {
    return num_animated_vert_norms; 
  }
};

/**
 * Genmesh animation control factory.
 */
class csGenmeshSkelAnimationControlFactory :
	public iGenMeshSkeletonControlFactory
{
private:
  csGenmeshSkelAnimationControlType* type;
  iObjectRegistry* object_reg;

  csStringArray autorun_scripts;

  csRefArray<csSkelBone> bones;
  csArray<size_t> parent_bones;
  csPDelArray<csSkelAnimControlScript> scripts;

  CalcNormsMethod calc_norms_method;

  // These flags are set during script compilation to see if
  // the script can possibly affect the given attributes.
  bool animates_vertices;
  bool animates_texels;
  bool animates_colors;
  bool animates_normals;

  // This flag is set to true if there are hierarchical bones.
  bool has_hierarchical_bones;

  csFlags flags;

  // This is a table that contains a mapping for every vertex to the bones
  // that contain that vertex.
  csArray<csArray<sac_bone_data> > bones_vertices;
  void UpdateBonesMapping ();

  const char* ParseBone (iDocumentNode* node, csSkelBone* parent);
  const char* ParseScript (iDocumentNode* node);

  csStringHash xmltokens;

#define CS_TOKEN_ITEM_FILE "plugins/mesh/genmesh/skelanim/gmeshskelanim.tok"
#include "cstool/tokenlist.h"
  csString error_buf;

public:

  //csAnimationUpdateLevel & GetAULevel() { return always_update_level; }
  const csFlags & GetFlags() {return flags; }

  void RegisterAUAnimation(csGenmeshSkelAnimationControl *anim);
  void UnregisterAUAnimation(csGenmeshSkelAnimationControl *anim);
  /// Constructor.
  csGenmeshSkelAnimationControlFactory (csGenmeshSkelAnimationControlType* type,
    iObjectRegistry* object_reg);
  /// Destructor.
  virtual ~csGenmeshSkelAnimationControlFactory ();

  virtual csPtr<iGenMeshAnimationControl> CreateAnimationControl (
  	iMeshObject *mesh);

  csGenmeshSkelAnimationControlType* GetType () { return type; }
  csSkelAnimControlScript* FindScript (const char* scriptname) const;
  csSkelBone* FindBone (const char* bonename) const;
  size_t FindBoneIndex (const char* bonename) const;

  void UpdateParentBones ();

  bool AnimatesVertices () const { return animates_vertices; }
  bool AnimatesTexels () const { return animates_texels; }
  bool AnimatesNormals () const { return animates_normals; }
  bool AnimatesColors () const { return animates_colors; }
  bool HasHierarchicalBones () const { return has_hierarchical_bones; }

  csRefArray<csSkelBone>& GetBones () { return bones; }
  csArray<size_t>& GetParentBones () { return parent_bones; }
  csArray<csArray<sac_bone_data> >& GetBonesVerticesMapping ()
  {
    return bones_vertices;
  }

  SCF_DECLARE_IBASE;

  // --- For iGenMeshAnimationControlFactory -------------------------
  virtual const char* Load (iDocumentNode* node);
  virtual const char* Save (iDocumentNode* parent);

  // --- For iGenMeshSkeletonControlFactory -------------------------
  virtual const char* LoadScriptFile(const char *filename);
  virtual void DeleteScript(const char *script_name);
  virtual void DeleteAllScripts();
};

/**
 * Genmesh animation control type.
 */
class csGenmeshSkelAnimationControlType : public iGenMeshAnimationControlType
{
private:
  iObjectRegistry* object_reg;
  csRef<iVirtualClock> vc;
  csArray<csGenmeshSkelAnimationControl*> always_update_animations;

  void UpdateAUAnimations(csTicks current_ticks)
  {
    size_t i;
    size_t au_anim_length = always_update_animations.Length();
    for(i = 0; i < au_anim_length; i++)
    {
      always_update_animations[i]->UpdateAnimation(current_ticks);
    }
  }

public:
  void RegisterAUAnimation(csGenmeshSkelAnimationControl *anim)
  {
    always_update_animations.Push(anim);
  }

  void UnregisterAUAnimation(csGenmeshSkelAnimationControl *anim)
  {
    always_update_animations.Delete(anim);
  }

  /// Constructor.
  csGenmeshSkelAnimationControlType (iBase*);
  /// Destructor.
  virtual ~csGenmeshSkelAnimationControlType ();
  /// Initialize.
  bool Initialize (iObjectRegistry* object_reg);
  /// Event handler.
  bool HandleEvent (iEvent& ev);

  virtual csPtr<iGenMeshAnimationControlFactory> CreateAnimationControlFactory (
  	);

  SCF_DECLARE_IBASE;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csGenmeshSkelAnimationControlType);
    virtual bool Initialize (iObjectRegistry* object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;

  class EventHandler : public iEventHandler
  {
  private:
    csGenmeshSkelAnimationControlType* parent;
  public:
    EventHandler (csGenmeshSkelAnimationControlType* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      EventHandler::parent = parent;
    }
    virtual ~EventHandler ()
    {
      SCF_DESTRUCT_IBASE();
    }
    SCF_DECLARE_IBASE;
    virtual bool HandleEvent (iEvent& ev)
    {
      return parent->HandleEvent (ev);
    }
  } *scfiEventHandler;
};

#endif // __CS_GENMESHSKELANIM_H__
