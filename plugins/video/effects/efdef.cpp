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

#include <cssysdef.h>
#include <cstypes.h>
#include <csutil/csvector.h>
#include <csutil/scf.h>
#include <csutil/hashmap.h>
#include <csutil/strset.h>
#include <csutil/util.h>

#include "ivideo/effects/efdef.h"
#include "efdef.h"
#include "ivideo/effects/eftech.h"
#include "eftech.h"

iEffectTechnique* csEffectDefinition::CreateTechnique()
{
  csEffectTechnique* techniqueobj = new csEffectTechnique();
  iEffectTechnique* technique = SCF_QUERY_INTERFACE( techniqueobj, iEffectTechnique );
  techniques.Push( technique );
  return technique;
}

int csEffectDefinition::GetTechniqueCount()
{
  return techniques.Length();
}

iEffectTechnique* csEffectDefinition::GetTechnique( int technique )
{
  return (iEffectTechnique*)(techniques.Get( technique ));
}

void csEffectDefinition::SetName( const char* name )
{
  techniquename = csStrNew( name );
}

const char* csEffectDefinition::GetName()
{
  return techniquename;
}
