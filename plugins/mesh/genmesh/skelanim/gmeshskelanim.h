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
#include "csutil/parray.h"
#include "csutil/refarr.h"
#include "csutil/stringarray.h"
#include "imesh/genmesh.h"
#include "imesh/gmeshskel.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/virtclk.h"
#include "ivaria/dynamics.h"

class csGenmeshSkelAnimationControl;
class csGenmeshSkelAnimationControlFactory;
class csGenmeshSkelAnimationControlType;
class csSkelAnimControlRunnable;
struct iEvent;

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
	csReversibleTransform transform;
	csReversibleTransform full_transform;

	BoneTransformMode bone_mode;
	iRigidBody *rigid_body;

	csGenmeshSkelAnimationControl *anim_control;

	struct bone_rotation
	{
		csQuaternion quat;
		union 
		{
			float data[3];
			struct
			{
				float x;
				float y;
				float z;
			};
		};
	} rot;

public:
	csQuaternion & GetQuaternion () { return rot.quat; };
	void CopyFrom (csSkelBone *other);
	void AddVertex (int idx, float weight, float col_weight);
	csArray<sac_vertex_data>& GetVertexData () { return vertices; }
	void AddBone (csRef<csSkelBone> & bone) { bones.Push (bone); }
	csRefArray<csSkelBone>& GetBones () { return bones; }

	void SetParent (csSkelBone* p) { parent = p; }
	void UpdateBones ();
	void UpdateBones (iRigidBody* parent_body);

	void SetAxisAngle (int axis, float angle);
	float GetAxisAngle (int axis);

	void UpdateRotation();
	void UpdatePosition();
	//------------------------------------------------------------------------

	csSkelBone (csGenmeshSkelAnimationControl *animation_control);
	virtual ~csSkelBone ();

	SCF_DECLARE_IBASE;
	virtual const char* GetName () const { return name; }
	virtual void SetName (const char* name) {csSkelBone::name = csStrNew (name); }
	virtual csReversibleTransform &GetTransform () { return transform; }
	virtual csReversibleTransform &GetFullTransform () { return full_transform; }
	virtual iGenMeshSkeletonBone* GetParent () { return parent; }
	virtual void GetSkinBox (csBox3 &box, csVector3 &center);
	virtual void SetMode (BoneTransformMode mode) { bone_mode = mode; }
	virtual BoneTransformMode GetMode () { return bone_mode; }
	virtual void SetRigidBody (iRigidBody *r_body) { rigid_body = r_body; }
	virtual iRigidBody *GetRigidBody () { return rigid_body; }
	virtual int GetChildrenCount () { return (int)bones.Length () ;}
	virtual iGenMeshSkeletonBone *GetChild (int i) { return bones[i]; }
	virtual iGenMeshSkeletonBone *FindChild (const char *name);
};

/**
 * Possible opcodes for instructions in a script.
 */
enum sac_opcode
{
	AC_STOP,
	AC_DELAY,
	AC_REPEAT,
	AC_MOVE,
	AC_ROT
};

/**
 * A script instruction with the parameter data for that instruction.
 * Every script has an array of these.
 */
struct sac_instruction
{
	sac_opcode opcode;
	union
	{
		struct
		{
			size_t bone_id;
			csTicks duration;
			float posx;
			float posy;
			float posz;
		} movement;
		struct
		{
			size_t bone_id;
			csTicks duration;
			float angle;
			float quat_x;
			float quat_y;
			float quat_z;
			float quat_r;
		} rotate;
		struct
		{
			csTicks time;
		} delay;
	};
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
	csTicks time;
public:

	csSkelAnimControlScript (const char* name);
	~csSkelAnimControlScript () { delete[] name; }

	sac_instruction& AddInstruction (sac_opcode opcode);
	const sac_instruction& GetInstruction (size_t idx) const
	{
		return instructions[idx];
	}
	const csArray<sac_instruction>& GetInstructions () const
	{
		return instructions;
	}

	const char* GetName () const { return name; }

	csTicks& GetTime () { return time; }
};

struct bone_transform_data
{
	csQuaternion quat;
	union
	{
		float data[3];
		struct
		{
			float x;
			float y;
			float z;
		};
	};
};

/**
 * A running rotate operation.
 */
struct sac_rotate_execution
{
	csSkelBone* bone;
	bone_transform_data* bone_rotation;
	csTicks final;
	csTicks duration;
	bool flag;
	csQuaternion quat, current_quat;
};

/**
 * A running movement operation.
 */

struct sac_move_execution
{
	csSkelBone* bone;
	bone_transform_data* bone_position;
	csTicks final;
	csVector3 delta_per_tick;
	csVector3 final_position;
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
	float morph_factor;
	float time_factor;

	// Current movement operations.
	csArray<sac_move_execution> moves;
	// Current rotate operations.
	csArray<sac_rotate_execution> rotates;
	// Current delay operation.
	struct del
	{
		csTicks final;
	} delay;

	TransformHash rotations;
	TransformHash positions;

	void release_tranform_data(TransformHash&);

public:
	// Return true if one of the bone transforms was actually modified.
	// 'stop' will be set to true if the runnable needs to end.
	bool Do (csTicks current, bool& stop);

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
	virtual size_t GetTime () { return (size_t) (script->GetTime ()*time_factor); };
	virtual void SetTime (size_t time) { time_factor = (float)time/ (float)script->GetTime (); }
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
	csRef<iVirtualClock> virt_clk;

	csGenmeshSkelAnimationControlFactory* factory;

	csRefArray<csSkelAnimControlRunnable> running_scripts;

	int num_animated_verts;
	csVector3* animated_verts;
	csVector3* transformed_verts;
	csColor* animated_colors;

	csTicks last_update_time;
	uint32 last_version_id;

	// Work tables.
	static csArray<csReversibleTransform> bone_transforms;
	static csArray<csColor> bone_colors;
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

	bool always_update;

	// Update the arrays to have correct size. If a realloc was
	// needed then last_version_id will be forced to ~0.
	void UpdateArrays (int num_verts);

	// Update animation state. Set the 'dirty_XXX' flags to true if
	// the arrays need updating too.
	//void UpdateAnimation (csTicks current, int num_verts, uint32 version_id);
	void UpdateAnimation (csTicks current);

	bool vertices_mapped;
	void TransformVerticesToBones (const csVector3* verts, int num_verts);

public:
	csRefArray<csSkelBone>& GetBones () { return bones; }
	csArray<size_t>& GetParentBones () { return parent_bones; }
	csRefArray<csSkelAnimControlRunnable> & GetRunningScripts () { return running_scripts; }

	bool HandleEvent (iEvent& ev);

	class EventHandler : public iEventHandler
	{
	private:
		csGenmeshSkelAnimationControl* parent;
	public:
		EventHandler (csGenmeshSkelAnimationControl* parent)
		{
			SCF_CONSTRUCT_IBASE (0);
			EventHandler::parent = parent;
		}
		virtual ~EventHandler ()
		{
			SCF_DESTRUCT_IBASE ();
		}
		SCF_DECLARE_IBASE;
		virtual bool HandleEvent (iEvent& ev)
		{
			return parent->HandleEvent (ev);
		}
	} *scfiEventHandler;

	/// Constructor.
	csGenmeshSkelAnimationControl (csGenmeshSkelAnimationControlFactory* fact, iObjectRegistry* object_reg);
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
	virtual const csColor* UpdateColors (csTicks current,
		const csColor* colors, int num_colors, uint32 version_id);

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

	virtual void SetAlwaysUpdate(bool always_update) 
		{ csGenmeshSkelAnimationControl::always_update = always_update; }
	virtual bool GetAlwaysUpdate() 
		{ return always_update; }
};

/**
 * Genmesh animation control factory.
 */
class csGenmeshSkelAnimationControlFactory : public iGenMeshAnimationControlFactory
{
private:
	csGenmeshSkelAnimationControlType* type;
	iObjectRegistry* object_reg;

	//csStringArray autorun_scripts;
	csStringArray autorun_scripts;

	csRefArray<csSkelBone> bones;
	csArray<size_t> parent_bones;
	csPDelArray<csSkelAnimControlScript> scripts;

	// These flags are set during script compilation to see if
	// the script can possibly affect the given attributes.
	bool animates_vertices;
	bool animates_texels;
	bool animates_colors;
	bool animates_normals;

	// This flag is set to true if there are hierarchical bones.
	bool has_hierarchical_bones;

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
	/// Constructor.
	csGenmeshSkelAnimationControlFactory (csGenmeshSkelAnimationControlType* type,
		iObjectRegistry* object_reg);
	/// Destructor.
	virtual ~csGenmeshSkelAnimationControlFactory ();

	virtual csPtr<iGenMeshAnimationControl> CreateAnimationControl ();

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
	csArray<csArray<sac_bone_data> >& GetBonesVerticesMapping () { return bones_vertices; }

	SCF_DECLARE_IBASE;

	// --- For iGenMeshAnimationControlFactory -------------------------
	virtual const char* Load (iDocumentNode* node);
	virtual const char* Save (iDocumentNode* parent);
};

/**
 * Genmesh animation control type.
 */
class csGenmeshSkelAnimationControlType : public iGenMeshAnimationControlType
{
private:
	iObjectRegistry* object_reg;

public:
	/// Constructor.
	csGenmeshSkelAnimationControlType (iBase*);
	/// Destructor.
	virtual ~csGenmeshSkelAnimationControlType ();
	/// Initialize.
	bool Initialize (iObjectRegistry* object_reg);

	virtual csPtr<iGenMeshAnimationControlFactory> CreateAnimationControlFactory ();

	SCF_DECLARE_IBASE;

	struct eiComponent : public iComponent
	{
		SCF_DECLARE_EMBEDDED_IBASE (csGenmeshSkelAnimationControlType);
		virtual bool Initialize (iObjectRegistry* object_reg)
		{
			return scfParent->Initialize (object_reg);
		}
	} scfiComponent;
};

#endif // __CS_GENMESHSKELANIM_H__
