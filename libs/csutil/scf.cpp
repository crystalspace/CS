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
#include "cssysdef.h"
#include "csutil/scf.h"
#include "csutil/csshlib.h"
#include "csutil/sysfunc.h"
#include "csutil/syspath.h"
#include "csutil/csstring.h"
#include "csutil/parray.h"
#include "csutil/memfile.h"
#include "csutil/scfstringarray.h"
#include "csutil/scopedmutexlock.h"
#include "csutil/strset.h"
#include "csutil/util.h"
#include "csutil/xmltiny.h"
#include "iutil/document.h"

#ifdef CS_REF_TRACKER
#include "reftrack.h"
#endif

/// This is the registry for all class factories
static class scfClassRegistry *ClassRegistry = 0;
/// If this bool is true, we should sort the registery
static bool SortClassRegistry = false;
/// This is our private instance of csSCF
static class csSCF *PrivateSCF = 0;

/**
 * This class manages all SCF functionality.
 * The SCF module loader routines (CreateInstance, RegisterClass, ...)
 * are all thread-safe.
 */
class csSCF : public iSCF
{
private:
  csRef<csMutex> mutex;

  csStringSet contexts;
  csStringID staticContextID;

  void RegisterClassesInt (char const* pluginPath, iDocumentNode* scfnode, 
    const char* context = 0);
#ifdef CS_REF_TRACKER
  csRefTracker* refTracker;
#endif
public:
  SCF_DECLARE_IBASE;

  /// The global table of all known interface names
  csStringSet InterfaceRegistry;
  
  /// constructor
  csSCF ();
  /// destructor
  virtual ~csSCF ();

  virtual void RegisterClasses (iDocument*, const char* context = 0);
  virtual void RegisterClasses (char const*, const char* context = 0);
  virtual void RegisterClasses (char const* pluginPath, 
    iDocument* scfnode, const char* context = 0);
  virtual bool RegisterClass (const char *iClassID,
    const char *iLibraryName, const char *iFactoryClass,
    const char *Description, const char *Dependencies = 0, 
    const char* context = 0);
  virtual bool RegisterClass (scfFactoryFunc, const char *iClassID,
    const char *Description, const char *Dependencies = 0, 
    const char* context = 0);
  virtual bool RegisterFactoryFunc (scfFactoryFunc, const char *FactClass);
  virtual bool ClassRegistered (const char *iClassID);
  virtual void *CreateInstance (const char *iClassID,
    const char *iInterfaceID, int iVersion);
  virtual const char *GetClassDescription (const char *iClassID);
  virtual const char *GetClassDependencies (const char *iClassID);
  virtual csRef<iDocument> GetPluginMetadata (char const *iClassID);
  virtual bool UnregisterClass (const char *iClassID);
  virtual void UnloadUnusedModules ();
  virtual void ScanPluginsPath (const char* path, bool recursive = false,
    const char* context = 0);
  virtual char const* GetInterfaceName (scfInterfaceID) const;
  virtual scfInterfaceID GetInterfaceID (const char *iInterface);
  virtual void Finish ();
  virtual csRef<iStringArray> QueryClassList (char const* pattern);
  virtual bool RegisterPlugin (const char* path);
};

#ifndef CS_STATIC_LINKED

class scfSharedLibrary;

class scfLibraryVector : public csPDelArray<scfSharedLibrary>
{
public:
  static int CompareName (scfSharedLibrary* const& x, char const* const& n);
  size_t FindLibrary(char const* name) const
  {
    return FindKey(
      csArrayCmp<scfSharedLibrary*,char const*>(name, CompareName));
  }
};

// This is the registry for all shared libraries
static scfLibraryVector *LibraryRegistry = 0;

/// A object of this class represents a shared library
class scfSharedLibrary
{
  friend class scfLibraryVector;
  friend class scfFactory;
  // Shared library name
  char *LibraryName;
  // Handle of shared module (if RefCount > 0)
  csLibraryHandle LibraryHandle;
  // Number of references to this shared library
  int RefCount;
  // Base name used for composition of library intialize and shutdown
  // functions.  All exported factory classes will implement initialize and
  // shutdown functions, but we choose only a single pair to perform this work
  // on behalf of the library.
  char *FuncCoreName;

public:
  /// Create a shared library and load it
  scfSharedLibrary (const char *iLibraryName, const char *iCoreName);

  /// Destroy a shared library object
  virtual ~scfSharedLibrary ();

  /// Check if library object is okay
  bool ok () const
  { return (LibraryHandle != 0); }

  /// Increment reference count for the library
  void IncRef ()
  { RefCount++; }

  /// Decrement reference count for the library
  void DecRef ()
  { RefCount--; }

  /// Get the reference count.
  int GetRefCount () const
  { return RefCount; }

  /// Try to free the library, if refcount is zero
  bool TryUnload ()
  {
    if (RefCount <= 0)
    {
      LibraryRegistry->DeleteIndex (LibraryRegistry->Find (this));
      return true;
    }
    return false;
  }
};

scfSharedLibrary::scfSharedLibrary (const char *lib, const char *core)
{
  LibraryRegistry->Push (this);

  RefCount = 0;
  LibraryName = csStrNew (lib);
  FuncCoreName = csStrNew (core);
  LibraryHandle = csLoadLibrary (LibraryName);

  if (LibraryHandle != 0)
  {
    typedef void (*scfInitFunc)(iSCF*);
    csString sym;
    sym << FuncCoreName << "_scfInitialize";
    scfInitFunc initfunc = (scfInitFunc)csGetLibrarySymbol(LibraryHandle, sym);
    if (initfunc)
      initfunc(PrivateSCF);
    else
    {
      fprintf (stderr, "SCF ERROR: '%s' doesn't export '%s'\n", LibraryName,
        sym.GetData ());
      csPrintLibraryError(sym);
    }
  }
  else
    csPrintLibraryError(LibraryName);
}

scfSharedLibrary::~scfSharedLibrary ()
{
  if (LibraryHandle)
  {
    typedef void (*scfFinisFunc)();
    csString sym;
    sym << FuncCoreName << "_scfFinalize";
    scfFinisFunc func = (scfFinisFunc)csGetLibrarySymbol(LibraryHandle, sym);
    if (func)
      func();
    else
    {
      fprintf (stderr, "SCF ERROR: '%s' doesn't export '%s'\n", LibraryName,
        sym.GetData ());
      csPrintLibraryError(sym);
    }
    csUnloadLibrary (LibraryHandle);
  }
  delete [] FuncCoreName;
  delete [] LibraryName;
}

int scfLibraryVector::CompareName (scfSharedLibrary* const& Item,
  char const* const& Key)
{
  return strcmp (Item->LibraryName, Key);
}

#endif // CS_STATIC_LINKED

/// This structure contains everything we need to know about a particular class
class scfFactory : public iFactory
{
public:
  SCF_DECLARE_IBASE;

  // Class identifier
  char *ClassID;
  // Class description
  char *Description;
  // The dependency list
  char *Dependencies;
  // Name of factory implementation class in shared library.
  char *FactoryClass;
  // Function which actually creates an instance
  scfFactoryFunc CreateFunc;
  /*
    "Context" of this class (used to decide whether a class conflict will 
    be reported)
  */
  csStringID classContext;
#ifndef CS_STATIC_LINKED
  // Shared module that implements this class or 0 for local classes
  char *LibraryName;
  // A pointer to shared library object (0 for local classes)
  scfSharedLibrary *Library;
#endif

  // Create the factory for a class located in a shared library
  scfFactory (const char *iClassID, const char *iLibraryName,
    const char *iFactoryClass, scfFactoryFunc iCreateFunc, const char *iDesc,
    const char *iDepend, csStringID context = csInvalidStringID);
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
class scfClassRegistry : public csPDelArray<scfFactory>
{
  typedef csPDelArray<scfFactory> superclass;
public:
  scfClassRegistry () : superclass(16, 16) {}
  static int CompareClass (scfFactory* const& Item, char const* const& key)
  { return strcmp (Item->ClassID, key); }
  size_t FindClass(char const* name, bool assume_sorted = false) const
  {
    if (assume_sorted)
      return FindSortedKey(csArrayCmp<scfFactory*,char const*>(name,
        CompareClass));
    return FindKey(csArrayCmp<scfFactory*,char const*>(name, CompareClass));
  }
  static int Compare (scfFactory* const& f1, scfFactory* const& f2)
  { return strcmp (f1->ClassID, f2->ClassID); }
  void Sort()
  { superclass::Sort(Compare); }
};

//----------------------------------------- Class factory implementation ----//

scfFactory::scfFactory (const char *iClassID, const char *iLibraryName,
  const char *iFactoryClass, scfFactoryFunc iCreate, const char *iDescription,
  const char *iDepend, csStringID context)
{
  csRefTrackerAccess::SetDescription (this, CS_TYPENAME(*this));
  // Don't use SCF_CONSTRUCT_IBASE (0) since it will call IncRef()
  scfWeakRefOwners = 0;
  scfRefCount = 0; scfParent = 0;
  ClassID = csStrNew (iClassID);
  Description = csStrNew (iDescription);
  Dependencies = csStrNew (iDepend);
  FactoryClass = csStrNew (iFactoryClass);
  CreateFunc = iCreate;
  classContext = context;
#ifndef CS_STATIC_LINKED
  LibraryName = csStrNew (iLibraryName);
  Library = 0;
#else
  (void)iLibraryName;
#endif

  SCF_INIT_TRACKER_ALIASES
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
  delete [] FactoryClass;
  delete [] Dependencies;
  delete [] Description;
  delete [] ClassID;

  csRefTrackerAccess::TrackDestruction (this, scfRefCount);
}

void scfFactory::IncRef ()
{
  csRefTrackerAccess::TrackIncRef (this, scfRefCount);
#ifndef CS_STATIC_LINKED
  if (!Library && LibraryName)
  {
    size_t libidx = LibraryRegistry->FindLibrary(LibraryName);
    if (libidx != (size_t)-1)
      Library = (scfSharedLibrary *)LibraryRegistry->Get (libidx);
    else
      Library = new scfSharedLibrary (LibraryName, FactoryClass);

    if (Library->ok ())
    {
      if (CreateFunc == 0)
      {
        csString sym;
	sym << FactoryClass << "_Create";
	CreateFunc =
	  (scfFactoryFunc)csGetLibrarySymbol(Library->LibraryHandle, sym);
	if (CreateFunc == 0)
	  csPrintLibraryError(sym);
      }
    }
      

    if (!Library->ok () || CreateFunc == 0)
    {
      Library = 0;
      return; // Signify that IncRef() failed by _not_ incrementing count.
    }
    
    Library->IncRef ();
  }
#endif
  scfRefCount++;
}

void scfFactory::DecRef ()
{
  csRefTrackerAccess::TrackDecRef (this, scfRefCount);
#ifdef CS_DEBUG
  if (scfRefCount == 0)
  {
    fprintf (stderr, "SCF WARNING: Extra calls to scfFactory::DecRef () for "
      "class %s\n", ClassID);
    return;
  }
#endif
  scfRefCount--;
#ifndef CS_STATIC_LINKED
  if (scfRefCount == 0)
  {
    // now we no longer need the library either
    if (Library)
    {
      Library->DecRef ();
      Library = 0;
    }
  }
#endif
}

void scfFactory::AddRefOwner (iBase**)
{
  // @@@ TODO
}

void scfFactory::RemoveRefOwner (iBase**)
{
  // @@@ TODO
}

int scfFactory::GetRefCount ()
{
  return scfRefCount;
}

void *scfFactory::QueryInterface (scfInterfaceID iInterfaceID, int iVersion)
{
  SCF_IMPLEMENTS_INTERFACE (iFactory);
  return 0;
}

void *scfFactory::CreateInstance ()
{
  IncRef ();
  // If IncRef won't succeed, we'll have a zero reference counter
  if (!scfRefCount)
    return 0;

  void *instance = CreateFunc(this);

  // No matter whenever we succeeded or not, decrement the refcount
  DecRef ();

  return instance;
}

void scfFactory::TryUnload ()
{
}

const char *scfFactory::QueryDescription ()
{
  if (scfRefCount)
    return Description;
  else
    return 0;
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

SCF_IMPLEMENT_IBASE (csSCF);
  /// @@@ plain IMPL_INTF calls the ref tracker,  problematic
  SCF_IMPLEMENTS_INTERFACE_COMMON (iSCF, this) 
#ifdef CS_REF_TRACKER
    if (refTracker) 
    { 
      SCF_IMPLEMENTS_INTERFACE_COMMON (iRefTracker, refTracker); 
    }
#endif
SCF_IMPLEMENT_IBASE_END;

static void scfScanPlugins (csPluginPaths* pluginPaths, const char* context)
{
#ifndef CS_STATIC_LINKED

  if (pluginPaths)
  {
    // Search plugins in pluginpaths
    csRef<iStringArray> plugins;

    size_t i, j;
    for (i = 0; i < pluginPaths->GetCount(); i++)
    {
      if (plugins) plugins->DeleteAll();

      csRef<iStringArray> messages = 
	csScanPluginDir ((*pluginPaths)[i].path, plugins, 
	  (*pluginPaths)[i].scanRecursive);

      if ((messages != 0) && (messages->Length() > 0))
      {
	fprintf (stderr,
	  "The following issue(s) came up while scanning '%s':",
	  (*pluginPaths)[i].path);

	for (j = 0; j < messages->Length(); j++)
	{
	  fprintf(stderr,
	    " %s\n", messages->Get (j));
	}
      }

      csRef<iDocument> metadata;
      csRef<iString> msg;
      for (j = 0; j < plugins->Length(); j++)
      {
	msg = csGetPluginMetadata (plugins->Get (j), metadata);
	if (msg != 0)
	{
	  fprintf (stderr,
	    "Error retrieving metadata for %s: %s\n",
	    plugins->Get (j), msg->GetData ());
	}
	// It is possible for an error or warning message to be generated even
	// when metadata is also obtained.  Likewise, it is possible for no
	// metadata to be obtained yet also to have no error message.  The
	// latter case is indicative of csScanPluginDir() returning "potential"
	// plugins which turn out to not be Crystal Space plugins after all.
	// For instance, on Windows, the scan might find a DLL which is not a
	// Crystal Space DLL; that is, which does not contain embedded
	// metadata.  This is a valid case, which we simply ignore since it is
	// legal for non-CS libraries to exist alongside CS plugins in the
	// scanned directories.
	if (metadata)
	{
	  PrivateSCF->RegisterClasses (plugins->Get (j), metadata, 
	    context ? context : (*pluginPaths)[i].type);
	}
      }
    }
  }
#endif
}

void scfInitialize (csPluginPaths* pluginPaths)
{
  if (!PrivateSCF)
    PrivateSCF = new csSCF ();

  scfScanPlugins (pluginPaths, 0);
}

void scfInitialize (int argc, const char* const argv[])
{
  csPluginPaths* pluginPaths = csGetPluginPaths (argv[0]);

  scfInitialize (pluginPaths);

  delete pluginPaths;
}

			       
csSCF::csSCF () : 
#ifdef CS_REF_TRACKER
  refTracker(0), 
#endif
  scfRefCount(1), scfWeakRefOwners(0), scfParent(0)
{
  SCF = PrivateSCF = this;
#ifdef CS_DEBUG
  object_reg = 0;
#endif

#ifdef CS_REF_TRACKER
  refTracker = new csRefTracker();
#endif

  if (!ClassRegistry)
    ClassRegistry = new scfClassRegistry ();

#ifndef CS_STATIC_LINKED
  if (!LibraryRegistry)
    LibraryRegistry = new scfLibraryVector ();
#endif

  // We need a recursive mutex.
  mutex = csMutex::Create (true);

  staticContextID = contexts.Request (SCF_STATIC_CLASS_CONTEXT);
}

csSCF::~csSCF ()
{
  delete ClassRegistry;
  ClassRegistry = 0;
#ifndef CS_STATIC_LINKED
  UnloadUnusedModules ();
  delete LibraryRegistry;
  LibraryRegistry = 0;
#endif

  mutex = 0;
#ifdef CS_REF_TRACKER
  refTracker->Report ();
  delete refTracker;
#endif
  SCF = PrivateSCF = 0;
}

void csSCF::RegisterClasses (char const* xml, const char* context)
{
  csMemFile file(xml, strlen(xml));
  csTinyDocumentSystem docsys;
  csRef<iDocument> doc = docsys.CreateDocument();
  if (doc->Parse(&file) == 0)
    RegisterClasses(doc, context);
}

void csSCF::RegisterClasses (iDocument* doc, const char* context)
{
  RegisterClasses (0, doc, context);
}

void csSCF::RegisterClasses (char const* pluginPath, 
    iDocument* doc, const char* context)
{
  if (doc)
  {
    csRef<iDocumentNode> rootnode = doc->GetRoot();
    if (rootnode != 0)
    {
      csRef<iDocumentNode> pluginnode = rootnode->GetNode("plugin");
      if (pluginnode)
      {
	csRef<iDocumentNode> scfnode = pluginnode->GetNode("scf");
	if (scfnode.IsValid())
	  RegisterClassesInt (pluginPath, scfnode, context);
	else
	  fprintf(stderr, "csSCF::RegisterClasses: Missing <scf> node.\n");
      }
      else
        fprintf(stderr,
	  "csSCF::RegisterClasses: Missing root <plugin> node.\n");
    }
  }
}

static char const* get_node_value(csRef<iDocumentNode> parent, char const* key)
{
  csRef<iDocumentNode> node = parent->GetNode(key);
  return node.IsValid() ? node->GetContentsValue() : "";
}

void csSCF::RegisterClassesInt(char const* pluginPath, iDocumentNode* scfnode, 
			       const char* context)
{
  csRef<iDocumentNode> classesnode = scfnode->GetNode("classes");
  if (classesnode)
  {
    csRef<iDocumentNodeIterator> classiter = classesnode->GetNodes("class");
    csRef<iDocumentNode> classnode;
    while ((classnode = classiter->Next()))
    {
      csString classname = get_node_value(classnode, "name");
      csString imp = get_node_value(classnode, "implementation");
      csString desc = get_node_value(classnode, "description");

      // For backward compatibility, we build a comma-delimited dependency
      // string from the individual dependency nodes.  In the future,
      // iSCF::GetClassDependencies() should be updated to return an
      // iStringArray, rather than a simple comma-delimited string.
      csString depend;
      csRef<iDocumentNode> depnode = classnode->GetNode("requires");
      if (depnode)
      {
	csRef<iDocumentNodeIterator> depiter = depnode->GetNodes("class");
	csRef<iDocumentNode> depclassnode;
	while ((depclassnode = depiter->Next()))
	{
	  if (!depend.IsEmpty()) depend << ", ";
	  depend << depclassnode->GetContentsValue();
	}
      }

      char const* pdepend = (depend.IsEmpty() ? 0 : depend.GetData());
      RegisterClass(classname, pluginPath, imp, desc, pdepend, context);
    }
  }
}

void csSCF::Finish ()
{
  delete this;
}

void *csSCF::CreateInstance (const char *iClassID, const char *iInterface,
  int iVersion)
{
  csScopedMutexLock lock (mutex);

  // Pre-sort class registry for doing binary searches
  if (SortClassRegistry)
  {
    ClassRegistry->Sort();
    SortClassRegistry = false;
  }

  size_t idx = ClassRegistry->FindClass(iClassID, true);
  void *instance = 0;

  if (idx != (size_t)-1)
  {
    iFactory *cf = (iFactory *)ClassRegistry->Get (idx);
    iBase *object = (iBase *)cf->CreateInstance ();
    if (object)
    {
      instance = object->QueryInterface(GetInterfaceID(iInterface), iVersion);
      object->DecRef ();

      if (!instance)
        fprintf (stderr,
	  "SCF_WARNING: factory returned a null instance for %s\n"
	  "\tif error messages are not self explanatory, recompile CS with "
	  "CS_DEBUG\n", iClassID);
    }
  } /* endif */

  UnloadUnusedModules ();

  return instance;
}

void csSCF::UnloadUnusedModules ()
{
#ifndef CS_STATIC_LINKED
  csScopedMutexLock lock (mutex);
  for (size_t i = LibraryRegistry->Length (); i > 0; i--)
  {
    scfSharedLibrary *sl = (scfSharedLibrary *)LibraryRegistry->Get (i - 1);
    sl->TryUnload ();
  }
#endif
}

inline static bool ContextClash (csStringID contextA, csStringID contextB)
{
  return (
    (contextA != csInvalidStringID) && 
    (contextB != csInvalidStringID) && 
    (contextA == contextB));
}

bool csSCF::RegisterClass (const char *iClassID, const char *iLibraryName,
  const char *iFactoryClass, const char *iDesc, const char *Dependencies, 
  const char* context)
{
  csScopedMutexLock lock (mutex);
  size_t idx;
  csStringID contextID = 
    context ? contexts.Request (context) : csInvalidStringID;
  if ((idx = ClassRegistry->FindClass(iClassID)) != (size_t)-1)
  {
    scfFactory *cf = (scfFactory *)ClassRegistry->Get (idx);
    if (ContextClash (cf->classContext, contextID))
    {
      fprintf (stderr, "SCF_WARNING: class %s has already been registered in "
        "the same context ('%s')\n", iClassID, context);
    }
    else
    {
      /*
        The user may want to override a standard CS plugin by putting
	a plugin exhibiting the same class ID into the e.g. app directory.
	In this case a warning is probably not desired. But for debugging
	purposes we emit something.
       */
    #ifdef CS_DEBUG
      // Don't report when the already registered class is static.
      if (cf->classContext != staticContextID)
      {
	// @@@ some way to have this warning in non-debug builds would be nice.
	fprintf (stderr, "SCF_NOTIFY: class %s has already been registered in "
	  "a different context ('%s' vs '%s') (this message appears only in "
	  "debug builds)\n", iClassID, context, 
	  (cf->classContext != csInvalidStringID) ?
	  contexts.Request (cf->classContext) : 0);
      }
    #endif
    }
    return false;
  }
  scfFactory* factory = new scfFactory (iClassID, iLibraryName, iFactoryClass,
    0, iDesc, Dependencies, contextID);
  ClassRegistry->Push (factory);
  SortClassRegistry = true;
  return true;
}

bool csSCF::RegisterClass (scfFactoryFunc Func, const char *iClassID,
  const char *Desc, const char *Dependencies, const char* context)
{
  csScopedMutexLock lock (mutex);
  size_t idx;
  csStringID contextID = 
    context ? contexts.Request (context) : csInvalidStringID;
  if ((idx = ClassRegistry->FindClass(iClassID)) != (size_t)-1)
  {
    scfFactory *cf = (scfFactory *)ClassRegistry->Get (idx);
    if (ContextClash (cf->classContext, contextID))
    {
      fprintf (stderr, "SCF_WARNING: class %s has already been registered in "
	"the same context (%s)\n", iClassID, context);
    }
    else
    {
      /*
        The user may want to override a standard CS plugin by putting
	a plugin exhibiting the same class ID into the e.g. app directory.
	In this case a warning is probably not desired. But for debugging
	purposes we emit something.
       */
    #ifdef CS_DEBUG
      // Don't report when the already registered class is static.
      if (cf->classContext != staticContextID)
      {
	// @@@ some way to have this warning in non-debug builds would be nice.
	fprintf (stderr, "SCF_NOTIFY: class %s has already been registered "
	  "in a different context (%s vs %s) (this message appears only in "
	  "debug builds)\n", iClassID, context,
	  contexts.Request (cf->classContext));
      }
    #endif
    }
    return false;
  }
  scfFactory* factory =
    new scfFactory (iClassID, 0, 0, Func, Desc, Dependencies, contextID);
  ClassRegistry->Push (factory);
  SortClassRegistry = true;
  return true;
}

bool csSCF::RegisterFactoryFunc (scfFactoryFunc Func, const char *FactClass)
{
  bool ok = false;
  csScopedMutexLock lock (mutex);
  for (size_t i = 0, n = ClassRegistry->Length(); i < n; i++)
  {
    scfFactory* fact = (scfFactory*)ClassRegistry->Get(i);
    if (fact->FactoryClass != 0 && strcmp(fact->FactoryClass, FactClass) == 0)
    {
      if (fact->CreateFunc == 0)
      {
        fact->CreateFunc = Func;
	ok = true;
      }
      //break;
      /*
        Don't break!
	Some factory functions are used for more than 1 class,
	so EVERY factory class has to be checked...
	Horribly inefficient!
       */
    }
  }
  return ok;
}

bool csSCF::UnregisterClass (const char *iClassID)
{
  csScopedMutexLock lock (mutex);

  // If we have no class registry, we aren't initialized (or were finalized)
  if (!ClassRegistry)
    return false;

  size_t idx = ClassRegistry->FindClass(iClassID);

  if (idx == (size_t)-1)
    return false;

  ClassRegistry->DeleteIndex (idx);
  SortClassRegistry = true;
  return true;
}

void csSCF::ScanPluginsPath (const char* path, bool recursive,
  const char* context)
{
  csPluginPaths paths;
  paths.AddOnce (path, recursive);
  scfScanPlugins (&paths, context);
}

bool csSCF::RegisterPlugin (const char* path)
{
  csRef<iDocument> metadata;
  csRef<iString> msg;

  if ((msg = csGetPluginMetadata (path, metadata)) != 0)
  {
    fprintf (stderr,
      "SCF_ERROR: couldn't retrieve metadata for '%s': %s\n", 
      path, msg->GetData ());
    return false;
  }

  RegisterClasses (path, metadata);

  return true;
}

const char *csSCF::GetClassDescription (const char *iClassID)
{
  csScopedMutexLock lock (mutex);

  size_t idx = ClassRegistry->FindClass(iClassID);
  if (idx != (size_t)-1)
  {
    iFactory *cf = (iFactory *)ClassRegistry->Get (idx);
    return cf->QueryDescription ();
  }

  return 0;
}

const char *csSCF::GetClassDependencies (const char *iClassID)
{
  csScopedMutexLock lock (mutex);

  size_t idx = ClassRegistry->FindClass(iClassID);
  if (idx != (size_t)-1)
  {
    iFactory *cf = (iFactory *)ClassRegistry->Get (idx);
    return cf->QueryDependencies ();
  }

  return 0;
}

csRef<iDocument> csSCF::GetPluginMetadata (char const *iClassID)
{
  csRef<iDocument> metadata;
#ifndef CS_STATIC_LINKED
  csScopedMutexLock lock (mutex);
  size_t idx = ClassRegistry->FindClass(iClassID);
  if (idx != (size_t)-1)
  {
    scfFactory *cf = ClassRegistry->Get (idx);
    if (cf->LibraryName != 0)
      csGetPluginMetadata(cf->LibraryName, metadata);
  }

#endif
  return metadata;
}

bool csSCF::ClassRegistered (const char *iClassID)
{
  csScopedMutexLock lock (mutex);
  return (ClassRegistry->FindClass(iClassID) >= 0);
}


char const* csSCF::GetInterfaceName (scfInterfaceID i) const
{
  csScopedMutexLock lock (mutex);
  return InterfaceRegistry.Request(i);
}

scfInterfaceID csSCF::GetInterfaceID (const char *iInterface)
{
#ifdef CS_REF_TRACKER
  /*
    Bit of a hack: when 'mutex' is assigned, this very function
    is called (to get the iRefTracker id). So mutex is 0 in this
    case, we need to test for that.
   */
  if (!mutex)
  {
    return (scfInterfaceID)InterfaceRegistry.Request (iInterface);
  }
#endif
  csScopedMutexLock lock (mutex);
  return (scfInterfaceID)InterfaceRegistry.Request (iInterface);
}

csRef<iStringArray> csSCF::QueryClassList (char const* pattern)
{
  iStringArray* v = new scfStringArray();

  csScopedMutexLock lock (mutex);
  size_t const rlen = ClassRegistry->Length();
  if (rlen != 0)
  {
    size_t const plen = (pattern ? strlen(pattern) : 0);
    for (size_t i = 0; i < rlen; i++)
    {
      char const* s = ((iFactory*)ClassRegistry->Get(i))->QueryClassID();
      if (plen == 0 || strncasecmp(pattern, s, plen) == 0)
        v->Push (s);
    }
  }
  return csPtr<iStringArray> (v);
}
