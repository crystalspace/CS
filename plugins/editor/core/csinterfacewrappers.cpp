/*
    Copyright (C) 2007 by Seth Yastrov

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

#include <cssysdef.h>
#include "csutil/scf.h"

#include <csutil/objreg.h>

#include "csinterfacewrappers.h"

using namespace CS::EditorApp;

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

SCF_IMPLEMENT_FACTORY(CSInterfaceWrappers)

CSInterfaceWrappers::CSInterfaceWrappers (iBase* parent)
  : scfImplementationType (this, parent)
{
}

CSInterfaceWrappers::~CSInterfaceWrappers ()
{
}


bool CSInterfaceWrappers::Initialize (iObjectRegistry* obj_reg)
{
  object_reg = obj_reg;

  interfaceManager = csQueryRegistry<iInterfaceWrapperManager> (object_reg);
  if (!interfaceManager)
    return false;

  interfaceManager->Register (new InterfaceWrapper::ObjectFactory ());
  interfaceManager->Register (new InterfaceWrapper::SceneNodeFactory ());
  interfaceManager->Register (new InterfaceWrapper::MeshFactoryWrapperFactory ());
  
  return true;
}

}
CS_PLUGIN_NAMESPACE_END(CSE)
