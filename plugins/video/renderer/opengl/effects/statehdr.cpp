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
#include "csutil/hashmap.h"
#include "csutil/strset.h"

#include "statehdr.h"

csStateHandler::csStateHandler()
{
  // Hardcoded hash size. Maybe it should be definable?
  // --Anders Stenberg
  states = new csHashMap( 31 );
  iterator = 0;
}

csStateHandler::~csStateHandler()
{
  if (iterator != 0)
    delete iterator;

  csGlobalHashIterator cIterator (states);
  while (cIterator.HasNext())
  {
    delete (statedata*)cIterator.Next();
  }
  
  states->DeleteAll();
  delete states;
}

void csStateHandler::SetStateFloat( csStringID state, float value )
{
  csHashIterator cIterator( states, state );
  if( cIterator.HasNext() )
  {
    statedata* data = (statedata*)cIterator.Next();
    if( data->type == CS_STATETYPE_FLOAT )
      data->float_value = value;
    return;
  }

  states->Put( state, new statedata( state, value ) );
}

void csStateHandler::SetStateString( csStringID state, csStringID value )
{
  csHashIterator cIterator( states, state );

  if( cIterator.HasNext() )
  {
    statedata* data = (statedata*)cIterator.Next();
    if( data->type == CS_STATETYPE_STRING )
      data->string_value = value;
    return;
  }
  states->Put( state, new statedata( state, value ) );
}

void csStateHandler::SetStateOpaque( csStringID state, void *value )
{
  csHashIterator cIterator( states, state );
  
  if( cIterator.HasNext() )
  {
    statedata* data = (statedata*)cIterator.Next();
    if( data->type == CS_STATETYPE_OPAQUE )
      data->opaque_value = value;
    return;
  }
  states->Put( state, new statedata( state, value ) );
}

void csStateHandler::SetStateVector4( csStringID state, csEffectVector4 value)
{
  csHashIterator cIterator( states, state );
  if( cIterator.HasNext() )
  {
    statedata* data = (statedata*)cIterator.Next();
    if( data->type == CS_STATETYPE_VECTOR4 )
      data->vector_value = value;
    return;
  }
  states->Put( state, new statedata( state, value ) );
}

float csStateHandler::GetStateFloat( csStringID state )
{
  csHashIterator cIterator( states, state );
  if( cIterator.HasNext() )
  {
    statedata* data = (statedata*)cIterator.Next();
    if( data->type == CS_STATETYPE_FLOAT )
      return data->float_value;
  }
  return 0;
}

csStringID csStateHandler::GetStateString( csStringID state )
{
  csHashIterator cIterator( states, state );
  if( cIterator.HasNext() )
  {
    statedata* data = (statedata*)cIterator.Next();
    if( data->type == CS_STATETYPE_STRING )
      return data->string_value;
  }
  return csInvalidStringID;
}

void *csStateHandler::GetStateOpaque( csStringID state )
{
  csHashIterator cIterator( states, state );
  if( cIterator.HasNext() )
  {
    statedata* data = (statedata*)cIterator.Next();
    if( data->type == CS_STATETYPE_OPAQUE )
      return data->opaque_value;
  }
  return 0;
}

csEffectVector4 csStateHandler::GetStateVector4(csStringID state)
{
  csHashIterator cIterator( states, state );
  if( cIterator.HasNext() )
  {
    statedata* data = (statedata*)cIterator.Next();
    if( data->name == state )
    {
      if( data->type == CS_STATETYPE_VECTOR4 )
        return data->vector_value;
    }
  }
  return csEffectVector4();
}

csStringID csStateHandler::GetFirstState()
{
  if (iterator != 0)
    delete iterator;

  iterator = new csGlobalHashIterator (states);
  if (iterator->HasNext())
    return ((statedata*)(iterator->Next()))->name;
  return csInvalidStringID;
}

csStringID csStateHandler::GetNextState()
{
  if( iterator == 0 )
    return csInvalidStringID;

  if( iterator->HasNext() )
    return ((statedata*)(iterator->Next()))->name;
  return csInvalidStringID;
}
