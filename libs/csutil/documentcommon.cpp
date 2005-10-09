 /*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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
#include "csutil/documentcommon.h"
#include "csutil/documenthelper.h"

using namespace CrystalSpace;


csEmptyDocumentNodeIterator::csEmptyDocumentNodeIterator ()
  : scfImplementationType (this)
{
}

csEmptyDocumentNodeIterator::~csEmptyDocumentNodeIterator ()
{
}

//---------------------------------------------------------------------------

csEmptyDocumentAttributeIterator::csEmptyDocumentAttributeIterator ()
  : scfImplementationType (this)
{
}

csEmptyDocumentAttributeIterator::~csEmptyDocumentAttributeIterator ()
{  
}

//---------------------------------------------------------------------------

void csDocumentNodeCommon::SetValueAsInt (int value)
{
  csString s;
  s.Format ("%d", value);
  SetValue (s);
}

void csDocumentNodeCommon::SetValueAsFloat (float value)
{
  csString s;
  s.Format ("%g", value);
  SetValue (s);
}

csRef<iDocumentNodeIterator> csDocumentNodeCommon::GetNodes (const char* value)
{
  csRef<iDocumentNodeIterator> it = GetNodes();
  if (!it.IsValid()) return 0;
  return DocumentHelper::FilterDocumentNodeIterator (it, 
    DocumentHelper::NodeValueTest (value));
}

const char* csDocumentNodeCommon::GetContentsValue ()
{
  csRef<iDocumentNodeIterator> nodes = GetNodes();

  while (nodes->HasNext())
  {
    csRef<iDocumentNode> node = nodes->Next();
    if (node->GetType() == CS_NODE_TEXT)
      return node->GetValue();
  }
  return 0;
}

int csDocumentNodeCommon::GetContentsValueAsInt ()
{
  const char* v = GetContentsValue ();
  if (!v) return 0;
  int val = 0;
  sscanf (v, "%d", &val);
  return val;
}

float csDocumentNodeCommon::GetContentsValueAsFloat ()
{
  const char* v = GetContentsValue ();
  if (!v) return 0;
  float val = 0.0;
  sscanf (v, "%f", &val);
  return val;
}

const char* csDocumentNodeCommon::GetAttributeValue (const char* name)
{
  csRef<iDocumentAttribute> attr = GetAttribute (name);
  if (attr) return attr->GetValue();
  return 0;
}

int csDocumentNodeCommon::GetAttributeValueAsInt (const char* name)
{
  csRef<iDocumentAttribute> attr = GetAttribute (name);
  if (attr) return attr->GetValueAsInt();
  return 0;
}

float csDocumentNodeCommon::GetAttributeValueAsFloat (const char* name)
{
  csRef<iDocumentAttribute> attr = GetAttribute (name);
  if (attr) return attr->GetValueAsFloat();
  return 0;
}

bool csDocumentNodeCommon::GetAttributeValueAsBool (const char* name, 
						    bool defaultvalue)
{
  csRef<iDocumentAttribute> attr = GetAttribute (name);
  if (attr) return attr->GetValueAsBool();
  return defaultvalue;
}

void csDocumentNodeCommon::SetAttributeAsInt (const char* name, int value)
{
  csString s;
  s.Format ("%d", value);
  SetAttribute (name, s);
}

void csDocumentNodeCommon::SetAttributeAsFloat (const char* name, float value)
{
  csString s;
  s.Format ("%g", value);
  SetAttribute (name, s);
}

//---------------------------------------------------------------------------

int csDocumentAttributeCommon::GetValueAsInt ()
{
  const char* v = GetValue ();
  if (!v) return 0;
  int val = 0;
  sscanf (v, "%d", &val);
  return val;
}

float csDocumentAttributeCommon::GetValueAsFloat ()
{
  const char* v = GetValue ();
  if (!v) return 0;
  float val = 0;
  sscanf (v, "%f", &val);
  return val;
}

bool csDocumentAttributeCommon::GetValueAsBool ()
{
  const char* v = GetValue ();
  return ((strcasecmp (v, "true")==0) 
    || (strcasecmp(v, "yes")==0) 
    || (atoi(v)!=0));
}

void csDocumentAttributeCommon::SetValueAsInt (int v)
{
  csString s;
  s.Format ("%d", v);
  SetValue (s);
}

void csDocumentAttributeCommon::SetValueAsFloat (float f)
{
  csString s;
  s.Format ("%g", f);
  SetValue (s);
}

