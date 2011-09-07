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

#include "csgeom/quaternion.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"
#include "csutil/array.h"
#include "csutil/cscolor.h"
#include "csutil/weakref.h"
#include "csutil/csstring.h"
#include "csutil/flags.h"
#include "csutil/hash.h"
#include "csutil/parray.h"
#include "csutil/refarr.h"
#include "csutil/strhash.h"
#include "csutil/stringarray.h"
#include "imesh/genmesh.h"
#include "imesh/gmeshskel2.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/virtclk.h"
#include "iutil/strset.h"
#include "csgfx/shadervar.h"
#include <imesh/object.h>
#include "iengine/mesh.h"
#include "iutil/plugin.h"
#include "imap/loader.h"
#include "imesh/skeleton.h"
#include <imap/reader.h>
#include <csutil/xmltiny.h>
#include <iutil/vfs.h>
#include <iutil/databuff.h>

class csGenmeshSkelAnimationControlFactory;
class csGenmeshSkelAnimationControlType;
class csSkelAnimControlRunnable;
struct iEvent;

/**
 * This is a bone of vertices. Every bone has
 * a set of vertices (with weights), children, and a transform
 * relative to a parent. Script operations operate on bones of vertices.
 */

/**
 * Genmesh animation control.
 */

class csGenmeshSkelAnimationControl :
  public scfImplementation2<csGenmeshSkelAnimationControl,
    iGenMeshAnimationControl, iGenMeshSkeletonControlState>
{
private:
  iObjectRegistry* object_reg;

  csWeakRef<iSkeleton> skeleton;

  csRef<csGenmeshSkelAnimationControlFactory> factory;
  CS::ShaderVarStringID bones_name;
  iMeshObject *mesh_obj;
  csArray<csReversibleTransform> in_trs;
  csArray<int> used_bones;

  int num_animated_verts;
  csVector3* animated_verts;
  csVector3* transformed_verts;
  csColor4* animated_colors;
  csVector3* animated_tangents;
  csVector3* animated_bitangents;

  int num_animated_vert_norms;
  csVector3* animated_vert_norms;
  int num_animated_face_norms;
  csVector3* animated_face_norms;
  bool* updated_face_norms;

  csTicks last_update_time;
  uint32 last_version_id;
  csTicks elapsed;

  // Work tables.
  static csArray<csColor4> bone_colors;

  // Copied from the factory.
  bool animates_vertices;
  bool animates_texels;
  bool animates_colors;
  bool animates_normals;
  bool animates_tangents;
  bool animates_bitangents;

  // Set to true if the animated version of that array needs updating.
  bool dirty_vertices;
  bool dirty_texels;
  bool dirty_colors;
  bool dirty_normals;

  // Update the arrays to have correct size. If a realloc was
  // needed then last_version_id will be forced to ~0.
  void UpdateArrays (int num_verts);
  void UpdateVertNormArrays (int num_norms);

  // Update animation state. Set the 'dirty_XXX' flags to true if
  // the arrays need updating too.
  //void UpdateAnimation (csTicks current, int num_verts, uint32 version_id);
  bool vertices_mapped;
  bool normals_mapped;
  bool tangents_mapped;
  bool bitangents_mapped;

  bool initialized;
  void Initialize ();
  bool use_parent;
  csArray<csString> v_bones;

public:

  /// Constructor.
  csGenmeshSkelAnimationControl (csGenmeshSkelAnimationControlFactory* fact, 
  	iMeshObject *mesh, iObjectRegistry* object_reg);
  /// Destructor.
  virtual ~csGenmeshSkelAnimationControl ();

  // --- For iGenMeshAnimationControl --------------------------------
  virtual bool AnimatesVertices () const { return animates_vertices; }
  virtual bool AnimatesTexels () const { return animates_texels; }
  virtual bool AnimatesNormals () const { return animates_normals; }
  virtual bool AnimatesColors () const { return animates_colors; }
  virtual bool AnimatesTangents () const { return animates_tangents; }
  virtual bool AnimatesBiTangents () const { return animates_bitangents; }
  virtual bool AnimatesBBoxRadius () const { return false; }
  virtual void Update (csTicks current, int, uint32);
  virtual const csVector3* UpdateVertices (csTicks current,
  	const csVector3* verts, int num_verts, uint32 version_id);
  virtual const csVector2* UpdateTexels (csTicks current,
  	const csVector2* texels, int num_texels, uint32 version_id);
  virtual const csVector3* UpdateNormals (csTicks current,
  	const csVector3* normals, int num_normals, uint32 version_id);
  virtual const csColor4* UpdateColors (csTicks current,
  	const csColor4* colors, int num_colors, uint32 version_id);
  virtual const csVector3* UpdateTangents (csTicks current,
  	const csVector3* tangents, int num_tangents, uint32 version_id);
  virtual const csVector3* UpdateBiTangents (csTicks current,
  	const csVector3* bitangents, int num_bitangents, uint32 version_id);
  virtual const csBox3& UpdateBoundingBox (csTicks current, uint32 version_id,
	const csBox3& bbox) { return bbox; }
  virtual const float UpdateRadius (csTicks current, uint32 version_id,
	const float radius) { return radius; }
  virtual const csBox3* UpdateBoundingBoxes (csTicks current, uint32 version_id)
  { return nullptr; }

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

  virtual csVector3* GetAnimatedTangents()
  {
    return animated_tangents;
  }

  virtual csVector3* GetAnimatedBiTangents()
  {
    return animated_bitangents;
  }

  virtual iSkeleton *GetSkeleton() { return skeleton; }
};

/**
 * Genmesh animation control factory.
 */
class csGenmeshSkelAnimationControlFactory :
  public scfImplementation1<csGenmeshSkelAnimationControlFactory,
    iGenMeshAnimationControlFactory>
{
private:
  csGenmeshSkelAnimationControlType* type;
  iObjectRegistry* object_reg;

  csStringArray autorun_scripts;

  csRef<iSkeletonFactory> skeleton_factory;

  csArray<size_t> parent_bones;

  csArray<int> used_bones;
  //csArray<csString> vert_bones;

  // These flags are set during script compilation to see if
  // the script can possibly affect the given attributes.
  bool animates_vertices;
  bool animates_texels;
  bool animates_colors;
  bool animates_normals;

  // This flag is set to true if there are hierarchical bones.
  bool has_hierarchical_bones;

  csFlags flags;

  csStringHash xmltokens;

#define CS_TOKEN_ITEM_FILE "plugins/mesh/genmesh/skelanim2/gmeshskelanim.tok"
#include "cstool/tokenlist.h"

  csString error_buf;
  iMeshObjectFactory *mesh_fact;
  csRef<iSkeletonGraveyard> the_graveyard;

  bool use_parent;
public:
  csArray<int> & GetUsedBones() { return used_bones; }
  virtual bool GetUseParent() { return use_parent; }
  virtual iSkeletonFactory* GetSkelFact() { return skeleton_factory; }
  inline iSkeletonGraveyard* GetSkeletonGraveyard () { return the_graveyard; }

  //csAnimationUpdateLevel & GetAULevel() { return always_update_level; }
  iSkeletonFactory * GetSkeletonFactory() {return skeleton_factory; }
  const csFlags & GetFlags() {return flags; }
  /// Constructor.
  csGenmeshSkelAnimationControlFactory (csGenmeshSkelAnimationControlType* type,
  	iObjectRegistry* object_reg);
  /// Destructor.
  virtual ~csGenmeshSkelAnimationControlFactory ();

  virtual csPtr<iGenMeshAnimationControl> CreateAnimationControl (
      iMeshObject *mesh);

  csGenmeshSkelAnimationControlType* GetType () { return type; }

  bool AnimatesVertices () const { return animates_vertices; }
  bool AnimatesTexels () const { return animates_texels; }
  bool AnimatesNormals () const { return animates_normals; }
  bool AnimatesColors () const { return animates_colors; }
  bool HasHierarchicalBones () const { return has_hierarchical_bones; }

  // --- For iGenMeshAnimationControlFactory -------------------------
  virtual const char* Load (iDocumentNode* node);
  virtual const char* Save (iDocumentNode* parent);
};

/**
 * Genmesh animation control type.
 */
class csGenmeshSkelAnimationControlType :
  public scfImplementation2<csGenmeshSkelAnimationControlType,
    iGenMeshAnimationControlType, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iVirtualClock> vc;
  csEventID PreProcess;
  csArray<csGenmeshSkelAnimationControl*> always_update_animations;

public:
  /// Constructor.
  csGenmeshSkelAnimationControlType (iBase*);
  /// Destructor.
  virtual ~csGenmeshSkelAnimationControlType ();
  /// Initialize.
  bool Initialize (iObjectRegistry* object_reg);

  virtual csPtr<iGenMeshAnimationControlFactory> CreateAnimationControlFactory (
      );
};

#endif // __CS_GENMESHSKELANIM_H__
