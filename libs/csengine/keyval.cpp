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
#include "csengine/engine.h"
#include "iengine/sector.h"

//---------------------------------------------------------------------------

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

const char *csKeyValuePair::GetKey () const
{
  return GetName ();
}

void csKeyValuePair::SetKey (const char *s)
{
  SetName (s);
}

const char *csKeyValuePair::GetValue () const
{
  return m_Value;
}

void csKeyValuePair::SetValue (const char* value)
{
  delete[] m_Value;
  m_Value = strnew (value);
}

//---------------------------------------------------------------------------

IMPLEMENT_IBASE_EXT (csMapNode)
  IMPLEMENTS_EMBEDDED_INTERFACE (iMapNode)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csMapNode::MapNode)
  IMPLEMENTS_INTERFACE (iMapNode)
IMPLEMENT_EMBEDDED_IBASE_END

csMapNode::csMapNode (const char* Name) : m_Position (0, 0, 0), m_pSector (NULL)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiMapNode);
  SetName (Name);
}

csMapNode::~csMapNode ()
{
}

void csMapNode::SetPosition (const csVector3& pos)
{
  m_Position = pos;
}

const csVector3& csMapNode::GetPosition () const
{
  return m_Position;
}

iSector *csMapNode::GetSector () const
{
  return m_pSector;
}

void csMapNode::SetSector (iSector *pSector)
{
  if (m_pSector) m_pSector->QueryObject ()->ObjRemove (this);
  m_pSector = pSector;
  if (m_pSector) m_pSector->QueryObject ()->ObjAdd (this);
}

iMapNode* csMapNode::GetNode (iSector *pSector, const char* name,
  const char* classname)
{
  for (csNodeIterator Iter(pSector,classname); !Iter.IsFinished(); Iter.Next())
  {
    iMapNode *pNode = Iter.GetNode ();
    if (strcmp (pNode->QueryObject ()->GetName (), name) == 0)
      return pNode;
  }

  return NULL;
}

//---------------------------------------------------------------------------

csNodeIterator::csNodeIterator (iSector* pSector, const char* classname)
  : Iterator (NULL), Classname (classname), CurrentNode (NULL)
{
  Reset (pSector, Classname);
}
  
csNodeIterator::~csNodeIterator ()
{
  if (CurrentNode) CurrentNode->DecRef ();
  if (Iterator) Iterator->DecRef ();
}

void csNodeIterator::Reset (iSector *pSector, const char *classname)
{
  if (CurrentNode) CurrentNode->DecRef ();
  if (Iterator) Iterator->DecRef ();

  Iterator = pSector->QueryObject ()->GetIterator ();
  Classname = classname;
  CurrentNode = QUERY_INTERFACE_FAST (Iterator->GetObject (), iMapNode);

  SkipWrongClassname ();
}

iMapNode *csNodeIterator::GetNode ()
{
  return CurrentNode;
}

void csNodeIterator::Next ()
{
  NextNode ();
  SkipWrongClassname ();
}

bool csNodeIterator::IsFinished () const
{
  return Iterator->IsFinished ();
}

void csNodeIterator::SkipWrongClassname ()
{
  if (Classname)
    while (!Iterator->IsFinished ())
    {
      iKeyValuePair *KeyVal = GET_NAMED_CHILD_OBJECT_FAST
        (CurrentNode->QueryObject (), iKeyValuePair, "classname");
      if (KeyVal)
      {
        bool done = !strcmp (KeyVal->GetValue (), Classname);
	KeyVal->DecRef ();
	if (done) return;
      }
      NextNode ();
    }
}

void csNodeIterator::NextNode ()
{
  if (CurrentNode) CurrentNode->DecRef ();
  Iterator->Next ();
  if (Iterator->IsFinished ())
    CurrentNode = NULL;
  else
    CurrentNode = QUERY_INTERFACE_FAST (Iterator->GetObject (), iMapNode);
}
