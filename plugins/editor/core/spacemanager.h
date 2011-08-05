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

#ifndef __SPACEMANAGER_H__
#define __SPACEMANAGER_H__

#include <csutil/refarr.h>
#include <csutil/hash.h>


#include "ieditor/space.h"


namespace CS {
namespace EditorApp {

class SpaceManager : public scfImplementation1<SpaceManager,iSpaceManager>
{
public:
  SpaceManager (iObjectRegistry* obj_reg, wxWindow* parent);
  virtual ~SpaceManager ();
  
  virtual void Initialize ();
  virtual void Uninitialize ();
  
  //iSpaceManager
  virtual bool Register (const char*);
  virtual const csHash<csRef<iSpaceFactory>, csString>& GetAll();
  
private:
  iObjectRegistry* object_reg;
  csHash<csRef<iSpaceFactory>, csString> factories;
};

} // namespace EditorApp
} // namespace CS

#endif
