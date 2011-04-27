 /*
  Copyright (C) 2007 by Jorrit Tyberghein
	            2007 by Frank Richter
                2007 by Marten Svanfeldt

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
#include "csutil/documenthelper.h"
#include "csutil/stringarray.h"

namespace CS
{
namespace DocSystem
{
  static csString FlattenNodeCommon (iDocumentNode* node, 
    csRef<iDocumentNodeIterator> children)
  {
    csString str;
    str.SetGrowsBy (0);
    str << int (node->GetType());
    str << ' ';
    str << node->GetValue ();
    csRef<iDocumentAttributeIterator> attrIter = node->GetAttributes ();
    if (attrIter)
    {
      csStringArray attrStrs;
      while (attrIter->HasNext())
      {
	csRef<iDocumentAttribute> attr = attrIter->Next();
	csString str;
	str << attr->GetName () << '=' << attr->GetValue() << ',';
	attrStrs.Push (str);
      }
      str << '[';
      attrStrs.Sort (true);
      for (size_t i = 0; i < attrStrs.GetSize(); i++)
        str << attrStrs[i]; 
      str << ']';
    }
    str << '(';
    if (children)
    {
      while (children->HasNext ())
      {
	  csRef<iDocumentNode> child = children->Next ();
	  str << FlattenNode (child);
	  str << ',';
      }
    }
    str << ')';
    
    return str;
  }

  csString FlattenNode (iDocumentNode* node)
  {
    return FlattenNodeCommon (node, node->GetNodes ());
  }

  csString FlattenNodeShallow (iDocumentNode* node)
  {
    // Ignore children, that's what the 'shallow' part is about ...
    return FlattenNodeCommon (node, 0);
  }
  
  csPtr<iDocument> MakeChangeable (iDocument* doc, iDocumentSystem* docsys)
  {
    csRef<iDocument> newDoc;
    
    int changeable = doc->Changeable ();
    if (changeable == CS_CHANGEABLE_YES)
      newDoc = doc;
    else
    {
      if (changeable == CS_CHANGEABLE_NEWROOT)
	newDoc = doc;
      else
	newDoc = docsys->CreateDocument ();

      csRef<iDocumentNode> oldRoot (doc->GetRoot ());
      csRef<iDocumentNode> newRoot (newDoc->CreateRoot ());
      CS::DocSystem::CloneNode (oldRoot, newRoot);
    }
    
    return csPtr<iDocument> (newDoc);
  }
  
  bool SetContentsValue (iDocumentNode* node, const char* contents)
  {
    if (node->GetType() != CS_NODE_ELEMENT) return false;
    
    csRef<iDocumentNode> textNode;
    {
      csRef<iDocumentNodeIterator> children (node->GetNodes());
      while (children->HasNext())
      {
	csRef<iDocumentNode> child (children->Next());
	if (child->GetType() == CS_NODE_TEXT)
	{
	  textNode = child;
	  break;
	}
      }
    }
    if (!textNode)
    {
      textNode = node->CreateNodeBefore (CS_NODE_TEXT);
      if (!textNode) return false;
    }
    textNode->SetValue (contents);
    return true;
  }
} // namespace DocSystem
} // namespace CS
