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

#include "interfacewrappermanager.h"

namespace CS {
namespace EditorApp {

InterfaceWrapperManager::InterfaceWrapperManager (iObjectRegistry* obj_reg)
  : scfImplementationType (this), object_reg (obj_reg)
{
  object_reg->Register (this, "iInterfaceWrapperManager");
}

InterfaceWrapperManager::~InterfaceWrapperManager ()
{
  object_reg->Unregister (this, "iInterfaceWrapperManager");
}

void InterfaceWrapperManager::Register (iInterfaceWrapperFactory* wrapper)
{
  interfaceHash.Put (wrapper->GetInterfaceID (), wrapper);
}

iInterfaceWrapperFactory* InterfaceWrapperManager::GetFactory (scfInterfaceID interface_id)
{
  return interfaceHash.Get (interface_id, 0);
}

} // namespace EditorApp
} // namespace CS
