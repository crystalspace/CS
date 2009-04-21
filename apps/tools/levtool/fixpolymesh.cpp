/*
    Copyright (C) 2007 by Frank Richter

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

#include "iutil/document.h"

#include "fixpolymesh.h"

FixPolyMesh::FixPolyMesh ()
{
  InitTokenTable (xmltokens);
}

void FixPolyMesh::HandlePolyMeshContainer (iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> nodes = node->GetNodes ();

  while (nodes->HasNext ())
  {
    csRef<iDocumentNode> child = nodes->Next ();
    if (child->GetType() != CS_NODE_ELEMENT) continue;

    csStringID id = xmltokens.Request (child->GetValue ());
    switch (id)
    {
      case XMLTOKEN_MESHFACT:
      case XMLTOKEN_MESHOBJ:
      case XMLTOKEN_SECTOR:
        HandlePolyMeshContainer (child);
        break;
      case XMLTOKEN_POLYMESH:
        HandlePolyMesh (child);
        break;
      default:
        break;
    }
  }
}

void FixPolyMesh::HandlePolyMesh (iDocumentNode* node)
{
  node->SetValue ("trimesh");

  csRef<iDocumentNodeIterator> nodes = node->GetNodes ();

  while (nodes->HasNext ())
  {
    csRef<iDocumentNode> child = nodes->Next ();
    if (child->GetType() != CS_NODE_ELEMENT) continue;

    csStringID id = xmltokens.Request (child->GetValue ());
    switch (id)
    {
      case XMLTOKEN_VISCULL:
      case XMLTOKEN_SHADOWS:
      case XMLTOKEN_COLLDET:
        {
          csRef<iDocumentNode> contentNode = 
            child->CreateNodeBefore (CS_NODE_TEXT);
          contentNode->SetValue (child->GetValue ());
          child->SetValue ("id");
        }
      default:
        break;
    }
  }
}

void FixPolyMesh::Fix (iDocumentNode* root)
{
  csRef<iDocumentNodeIterator> nodes = root->GetNodes ();

  while (nodes->HasNext ())
  {
    csRef<iDocumentNode> child = nodes->Next ();
    if (child->GetType() != CS_NODE_ELEMENT) continue;

    HandlePolyMeshContainer (child);
  }
}
