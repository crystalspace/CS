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

#ifndef __INTERFACEWRAPPERMANAGER_H__
#define __INTERFACEWRAPPERMANAGER_H__

#include "ieditor/interfacewrappermanager.h"
#include "ieditor/interfacewrapper.h"

#include <csutil/hash.h>

namespace CS {
namespace EditorApp {

class InterfaceWrapperManager : public scfImplementation1<InterfaceWrapperManager, iInterfaceWrapperManager>
{
public:
  InterfaceWrapperManager (iObjectRegistry* obj_reg);

  virtual ~InterfaceWrapperManager ();

  virtual void Register (iInterfaceWrapperFactory* wrapper);

  virtual iInterfaceWrapperFactory* GetFactory (scfInterfaceID interface_id);
  
protected:
  iObjectRegistry* object_reg;
  
  csHash<csRef<iInterfaceWrapperFactory>, scfInterfaceID> interfaceHash;
};

} // namespace EditorApp
} // namespace CS

#endif
