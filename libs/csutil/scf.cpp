/*
    Copyright (C) 1999,2000 by Andrew Zabolotny
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
#define SYSDEF_ALLOCA
#include "cssysdef.h"
#include "cssys/csshlib.h"
#include "csutil/scf.h"
#include "csutil/util.h"
#include "csutil/csvector.h"
#include "csutil/csobjvec.h"
#include "csutil/cfgfile.h"

/// This is the registry for all class factories
static class scfClassRegistry *ClassRegistry = NULL;
/// If this bool is true, we should sort the registery
static bool SortClassRegistry = false;
/// This is our private instance of csSCF
static class csSCF *PrivateSCF = NULL;
/// This is the public instance
iSCF *iSCF::SCF = NULL;

/// This class manages all SCF functionality
class csSCF : public iSCF
{
public:
  DECLARE_IBASE;

  /// constructor
  csSCF ();
  /// destructor
  ~csSCF ();

  /// Read config file
  virtual void RegisterConfigClassList (iConfigFile *cfg);

  /// Wrapper for scfClassRegistered ()
  virtual bool ClassRegistered (const char *iClassID);

  /// Wrapper for scfCreateInstance ()
  virtual void *CreateInstance (const char *iClassID,
    const char *iInterfaceID, int iVersion);

  /// Wrapper for scfGetClassDescription ()
  virtual const char *GetClassDescription (const char *iClassID);

  /// Wrapper for scfGetClassDependencies ()
  virtual const char *GetClassDependencies (const char *iClassID);

  /// Wrapper for scfRegisterClass ()
  virtual bool RegisterClass (const char *iClassID,
    const char *iLibraryName, const char *Dependencies = NULL);

  /// Wrapper for scfRegisterStaticClass ()
  virtual bool RegisterStaticClass (scfClassInfo *iClassInfo);

  /// Wrapper for scfRegisterClassList ()
  virtual bool RegisterClassList (scfClassInfo *iClassInfo);

  /// Wrapper for scfUnregisterClass ()
  virtual bool UnregisterClass (char *iClassID);

  /// Wrapper for scfUnloadUnusedModules
  virtual void UnloadUnusedModules ();

  /// Wrapper for scfFinish ()
  virtual void Finish ();
};

#ifndef CS_STATIC_LINKED

class scfLibraryVector : public csObjVector
{
public:
  /// Find a shared library by name
  virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const;
};

// This is the registry for all shared libraries
static scfLibraryVector *LibraryRegistry = 0;

/// A object of this class represents a shared library
class scfSharedLibrary : public csBase
{
  friend class scfLibraryVector;
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
  { RefCount--; }

  /// Try to free the library, if refcount is zero
  bool TryUnload ()
  {
    if (RefCount <= 0)
    {
      LibraryRegistry->Delete (LibraryRegistry->Find (this));
      return true;
    }
    return false;
  }

  /// Find a scfClassInfo for given class ID
  scfClassInfo *Find (const char *iClassID);
};

scfSharedLibrary::scfSharedLibrary (const char *iLibraryName)
{
  LibraryRegistry->Push (this);

  RefCount = 0;
  ClassTable = NULL;
  LibraryHandle = csFindLoadLibrary (LibraryName = iLibraryName);
  if (!LibraryHandle)
    return;

  // This is the prototype for the only function that
  // a shared library should export
  typedef scfClassInfo *(*scfInitializeFunc) (iSCF*);

  // To get the library name, split the input name into path and file name.
  // Then append "_scfInitialize" to the file name to get the name of
  // the exported function.
  char name [200];
  splitpath (iLibraryName, NULL, 0, name, 200);
  strcat (name, "_scfInitialize");

  scfInitializeFunc func =
    (scfInitializeFunc)csGetLibrarySymbol (LibraryHandle, name);
  if (func)
    ClassTable = func (PrivateSCF);
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

int scfLibraryVector::CompareKey (csSome Item, csConstSome Key, int) const
{
  return (strcmp (((scfSharedLibrary *)Item)->LibraryName, (char *)Key));
}

#endif // CS_STATIC_LINKED

/// This structure contains everything we need to know about a particular class
class scfFactory : public iFactory
{
public:
  DECLARE_IBASE;

  // Class identifier
  char *ClassID;
  // The dependency list
  char *Dependencies;
  // Information about this class
  const scfClassInfo *ClassInfo;
#ifndef CS_STATIC_LINKED
  // Shared module that implements this class or NULL for local classes
  char *LibraryName;
  // A pointer to shared library object (NULL for local classes)
  scfSharedLibrary *Library;
#endif

  // Create the factory for a class located in a shared library
  scfFactory (const char *iClassID, const char *iLibraryName,
    const char *iDepend);
  // Create the factory for a class located in client module
  scfFactory (const scfClassInfo *iClassInfo);
  // Free the factory object (but not objects created by this factory)
  virtual ~scfFactory ();
  // Create a insance of class this factory represents
  virtual void *CreateInstance ();
  // Try to unload class module (i.e. shared module)
  virtual void TryUnload ();
  /// Query class description string
  virtual const char *QueryDescription ();
  /// Query class dependency strings
  virtual const char *QueryDependencies ();
  /// Query class ID
  virtual const char *QueryClassID();
};

/// This class holds a number of scfFactory structures
class scfClassRegistry : public csVector
{
public:
  scfClassRegistry () : csVector (16, 16) {}
  virtual ~scfClassRegistry () { DeleteAll (); }
  virtual bool FreeItem (csSome Item)
  { delete (scfFactory *)Item; return true; }
  virtual int CompareKey (csSome Item, csConstSome Key, int) const
  { return strcmp (((scfFactory *)Item)->ClassID, (char *)Key); }
  virtual int Compare (csSome Item1, csSome Item2, int) const
  { return strcmp (((scfFactory *)Item1)->ClassID,
                   ((scfFactory *)Item2)->ClassID); }
};

//----------------------------------------- Class factory implementation ----//

scfFactory::scfFactory (const char *iClassID, const char *iLibraryName,
  const char *iDepend)
{
  // Don't use CONSTRUCT_IBASE (NULL) since it will call IncRef()
  scfRefCount = 0; scfParent = NULL;
  ClassID = strnew (iClassID);
  ClassInfo = NULL;
  Dependencies = strnew (iDepend);
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
  // Don't use CONSTRUCT_IBASE (NULL) since it will call IncRef()
  scfRefCount = 0; scfParent = NULL;
  ClassID = strnew (iClassInfo->ClassID);
  ClassInfo = iClassInfo;
  Dependencies = strnew (iClassInfo->Dependencies);
#ifndef CS_STATIC_LINKED
  LibraryName = NULL;
  Library = NULL;
#endif
}

scfFactory::~scfFactory ()
{
#ifdef CS_DEBUG
  // Warn user about unreleased instances of this class
  if (scfRefCount)
    fprintf (stderr, "SCF WARNING: %d unreleased instances of class %s!\n",
      scfRefCount, ClassID);
#endif

#ifndef CS_STATIC_LINKED
  if (Library)
    Library->DecRef ();
  delete [] LibraryName;
#endif
  delete [] Dependencies;
  delete [] ClassID;
}

void scfFactory::IncRef ()
{
#ifndef CS_STATIC_LINKED
  if (!Library && LibraryName)
  {
    int libidx = LibraryRegistry->FindKey (LibraryName);
    if (libidx >= 0)
      Library = (scfSharedLibrary *)LibraryRegistry->Get (libidx);
    else
      Library = new scfSharedLibrary (LibraryName);
    if (Library->ok ())
      ClassInfo = Library->Find (ClassID);
    if (!Library->ok () || !ClassInfo)
    {
      Library = NULL;
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
#ifdef CS_DEBUG
  if (!scfRefCount)
  {
    fprintf (stderr, "SCF WARNING: Extra calls to scfFactory::DecRef () for "
      "class %s\n", ClassID);
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
  IMPLEMENTS_INTERFACE (iFactory);
  return NULL;
}

void *scfFactory::CreateInstance ()
{
  IncRef ();
  // If IncRef won't succeed, we'll have a zero reference counter
  if (!scfRefCount)
    return NULL;

  void *instance = ClassInfo->Factory (this);

  // No matter whenever we succeeded or not, decrement the refcount
  DecRef ();

  return instance;
}

void scfFactory::TryUnload ()
{
#ifndef CS_STATIC_LINKED
  if (Library)
    if (Library->TryUnload ())
      Library = NULL;
#endif
}

const char *scfFactory::QueryDescription ()
{
  if (scfRefCount)
    return ClassInfo->Description;
  else
    return NULL;
}

const char *scfFactory::QueryDependencies ()
{
  return Dependencies;
}

const char *scfFactory::QueryClassID ()
{
  return ClassID;
}

//------------------------------------ Implementation of csSCF functions ----//

IMPLEMENT_IBASE (csSCF);
  IMPLEMENTS_INTERFACE (iSCF);
IMPLEMENT_IBASE_END;

void scfInitialize (iConfigFile *iConfig)
{
  if (!PrivateSCF)
    PrivateSCF = new csSCF ();
  if (iConfig)
    PrivateSCF->RegisterConfigClassList (iConfig);
}

csSCF::csSCF ()
{
  SCF = PrivateSCF = this;

  if (!ClassRegistry)
    ClassRegistry = new scfClassRegistry ();

#ifndef CS_STATIC_LINKED
  if (!LibraryRegistry)
    LibraryRegistry = new scfLibraryVector ();
#endif
}

csSCF::~csSCF ()
{
  delete ClassRegistry;
  ClassRegistry = NULL;
#ifndef CS_STATIC_LINKED
  delete LibraryRegistry;
  LibraryRegistry = NULL;
#endif
  
  SCF = PrivateSCF = NULL;
}

void csSCF::RegisterConfigClassList (iConfigFile *iConfig)
{
#ifndef CS_STATIC_LINKED
  if (iConfig)
  {
    iConfigIterator *iterator = iConfig->Enumerate ();
    if (iterator)
    {
      while (iterator->Next ())
      {
        const char *data = iterator->GetStr ();
        char *val = (char *)alloca (strlen (data) + 1);
        strcpy (val, data);
        char *depend = strchr (val, ':');
        if (depend) *depend++ = 0;
        RegisterClass (iterator->GetKey (true), val, depend);
      }
      iterator->DecRef ();
    }
  }
#else
  (void)iConfig;
#endif
}

void csSCF::Finish ()
{
  delete this;
}

void *csSCF::CreateInstance (const char *iClassID, const char *iInterfaceID,
  int iVersion)
{
  // Pre-sort class registry for doing binary searches
  if (SortClassRegistry)
  {
    ClassRegistry->QuickSort ();
    SortClassRegistry = false;
  }

  int idx = ClassRegistry->FindSortedKey (iClassID);
  void *instance = NULL;

  if (idx >= 0)
  {
    iFactory *cf = (iFactory *)ClassRegistry->Get (idx);
    iBase *object = (iBase *)cf->CreateInstance ();
    if (object)
    {
      instance = object->QueryInterface (iInterfaceID, iVersion);
      object->DecRef ();
    }
  } /* endif */

  UnloadUnusedModules ();

  return instance;
}

void csSCF::UnloadUnusedModules ()
{
  for (int i = ClassRegistry->Length () - 1; i >= 0; i--)
  {
    iFactory *cf = (iFactory *)ClassRegistry->Get (i);
    cf->TryUnload ();
  }
}

bool csSCF::RegisterClass (const char *iClassID, const char *iLibraryName,
  const char *Dependencies)
{
#ifndef CS_STATIC_LINKED
  if (ClassRegistry->FindKey (iClassID) >= 0)
    return false;
  // Create a factory and add it to class registry
  ClassRegistry->Push (new scfFactory (iClassID, iLibraryName, Dependencies));
  SortClassRegistry = true;
  return true;
#else
  (void)iLibraryName;
  return false;
#endif
}

bool csSCF::RegisterStaticClass (scfClassInfo *iClassInfo)
{
  if (ClassRegistry->FindKey (iClassInfo->ClassID) >= 0)
    return false;
  // Create a factory and add it to class registry
  ClassRegistry->Push (new scfFactory (iClassInfo));
  SortClassRegistry = true;
  return true;
}

bool csSCF::RegisterClassList (scfClassInfo *iClassInfo)
{
  while (iClassInfo->ClassID)
    if (!RegisterStaticClass (iClassInfo++))
      return false;
  return true;
}

bool csSCF::UnregisterClass (char *iClassID)
{
  // If we have no class registry, we aren't initialized (or were finalized)
  if (!ClassRegistry)
    return false;

  int idx = ClassRegistry->FindKey (iClassID);

  if (idx < 0)
    return false;

  ClassRegistry->Delete (idx);
  SortClassRegistry = true;
  return true;
}

const char *csSCF::GetClassDescription (const char *iClassID)
{
  int idx = ClassRegistry->FindKey (iClassID);

  if (idx >= 0)
  {
    iFactory *cf = (iFactory *)ClassRegistry->Get (idx);
    return cf->QueryDescription ();
  } /* endif */

  return NULL;
}

const char *csSCF::GetClassDependencies (const char *iClassID)
{
  int idx = ClassRegistry->FindKey (iClassID);

  if (idx >= 0)
  {
    iFactory *cf = (iFactory *)ClassRegistry->Get (idx);
    return cf->QueryDependencies ();
  } /* endif */

  return NULL;
}

bool csSCF::ClassRegistered (const char *iClassID)
{
  return (ClassRegistry->FindKey (iClassID) >= 0);
}
