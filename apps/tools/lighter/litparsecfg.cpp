/*
    Copyright (C) 2004 by Jorrit Tyberghein

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
#include "csutil/xmltiny.h"
#include "litparsecfg.h"
#include "lighter.h"

litConfigParser::litConfigParser (Lighter* lighter, iObjectRegistry* object_reg)
{
  litConfigParser::lighter = lighter;
  litConfigParser::object_reg = object_reg;
  InitTokenTable (xmltokens);
}

csRef<iDocumentNode> litConfigParser::FindChildNode (iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    return child;
  }
  return 0;
}

bool litConfigParser::ParseMulti (iDocumentNode* multi_node,
    litObjectSelectChildren* objsel)
{
  csRef<iDocumentNodeIterator> it = multi_node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csRef<litObjectSelect> a;
    if (!ParseObjectSelect (child, a))
      return false;
    objsel->AddMeshSelect (a);
  }
  return true;
}

bool litConfigParser::ParseObjectSelect (iDocumentNode* objsel_node,
    csRef<litObjectSelect>& objsel)
{
  if (objsel_node == 0)
    return lighter->Report ("Missing selector node!");
  else
  {
    const char* value = objsel_node->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_AND:
	objsel.AttachNew (new litObjectSelectAnd ());
	if (!ParseMulti (objsel_node,
		(litObjectSelectChildren*)(litObjectSelect*)objsel))
	  return false;
	break;
      case XMLTOKEN_OR:
	objsel.AttachNew (new litObjectSelectOr ());
	if (!ParseMulti (objsel_node,
		(litObjectSelectChildren*)(litObjectSelect*)objsel))
	  return false;
	break;
      case XMLTOKEN_NOT:
	{
	  csRef<litObjectSelect> a;
	  if (!ParseObjectSelect (FindChildNode (objsel_node), a))
	    return false;
	  objsel.AttachNew (new litObjectSelectNot (a));
	}
	break;
      case XMLTOKEN_KEY:
        {
	  csRef<iDocumentAttribute> attr = objsel_node->GetAttribute ("regex");
	  if (attr)
	  {
	    // Regex version.
	    objsel.AttachNew (new litObjectSelectByKeyValueRE (
	    	objsel_node->GetAttributeValue ("name"),
	    	objsel_node->GetAttributeValue ("attr"),
	    	objsel_node->GetAttributeValue ("regex")));
	  }
	  else
	  {
	    // Normal version.
	    objsel.AttachNew (new litObjectSelectByKeyValue (
	    	objsel_node->GetAttributeValue ("name"),
	    	objsel_node->GetAttributeValue ("attr"),
	    	objsel_node->GetAttributeValue ("value")));
	  }
	}
        break;
      case XMLTOKEN_NONE:
	objsel.AttachNew (new litObjectSelectNone ());
	break;
      case XMLTOKEN_ALL:
	objsel.AttachNew (new litObjectSelectAll ());
	break;
      case XMLTOKEN_STATICPOS:
	objsel.AttachNew (new litObjectSelectByMOFlags (CS_MESH_STATICPOS,
	      CS_MESH_STATICPOS));
	break;
      case XMLTOKEN_STATICSHAPE:
	objsel.AttachNew (new litObjectSelectByMOFlags (CS_MESH_STATICSHAPE,
	      CS_MESH_STATICSHAPE));
	break;
      case XMLTOKEN_STATIC:
	objsel.AttachNew (new litObjectSelectByMOFlags (
	      CS_MESH_STATICSHAPE|CS_MESH_STATICPOS,
	      CS_MESH_STATICSHAPE|CS_MESH_STATICPOS));
	break;
      case XMLTOKEN_NAME:
	objsel.AttachNew (new litObjectSelectByName (
		objsel_node->GetContentsValue ()));
	break;
      case XMLTOKEN_REGEXP:
	objsel.AttachNew (new litObjectSelectByNameRE (
		objsel_node->GetContentsValue ()));
	break;
      case XMLTOKEN_TYPE:
	objsel.AttachNew (new litObjectSelectByType (
		objsel_node->GetContentsValue ()));
	break;
      default:
        return lighter->Report ("Unknown token <%s> in selector!", value);
    }
    const char* err = objsel->IsValid ();
    if (err)
      return lighter->Report ("Error in selector: '%s'!", err);
    return true;
  }
  CS_ASSERT (false);
  return lighter->Report ("Missing selector!");
}

bool litConfigParser::ParseLighter (iDocumentNode* lighter_node,
	litConfig& litconfig)
{
  csRef<iDocumentNodeIterator> it = lighter_node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_SELECT_PORTALS:
        if (!ParseObjectSelect (FindChildNode (child),
		litconfig.portals_selector))
	  return false;
        break;
      case XMLTOKEN_SELECT_SECTORS:
        if (!ParseObjectSelect (FindChildNode (child),
		litconfig.sectors_selector))
	  return false;
        break;
      case XMLTOKEN_SELECT_LIGHTS:
        if (!ParseObjectSelect (FindChildNode (child),
		litconfig.lights_selector))
	  return false;
        break;
      case XMLTOKEN_SELECT_CASTERS:
        if (!ParseObjectSelect (FindChildNode (child),
		litconfig.casters_selector))
	  return false;
        break;
      case XMLTOKEN_SELECT_RECEIVERS:
        if (!ParseObjectSelect (FindChildNode (child),
		litconfig.receivers_selector))
	  return false;
        break;
      default:
        return lighter->Report ("Unknown token <%s> in <lighter>!", value);
    }
  }
  return true;
}

bool litConfigParser::ParseConfigFile (const char* vfsfile,
	litConfig& litconfig)
{
  csRef<iFile> databuff = lighter->vfs->Open (vfsfile, VFS_FILE_READ);
  if (!databuff || !databuff->GetSize ())
    return lighter->Report ("Could not open config file '%s' on VFS!",
    	vfsfile);

  csRef<iDocumentSystem> docsys =
      CS_QUERY_REGISTRY (object_reg, iDocumentSystem);
  if (!docsys) docsys = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
  csRef<iDocument> doc = docsys->CreateDocument ();
  const char* error = doc->Parse (databuff, true);
  if (error != 0)
    return lighter->Report ("Document system error for file '%s': %s!",
    	vfsfile, error);
  csRef<iDocumentNode> lighter_node = doc->GetRoot ()->GetNode ("lighter");
  if (!lighter_node)
    return lighter->Report ("Config file '%s' does not start with <lighter>!",
    	vfsfile);

  return ParseLighter (lighter_node, litconfig);
}

