/*
    Copyright (C) 2000 by Thomas Heiber

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
#include "csengine/keyval.h"
#include "csengine/sector.h"
#include "csengine/engine.h"

//---------------------------------------------------------------------------

IMPLEMENT_CSOBJTYPE (csKeyValuePair,csObject);

IMPLEMENT_IBASE_EXT (csKeyValuePair)
  IMPLEMENTS_EMBEDDED_INTERFACE (iKeyValuePair)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csKeyValuePair::KeyValuePair)
  IMPLEMENTS_INTERFACE (iKeyValuePair)
IMPLEMENT_EMBEDDED_IBASE_END

csKeyValuePair::csKeyValuePair (const char* Key, const char* Value)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiKeyValuePair);
  SetName (Key);
  m_Value = strnew (Value);
}

csKeyValuePair::~csKeyValuePair ()
{
  delete [] m_Value;
}

void csKeyValuePair::SetValue (const char* value)
{
  delete[] m_Value;
  m_Value = strnew (value);
}

//---------------------------------------------------------------------------

IMPLEMENT_CSOBJTYPE (csMapNode,csObject);

IMPLEMENT_IBASE_EXT (csMapNode)
  IMPLEMENTS_EMBEDDED_INTERFACE (iMapNode)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csMapNode::MapNode)
  IMPLEMENTS_INTERFACE (iMapNode)
IMPLEMENT_EMBEDDED_IBASE_END

csMapNode::csMapNode (const char* Name) : m_Position (0, 0, 0)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiMapNode);
  SetName (Name);
}

csMapNode::~csMapNode ()
{
}

csMapNode* csMapNode::GetNode (csSector *pSector, const char* name,
  const char* classname)
{
  for (csNodeIterator Iter(pSector,classname); !Iter.IsFinished(); Iter.Next())
  {
    csMapNode *pNode = Iter.GetNode ();
    if (strcmp (pNode->GetName (), name) == 0)
      return pNode;
  }

  return NULL;
}

void csMapNode::MapNode::SetSector (iSector *pSector)
{
  scfParent->SetSector (pSector->GetPrivateObject ());
}

iSector* csMapNode::MapNode::GetSector () const
{
  iSector* sec = QUERY_INTERFACE (scfParent->GetSector (), iSector);
  sec->DecRef ();
  return sec;
}

//---------------------------------------------------------------------------

csNodeIterator::csNodeIterator (const csSector* pSector, const char* classname)
  : m_Iterator (csMapNode::Type, *pSector), m_Classname (classname),
    m_pCurrentNode (NULL)
{
  SkipWrongClassname ();
  if (!m_Iterator.IsFinished ())
    m_pCurrentNode = (csMapNode *)m_Iterator.GetObj ();
}
  
csNodeIterator::~csNodeIterator ()
{
}

void csNodeIterator::Reset (const csSector *pSector, const char *classname)
{
  m_Iterator.Reset (csMapNode::Type, *pSector);
  m_Classname = classname;
  SkipWrongClassname ();
  if (m_Iterator.IsFinished ())
    m_pCurrentNode = NULL;
  else
    m_pCurrentNode = (csMapNode *)m_Iterator.GetObj ();
}

csMapNode *csNodeIterator::GetNode ()
{
  return m_pCurrentNode;
}

void csNodeIterator::Next ()
{
  m_Iterator.Next ();
  SkipWrongClassname ();
  if (m_Iterator.IsFinished ())
    m_pCurrentNode = NULL;
  else
    m_pCurrentNode = (csMapNode *)m_Iterator.GetObj ();
}

bool csNodeIterator::IsFinished () const
{
  return m_Iterator.IsFinished ();
}

void csNodeIterator::SkipWrongClassname ()
{
  if (m_Classname)
    while (!m_Iterator.IsFinished ())
    {
      csMapNode *pNode = GetNode ();
      iKeyValuePair *KeyVal =
        GET_NAMED_CHILD_OBJECT_FAST (pNode, iKeyValuePair, "classname");
      if (!KeyVal || strcmp (KeyVal->GetValue (), m_Classname) != 0)
        m_Iterator.Next ();
      else
        return;
    }
}
