/*
    Copyright (C) 1999 by Andrew Zabolotny
    Crystal Space Shared Class Facility (SCF)

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

#include "cssys/csscf.h"
#include "csutil/csshlib.h"
#include "csutil/util.h"
#include "csutil/csvector.h"
#include "csutil/inifile.h"
#include "debug/memory.h"

#ifndef CS_STATIC_LINKED
#  define COM_CONFIG_FILENAME	"scf.cfg"
#  if defined (OS_OS2)
#    define COM_CONFIG_SECTION	"registry.os2"
#  elif defined (OS_UNIX)
#    define COM_CONFIG_SECTION	"registry.unix"
#  elif defined (OS_WIN32)
#    define COM_CONFIG_SECTION	"registry.win32"
#  elif defined (OS_MACOS)
#    define COM_CONFIG_SECTION	"registry.mac"
#  elif defined (OS_AMIGAOS)
#    define COM_CONFIG_SECTION	"registry.amiga"
#  else
#    error "Please set the COM_CONFIG_SECTION macro in com/cscom.cpp for your system!"
#  endif
#endif

/// This structure contains everything we need to know about a particular class
class scfFactory : public IBase
{
public:
  // Class name
  char *ClassName;
  // Function that creates an instance of this class (if RefCount > 0)
  IBase *(*CreateInstance) (IBase *iParent);
#ifndef CS_STATIC_LINKED
  // Shared module that implements this class
  char *ModuleName;
  // Handle of shared module (if RefCount > 0)
  csLibraryHandle ModuleHandle;
#endif

  // Create a object of scfFactory class
  scfFactory ();
  // Free the factory object (but not objects created by this factory)
  virtual ~scfFactory ();

  DECLARE_IBASE
};

/// This class holds a number of scfFactory structures
class scfClassRegistry : public csVector
{
public:
  scfClassRegistry () : csVector (16, 16) {}
  virtual ~scfClassRegistry () { DeleteAll (); }
  virtual bool FreeItem (csSome Item)
  { if (Item) { CHK (delete (scfFactory *)Item); } return true; }
  virtual int CompareKey (csSome Item, csConstSome Key, int /*Mode*/) const
  { return strcmp (((scfFactory *)Item)->ClassName, (char *)Key); }
};

//------------------------------------------ Class factory implementation ----//

scfFactory::scfFactory ()
{
  CONSTRUCT_IBASE (NULL);
}

scfFactory::~scfFactory ()
{
#ifdef DEBUG
  // Warn user about unreleased instances of this class
  if (scfRefCount > 1)
    fprintf (stderr, "SCF WARNING: %d unreleased instances of class %s left!\n",
      scfRefCount - 1, ClassName);
#endif

#ifndef CS_STATIC_LINKED
  if (scfRefCount)
    csUnloadLibrary (ModuleHandle);
  if (ModuleName)
    delete [] ModuleName;
#endif
  delete [] ClassName;
}

void scfFactory::AddRef ()
{
#ifndef CS_STATIC_LINKED
  if (!scfRefCount && ModuleName)
  {
    // Load the shared library
    ModuleHandle = csLoadLibrary (ModuleName);
    if (!ModuleHandle)
      return;
    char tmp [200];
    sprintf (tmp, "Create_%s", ClassName);
    CreateInstance = (IBase *(*)(IBase *))csGetLibrarySymbol (ModuleHandle, tmp);
    // If the library doesn't contain such a class, free shared lib and fail
    if (!CreateInstance)
    {
      csUnloadLibrary (ModuleHandle);
      return;
    }
  }
#endif
  scfRefCount++;
}

void scfFactory::Release ()
{
  if (!scfRefCount)
  {
#ifdef DEBUG
    fprintf (stderr, "SCF WARNING: Extra calls to scfFactory::Release () for class %s\n", ClassName);
#endif
    return;
  }
  scfRefCount--;
#ifndef CS_STATIC_LINKED
  if (!scfRefCount)
    csUnloadLibrary (ModuleHandle);
#endif
}

IBase *scfFactory::QueryInterface (const char *iItfName, int)
{
  AddRef ();
  // If AddRef won't succeed, we'll have a zero reference counter
  if (!scfRefCount)
    return NULL;

  return CreateInstance (this);
}

//-------------------------------------------------- Client SCF functions ----//

scfClassRegistry *ClassRegistry;

#ifndef CS_STATIC_LINKED
// This is the iterator used to enumerate configuration file entries
static bool ConfigIterator (csSome, char *Name, size_t, csSome Data)
{
  // Create a new registry entry object
  CHK (scfFactory *cf = new scfFactory ());
  cf->ClassName = strnew (Name);
  cf->ModuleName = strnew ((char *)Data);

  // Add the new entry to the registry
  ClassRegistry->Push (cf);

  return false;
}
#endif

void scfInitialize (/* csVFS *Vfs */)
{
  if (!ClassRegistry)
    ClassRegistry = new scfClassRegistry ();
#ifndef CS_STATIC_LINKED
  // load the class database from the configuration file
  csIniFile config (COM_CONFIG_FILENAME /* , Vfs */);
  config.EnumData (COM_CONFIG_SECTION, ConfigIterator, NULL);
#endif
}

void scfFinish ()
{
  if (ClassRegistry)
    delete ClassRegistry;
  ClassRegistry = NULL;
}

IBase *scfCreateInstance (const char *iClassName, const char *iItfName,
  int iVersion)
{
  int idx = ClassRegistry->FindKey (iClassName);
  IBase *instance = NULL;

  if (idx >= 0)
  {
    scfFactory *cf = (scfFactory *)ClassRegistry->Get (idx);
    IBase *object = cf->QueryInterface (iClassName, 0);
    if (object)
      instance = object->QueryInterface (iItfName, iVersion);
  } /* endif */

  scfUnloadUnusedModules ();

  return instance;
}

void scfUnloadUnusedModules ()
{
  for (int i = ClassRegistry->Length () - 1; i >= 0; i--)
  {
    scfFactory *cf = (scfFactory *)ClassRegistry->Get (i);
    if (cf->scfRefCount == 1)
      cf->Release ();
  }
}

bool scfRegisterClass (char *iClassName, IBase *(*iCreateInstance) (IBase *iParent))
{
  // Since when using static linkage we're called before main(),
  // we can get called before scfInitialize() as well ...
  if (!ClassRegistry)
    ClassRegistry = new scfClassRegistry ();

  // Create a new registry entry object
  CHK (scfFactory *cf = new scfFactory ());
#ifndef CS_STATIC_LINKED
  cf->ModuleName = NULL;
#endif
  cf->ClassName = strnew (iClassName);
  cf->CreateInstance = iCreateInstance;

  // Add the new entry to the registry
  ClassRegistry->Push (cf);
  return true;
}

bool scfUnregisterClass (char *iClassName)
{
  // If we have no class registry, we aren't initialized (or were finalized)
  if (!ClassRegistry)
    return false;

  int idx = ClassRegistry->FindKey (iClassName);

  if (idx < 0)
    return false;

  ClassRegistry->Delete (idx);
  return true;
}
