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

#include "../ieffects/efpass.h"
#include "efpass.h"
#include "../ieffects/eflayer.h"
#include "eflayer.h"

csEffectPass::csEffectPass()
{
  SCF_CONSTRUCT_IBASE(0);
}

csEffectPass::~csEffectPass ()
{
  SCF_DESTRUCT_IBASE();
}

csPtr<iEffectLayer> csEffectPass::CreateLayer()
{
  csRef<iEffectLayer> layer = 
    csPtr<iEffectLayer> (new csEffectLayer());
  layers.Push (layer);
  return csPtr<iEffectLayer> (layer);
}

int csEffectPass::GetLayerCount()
{
  return layers.Length();
}

iEffectLayer* csEffectPass::GetLayer(int layer)
{
  return layers.Get (layer);
}

iBase* csEffectPass::GetRendererData ()
{
  return rendererData;
}

void csEffectPass::SetRendererData (iBase* data)
{
  rendererData = data;
}

SCF_IMPLEMENT_IBASE(csEffectPass)
  SCF_IMPLEMENTS_INTERFACE (iEffectPass)
SCF_IMPLEMENT_IBASE_END
