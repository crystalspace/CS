/*
    Copyright (C) 2005 by Christopher Nelson

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
#include "xml_def.h"
#include "registrar.h"

#include "iutil/document.h"


namespace aws2
{

void defFile::ParseNode(autom::scope *sc, csRef< iDocumentNodeIterator> &pos)
{
  // Walk through all of the nodes and
  while(pos->HasNext())
  {
    bool had_value=false;

    csRef<iDocumentNode> child = pos->Next ();

    // Don't process comments.
    if (child->GetType()==CS_NODE_COMMENT) continue;

    csString name(child->GetValue());

    if ((name == "component") || (name == "window") || (name == "skin"))
    {
      csRef<iDocumentNodeIterator> new_pos = child->GetNodes();
      autom::scope *child_sc = new autom::scope(sc);

      /* Add the child.  It has a name to distinguish it from other scopes in the parent. */
      sc->addChild(child->GetAttributeValue("name"), child_sc);

      /* Give the type of the component to it automatically. */
      child_sc->set("type", autom::keeper(new autom::string(name)));
            
      /* Parse all children nodes into this scope. */
      ParseNode(child_sc, new_pos);
    }
    else
    {			
      csRef<iDocumentAttributeIterator > attr_pos = child->GetAttributes();

      /* Loop over all of the attributes in the element and add them as keys 
       * into the map. */
      while(attr_pos->HasNext())
      {
	csRef<iDocumentAttribute > attr = attr_pos->Next();
	      
	scfString a_name(attr->GetName());
	std::string a_value(attr->GetValue());

	/* If the name of the attribute is value, then the name of the key is 
	 * the same as the name of the element, otherwise we use 
	 * element_name.attribute_name as the key name. */
	if (a_name=="value")			
	{
	  sc->set(child->GetValue(), 
	    autom::keeper(autom::Compile(a_value)));				
	  had_value=true;
	}
	else
	{
	  csString tmp = name;

	  tmp+=".";
	  tmp+=a_name;
	  
	  sc->set(tmp, 
	    autom::keeper(autom::Compile(a_value)));				
	}
      }
    }

    if (!had_value)
    {
      const char *_txt = child->GetContentsValue();
      if (_txt)
      {
	std::string txt(_txt);		
	sc->set(name, autom::Compile(txt));
      }
    }
  }
}

bool defFile::Parse(const scfString &txt, autom::scope *sc)
{
  /*csRef<iPluginManager> plugin_mgr =  CS_QUERY_REGISTRY (object_reg, 
    iPluginManager);	
  csRef<iDocumentSystem> inputDS = csPtr<iDocumentSystem> (
    CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.documentsystem.xmltiny", 
    iDocumentSystem));*/

  csRef<iDocumentSystem> xml;
  xml.AttachNew (new csTinyDocumentSystem ());
  csRef<iDocument> doc = xml->CreateDocument ();

  doc->Parse(txt.GetData(), true);

  csRef< iDocumentNode > node = doc->GetRoot();
  csRef< iDocumentNodeIterator> pos = node->GetNodes();

  ParseNode(sc, pos);

  return true;
}

} // end namespace
