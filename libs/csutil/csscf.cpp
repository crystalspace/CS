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

#define SYSDEF_PATH
#include "sysdef.h"
#include "cssys/csshlib.h"
#include "csutil/csscf.h"
#include "csutil/util.h"
#include "csutil/csvector.h"
#include "csutil/csobjvec.h"
#include "csutil/inifile.h"
#include "debug/memory.h"

#ifndef CS_STATIC_LINKED
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

#ifndef CS_STATIC_LINKED

// This is the registry for all shared libraries
csObjVector *LibraryRegistry;

/// A object of this class represents a shared library
class scfSharedLibrary : public csBase
{
  // Shared library name
  const char *LibraryName;
  // Handle of shared module (if RefCount > 0)
  csLibraryHandle LibraryHandle;
  // Number of references to this shared library
  int RefCount;
  // The table of classes exported from this library
  scfClassInfo *ClassTable;

public:
  /// Create a shared library and load it
  scfSharedLibrary (const char *iLibraryName);

  /// Destroy a shared library object
  virtual ~scfSharedLibrary ();

  /// Check if library object is okay
  bool ok ()
  { return (LibraryHandle != NULL) && (ClassTable != NULL); }

  /// Increment reference count for the library
  void IncRef ()
  { RefCount++; }

  /// Decrement reference count for the library
  void DecRef ()
  { if (!--RefCount) LibraryRegistry->Delete (LibraryRegistry->Find (this)); }

  /// Find a scfClassInfo for given class ID
  scfClassInfo *Find (const char *iClassID);

  /// Find a shared library by name
  virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const
  { return (strcmp (((scfSharedLibrary *)Item)->LibraryName, (char *)Key) == 0); }
};

scfSharedLibrary::scfSharedLibrary (const char *iLibraryName)
{
  LibraryRegistry->Push (this);

  RefCount = 0;
  ClassTable = NULL;
  LibraryHandle = csLoadLibrary (LibraryName = iLibraryName);
  if (!LibraryHandle)
    return;

  // This is the prototype for the only function that
  // a shared library should export
  typedef scfClassInfo *(*scfGetClassInfo) ();
  char name [200];

  const char *tmp = LibraryName + strlen (LibraryName);
  // remove path name from library
  while ((tmp > LibraryName) && (*tmp != '/') && (*tmp != PATH_SEPARATOR))
    tmp--;
  // if library name starts with "lib", skip this prefix
  if ((tmp [0] == 'l') && (tmp [1] == 'i') && (tmp [2] == 'b'))
    tmp += 3;
  strcpy (name, tmp);
  char *dot = strrchr (name, '.');
  if (dot)
    *dot = 0;
  strcat (name, "_GetClassTable");

  scfGetClassInfo func = (scfGetClassInfo)csGetLibrarySymbol (LibraryHandle, name);
  if (func)
    ClassTable = func ();
}

scfSharedLibrary::~scfSharedLibrary ()
{
  if (LibraryHandle)
    csUnloadLibrary (LibraryHandle);
}

scfClassInfo *scfSharedLibrary::Find (const char *iClassID)
{
  for (scfClassInfo *cur = ClassTable; cur->ClassID; cur++)
    if (strcmp (iClassID, cur->ClassID) == 0)
      return cur;
  return NULL;
}

#endif // CS_STATIC_LINKED

/// This structure contains everything we need to know about a particular class
class scfFactory : public IFactory
{
public:
  DECLARE_IBASE;

  // Class identifier
  char *ClassID;
  // Information about this class
  const scfClassInfo *ClassInfo;
#ifndef CS_STATIC_LINKED
  // Shared module that implements this class or NULL for local classes
  char *LibraryName;
  // A pointer to shared library object (NULL for local classes)
  scfSharedLibrary *Library;
#endif

  // Create the factory for a class located in a shared library
  scfFactory (const char *iClassID, const char *iLibraryName);
  // Create the factory for a class located in client module
  scfFactory (const scfClassInfo *iClassInfo);
  // Free the factory object (but not objects created by this factory)
  virtual ~scfFactory ();
  // Create a insance of class this factory represents
  virtual void *CreateInstance ();
  // Try to unload class module (i.e. shared module)
  virtual void TryUnload ();
};

/// This class holds a number of scfFactory structures
class scfClassRegistry : public csVector
{
public:
  scfClassRegistry () : csVector (16, 16) {}
  virtual ~scfClassRegistry () { DeleteAll (); }
  virtual bool FreeItem (csSome Item)
  { if (Item) CHKB (delete (scfFactory *)Item); return true; }
  virtual int CompareKey (csSome Item, csConstSome Key, int /*Mode*/) const
  {
    const char *id1 = ((scfFactory *)Item)->ClassID;
    const char *id2 = (char *)Key;
    if (scfIsFastClassID (id2))
      return !(scfIsFastClassID (id1) && scfEqualFastClassID (id1, id2));
    else
      return strcmp (id1, id2);
  }
};

//------------------------------------------ Class factory implementation ----//

scfFactory::scfFactory (const char *iClassID, const char *iLibraryName)
{
  CONSTRUCT_IBASE (NULL);
  ClassID = strnew (iClassID);
#ifndef CS_STATIC_LINKED
  LibraryName = strnew (iLibraryName);
  Library = NULL;
#else
  (void)iLibraryName;
  // this branch should never be called
  abort ();
#endif
}

scfFactory::scfFactory (const scfClassInfo *iClassInfo)
{
  CONSTRUCT_IBASE (NULL);
  ClassID = strnew (iClassInfo->ClassID);
  ClassInfo = iClassInfo;
#ifndef CS_STATIC_LINKED
  LibraryName = NULL;
  Library = NULL;
#endif
}

scfFactory::~scfFactory ()
{
#ifdef DEBUG
  // Warn user about unreleased instances of this class
  if (scfRefCount > 1)
    fprintf (stderr, "SCF WARNING: %d unreleased instances of class %s left!\n",
      scfRefCount - 1, ClassID);
#endif

#ifndef CS_STATIC_LINKED
  if (Library)
    Library->DecRef ();
  if (LibraryName)
    delete [] LibraryName;
#endif
  delete [] ClassID;
}

void scfFactory::IncRef ()
{
#ifndef CS_STATIC_LINKED
  if (!Library && LibraryName)
  {
    int libidx = LibraryRegistry->FindKey (LibraryName);
    Library = libidx >= 0 ?
      (scfSharedLibrary *)LibraryRegistry->Get (libidx) :
      new scfSharedLibrary (LibraryName);
    if (!Library->ok ())
    {
      delete Library;
      return;
    }
    ClassInfo = Library->Find (ClassID);
    if (!ClassInfo)
    {
      delete Library;
      return;
    }
  }
  if (Library)
    Library->IncRef ();
#endif
  scfRefCount++;
}

void scfFactory::DecRef ()
{
#ifdef DEBUG
  if (!scfRefCount)
  {
    fprintf (stderr, "SCF WARNING: Extra calls to scfFactory::DecRef () for class %s\n", ClassID);
    return;
  }
#endif
  scfRefCount--;
#ifndef CS_STATIC_LINKED
  if (Library)
    Library->DecRef ();
#endif
}

void *scfFactory::QueryInterface (const char *iInterfaceID, int iVersion)
{
  IMPLEMENTS_INTERFACE (IFactory);
  return NULL;
}

void *scfFactory::CreateInstance ()
{
  IncRef ();
  // If IncRef won't succeed, we'll have a zero reference counter
  if (!scfRefCount)
    return NULL;

  return ClassInfo->Factory (this);
}

void scfFactory::TryUnload ()
{
  if (scfRefCount == 1)
    DecRef ();
}

//-------------------------------------------------- Client SCF functions ----//

// This is the registry for all class factories
scfClassRegistry *ClassRegistry;

#ifndef CS_STATIC_LINKED
// This is the iterator used to enumerate configuration file entries
static bool ConfigIterator (csSome, char *Name, size_t, csSome Data)
{
  scfRegisterClass (Name, (char *)Data);
  return false;
}
#endif

void scfInitialize (csIniFile *iConfig)
{
  if (!ClassRegistry)
    ClassRegistry = new scfClassRegistry ();
#ifndef CS_STATIC_LINKED
  if (!LibraryRegistry)
    LibraryRegistry = new csObjVector (16, 16);
  if (iConfig)
    iConfig->EnumData (COM_CONFIG_SECTION, ConfigIterator, NULL);
#else
  (void)iConfig;
#endif
}

void scfFinish ()
{
  if (ClassRegistry)
    delete ClassRegistry;
  ClassRegistry = NULL;
#ifndef CS_STATIC_LINKED
  if (LibraryRegistry)
    delete LibraryRegistry;
  LibraryRegistry = NULL;
#endif
}

void *scfCreateInstance (const char *iClassID, const char *iInterfaceID,
  int iVersion)
{
  int idx = ClassRegistry->FindKey (iClassID);
  void *instance = NULL;

  if (idx >= 0)
  {
    IFactory *cf = (IFactory *)ClassRegistry->Get (idx);
    IBase *object = (IBase *)cf->CreateInstance ();
    if (object)
    {
      instance = object->QueryInterface (iInterfaceID, iVersion);
      if (!instance)
        object->DecRef ();
    }
  } /* endif */

  scfUnloadUnusedModules ();

  return instance;
}

void scfUnloadUnusedModules ()
{
  for (int i = ClassRegistry->Length () - 1; i >= 0; i--)
  {
    IFactory *cf = (IFactory *)ClassRegistry->Get (i);
    cf->TryUnload ();
  }
}

bool scfRegisterClass (const char *iClassID, const char *iLibraryName)
{
#ifndef CS_STATIC_LINKED
  if (ClassRegistry->FindKey (iClassID) >= 0)
    return false;
  // Create a factory and add it to class registry
  ClassRegistry->Push (new scfFactory (iClassID, iLibraryName));
  return true;
#else
  (void)iLibraryName;
  return false;
#endif
}

bool scfRegisterClass (scfClassInfo *iClassInfo)
{
  if (ClassRegistry->FindKey (iClassInfo->ClassID) >= 0)
    return false;
  // Create a factory and add it to class registry
  ClassRegistry->Push (new scfFactory (iClassInfo));
  return true;
}

bool scfRegisterClassList (scfClassInfo *iClassInfo)
{
  // We can be called during initialization
  if (!ClassRegistry)
    scfInitialize ();

  while (iClassInfo->ClassID)
    if (!scfRegisterClass (iClassInfo++))
      return false;
  return true;
}

bool scfUnregisterClass (char *iClassID)
{
  // If we have no class registry, we aren't initialized (or were finalized)
  if (!ClassRegistry)
    return false;

  int idx = ClassRegistry->FindKey (iClassID);

  if (idx < 0)
    return false;

  ClassRegistry->Delete (idx);
  return true;
}

scfClassID &scfGenerateUniqueID ()
{
  static scfClassID last = { 0 };

  if (!last [0])
  {
    memset (last, 1, sizeof (last) - 2);
    last [sizeof (last) - 1] = 0;
    last [sizeof (last) - 2] = 0;
  }

  unsigned char *cur = ((unsigned char *)&last) + sizeof (last) - 2;
  for (unsigned int i = sizeof (last) - 1; i >= 0; i--)
  {
    if (++(*cur))
      break;
    *cur = 1;
    cur--;
  }
  // Wrap first character around ' '
  cur = (unsigned char *)&last;
  if (*cur > ' ')
    *cur = 1;
  return last;
}
