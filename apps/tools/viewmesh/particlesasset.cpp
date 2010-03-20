/*
    Copyright (C) 2010 by Mike Gist

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
#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "imesh/particles.h"
#include "imesh/object.h"
#include "iutil/objreg.h"

#include "particlesasset.h"

bool ParticlesAsset::Support(iMeshWrapper* mesh)
{
  csRef<iParticleSystem> x = scfQueryInterface<iParticleSystem> (mesh->GetMeshObject());
  return x.IsValid();
}

ParticlesAsset::ParticlesAsset(iObjectRegistry* obj_reg, iMeshWrapper* mesh)
  : AssetBase(obj_reg, mesh)
{
  object = scfQueryInterface<iParticleSystem> (mesh->GetMeshObject());
  factory = scfQueryInterface<iParticleSystemFactory> (mesh->GetFactory()->GetMeshObjectFactory());
}

ParticlesAsset::~ParticlesAsset()
{
  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  engine->RemoveObject(spritewrapper);
  engine->RemoveObject(spritewrapper->GetFactory());
  spritewrapper.Invalidate();
  object.Invalidate();
  factory.Invalidate();
}

// Animations

bool ParticlesAsset::SupportsAnimations() 
{ 
  return false; 
}

csPtr<iStringArray> ParticlesAsset::GetAnimations()
{
  return 0;
}

bool ParticlesAsset::PlayAnimation(const char* animationName, bool cycle)
{
  return false;
}

bool ParticlesAsset::StopAnimation(const char* animationName)
{
  return false;
}

bool ParticlesAsset::GetReverseAction()
{
  return false;
}

void ParticlesAsset::SetReverseAction(bool value)
{
}


// Sockets
bool ParticlesAsset::SupportsSockets() 
{ 
  return false;
}

csPtr<iStringArray> ParticlesAsset::GetSockets()
{
  return 0;
}

bool ParticlesAsset::AttachMesh(const char* socketName, iMeshWrapper* mesh)
{
  return false;
}

bool ParticlesAsset::AddSocket(const char* socketName)
{
  return false;
}

bool ParticlesAsset::DeleteSocket(const char* socketName)
{
  return false;
}

SocketDescriptor ParticlesAsset::GetSocketTransform(const char* socketName)
{
  SocketDescriptor desc;
  return desc;
}

bool ParticlesAsset::SetSocketTransform(const char* socketName, const SocketDescriptor& desc)
{
  return false;
}

bool ParticlesAsset::DetachAll()
{
  return false;
}


// SubMeshes
bool ParticlesAsset::SupportsSubMeshes() 
{ 
  return false; 
}

csPtr<iStringArray> ParticlesAsset::GetSubMeshes()
{
  return 0;
}

bool ParticlesAsset::SetSubMeshRendering(const char* subMeshName, bool value)
{
  return false;
}

bool ParticlesAsset::SetSubMeshMaterial(const char* subMeshName, iMaterialWrapper* mat)
{
  return false;
}


// MorphTargets
bool ParticlesAsset::SupportsMorphTargets() 
{ 
  return false; 
}

csPtr<iStringArray> ParticlesAsset::GetMorphTargets()
{
  return 0;
}

float ParticlesAsset::GetMorphTargetWeight(const char* name)
{
  return 0.0f;
}

bool ParticlesAsset::SetMorphTargetWeight(const char* name, float value)
{
  return false;
}
