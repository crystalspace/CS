/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include "sysdef.h"
#include "csengine/keyval.h"


//---------------------------------------------------------------------------

IMPLEMENT_CSOBJTYPE (csKeyValuePair,csObject);

csKeyValuePair::csKeyValuePair(const char* Key, const char* Value)
{
  SetName(Key);
  m_Value = strnew (Value);
}

csKeyValuePair::~csKeyValuePair()
{
  delete [] m_Value;
}

//---------------------------------------------------------------------------

IMPLEMENT_CSOBJTYPE (csMapNode,csObject);

csMapNode::csMapNode(const char* Name)
  : m_Position(0,0,0), m_Angle(0)
{
  SetName(Name);
}

csMapNode::~csMapNode()
{
}

