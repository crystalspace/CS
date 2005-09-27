/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __CS_GENMESHANIM_H__
#define __CS_GENMESHANIM_H__

#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"
#include "csutil/array.h"
#include "csutil/cscolor.h"
#include "csutil/csstring.h"
#include "csutil/parray.h"
#include "csutil/strhash.h"
#include "csutil/stringarray.h"
#include "imesh/genmesh.h"
#include "imesh/gmeshanim.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"

struct iEvent;
class csGenmeshAnimationControlFactory;
class csGenmeshAnimationControlType;
struct iMeshObject;

/**
 * Vertex information for a group.
 */
struct ac_vertex_data
{
  int idx;
  float weight;
  float col_weight;
};

/**
 * Group information for a vertex.
 */
struct ac_group_data
{
  int idx;
  float weight;
};

/**
 * This is a group of vertices. Every group has
 * a set of vertices (with weights), children, and a transform
 * relative to a parent. Script operations operate on groups of vertices.
 */
class csAnimControlGroup
{
private:
  char* name;
  csArray<ac_vertex_data> vertices;
  csAnimControlGroup* parent;
  csArray<csAnimControlGroup*> groups;
  csReversibleTransform transform;
  csColor color;

public:
  csAnimControlGroup (const char* name);
  ~csAnimControlGroup () { delete[] name; }

  const char* GetName () const { return name; }

  void AddVertex (int idx, float weight, float col_weight);
  const csArray<ac_vertex_data>& GetVertexData () const { return vertices; }

  void AddGroup (csAnimControlGroup* group) { groups.Push (group); }
  const csArray<csAnimControlGroup*>& GetGroups () const { return groups; }

  csColor& GetColor () { return color; }
  csColor GetFullColor ()
  {
    if (!parent) return GetColor ();
    else return GetColor () * parent->GetFullColor ();
  }
  csReversibleTransform& GetTransform () { return transform; }
  csReversibleTransform GetFullTransform ()
  {
    if (!parent) return GetTransform ();
    else return GetTransform () * parent->GetFullTransform ();
  }
  void SetParent (csAnimControlGroup* p) { parent = p; }
  csAnimControlGroup* GetParent () const { return parent; }
};

/**
 * Possible opcodes for instructions in a script.
 */
enum ac_opcode
{
  AC_STOP,
  AC_DELAY,
  AC_REPEAT,
  AC_COLOR,
  AC_MOVE,
  AC_SCALEX,
  AC_SCALEY,
  AC_SCALEZ,
  AC_ROTX,
  AC_ROTY,
  AC_ROTZ
};

/**
 * A script instruction with the parameter data for that instruction.
 * Every script has an array of these.
 */
struct ac_instruction
{
  ac_opcode opcode;
  union
  {
    struct
    {
      size_t group_id;
      csTicks duration;
      float dx;
      float dy;
      float dz;
    } movement;
    struct
    {
      size_t group_id;
      csTicks duration;
      float red;
      float green;
      float blue;
    } color;
    struct
    {
      size_t group_id;
      csTicks duration;
      float angle;
    } rotate;
    struct
    {
      size_t group_id;
      csTicks duration;
      float scale;
    } scale;
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
class csAnimControlScript
{
private:
  char* name;
  csArray<ac_instruction> instructions;

public:
  csAnimControlScript (const char* name);
  ~csAnimControlScript () { delete[] name; }

  ac_instruction& AddInstruction (ac_opcode opcode);
  const ac_instruction& GetInstruction (size_t idx) const
  {
    return instructions[idx];
  }
  const csArray<ac_instruction>& GetInstructions () const
  {
    return instructions;
  }

  const char* GetName () const { return name; }
};

/**
 * A running movement operation.
 */
struct ac_move_execution
{
  csAnimControlGroup* group;
  csTicks final;
  csVector3 delta_per_tick;
  csVector3 final_position;
};

/**
 * A running scale operation.
 */
struct ac_scale_execution
{
  csAnimControlGroup* group;
  csTicks final;
  int axis;
  csReversibleTransform base_transform;
  float delta_scale_per_tick;
  float final_scale;
};

/**
 * A running rotate operation.
 */
struct ac_rotate_execution
{
  csAnimControlGroup* group;
  csTicks final;
  int axis;
  csReversibleTransform base_transform;
  float delta_angle_per_tick;
  float final_angle;
};

/**
 * A running color operation.
 */
struct ac_color_execution
{
  csAnimControlGroup* group;
  csTicks final;
  csColor delta_per_tick;
  csColor final_color;
};

/**
 * The runtime state information for a running script. This class does
 * the actual operations in the script in a time based fashion.
 */
class csAnimControlRunnable
{
private:
  csAnimControlScript* script;
  csGenmeshAnimationControlFactory* factory;
  size_t current_instruction;

  // Current color operations.
  csArray<ac_color_execution> colors;
  // Current movement operations.
  csArray<ac_move_execution> moves;
  // Current rotate operations.
  csArray<ac_rotate_execution> rotates;
  // Current scale operations.
  csArray<ac_scale_execution> scales;
  // Current delay operation.
  struct del
  {
    csTicks final;
  } delay;

public:
  csAnimControlRunnable (csAnimControlScript* script,
  	csGenmeshAnimationControlFactory* factory);
  ~csAnimControlRunnable ();

  // Return true if one of the group transforms was actually modified.
  // 'stop' will be set to true if the runnable needs to end.
  bool Do (csTicks current, bool& stop);
};

/**
 * Genmesh animation control.
 */
class csGenmeshAnimationControl :
	public iGenMeshAnimationControl,
	public iGenMeshAnimationControlState
{
private:
  csGenmeshAnimationControlFactory* factory;

  csPDelArray<csAnimControlRunnable> running_scripts;

  int num_animated_verts;
  csVector3* animated_verts;
  csColor4* animated_colors;

  csTicks last_update_time;
  uint32 last_version_id;

  // Work tables.
  static csArray<csReversibleTransform> group_transforms;
  static csArray<csColor4> group_colors;

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

  // Update the arrays to have correct size. If a realloc was
  // needed then last_version_id will be forced to ~0.
  void UpdateArrays (int num_verts);

  // Update animation state. Set the 'dirty_XXX' flags to true if
  // the arrays need updating too.
  void UpdateAnimation (csTicks current, int num_verts, uint32 version_id);

public:
  /// Constructor.
  csGenmeshAnimationControl (csGenmeshAnimationControlFactory* fact);
  /// Destructor.
  virtual ~csGenmeshAnimationControl ();

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

  // --- For iGenMeshAnimationControlState ---------------------------
  virtual bool Execute (const char* scriptname);
  virtual void Stop () { }
  virtual void Stop (const char* scriptname) { }
};

/**
 * Genmesh animation control factory.
 */
class csGenmeshAnimationControlFactory : public iGenMeshAnimationControlFactory
{
private:
  csGenmeshAnimationControlType* type;
  iObjectRegistry* object_reg;

  csStringArray autorun_scripts;

  csPDelArray<csAnimControlGroup> groups;
  csPDelArray<csAnimControlScript> scripts;

  // These flags are set during script compilation to see if
  // the script can possibly affect the given attributes.
  bool animates_vertices;
  bool animates_texels;
  bool animates_colors;
  bool animates_normals;

  // This flag is set to true if there are hierarchical groups.
  bool has_hierarchical_groups;

  // This is a table that contains a mapping for every vertex to the groups
  // that contain that vertex.
  csArray<csArray<ac_group_data> > groups_vertices;
  csArray<csArray<ac_group_data> > groups_colors;
  void UpdateGroupsMapping ();

  const char* ParseGroup (iDocumentNode* node, csAnimControlGroup* parent);
  const char* ParseScript (iDocumentNode* node);

  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE "plugins/mesh/genmesh/anim/gmeshanim.tok"
#include "cstool/tokenlist.h"
  csString error_buf;

public:
  /// Constructor.
  csGenmeshAnimationControlFactory (csGenmeshAnimationControlType* type,
  	iObjectRegistry* object_reg);
  /// Destructor.
  virtual ~csGenmeshAnimationControlFactory ();

  virtual csPtr<iGenMeshAnimationControl> CreateAnimationControl (iMeshObject *mesh);

  csGenmeshAnimationControlType* GetType () { return type; }
  csAnimControlScript* FindScript (const char* scriptname) const;
  csAnimControlGroup* FindGroup (const char* groupname) const;
  size_t FindGroupIndex (const char* groupname) const;

  bool AnimatesVertices () const { return animates_vertices; }
  bool AnimatesTexels () const { return animates_texels; }
  bool AnimatesNormals () const { return animates_normals; }
  bool AnimatesColors () const { return animates_colors; }
  bool HasHierarchicalGroups() const { return has_hierarchical_groups; }

  const csPDelArray<csAnimControlGroup>& GetGroups () const { return groups; }
  const csArray<csArray<ac_group_data> >& GetGroupsVerticesMapping () const
  {
    return groups_vertices;
  }
  const csArray<csArray<ac_group_data> >& GetGroupsColorsMapping () const
  {
    return groups_colors;
  }

  SCF_DECLARE_IBASE;

  // --- For iGenMeshAnimationControlFactory -------------------------
  virtual const char* Load (iDocumentNode* node);
  virtual const char* Save (iDocumentNode* parent);
};

/**
 * Genmesh animation control type.
 */
class csGenmeshAnimationControlType : public iGenMeshAnimationControlType
{
private:
  iObjectRegistry* object_reg;

public:
  /// Constructor.
  csGenmeshAnimationControlType (iBase*);
  /// Destructor.
  virtual ~csGenmeshAnimationControlType ();
  /// Initialize.
  bool Initialize (iObjectRegistry* object_reg);
  /// Event handler.
  bool HandleEvent (iEvent& ev);

  virtual csPtr<iGenMeshAnimationControlFactory> CreateAnimationControlFactory
  	();

  SCF_DECLARE_IBASE;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGenmeshAnimationControlType);
    virtual bool Initialize (iObjectRegistry* object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;

  class EventHandler : public iEventHandler
  {
  private:
    csGenmeshAnimationControlType* parent;
  public:
    EventHandler (csGenmeshAnimationControlType* parent)
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

#endif // __CS_GENMESHANIM_H__

