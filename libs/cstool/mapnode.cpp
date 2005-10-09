/*
    Copyright (C) 2001 by Jorrit Tyberghein
    Copyright (C) 2000 by Thomas Hieber

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
#include "cstool/mapnode.h"
#include "iengine/sector.h"
#include "ivaria/keyval.h"

//---------------------------------------------------------------------------


csMapNode::csMapNode (const char* Name) :
  scfImplementationType (this),
  sector (0), position (0, 0, 0), xvector (1, 0, 0), yvector (0, 1, 0),
  zvector (0, 0, 1)
{
  SetName (Name);
}

csMapNode::~csMapNode ()
{
}

void csMapNode::SetSector (iSector *pSector)
{
  if (sector) sector->QueryObject ()->ObjRemove (this);
  sector = pSector;
  if (sector) sector->QueryObject ()->ObjAdd (this);
}

iMapNode* csMapNode::GetNode (iSector *pSector, const char* name,
  const char* classname)
{
  csNodeIterator Iter (pSector,classname);
  while (Iter.HasNext ())
  {
    iMapNode *pNode = Iter.Next ();
    if (strcmp (pNode->QueryObject ()->GetName (), name) == 0)
      return pNode;
  }

  return 0;
}

//---------------------------------------------------------------------------

csNodeIterator::csNodeIterator (iSector* pSector, const char* classname)
  : Classname (classname)
{
  Reset (pSector, Classname);
}

csNodeIterator::~csNodeIterator ()
{
}

void csNodeIterator::Reset (iSector *pSector, const char *classname)
{
  Iterator = pSector->QueryObject ()->GetIterator ();
  Classname = classname;
  iObject* n = Iterator->Next ();
  if (n)
    CurrentNode = SCF_QUERY_INTERFACE (n, iMapNode);
  else
    CurrentNode = 0;

  SkipWrongClassname ();
}

iMapNode* csNodeIterator::Next ()
{
  iMapNode* c = CurrentNode;
  NextNode ();
  SkipWrongClassname ();
  return c;
}

bool csNodeIterator::HasNext () const
{
  return CurrentNode != 0;
}

void csNodeIterator::SkipWrongClassname ()
{
  if (Classname)
    while (Iterator->HasNext ())
    {
      csRef<iKeyValuePair> KeyVal (CS_GET_NAMED_CHILD_OBJECT
        (CurrentNode->QueryObject (), iKeyValuePair, "classname"));
      if (KeyVal)
      {
        bool done = !strcmp (KeyVal->GetValue (), Classname);
	if (done) return;
      }
      NextNode ();
    }
}

void csNodeIterator::NextNode ()
{
  iObject* obj = Iterator->Next ();
  if (obj)
    CurrentNode = SCF_QUERY_INTERFACE (obj, iMapNode);
  else
    CurrentNode = 0;
}

