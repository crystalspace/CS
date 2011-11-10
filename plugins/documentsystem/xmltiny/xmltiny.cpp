/*
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

#include "cssysdef.h"
#include "iutil/comp.h"
#include "iutil/databuff.h"
#include "iutil/vfs.h"
#include "csutil/scf.h"
#include "csutil/xmltiny.h"

class csTinyXMLPlugin :
  public scfImplementationExt1<csTinyXMLPlugin, csTinyDocumentSystem,
    iComponent>
{
public:
  csTinyXMLPlugin (iBase* parent) :
    scfImplementationType(this, parent) {}

  virtual bool Initialize (iObjectRegistry* objreg);
};

bool csTinyXMLPlugin::Initialize (iObjectRegistry* /*objreg*/)
{
  return true;
}

SCF_IMPLEMENT_FACTORY (csTinyXMLPlugin)
