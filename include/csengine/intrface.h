/*
	  Copyright (C) 1998 by Jorrit Tyberghein
		CSScript module created by Brandon Ehle
  
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

#ifndef __INTRFACE_H__
#define __INTRFACE_H__

#include "csengine/ibase.h"
#include "csengine/newclass.h"
#include "isystem.h"

extern const IID IID_IExtension;

interface IExtension:public IBase {
	STDMETHOD (GetClassSpawner)(IClassSpawner **cs) PURE;
};


extern const IID IID_IExtensionLoader;

interface IExtensionLoader:public IExtension {
	/// Because some extensions may represent multiple types (not my fault), return S_OK if true, return S_FALSE if not
	STDMETHOD (IsThisType) (IString *data) PURE;

	/// Loads an object to the world (IWorld)
	STDMETHOD (LoadToWorld)(IClassSpawner *ics, IWorld *iworld, IString *name, IString *data) PURE;
};


extern const IID IID_ISpriteExtensionLoader;

interface ISpriteExtensionLoader:public IExtensionLoader {
	/// Creates the sprite template and gives it a name (csSpriteTemplate)
  STDMETHOD (LoadToSprite) (IClassSpawner *ics, ISpriteTemplate **itmpl, IWorld *iworld, IString *name, IString *data) PURE;
};


extern const GUID IID_ILanguageExtension;

interface ILanguageExtension:public IExtension {
	STDMETHOD (Init)(ISystem* iSys) PURE;
};


#endif
