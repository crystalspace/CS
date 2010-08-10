/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

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

#include "cssysdef.h"
#include "csutil/scf.h"
#include "softanim.h"
#include "ivaria/reporter.h"
#include "ivaria/bullet.h"
#include "imesh/object.h"
#include "imesh/objmodel.h"
#include "iengine/mesh.h"
#include "cstool/objmodel.h"

CS_PLUGIN_NAMESPACE_BEGIN(SoftAnim)
{

  //-------------------------- SoftBodyControlType --------------------------

  SCF_IMPLEMENT_FACTORY(SoftBodyControlType);

  CS_LEAKGUARD_IMPLEMENT(SoftBodyControlType);

  SoftBodyControlType::SoftBodyControlType (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  csPtr<iGenMeshAnimationControlFactory>
    SoftBodyControlType::CreateAnimationControlFactory ()
  {
    SoftBodyControlFactory* control = new SoftBodyControlFactory ();
    return csPtr<iGenMeshAnimationControlFactory> (control);
  }

  bool SoftBodyControlType::Initialize (iObjectRegistry* object_reg)
  {
    this->object_reg = object_reg;
    return true;
  }

  void SoftBodyControlType::Report (int severity, const char* msg, ...) const
  {
    va_list arg;
    va_start (arg, msg);
    csRef<iReporter> rep (csQueryRegistry<iReporter> (object_reg));
    if (rep)
      rep->ReportV (severity,
		    "crystalspace.mesh.animesh.controllers.ragdoll",
		    msg, arg);
    else
    {
      csPrintfV (msg, arg);
      csPrintf ("\n");
    }
    va_end (arg);
  }

  //-------------------------- SoftBodyControlFactory --------------------------

  CS_LEAKGUARD_IMPLEMENT(SoftBodyControlFactory);

  SoftBodyControlFactory::SoftBodyControlFactory ()
    : scfImplementationType (this)
  {
  }

  csPtr<iGenMeshAnimationControl> SoftBodyControlFactory::CreateAnimationControl
    (iMeshObject* mesh)
  {
    SoftBodyControl* control = new SoftBodyControl (mesh);
    return csPtr<iGenMeshAnimationControl> (control);
  }

  const char* SoftBodyControlFactory::Load (iDocumentNode* node)
  {
    return 0;
  }

  const char* SoftBodyControlFactory::Save (iDocumentNode* parent)
  {
    return 0;
  }

  //-------------------------- SoftBodyControl --------------------------

  CS_LEAKGUARD_IMPLEMENT(SoftBodyControl);

  SoftBodyControl::SoftBodyControl (iMeshObject* mesh)
    : scfImplementationType (this), mesh (mesh), lastTicks (0),
    meshPosition (0.0f)
  {
  }

  void SoftBodyControl::SetSoftBody (CS::Physics::Bullet::iSoftBody* body, bool doubleSided)
  {
    softBody = body;
    this->doubleSided = doubleSided;
    vertices.SetSize (softBody->GetVertexCount () * 2);
    normals.SetSize (softBody->GetVertexCount () * 2);

    // initialize the vertices and mesh position
    meshPosition.Set (0.0f);
    for (size_t i = 0; i < softBody->GetVertexCount (); i++)
      meshPosition += softBody->GetVertexPosition (i);
    meshPosition /= softBody->GetVertexCount ();

    Update (0, 0, 0);
  }

  CS::Physics::Bullet::iSoftBody* SoftBodyControl::GetSoftBody ()
  {
    return softBody;
  }

  bool SoftBodyControl::AnimatesColors () const
  {
    return false;
  }

  bool SoftBodyControl::AnimatesNormals () const
  {
    return true;
  }

  bool SoftBodyControl::AnimatesTexels () const
  {
    return false;
  }

  bool SoftBodyControl::AnimatesVertices () const
  {
    return true;
  }

  void SoftBodyControl::Update (csTicks current, int num_verts, uint32 version_id)
  {
    if (!softBody)
      return;

    // update the position of the vertices and compute the next position of the mesh
    csVector3 lastPosition = meshPosition;
    meshPosition.Set (0.0f);
    for (size_t i = 0; i < softBody->GetVertexCount (); i++)
    {
      csVector3 position = softBody->GetVertexPosition (i);
      vertices[i] = position - lastPosition;
      meshPosition += position;

      normals[i] = softBody->GetVertexNormal (i);

      if (doubleSided)
      {
	vertices[i + softBody->GetVertexCount ()] = vertices[i];
	normals[i + softBody->GetVertexCount ()] = - normals[i];
      }
    }
    meshPosition /= softBody->GetVertexCount ();

    // update the position of the mesh
    mesh->GetMeshWrapper ()->GetMovable ()->SetPosition (lastPosition);
    mesh->GetMeshWrapper ()->GetMovable ()->UpdateMove ();

    // invalidate the vertices of the factory of the mesh. This will cause the genmesh
    // to recompute its bounding box.
    csRef<iGeneralFactoryState> meshState =
      scfQueryInterface<iGeneralFactoryState> (mesh->GetFactory());
    meshState->Invalidate ();
  }

  const csColor4* SoftBodyControl::UpdateColors (csTicks current, const csColor4* colors,
						 int num_colors, uint32 version_id)
  {
    return colors;
  }

  const csVector3* SoftBodyControl::UpdateNormals (csTicks current, const csVector3* normals,
						   int num_normals, uint32 version_id)
  {
    if (!softBody)
      return normals;

    CS_ASSERT(doubleSided ? num_normals == 2 * (int) softBody->GetVertexCount ()
	      : num_normals == (int) softBody->GetVertexCount ());

    return this->normals.GetArray ();
  }

  const csVector2* SoftBodyControl::UpdateTexels (csTicks current, const csVector2* texels,
						  int num_texels, uint32 version_id)
  {
    return texels;
  }

  const csVector3* SoftBodyControl::UpdateVertices (csTicks current, const csVector3* verts,
						    int num_verts, uint32 version_id)
  {
    if (!softBody)
      return verts;

    CS_ASSERT(doubleSided ? num_verts == 2 * (int) softBody->GetVertexCount ()
	      : num_verts == (int) softBody->GetVertexCount ());

    return vertices.GetArray ();
  }

}
CS_PLUGIN_NAMESPACE_END(SoftAnim)
