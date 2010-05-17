/*
    Copyright (C) 2009 by Jelle Hellemans

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

#include "genmeshasset.h"

#include "iengine/engine.h"
#include "iutil/objreg.h"
#include "iengine/mesh.h"
#include "csutil/scfstringarray.h"
#include "iengine/scenenode.h"
#include "iengine/movable.h"
#include "imesh/object.h"

#include "imesh/genmesh.h"
#include "imesh/skeleton.h"
#include "imesh/gmeshskel2.h"

bool GenmeshAsset::Support(iMeshWrapper* mesh)
{
  csRef<iGeneralMeshState> x = scfQueryInterface<iGeneralMeshState> (mesh->GetMeshObject());
  return x.IsValid();
}

GenmeshAsset::GenmeshAsset(iObjectRegistry* obj_reg, iMeshWrapper* mesh)
  : AssetBase(obj_reg, mesh)
{
  state = scfQueryInterface<iGeneralMeshState> (mesh->GetMeshObject());
  sprite = scfQueryInterface<iGeneralFactoryState> (mesh->GetFactory()->GetMeshObjectFactory());
  animcontrol = scfQueryInterfaceSafe<iGenMeshSkeletonControlState> (state->GetAnimationControl());
  if (animcontrol) skeleton = animcontrol->GetSkeleton ();
}

GenmeshAsset::~GenmeshAsset()
{
  DetachAll();
  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  engine->RemoveObject(spritewrapper);
  engine->RemoveObject(spritewrapper->GetFactory());
  spritewrapper.Invalidate();
  state.Invalidate();
  sprite.Invalidate();
  animcontrol.Invalidate();
  skeleton.Invalidate();
}

// Animations

bool GenmeshAsset::SupportsAnimations() 
{ 
  return skeleton.IsValid(); 
}

csPtr<iStringArray> GenmeshAsset::GetAnimations()
{
  scfStringArray* arr = new scfStringArray;
  if (!skeleton) return csPtr<iStringArray>(arr);

  for (size_t i = 0; i < skeleton->GetFactory()->GetAnimationsCount(); i++)
  {
    iSkeletonAnimation* action = skeleton->GetFactory()->GetAnimation(i);
    if (!action) continue;

    csString s = action->GetName();
    arr->Push (s);
  }

  return csPtr<iStringArray>(arr);
}

bool GenmeshAsset::PlayAnimation(const char* animationName, bool cycle)
{
  if (!skeleton) return false;
  iSkeletonAnimation* script = skeleton->Execute (animationName);
  if (script) script->SetLoop (cycle);
  return script != 0;
}

bool GenmeshAsset::StopAnimation(const char* animationName)
{
  if (!skeleton) return false;
  skeleton->StopAll ();
  return true;
}

bool GenmeshAsset::GetReverseAction()
{
  return false;
}

void GenmeshAsset::SetReverseAction(bool value)
{
}


// Sockets
bool GenmeshAsset::SupportsSockets() 
{ 
  return skeleton.IsValid(); 
}

csPtr<iStringArray> GenmeshAsset::GetSockets()
{
  if (!skeleton) return 0;
  scfStringArray* arr = new scfStringArray;

  for (size_t i = 0; i < skeleton->GetFactory()->GetSocketsCount(); i++)
  {
    iSkeletonSocketFactory* sock = skeleton->GetFactory()->GetSocket((int)i);
    if (!sock) continue;

    csString s = sock->GetName();
    arr->Push (s);
  }

  return csPtr<iStringArray>(arr);
}

bool GenmeshAsset::AttachMesh(const char* socketName, iMeshWrapper* mesh)
{
  iSkeletonSocket* sock = skeleton->FindSocket (socketName);
  if (!sock) return false;


  if (mesh)
  {
    mesh->QuerySceneNode ()->SetParent (spritewrapper->QuerySceneNode ());
    sock->SetSceneNode (mesh->QuerySceneNode ());
  }
  else
  {
    csRef<iMeshWrapper> meshWrapOld;
    if (sock->GetSceneNode ())
    meshWrapOld = sock->GetSceneNode ()->QueryMesh();

    if (!meshWrapOld ) return false;

    meshWrapOld->QuerySceneNode ()->SetParent (0);
    sock->SetSceneNode(0);
    csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
    engine->RemoveObject(meshWrapOld);
    engine->RemoveObject(meshWrapOld->GetFactory());
  }

  return true;
}

bool GenmeshAsset::AddSocket(const char* socketName)
{
  return false;
}

bool GenmeshAsset::DeleteSocket(const char* socketName)
{
  return false;
}

SocketDescriptor GenmeshAsset::GetSocketTransform(const char* socketName)
{
  SocketDescriptor desc;

  iSkeletonSocket* sock = skeleton->FindSocket (socketName);
  if (!sock) return desc;

  csString str;

  desc["Name"] = TypeValue("String", sock->GetName());
  iSkeletonBone* bone = sock->GetBone();
  desc["Bone"] = TypeValue("String", bone->GetName());

  csVector3 offset = sock->GetTransform().GetOrigin();
  desc["Offset"] = TypeValue("Vector3", str.Format("%.2f, %.2f, %.2f", offset.x, offset.y, offset.z).GetData());

  csVector3 rot = Decompose(sock->GetTransform().GetO2T());
  desc["Rotation"] = TypeValue("Vector3", str.Format("%.2f, %.2f, %.2f", rot.x, rot.y, rot.z).GetData());

  return desc;
}

bool GenmeshAsset::SetSocketTransform(const char* socketName, const SocketDescriptor& desc)
{
  iSkeletonSocket* sock = skeleton->FindSocket (socketName);
  if (!sock) return false;

  sock->GetFactory()->SetName(desc.find("Name")->second.second.c_str());

  iSkeletonBone* bone = skeleton->FindBone(desc.find("Bone")->second.second.c_str());
  if (bone) sock->SetBone(bone);

  csVector3 offset;
  if (sscanf(desc.find("Offset")->second.second.c_str(), "%f, %f, %f", &offset[0], &offset[1], &offset[2]) != 3)
    return false;
  csReversibleTransform trans = sock->GetTransform();
  trans.SetOrigin(offset);
  sock->SetTransform(trans);

  csVector3 rot;
  if (sscanf(desc.find("Rotation")->second.second.c_str(), "%f, %f, %f", &rot[0], &rot[1], &rot[2]) != 3)
    return false;
  csReversibleTransform transr = sock->GetTransform();
  csMatrix3 m;
  m *= csXRotMatrix3 (rot.x);
  m *= csYRotMatrix3 (rot.y);
  m *= csZRotMatrix3 (rot.z);
  transr.SetO2T(m);
  sock->SetTransform(transr);

  return true;
}

bool GenmeshAsset::DetachAll()
{
  if (!skeleton) return false;
  for (size_t i = 0; i < skeleton->GetFactory()->GetSocketsCount(); i++)
  {
    iSkeletonSocketFactory* sock = skeleton->GetFactory()->GetSocket((int)i);
    if (!sock) continue;

    AttachMesh(sock->GetName(), 0);
  }

  return true;
}


// SubMeshes
bool GenmeshAsset::SupportsSubMeshes() 
{ 
  return true; 
}

csPtr<iStringArray> GenmeshAsset::GetSubMeshes()
{
  scfStringArray* arr = new scfStringArray;
  if (!skeleton) return csPtr<iStringArray>(arr);

  for (size_t i = 0; i < sprite->GetSubMeshCount(); i++)
  {
    iGeneralMeshSubMesh* sm = sprite->GetSubMesh(i);
    if (!sm) continue;

    csString s = sm->GetName();
    arr->Push (s);
  }

  return csPtr<iStringArray>(arr);
}

bool GenmeshAsset::SetSubMeshRendering(const char* subMeshName, bool value)
{
  iGeneralMeshSubMesh* sm = state->FindSubMesh(subMeshName);
  if (!sm) return false;
  ///TODO: How to get the render state of a submesh?
  return false;
}

bool GenmeshAsset::SetSubMeshMaterial(const char* subMeshName, iMaterialWrapper* mat)
{
  iGeneralMeshSubMesh* sm = state->FindSubMesh(subMeshName);
  if (!sm) return false;
  sm->SetMaterial(mat);
  return true;
}
