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

#include "csgeom/vector3.h"
#include "csgeom/transfrm.h"
#include "csutil/array.h"
#include "csutil/parray.h"
#include "csutil/csstring.h"
#include "imesh/genmesh.h"
#include "imesh/gmeshanim.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"

struct iEvent;
class csGenmeshAnimationControlFactory;
class csGenmeshAnimationControlType;

/**
 * Vertex information for a group.
 */
struct ac_vertex_data
{
  int idx;
  float weight;
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

public:
  csAnimControlGroup (const char* name);
  ~csAnimControlGroup () { delete[] name; }

  const char* GetName () const { return name; }

  void AddVertex (int idx, float weight);
  const csArray<ac_vertex_data>& GetVertexData () const { return vertices; }

  void AddGroup (csAnimControlGroup* group) { groups.Push (group); }
  const csArray<csAnimControlGroup*>& GetGroups () const { return groups; }

  csReversibleTransform& GetTransform () { return transform; }
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
  AC_MOVE
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
      int group_id;
      csTicks duration;
      float dx;
      float dy;
      float dz;
    } movement;
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
 * The runtime state information for a running script. This class does
 * the actual operations in the script in a time based fashion.
 */
class csAnimControlRunnable
{
private:
  csAnimControlScript* script;
  size_t current_instruction;

  // Current movement operation.
  struct mov
  {
    csTicks final;
    csVector3 delta_per_tick;
    csVector3 final_position;
  } movement;
  // Current delay operation.
  struct del
  {
    csTicks final;
  } delay;

public:
  csAnimControlRunnable (csAnimControlScript* script);
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

  csTicks last_update_time;
  uint32 last_version_id;

  // Update the arrays to have correct size. If a realloc was
  // needed then last_version_id will be forced to ~0.
  void UpdateArrays (int num_verts);

  // Return true if one of scripts updated something.
  bool UpdateAnimation (csTicks current);

public:
  /// Constructor.
  csGenmeshAnimationControl (csGenmeshAnimationControlFactory* fact);
  /// Destructor.
  virtual ~csGenmeshAnimationControl ();

  SCF_DECLARE_IBASE;

  // --- For iGenMeshAnimationControl --------------------------------
  virtual bool AnimatesVertices () const { return true; }
  virtual bool AnimatesTexels () const { return false; }
  virtual bool AnimatesNormals () const { return false; }
  virtual bool AnimatesColors () const { return false; }
  virtual const csVector3* UpdateVertices (csTicks current,
  	const csVector3* verts, int num_verts, uint32 version_id);
  virtual const csVector2* UpdateTexels (csTicks current,
  	const csVector2* texels, int num_texels, uint32 version_id);
  virtual const csVector3* UpdateNormals (csTicks current,
  	const csVector3* normals, int num_normals, uint32 version_id);
  virtual const csColor* UpdateColors (csTicks current,
  	const csColor* colors, int num_colors, uint32 version_id);

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

  char* autorun_script;

  csPDelArray<csAnimControlGroup> groups;
  csPDelArray<csAnimControlScript> scripts;

  // This is a table that contains a mapping for every vertex to the groups
  // that contain that vertex.
  csArray<csArray<ac_group_data> > groups_per_vertex;
  void UpdateGroupsPerVertexMapping ();

  const char* ParseGroup (iDocumentNode* node, csAnimControlGroup* parent);
  const char* ParseScript (iDocumentNode* node);

  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE "plugins/mesh/genmesh/anim/gmeshanim.tok"
#include "cstool/tokenlist.h"
  char error_buf[256];

public:
  /// Constructor.
  csGenmeshAnimationControlFactory (csGenmeshAnimationControlType* type,
  	iObjectRegistry* object_reg);
  /// Destructor.
  virtual ~csGenmeshAnimationControlFactory ();

  virtual csPtr<iGenMeshAnimationControl> CreateAnimationControl ();

  csGenmeshAnimationControlType* GetType () { return type; }
  csAnimControlScript* FindScript (const char* scriptname) const;
  csAnimControlGroup* FindGroup (const char* groupname) const;
  size_t FindGroupIndex (const char* groupname) const;

  const csPDelArray<csAnimControlGroup>& GetGroups () const { return groups; }
  const csArray<csArray<ac_group_data> >& GetGroupsPerVertexMapping () const
  {
    return groups_per_vertex;
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

