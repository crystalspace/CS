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
#include <csutil/scf.h>
#include <csutil/hashmap.h>
#include <csutil/strset.h>
#include <csutil/util.h>

#include "../ieffects/efdef.h"
#include "efdef.h"
#include "../ieffects/eftech.h"
#include "eftech.h"

SCF_IMPLEMENT_IBASE( csEffectDefinition )
  SCF_IMPLEMENTS_INTERFACE( iEffectDefinition )
SCF_IMPLEMENT_IBASE_END

csPtr<iEffectTechnique> csEffectDefinition::CreateTechnique()
{
  csEffectTechnique* techniqueobj = new csEffectTechnique();
  csRef<iEffectTechnique> technique 
    (SCF_QUERY_INTERFACE(techniqueobj, iEffectTechnique));
  techniques.Push (technique);
  return csPtr<iEffectTechnique> ((iEffectTechnique*)technique);
}

int csEffectDefinition::GetTechniqueCount()
{
  return techniques.Length();
}

iEffectTechnique* csEffectDefinition::GetTechnique( int technique )
{
  return techniques.Get (technique);
}

void csEffectDefinition::SetName( const char* name )
{
  techniquename = csStrNew( name );
}

const char* csEffectDefinition::GetName()
{
  return techniquename;
}

float csEffectDefinition::GetVariableFloat(int variableID)
{
  if ( (variableID < 0 ) || (variableID > variables.Length() ) )
    return 0.0f;

  if ( (variables[variableID])->type == CS_EFVARIABLETYPE_FLOAT )
    return (variables[variableID])->float_value;
  else
    return 0.0f;
}

csEffectVector4 csEffectDefinition::GetVariableVector4(int variableID)
{
  if ( (variableID < 0 ) || (variableID > variables.Length() ) )
    return csEffectVector4();

  if ( (variables[variableID])->type == CS_EFVARIABLETYPE_VECTOR4 )
    return (variables[variableID])->vector_value;
  else
    return csEffectVector4();
}

void csEffectDefinition::SetVariableFloat(int variableID, float value)
{
  if ( (variableID < 0 ) || (variableID > variables.Length() ) )
    return;

  if ( (variables[variableID])->type == CS_EFVARIABLETYPE_FLOAT )
    (variables[variableID])->float_value = value;
  else if ( (variables[variableID])->type == CS_EFVARIABLETYPE_UNDEFINED )
  {
    (variables[variableID])->float_value = value;
    (variables[variableID])->type = CS_EFVARIABLETYPE_FLOAT;
  }
}

void csEffectDefinition::SetVariableVector4(int variableID, csEffectVector4 value)
{
  if ( (variableID < 0 ) || (variableID > variables.Length() ) )
  return;

  if ( (variables[variableID])->type == CS_EFVARIABLETYPE_VECTOR4 )
    (variables[variableID])->vector_value = value;
  else if ( (variables[variableID])->type == CS_EFVARIABLETYPE_UNDEFINED )
  {
    (variables[variableID])->vector_value = value;
    (variables[variableID])->type = CS_EFVARIABLETYPE_VECTOR4;
  }
}

int csEffectDefinition::GetTopmostVariableID(int id)
{
  if ((id < 0) || (id > variables.Length() ) )
    return -1;

  int curid = id;
  int parent = (variables[curid])->point_to;
  while (parent >= 0)
  {
    curid = parent;
    parent = (variables[curid])->point_to;
  }
  return curid;
}

int csEffectDefinition::GetVariableID(csStringID string, bool create )
{
  for (int i = 0; i<variables.Length();i++)
  {
    if ( (variables[i])->id ==  string )
    {
      if( (variables[i])->point_to >= 0 )
        return GetTopmostVariableID( i );
      else
        return i;
    }
  }

  if (create)
  {
    variables.Push(new efvariable(string) );
    return variables.Length()-1;
  }
  else
    return -1;
}

//csBasicVector& csEffectDefinition::GetAllVariableNames()
//{
  //return variables;
//}

char csEffectDefinition::GetVariableType(int variableID)
{
  if ( (variableID < 0 ) || (variableID > variables.Length() ) )
    return CS_EFVARIABLETYPE_UNDEFINED;

  return (variables[variableID])->type;
}
