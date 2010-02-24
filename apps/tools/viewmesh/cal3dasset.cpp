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

#include "cal3dasset.h"

#include "iengine/engine.h"
#include "iutil/objreg.h"
#include "iengine/mesh.h"
#include "csutil/scfstringarray.h"
#include "iengine/scenenode.h"
#include "iengine/movable.h"
#include "imesh/object.h"

#include "imesh/spritecal3d.h"

bool Cal3DAsset::Support(iMeshWrapper* mesh)
{
  csRef<iSpriteCal3DState> x = scfQueryInterface<iSpriteCal3DState> (mesh->GetMeshObject());
  return x.IsValid();
}

Cal3DAsset::Cal3DAsset(iObjectRegistry* obj_reg, iMeshWrapper* mesh)
  : AssetBase(obj_reg, mesh)
{
  cal3dstate = scfQueryInterface<iSpriteCal3DState> (mesh->GetMeshObject());
  cal3dsprite = scfQueryInterface<iSpriteCal3DFactoryState> (mesh->GetFactory()->GetMeshObjectFactory());
}

Cal3DAsset::~Cal3DAsset()
{
  DetachAll();
  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  engine->RemoveObject(spritewrapper);
  engine->RemoveObject(spritewrapper->GetFactory());
  spritewrapper.Invalidate();
  cal3dstate.Invalidate();
  cal3dsprite.Invalidate();
}

// Animations

bool Cal3DAsset::SupportsAnimations() 
{ 
  return true; 
}

csPtr<iStringArray> Cal3DAsset::GetAnimations()
{
  scfStringArray* arr = new scfStringArray;

  for (int i = 0; i < cal3dstate->GetAnimCount(); i++)
  {
    const char* animname = cal3dstate->GetAnimName(i);
    if (!animname) continue;

    csString s = animname;
    arr->Push (s);
  }

  return csPtr<iStringArray>(arr);
}

bool Cal3DAsset::PlayAnimation(const char* animationName, bool cycle)
{
  int anim = cal3dstate->FindAnim(selectedAnimation);
  if (anim == -1) return false;
  if (cycle)
    cal3dstate->AddAnimCycle(anim,1,3);
  else
    cal3dstate->SetAnimAction(anim,1,1);

  return true;
}

bool Cal3DAsset::StopAnimation(const char* animationName)
{
  int anim = cal3dstate->FindAnim(selectedAnimation);
  if (anim == -1) return false;
  cal3dstate->ClearAnimCycle(anim,3);

  return true;
}

bool Cal3DAsset::GetReverseAction()
{
  return cal3dstate->GetAnimationTime()==-1;
}

void Cal3DAsset::SetReverseAction(bool value)
{
  cal3dstate->SetAnimationTime(value?-1:0);
}


// Sockets
bool Cal3DAsset::SupportsSockets() 
{ 
  return true; 
}

csPtr<iStringArray> Cal3DAsset::GetSockets()
{
  scfStringArray* arr = new scfStringArray;
  for (int i = 0; i < cal3dsprite->GetSocketCount(); i++)
  {
    iSpriteCal3DSocket* sock = cal3dsprite->GetSocket(i);
    if (!sock) continue;

    csString s = sock->GetName();
    arr->Push (s);
  }

  return csPtr<iStringArray>(arr);
}

bool Cal3DAsset::AttachMesh(const char* socketName, iMeshWrapper* mesh)
{
  iSpriteCal3DSocket* sock = cal3dstate->FindSocket(socketName);
  if (!sock) return false;


  if (mesh)
  {
    csReversibleTransform t;
    sock->SetTransform(t);
    mesh->QuerySceneNode ()->SetParent (spritewrapper->QuerySceneNode ());
    sock->SetMeshWrapper( mesh );
    spritewrapper->GetMovable()->UpdateMove();
  }
  else
  {
    csRef<iMeshWrapper> meshWrapOld;
    meshWrapOld = sock->GetMeshWrapper();;

    if (!meshWrapOld ) return false;

    meshWrapOld->QuerySceneNode ()->SetParent (0);
    sock->SetMeshWrapper( 0 );
    csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
    engine->RemoveObject(meshWrapOld);
    engine->RemoveObject(meshWrapOld->GetFactory());
  }

  return true;
}

bool Cal3DAsset::AddSocket(const char* socketName)
{
  return false;
}

bool Cal3DAsset::DeleteSocket(const char* socketName)
{
  return false;
}

SocketDescriptor Cal3DAsset::GetSocketTransform(const char* socketName)
{
  SocketDescriptor desc;

  iSpriteCal3DSocket* sock = cal3dstate->FindSocket(socketName);
  if (!sock) return desc;

/*
  csString str;

  desc["Name"] = TypeValue("String", sock->GetName());
  desc["Bone"] = TypeValue("String", str.Format("%d", sock->GetBone()).GetData());

  //csVector3 offset = sock->GetFactory()->GetTransform().GetOrigin();
  csVector3 offset = sock->GetTransform().GetOrigin();
  desc["Offset"] = TypeValue("Vector3", str.Format("%.2f, %.2f, %.2f", offset.x, offset.y, offset.z).GetData());

  //csVector3 rot = Decompose(sock->GetFactory()->GetTransform().GetO2T());
  csVector3 rot = Decompose(sock->GetTransform().GetO2T());
  desc["Rotation"] = TypeValue("Vector3", str.Format("%.2f, %.2f, %.2f", rot.x, rot.y, rot.z).GetData());
*/
  return desc;
}

bool Cal3DAsset::SetSocketTransform(const char* socketName, const SocketDescriptor& desc)
{
  iSpriteCal3DSocket* sock = cal3dstate->FindSocket(socketName);
  if (!sock) return false;
/*
  sock->GetFactory()->SetName(desc.find("Name")->second.second.c_str());

  int bone;
  sscanf(desc.find("Bone")->second.second.c_str(), "%d", &bone);
  sock->GetFactory()->SetBone(bone);

  csVector3 offset;
  if (sscanf(desc.find("Offset")->second.second.c_str(), "%f, %f, %f", &offset[0], &offset[1], &offset[2]) != 3)
    return false;
  //csReversibleTransform trans = sock->GetFactory()->GetTransform();
  csReversibleTransform trans = sock->GetTransform();
  trans.SetOrigin(offset);
  //sock->GetFactory()->SetTransform(trans);
  sock->SetTransform(trans);

  csVector3 rot;
  if (sscanf(desc.find("Rotation")->second.second.c_str(), "%f, %f, %f", &rot[0], &rot[1], &rot[2]) != 3)
    return false;
  //csReversibleTransform transr = sock->GetFactory()->GetTransform();
  csReversibleTransform transr = sock->GetTransform();
  csMatrix3 m;
  m *= csXRotMatrix3 (rot.x);
  m *= csYRotMatrix3 (rot.y);
  m *= csZRotMatrix3 (rot.z);
  transr.SetO2T(m);
  //sock->GetFactory()->SetTransform(transr);
  sock->SetTransform(transr);
*/
  return true;
}

bool Cal3DAsset::DetachAll()
{
  for (int i = 0; i < cal3dsprite->GetSocketCount(); i++)
  {
    iSpriteCal3DSocket* sock = cal3dsprite->GetSocket(i);
    if (!sock) continue;

    AttachMesh(sock->GetName(), 0);
  }

  return true;
}


// SubMeshes
bool Cal3DAsset::SupportsSubMeshes() 
{ 
  return true; 
}

csPtr<iStringArray> Cal3DAsset::GetSubMeshes()
{
  scfStringArray* arr = new scfStringArray;
  for(int i=0; i<cal3dsprite->GetMeshCount(); ++i)
  {
    const char* name = cal3dsprite->GetMeshName(i);
    if (!name) continue;
    csString s = name;
    arr->Push (s);
  }

  return csPtr<iStringArray>(arr);
}

bool Cal3DAsset::SetSubMeshRendering(const char* subMeshName, bool value)
{
  if (value)
    return cal3dstate->AttachCoreMesh(subMeshName);
  else
    return cal3dstate->DetachCoreMesh(subMeshName);
}

bool Cal3DAsset::SetSubMeshMaterial(const char* subMeshName, iMaterialWrapper* mat)
{
  return cal3dstate->SetMaterial(subMeshName, mat);
}


// MorphTargets
bool Cal3DAsset::SupportsMorphTargets() 
{ 
  return true; 
}

csPtr<iStringArray> Cal3DAsset::GetMorphTargets()
{
  scfStringArray* arr = new scfStringArray;
  for (int i = 0; i < cal3dsprite->GetMorphAnimationCount(); i++)
  {
    const char* morphname = cal3dsprite->GetMorphAnimationName(i);
    if (!morphname) continue;
    csString s = morphname;
    arr->Push (s);
  }

  return csPtr<iStringArray>(arr);
}

float Cal3DAsset::GetMorphTargetWeight(const char* name)
{
  ///TODO: There is no way to get the weight value?
  return 0.0f;
}

bool Cal3DAsset::SetMorphTargetWeight(const char* name, float value)
{
  int target = cal3dsprite->FindMorphAnimationName(name);
  if (target == -1) return false;

  return cal3dstate->BlendMorphTarget(target, value, /*delay*/ 0.0f);
}
