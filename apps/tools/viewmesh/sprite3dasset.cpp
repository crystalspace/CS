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

#include "sprite3dasset.h"

#include "iengine/engine.h"
#include "iutil/objreg.h"
#include "iengine/mesh.h"
#include "csutil/scfstringarray.h"
#include "iengine/scenenode.h"
#include "iengine/movable.h"
#include "imesh/object.h"

#include "imesh/sprite3d.h"

bool Sprite3DAsset::Support(iMeshWrapper* mesh)
{
  csRef<iSprite3DState> x = scfQueryInterface<iSprite3DState> (mesh->GetMeshObject());
  return x.IsValid();
}

Sprite3DAsset::Sprite3DAsset(iObjectRegistry* obj_reg, iMeshWrapper* mesh)
  : AssetBase(obj_reg, mesh)
{
  state = scfQueryInterface<iSprite3DState> (mesh->GetMeshObject());
  sprite = scfQueryInterface<iSprite3DFactoryState> (mesh->GetFactory()->GetMeshObjectFactory());
}

Sprite3DAsset::~Sprite3DAsset()
{
  DetachAll();
  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  engine->RemoveObject(spritewrapper);
  engine->RemoveObject(spritewrapper->GetFactory());
  spritewrapper.Invalidate();
  state.Invalidate();
  sprite.Invalidate();
}

// Animations

bool Sprite3DAsset::SupportsAnimations() 
{ 
  return true; 
}

csPtr<iStringArray> Sprite3DAsset::GetAnimations()
{
  scfStringArray* arr = new scfStringArray;

  for (int i = 0; i < sprite->GetActionCount(); i++)
  {
    iSpriteAction* action = sprite->GetAction(i);
    if (!action) continue;

    csString s = action->GetName();
    arr->Push (s);
  }

  return csPtr<iStringArray>(arr);
}

bool Sprite3DAsset::PlayAnimation(const char* animationName, bool cycle)
{
  return state->SetAction(selectedAnimation, cycle);
}

bool Sprite3DAsset::StopAnimation(const char* animationName)
{
  ///TODO: How to stop an animation?
  return false;
}

bool Sprite3DAsset::GetReverseAction()
{
  return state->GetReverseAction();
}

void Sprite3DAsset::SetReverseAction(bool value)
{
  state->SetReverseAction(value);
}


// Sockets
bool Sprite3DAsset::SupportsSockets() 
{ 
  return true; 
}

csPtr<iStringArray> Sprite3DAsset::GetSockets()
{
  scfStringArray* arr = new scfStringArray;
  for (int i = 0; i < sprite->GetSocketCount(); i++)
  {
    iSpriteSocket* sock = sprite->GetSocket(i);
    if (!sock) continue;

    csString s = sock->GetName();
    arr->Push (s);
  }

  return csPtr<iStringArray>(arr);
}

bool Sprite3DAsset::AttachMesh(const char* socketName, iMeshWrapper* mesh)
{
  iSpriteSocket* sock = state->FindSocket(socketName);
  if (!sock) return false;


  if (mesh)
  {
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

bool Sprite3DAsset::AddSocket(const char* socketName)
{
  return false;
}

bool Sprite3DAsset::DeleteSocket(const char* socketName)
{
  return false;
}

SocketDescriptor Sprite3DAsset::GetSocketTransform(const char* socketName)
{
  SocketDescriptor desc;

  iSpriteSocket* sock = state->FindSocket(socketName);
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

bool Sprite3DAsset::SetSocketTransform(const char* socketName, const SocketDescriptor& desc)
{
  iSpriteSocket* sock = state->FindSocket(socketName);
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

bool Sprite3DAsset::DetachAll()
{
  for (int i = 0; i < sprite->GetSocketCount(); i++)
  {
    iSpriteSocket* sock = sprite->GetSocket(i);
    if (!sock) continue;

    AttachMesh(sock->GetName(), 0);
  }

  return true;
}
