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

#ifndef __NEWCLASS_H__
#define __NEWCLASS_H__

#include "cscom/com.h"
#include "css/cssdefs.h"

extern const GUID IID_IClassSpawner;

interface IClassSpawner:public IUnknown {
	STDMETHOD (SpawnClass) (REFIID riid, REFCLSID rcid, void** i) PURE;
};


extern const GUID CLSID_ClassSpawner;

class csClassSpawner:public IClassSpawner {
public:
	STDMETHODIMP SpawnClass(REFIID riid, REFCLSID rcid, void** i);

	DEFAULT_COM(ClassSpawner);
};

#define spawn(Spawner, ClassName) ((I##ClassName*)SPAWN_CLASS(Spawner, IID_I##ClassName, CLSID_##ClassName))

__inline void* SPAWN_CLASS(IClassSpawner *Spawner, REFIID riid, REFCLSID rcid) {
	void *Ptr;
	Spawner->SpawnClass(riid, rcid, (void**)&Ptr); 
	return Ptr;
}

__inline void* SPAWN_CLASS(csClassSpawner *Spawner, REFIID riid, REFCLSID rcid) {
	void *Ptr;
	Spawner->SpawnClass(riid, rcid, (void**)&Ptr); 
	return Ptr;
}

#endif
