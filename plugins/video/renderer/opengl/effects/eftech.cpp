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
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/hashmap.h"
#include "csutil/strset.h"

#include "../ieffects/eftech.h"
#include "eftech.h"
#include "../ieffects/efpass.h"
#include "efpass.h"

csEffectTechnique::csEffectTechnique()
{
  SCF_CONSTRUCT_IBASE(0);
  valid = CS_TECHNIQUE_NOTVALIDATED;
  quality = 0;
  clientflags = 0;
}

csEffectTechnique::~csEffectTechnique ()
{
  SCF_DESTRUCT_IBASE();
}

csPtr<iEffectPass> csEffectTechnique::CreatePass()
{
  csRef <iEffectPass> pass = csPtr<iEffectPass> (new csEffectPass());
  passes.Push(pass);
  return csPtr<iEffectPass> (pass);
}

int csEffectTechnique::GetPassCount()
{
  return passes.Length();
}

iEffectPass* csEffectTechnique::GetPass (int passnumber)
{
  return passes.Get (passnumber);
}

void csEffectTechnique::SetValidation (int validation)
{
  valid = validation;
}

int csEffectTechnique::GetValidation()
{
  return valid;
}

void csEffectTechnique::SetQuality(float q)
{
  quality = q;
}

float csEffectTechnique::GetQuality()
{
  return quality;
}

void csEffectTechnique::SetClientFlags(uint32 flags)
{
  clientflags = flags;
}

uint32 csEffectTechnique::GetClientFlags()
{
  return clientflags;
}

SCF_IMPLEMENT_IBASE(csEffectTechnique)
  SCF_IMPLEMENTS_INTERFACE(iEffectTechnique)
SCF_IMPLEMENT_IBASE_END
