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

#include "sysdef.h"
#define CS_DISABLE_MODULE_LOCKING
#include "cscom/com.h"
#undef  CS_DISABLE_MODULE_LOCKING
#include "csengine/newclass.h"
#include "csutil/csbase.h"
#include "csengine/intrface.h"
#include "csengine/skeleton.h"

IMPLEMENT_UNKNOWN(csClassSpawner);
BEGIN_INTERFACE_TABLE(csClassSpawner)
	IMPLEMENTS_INTERFACE(IClassSpawner)
END_INTERFACE_TABLE()

#define SPAWN_CLASS_HANDLER(ClassName) \
	if(rcid==CLSID_##ClassName) { \
		cs##ClassName *obj=CHK(new cs##ClassName); \
		if(obj==NULL) return E_OUTOFMEMORY; \
		obj->QueryInterface(IID_IUnknown, (void**)&c); \
		if(c==NULL) {\
			CHK(delete obj); \
			return E_OUTOFMEMORY; \
		} \
	} else

/**
* Object Spawner
* Classes handled here need to have a corresponding
* class cs<ClassName>
* interface I<ClassName>
* GUID IID_I<ClassName>
* GUID CLSID_<ClassName>
*/
STDMETHODIMP csClassSpawner::SpawnClass(REFIID riid, REFCLSID rcid, void** i) {
	IUnknown *c=NULL;
	*i=NULL;

//ADD CLASSES HERE (no ; allowed in here)

//	SPAWN_CLASS_HANDLER(Skeleton)
	SPAWN_CLASS_HANDLER(ClassSpawner)

//END ADD CLASSES
	{ //Do not delete this, although goofy, its needed for speed
		return E_FAIL;
	} 

	c->QueryInterface(riid, i);

	if(*i==NULL)
		c->Release();

	return S_OK;
}