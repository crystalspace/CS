/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "iutil/comp.h"
#include "csutil/scf.h"
#include "csutil/xmltiny.h"

class csTinyXMLPlugin : public iDocumentSystem, public iComponent
{
private:
  csTinyDocumentSystem *tiny;
public:
  SCF_DECLARE_IBASE;

  csTinyXMLPlugin (iBase* parent = NULL);
  virtual ~csTinyXMLPlugin ();

  virtual bool Initialize (iObjectRegistry* objreg);

  csRef<iDocument> CreateDocument ();
};

SCF_IMPLEMENT_IBASE(csTinyXMLPlugin)
  SCF_IMPLEMENTS_INTERFACE(iDocumentSystem)
  SCF_IMPLEMENTS_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

csTinyXMLPlugin::csTinyXMLPlugin(iBase* parent)
{
  SCF_CONSTRUCT_IBASE(parent);

  tiny = new csTinyDocumentSystem ();
}

csTinyXMLPlugin::~csTinyXMLPlugin()
{
  delete tiny;
}

bool csTinyXMLPlugin::Initialize (iObjectRegistry* objreg)
{
  return true;
}

csRef<iDocument> csTinyXMLPlugin::CreateDocument ()
{
  return tiny->CreateDocument ();
}

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csTinyXMLPlugin)

SCF_EXPORT_CLASS_TABLE(xmltiny)
  SCF_EXPORT_CLASS(csTinyXMLPlugin, "crystalspace.documentsystem.tinyxml",
      "Crystal Space TinyXML document system")
SCF_EXPORT_CLASS_TABLE_END

