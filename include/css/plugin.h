/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Brandon Ehle

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

/* This file is for being included with the header for a plugin for the CS engine */

#include "sysdef.h"
#include "cscom/com.h"
#include "css/cssint.h"
#include "css/interfaces.h"

#ifdef OS_WIN32
#define STATIC_EXTENSION ".lib"
#define SHARED_EXTENSION ".dll"
#endif

//***** Huge define mess that shouldn't change (i.e. do not read after this point, skip to end of file)

#define EXTENSION_FACTORY(ClassName) \
class cse##ClassName##Factory:public IClassFactory { \
  STDMETHODIMP CreateInstance(IUnknown *i, REFIID riid, void** ppv); \
  STDMETHODIMP LockServer(COMBOOL bLock); \
  DECLARE_IUNKNOWN() \
  DECLARE_INTERFACE_TABLE(cse##ClassName##Factory) \
}; \
STDMETHODIMP cse##ClassName##Factory::CreateInstance(IUnknown *, REFIID riid, void** ppv) { \
	cse##ClassName *pNew=new cse##ClassName(); \
	if(!pNew) {*ppv = 0; return E_OUTOFMEMORY;} \
  return pNew->QueryInterface (riid, ppv); \
} \
STDMETHODIMP cse##ClassName##Factory::LockServer(COMBOOL bLock) { if(bLock) gRefCount++; else gRefCount--; return S_OK;} \
IMPLEMENT_UNKNOWN_NODELETE(cse##ClassName##Factory) \
BEGIN_INTERFACE_TABLE(cse##ClassName##Factory) \
  IMPLEMENTS_INTERFACE(IClassFactory) \
END_INTERFACE_TABLE()


#define BEGIN_EXTENSION_REGISTRY() \
static DllRegisterData gRegData[] = {

#define ADD_EXTENSION_REGISTRY(ClassName, ProgID, ClassNameText) { &CLSID_##ClassName, ProgID, ClassNameText},

#define END_EXTENSION_REGISTRY() {NULL, NULL, NULL} };

//TODO fix mem leaks

#define DEFINE_EXTENSION(DLLName) \
static unsigned int gRefCount = 0; \
void STDAPICALLTYPE ModuleRelease() {gRefCount--;} \
void STDAPICALLTYPE ModuleAddRef() {gRefCount++;} \
STDAPI DllInitialize () {	\
	char *dll=new char[strlen(SHARED_EXTENSION)+strlen(DLLName)+1]; \
	csCoInitialize(0);  \
	strcat(strcpy(dll, DLLName), SHARED_EXTENSION); \
	int i=0; \
	while(gRegData[i].clsid) gRegData[i++].szInProcServer=dll; \
	return TRUE; \
} \
extern HINSTANCE ModuleHandle; \
extern "C" BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID) { \
	if(fdwReason == DLL_PROCESS_ATTACH) {ModuleHandle = hinstDLL; DllInitialize(); } \
  return TRUE; \
} \
STDAPI DllCanUnloadNow() {return gRefCount ? S_FALSE : S_OK;} \
STDAPI DllRegisterServer () { \
	HRESULT hr=S_FALSE; int i=0; \
	while(gRegData[i].clsid) hr=csRegisterServer (&(gRegData[i++])); \
	return hr; \
} \
STDAPI DllUnregisterServer () { \
	HRESULT hr=S_FALSE;	int i=0; \
	while(gRegData[i].clsid) hr=csUnregisterServer (&(gRegData[i++])); \
	return hr; \
} \
void cleanup() {} \
int csMain (int, char** const) {return 0;}

#define BEGIN_EXTENSION() \
STDAPI DllGetClassObject (REFCLSID rclsid, REFIID riid, void** ppv) {

#define ADD_EXTENSION(ClassName) \
  static cse##ClassName##Factory gFactory; \
  if (rclsid == CLSID_##ClassName) \
    return gFactory.QueryInterface(riid, ppv);

#define END_EXTENSION() \
  *ppv = NULL; \
  return CLASS_E_CLASSNOTAVAILABLE; \
}

#define BEGIN_EXTENSION_HEADER(ClassName, BaseClass) \
class cse##ClassName:public BaseClass { \
public:

#define END_EXTENSION_HEADER(ClassName)  \
  DECLARE_IUNKNOWN() \
  DECLARE_INTERFACE_TABLE(cse##ClassName) \
}; \
EXTENSION_FACTORY(ClassName) \
IMPLEMENT_UNKNOWN_NODELETE(cse##ClassName) \
BEGIN_INTERFACE_TABLE(cse##ClassName) \
  IMPLEMENTS_INTERFACE(IExtension)

#define EXTENSION(ClassName) STDMETHODIMP cse##ClassName

//***** Big define mess you might need to add to

#define EXTENSION_HEADER(ClassName) \
BEGIN_EXTENSION_HEADER(ClassName, IExtension) \
END_EXTENSION_HEADER(ClassName) \
END_INTERFACE_TABLE()

#define EXTENSION_LOADER_HEADER(ClassName) \
BEGIN_EXTENSION_HEADER(ClassName, IExtensionLoader) \
END_EXTENSION_HEADER(ClassName) \
  IMPLEMENTS_INTERFACE(IExtensionLoader) \
  STDMETHODIMP IsThisType(IString *data); \
  STDMETHODIMP LoadToWorld(IClassSpawner *ics, IWorld *iworld, IString *name, IString *data); \
END_INTERFACE_TABLE()

#define EXTENSION_SPRITE_HEADER(ClassName) \
BEGIN_EXTENSION_HEADER(ClassName, ISpriteExtensionLoader) \
  STDMETHODIMP IsThisType(IString *data); \
  STDMETHODIMP LoadToWorld(IClassSpawner *ics, IWorld *iworld, IString *name, IString *data); \
  STDMETHODIMP LoadToSprite(IClassSpawner *ics, ISpriteTemplate **itmpl, IWorld *iworld, IString *name, IString *data); \
END_EXTENSION_HEADER(ClassName) \
  IMPLEMENTS_INTERFACE(IExtensionLoader) \
	IMPLEMENTS_INTERFACE(ISpriteExtensionLoader) \
END_INTERFACE_TABLE()

#define EXTENSION_LANGUAGE_HEADER(ClassName) \
BEGIN_EXTENSION_HEADER(ClassName, ILanguageExtension) \
	STDMETHODIMP GetClassSpawner(IClassSpawner *cs); \
	STDMETHODIMP Init(ISystem* iSys); \
END_EXTENSION_HEADER(ClassName) \
  IMPLEMENTS_INTERFACE(ILanguageExtension) \
END_INTERFACE_TABLE()
