/*
    Copyright (C) 2002 by Anders Stenberg
    Written by Anders Stenberg

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
#include "cstypes.h"
#include "csutil/scf.h"
#include "csutil/objreg.h"
#include "csutil/strset.h"
#include "ivideo/graph3d.h"

#include "../ieffects/efclient.h"
#include "../ieffects/efserver.h"
#include "efserver.h"
#include "../ieffects/efdef.h"
#include "efdef.h"
#include "../ieffects/eftech.h"
#include "eftech.h"

csEffectServer::csEffectServer(iBase* parent)
{
  SCF_CONSTRUCT_IBASE(parent);
  seqnr = 0;
  
  efstrings = new csEffectStrings();
  efstrings->InitStrings(this);
}

csEffectServer::~csEffectServer () 
{
  delete efstrings;
  SCF_DESTRUCT_IBASE();
}

bool csEffectServer::Initialize(iObjectRegistry* reg)
{
  objectreg = reg;
  return true;
}

csPtr<iEffectDefinition> csEffectServer::CreateEffect()
{
  csEffectDefinition* effectobj = new csEffectDefinition();

  csRef<iEffectDefinition> effect = 
    SCF_QUERY_INTERFACE(effectobj, iEffectDefinition);

  char name[17];
  sprintf(name, "effect%d", seqnr);
  seqnr++;
  effect->SetName(name);

  effects.Push(effect);
  return csPtr<iEffectDefinition> ((iEffectDefinition*)effect);
}

bool csEffectServer::Validate(iEffectDefinition* effect)
{
  csRef<iGraphics3D> g3d (CS_QUERY_REGISTRY(objectreg, iGraphics3D));
  if (g3d)
  {
    csRef<iEffectClient> client (SCF_QUERY_INTERFACE(g3d, iEffectClient));
    if (client)
    {
      bool valideffect = false;
      for (int i=0; i<effect->GetTechniqueCount(); i++)
        if (client->Validate(effect, effect->GetTechnique(i)))
        {
          effect->GetTechnique(i)->SetValidation(CS_TECHNIQUE_PASSED);
          valideffect = true;
        } else effect->GetTechnique(i)->SetValidation(CS_TECHNIQUE_FAILED);
      return valideffect;
    }
  }
  return false;
}

iEffectTechnique* csEffectServer::SelectAppropriateTechnique(
	iEffectDefinition* effect)
{
  if (!effect)
    return 0;

  float maxquality = -1;
  csRef<iEffectTechnique> tech = 0;
  for (int i=0; i<effect->GetTechniqueCount(); i++)
    if ((effect->GetTechnique(i)->GetValidation() == CS_TECHNIQUE_PASSED) && 
        (effect->GetTechnique(i)->GetQuality()>maxquality))
    {
      maxquality = effect->GetTechnique(i)->GetQuality();
      tech = effect->GetTechnique(i);
    }
  return tech;
}

iEffectDefinition* csEffectServer::GetEffect(const char* name)
{
  for (size_t i = 0; i < effects.Length(); i++)
  {
    if (strcasecmp(name, effects.Get(i)->GetName()) == 0)
      //is this, return it
      return effects.Get (i);
  }
  return 0;
}

csStringID csEffectServer::RequestString(const char *s)
{
  return strset.Request(s);
}

const char* csEffectServer::RequestString(csStringID id)
{
  return strset.Request(id);
}

// Plugin part

SCF_IMPLEMENT_IBASE(csEffectServer)
  SCF_IMPLEMENTS_INTERFACE(iEffectServer)
  SCF_IMPLEMENTS_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END


