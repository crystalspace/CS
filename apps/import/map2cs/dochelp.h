/*
    Document system helper routines
    Copyright (C) 2003 by Jorrit Tyberghein
              (C) 2003 by Frank Richter

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

#ifndef __DOCHELP_H__
#define __DOCHELP_H__

#include "iutil/document.h"

// Helper type for often used stuff
typedef csRef<iDocumentNode> DocNode;

static inline DocNode CreateNode (DocNode parent, const char* name, 
				  const char* content = 0)
{
  DocNode temp = parent->CreateNodeBefore (CS_NODE_ELEMENT);
  temp->SetValue (name);
  if (content)
  {
    DocNode cont = temp->CreateNodeBefore (CS_NODE_TEXT);
    cont->SetValue (content);
  }
  return temp;
}

static inline DocNode CreateNode (DocNode parent, const char* name, 
				  float content)
{
  DocNode temp = parent->CreateNodeBefore (CS_NODE_ELEMENT);
  temp->SetValue (name);
  DocNode cont = temp->CreateNodeBefore (CS_NODE_TEXT);
  cont->SetValueAsFloat (content);
  return temp;
}

static inline DocNode CreateNode (DocNode parent, const char* name, 
				  int content)
{
  DocNode temp = parent->CreateNodeBefore (CS_NODE_ELEMENT);
  temp->SetValue (name);
  DocNode cont = temp->CreateNodeBefore (CS_NODE_TEXT);
  cont->SetValueAsInt (content);
  return temp;
}

#endif

