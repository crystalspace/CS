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

class csGenmeshAnimationControlFactory;

struct ac_vertex_data
{
  int idx;
  float weight;
};

class csAnimControlGroup
{
private:
  char* name;
  csArray<ac_vertex_data> vertices;
  csReversibleTransform transf;

public:
  csAnimControlGroup (const char* name);
  ~csAnimControlGroup () { delete[] name; }
  void AddVertex (int idx, float weight);
  const char* GetName () const { return name; }
  const csArray<ac_vertex_data>& GetVertexData () const { return vertices; }
};

enum ac_opcode
{
  AC_DELAY,
  AC_STOP,
  AC_REPEAT,
  AC_MOVE
};

struct ac_instruction
{
  ac_opcode opcode;
  union
  {
    struct
    {
      int group_id;
      csTicks duration;
      float delta_x;
      float delta_y;
      float delta_z;
    } movement;
    struct
    {
      csTicks delay;
    } delay;
  };
};

class csAnimControlScript
{
private:
  char* name;
  csArray<ac_instruction> instructions;

public:
  csAnimControlScript (const char* name);
  ~csAnimControlScript () { delete[] name; }
};

class csAnimControlRunnable
{
private:
  csAnimControlScript* script;
  size_t current_instructions;

  // Current movement operation.
  struct
  {
    csTicks todo;
    csVector3 delta_per_tick;
    csVector3 final;
  } movement;

public:

};

/**
 * Genmesh animation control.
 */
class csGenmeshAnimationControl :
	public iGenMeshAnimationControl,
	public iGenMeshAnimationControlSetup
{
private:
  csPDelArray<csAnimControlGroup> groups;
  csPDelArray<csAnimControlScript> scripts;
  csGenmeshAnimationControlFactory* factory;

  const char* ParseGroup (iDocumentNode* node);
  const char* ParseScript (iDocumentNode* node);

public:
  /// Constructor.
  csGenmeshAnimationControl (csGenmeshAnimationControlFactory* fact);
  /// Destructor.
  virtual ~csGenmeshAnimationControl ();

  SCF_DECLARE_IBASE;

  // --- For iGenMeshAnimationControl --------------------------------
  virtual bool UpdateVertices (csTicks current, csVector3* verts,
  	int num_verts, csBox3& bbox);
  virtual bool UpdateTexels (csTicks current, csVector2* texels,
  	int num_texels);
  virtual bool UpdateNormals (csTicks current, csVector3* normals,
  	int num_normals);
  virtual bool UpdateColors (csTicks current, csColor* colors,
  	int num_colors);

  virtual const char* Load (iDocumentNode* node);
  virtual const char* Save (iDocumentNode* parent);

  // --- For iGenMeshAnimationControlSetup ---------------------------
};

/**
 * Genmesh animation control factory.
 */
class csGenmeshAnimationControlFactory : public iGenMeshAnimationControlFactory
{
private:
  iObjectRegistry* object_reg;

public:
  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE "plugins/mesh/genmesh/anim/gmeshanim.tok"
#include "cstool/tokenlist.h"
  char error_buf[256];

public:
  /// Constructor.
  csGenmeshAnimationControlFactory (iBase*);
  /// Destructor.
  virtual ~csGenmeshAnimationControlFactory ();
  /// Initialize.
  bool Initialize (iObjectRegistry* object_reg);

  virtual csPtr<iGenMeshAnimationControl> CreateAnimationControl ();

  SCF_DECLARE_IBASE;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGenmeshAnimationControlFactory);
    virtual bool Initialize (iObjectRegistry* object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

#endif // __CS_GENMESHANIM_H__

