/*
    Copyright (C) 2011 by Jelle Hellemans

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
#include "csutil/scf.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include "ieditor/space.h"
#include "ieditor/layout.h"

#include "objectpanel.h"

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{
  
SCF_IMPLEMENT_FACTORY (ObjectPanel)

ObjectPanel::ObjectPanel (iBase* parent) : scfImplementationType (this, parent)
{
}

ObjectPanel::~ObjectPanel ()
{
}

bool ObjectPanel::Initialize (iObjectRegistry* obj_reg)
{
  object_reg = obj_reg;
  
  csRef<iSpaceManager> mgr = csQueryRegistry<iSpaceManager> (object_reg);
  return mgr->Register(this);
}

bool ObjectPanel::Poll(iContext*)
{
  return true;
}

void ObjectPanel::Draw(iContext*, iLayout* layout)
{
  printf("ObjectPanel::Draw\n");
  layout->AppendLabel("object");
  layout->AppendOperator("cs.editor.operator.select", "Select", "");
}

void ObjectPanel::Prepend(iLayoutExtension*)
{
}

void ObjectPanel::Append(iLayoutExtension*)
{
}

}
CS_PLUGIN_NAMESPACE_END(CSE)
