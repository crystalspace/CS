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

#ifndef __IBASE_H__
#define __IBASE_H__

#include "csutil/csbase.h"
#include "csutil/csstring.h"

interface IWorld;
interface IString;
/*
#define DECLARE_IOBJECT() STDMETHODIMP AddName(IString *name);

#define IMPLEMENT_OBJECT(ClassName) \
STDMETHODIMP ClassName::AddName(IString *iname) { \
	csSTR name(iname); \
	csNameObject::AddName(*this, name); \
	return S_OK; \
}
*/
extern const GUID IID_IBase;

/// Interface for csBase
interface IBase : public IUnknown
{
};

class rBase:public IBase {
  csBase* ptr;
public:
	rBase(csBase* base)
		:ptr(base) {}
};

extern const GUID IID_IObject;

interface IObject:public IBase {
	STDMETHOD (AddName) (IString *name) PURE;
};

extern const GUID IID_IFrame;

interface IFrame:public IBase {
};


extern const GUID IID_ISpriteAction;

interface ISpriteAction:public IBase {
	STDMETHOD (AddFrame)(IFrame* iframe, int delay) PURE;
};


extern const GUID IID_ISkeleton;
extern const GUID CLSID_Skeleton;

interface ISkeleton:public IBase {
};

extern const GUID IID_ISpriteTemplate;
extern const GUID CLSID_SpriteTemplate;

interface ISpriteTemplate:public IObject {
	STDMETHOD (SetTexture)(IWorld* iworld, IString* name) PURE;
	
	STDMETHOD (SetSkeleton)(ISkeleton* iskel) PURE;

	STDMETHOD (CreateFrame)(IFrame** iframe, IString* name) PURE;
	
	STDMETHOD (CreateAction)(ISpriteAction** iaction, IString* name) PURE;
	
	STDMETHOD (Prepare)() PURE;
};


extern const GUID IID_IWorld;

/// Interface for csWorld
interface IWorld:public IObject {
	STDMETHOD (GetSpriteTemplate) (IString *name, ISpriteTemplate **tmpl) PURE;

	STDMETHOD (PushSpriteTemplate) (ISpriteTemplate *tmpl) PURE;
};

#endif
