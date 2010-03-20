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
