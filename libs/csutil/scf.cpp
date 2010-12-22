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

// for setlocale(), see below why that is called
#include <locale.h>

#if defined(CS_REF_TRACKER) && !defined(CS_REF_TRACKER_EXTENSIVE)
  // Performance hack
  #undef CS_REF_TRACKER
  #define CS_REF_TRACKER_REDEFINE
#endif
#include "csutil/ref.h"
#ifdef CS_REF_TRACKER_REDEFINE
  #define CS_REF_TRACKER
#endif

#include "csutil/scf.h"
#include "csutil/csshlib.h"
#include "csutil/sysfunc.h"
#include "csutil/syspath.h"
#include "csutil/csstring.h"
#include "csutil/parray.h"
#include "csutil/memfile.h"
#include "csutil/scfstringarray.h"
#include "csutil/threading/mutex.h"
#include "csutil/strset.h"
#include "csutil/util.h"
#include "csutil/verbosity.h"
#include "csutil/xmltiny.h"
#include "iutil/document.h"

#define NEW_PLUGIN_PATHS

#ifdef NEW_PLUGIN_PATHS
#define csGetPluginPaths  csInstallationPathsHelper::GetPluginPaths
#endif

#ifdef CS_REF_TRACKER
#include "reftrack.h"
#endif

#if (defined(CS_EXTENSIVE_MEMDEBUG) && defined(CS_COMPILER_MSVC)) || \
  defined(CS_MEMORY_TRACKER) || defined(CS_REF_TRACKER)
/*
  Lazily unload libs - Because if unloading happens immediately,
  the source file information for leaked objects would get lost.
  But unload them nevertheless so memory checker don't report leaked
  handles.
  Similar for reftracking - but here it's also debug symbols.
 */
#define LAZY_UNLOAD
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
class csSCF : public iSCF, public scfImplementation<csSCF>
{
private:
  mutable CS::Threading::RecursiveMutex mutex;
  unsigned int verbose;

  csStringSet contexts;
  csStringID staticContextID;
  csStringSet scannedDirs;

#ifdef CS_REF_TRACKER
  csRefTracker* refTracker;
#endif

  char const* GetContextName(csStringID s)
  { return s != csInvalidStringID ? contexts.Request(s) : "{none}"; }
  char const* GetContextName(char const* s)
  { return s != 0 ? s : "{none}"; }
  void RegisterClassesInt (char const* pluginPath, iDocumentNode* scfnode, 
    const char* context = 0);
  void ScanPluginsInt(csPathsList const*, char const* context);

  friend void scfInitialize (csPathsList const*, unsigned int);

  virtual size_t GetInterfaceMetadataCount () const { return 0;}
  virtual void FillInterfaceMetadata (size_t n) {;}

  virtual void *QueryInterface (scfInterfaceID iInterfaceID,
    scfInterfaceVersion iVersion);

public:

  /// The global table of all known interface names
  csStringSet InterfaceRegistry;
#ifdef LAZY_UNLOAD
  /// Modules to lazily unload at end
  csArray<csLibraryHandle> lazyUnloadLibs;
#endif
  
  /**
   * Constructor.
   * \param verbosity One or more of the \c SCF_VERBOSE_FOO constants combined
   *   with bitwise-or to control verbosity.
   */
  csSCF (unsigned int verbosity);
  /// Destructor.
  virtual ~csSCF ();

  bool IsVerbose(unsigned int mask) const { return (verbose & mask) != 0; }
  unsigned int GetVerbose() const { return verbose; }
  void SetVerbose(unsigned int v) { verbose = v; }

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
  virtual iBase *CreateInstance (const char *iClassID);
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

class scfSharedLibrary;

static csStringSet* libraryNames = 0; 
static char const* get_library_name(csStringID s)
{ return s != csInvalidStringID ? libraryNames->Request(s) : "{none}"; }

class scfLibraryVector : public csPDelArray<scfSharedLibrary>
{
public:
  static int CompareName (scfSharedLibrary* const& x, csStringID const& n);
  size_t FindLibrary(csStringID name) const
  {
    return FindKey(csArrayCmp<scfSharedLibrary*,csStringID>(name,CompareName));
  }
};

// This is the registry for all shared libraries
static scfLibraryVector *LibraryRegistry = 0;

/// A object of this class represents a shared library
class scfSharedLibrary : public CS::Memory::CustomAllocated
{
  typedef void (*scfInitFunc)(iSCF*);
  typedef void (*scfFinisFunc)();

  friend class scfLibraryVector;
  friend class scfFactory;
  // Shared library name
  csStringID LibraryName;
  // Handle of shared module (if RefCount > 0)
  csLibraryHandle LibraryHandle;
  // Number of references to this shared library
  int RefCount;
  // Intialize and shutdown functions.  All exported factory classes will
  // implement initialize and shutdown functions, but we choose only a single
  // pair to perform this work on behalf of the library.
  scfInitFunc initFunc;
  scfFinisFunc finisFunc;

public:
  /// Create a shared library and load it
  scfSharedLibrary (csStringID iLibraryName, const char *iCoreName);

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

scfSharedLibrary::scfSharedLibrary (csStringID libraryName, const char *core)
{
  LibraryRegistry->Push (this);

  RefCount = 0;
  LibraryName = libraryName;
  const char* lib = get_library_name(LibraryName);

  if (PrivateSCF->IsVerbose(SCF_VERBOSE_PLUGIN_LOAD))
    csPrintfErr("SCF_NOTIFY: loading plugin %s to satisfy request for %s\n",
      lib, core);

  LibraryHandle = csLoadLibrary (lib);

  if (LibraryHandle != 0)
  {
    csString sym;
    sym << core << "_scfInitialize";
    initFunc = (scfInitFunc)csGetLibrarySymbol(LibraryHandle, sym);
    if (!initFunc)
    {
      csPrintfErr("SCF_ERROR: '%s' doesn't export '%s'\n", lib, sym.GetData());
      csPrintLibraryError (sym);
    }
    sym.Clear ();
    sym << core << "_scfFinalize";
    finisFunc = (scfFinisFunc)csGetLibrarySymbol(LibraryHandle, sym);
    if (!finisFunc)
    {
      csPrintfErr("SCF_ERROR: '%s' doesn't export '%s'\n", lib, sym.GetData());
      csPrintLibraryError (sym);
    }
    if (initFunc && finisFunc)
      initFunc (PrivateSCF);
  }
  else
    csPrintLibraryError (lib);
}

scfSharedLibrary::~scfSharedLibrary ()
{
  if (LibraryHandle)
  {
    if (initFunc && finisFunc)
      finisFunc();
    if (PrivateSCF->IsVerbose(SCF_VERBOSE_PLUGIN_LOAD))
      csPrintfErr("SCF_NOTIFY: unloading plugin %s\n",
	get_library_name(LibraryName));
#ifndef LAZY_UNLOAD
    csUnloadLibrary (LibraryHandle);
#else
    PrivateSCF->lazyUnloadLibs.Push (LibraryHandle);
#endif
  }
}

int scfLibraryVector::CompareName (scfSharedLibrary* const& Item,
  csStringID const& Key)
{
  return Item->LibraryName < Key ? -1 : (Item->LibraryName > Key ? 1 : 0);
}

/// This structure contains everything we need to know about a particular class
class scfFactory : public iFactory, public CS::Memory::CustomAllocated
{
public:
  // Class identifier
  const char *ClassID;
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
  // Shared module that implements this class or 0 for local classes
  csStringID LibraryName;
  // A pointer to shared library object (0 for local classes)
  scfSharedLibrary *Library;

  // Create the factory for a class located in a shared library
  scfFactory (const char *iClassID, const char *iLibraryName,
    const char *iFactoryClass, scfFactoryFunc iCreateFunc, const char *iDesc,
    const char *iDepend, csStringID context = csInvalidStringID);
  // Free the factory object (but not objects created by this factory)
  virtual ~scfFactory ();
  // Create a insance of class this factory represents
  virtual iBase *CreateInstance ();
  // Try to unload class module (i.e. shared module)
  virtual void TryUnload ();
  /// Query class description string
  virtual const char *QueryDescription ();
  /// Query class dependency strings
  virtual const char *QueryDependencies ();
  /// Query class ID
  virtual const char *QueryClassID();
  /// Query library module name.
  virtual const char *QueryModuleName ();

protected:
  /* SCF goop, this class cannot use scfImplementation1 due to its special 
    ref-counting usage */
  int scfRefCount;
  typedef csArray<void**,
    csArrayElementHandler<void**>,
    CS::Memory::AllocatorMalloc,
    csArrayCapacityLinear<csArrayThresholdFixed<4> > > WeakRefOwnerArray;
  WeakRefOwnerArray* scfWeakRefOwners;

  virtual void IncRef ();
  virtual void DecRef ();
  virtual int GetRefCount ();
  virtual void AddRefOwner (void** ref_owner);
  virtual void RemoveRefOwner (void** ref_owner);
  scfInterfaceMetadataList* GetInterfaceMetadata () { return 0; }
  virtual void *QueryInterface (scfInterfaceID iInterfaceID, int iVersion);
};

/// This class holds a number of scfFactory structures
class scfClassRegistry : public csPDelArray<scfFactory>
{
  typedef csPDelArray<scfFactory> superclass;
public:
  scfClassRegistry () : superclass(16) {}
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

#ifdef CS_REF_TRACKER
static csStringHash* classIDs = 0; 
#endif

scfFactory::scfFactory (const char *iClassID, const char *iLibraryName,
  const char *iFactoryClass, scfFactoryFunc iCreate, const char *iDescription,
  const char *iDepend, csStringID context)
{
  csRefTrackerAccess::TrackConstruction (this);  
  csRefTrackerAccess::SetDescription (this, CS_TYPENAME(*this));
  scfWeakRefOwners = 0;
  scfRefCount = 0;
#ifdef CS_REF_TRACKER
  /* Reftracker: make sure class ID string survives destruction of this 
   * instance */
  ClassID = classIDs->Register (iClassID);
#else
  ClassID = CS::StrDup (iClassID);
#endif
  Description = CS::StrDup (iDescription);
  Dependencies = CS::StrDup (iDepend);
  FactoryClass = CS::StrDup (iFactoryClass);
  CreateFunc = iCreate;
  classContext = context;
  LibraryName =
    iLibraryName ? libraryNames->Request (iLibraryName) : csInvalidStringID;
  Library = 0;

  csRefTrackerAccess::AddAlias(
    static_cast<scfInterfaceTraits<iFactory>::InterfaceType*> (this),
    this);
}

scfFactory::~scfFactory ()
{
#ifdef CS_DEBUG
  // Warn user about unreleased instances of this class
  if (scfRefCount)
    csPrintfErr("SCF WARNING: %d unreleased instances of class %s!\n",
      scfRefCount, ClassID);
#endif

  if (scfWeakRefOwners)
  {
    for (size_t i = 0; i < scfWeakRefOwners->GetSize (); i++)
    {
      void** p = (*scfWeakRefOwners)[i];
      *p = 0;
    }
    delete scfWeakRefOwners;
    scfWeakRefOwners = 0;
  }

  if (Library)
    Library->DecRef ();
  cs_free (FactoryClass);
  cs_free (Dependencies);
  cs_free (Description);
#ifndef CS_REF_TRACKER
  cs_free (const_cast<char*> (ClassID));
#endif

  csRefTrackerAccess::TrackDestruction (this, scfRefCount);
}

void scfFactory::IncRef ()
{
  csRefTrackerAccess::TrackIncRef (this, scfRefCount);
  if (!Library && (LibraryName != csInvalidStringID))
  {
    size_t libidx = LibraryRegistry->FindLibrary(LibraryName);
    if (libidx != (size_t)-1)
      Library = (scfSharedLibrary *)LibraryRegistry->Get (libidx);
    else
      Library = new scfSharedLibrary (LibraryName, FactoryClass);

    if (Library->ok ())
    {
      csString sym;
      sym << FactoryClass << "_Create";
      CreateFunc =
        (scfFactoryFunc)csGetLibrarySymbol(Library->LibraryHandle, sym);
      if (CreateFunc == 0)
        csPrintLibraryError(sym);
    }

    if (!Library->ok () || CreateFunc == 0)
    {
      Library = 0;
      return; // Signify that IncRef() failed by _not_ incrementing count.
    }

    Library->IncRef ();
  }
  scfRefCount++;
}

void scfFactory::DecRef ()
{
  csRefTrackerAccess::TrackDecRef (this, scfRefCount);
#ifdef CS_DEBUG
  if (scfRefCount == 0)
  {
    csPrintfErr("SCF WARNING: extra invocations of scfFactory::DecRef() "
      "for class %s\n", ClassID);
    return;
  }
#endif
  scfRefCount--;
  if (scfRefCount == 0)
  {
    // now we no longer need the library either
    if (Library)
    {
      Library->DecRef ();
      Library = 0;
    }
  }
}

void scfFactory::AddRefOwner (void** ref_owner)
{
  if (!scfWeakRefOwners)						
    scfWeakRefOwners = new WeakRefOwnerArray (0, 4);			
  scfWeakRefOwners->InsertSorted (ref_owner);				
}

void scfFactory::RemoveRefOwner (void** ref_owner)
{
  if (!scfWeakRefOwners)						
    return;
  size_t index = scfWeakRefOwners->FindSortedKey (			
    csArrayCmp<void**, void**> (ref_owner)); 				
  if (index != csArrayItemNotFound) scfWeakRefOwners->DeleteIndex (	
    index); 								
}

int scfFactory::GetRefCount ()
{
  return scfRefCount;
}

void *scfFactory::QueryInterface (scfInterfaceID iInterfaceID, int iVersion)
{
  if (iInterfaceID == scfInterfaceTraits<iFactory>::GetID () &&
    scfCompatibleVersion (iVersion, scfInterfaceTraits<iFactory>::GetVersion ()))
  {
    this->IncRef ();
    return static_cast<iFactory*> (this);
  }
  return 0;
}

iBase *scfFactory::CreateInstance ()
{
  IncRef ();
  // If IncRef won't succeed, we'll have a zero reference counter
  if (!scfRefCount)
    return 0;

  iBase *instance = CreateFunc(this);
  csRefTrackerAccess::SetDescriptionWeak (instance, ClassID);

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

const char *scfFactory::QueryModuleName ()
{
  const char* libname = 
    LibraryName != csInvalidStringID ? libraryNames->Request(LibraryName) : 0; 
  return libname;
}

//------------------------------------ Implementation of csSCF functions ----//

void* csSCF::QueryInterface (scfInterfaceID iInterfaceID,
                             scfInterfaceVersion iVersion)
{
  if (iInterfaceID == scfInterfaceTraits<iSCF>::GetID () &&
    scfCompatibleVersion (iVersion, scfInterfaceTraits<iSCF>::GetVersion ()))
  {
    GetSCFObject()->IncRef ();
    return static_cast<iSCF*> (GetSCFObject());
  }
#ifdef CS_REF_TRACKER
  if (refTracker)
  {
    if (iInterfaceID == scfInterfaceTraits<iRefTracker>::GetID () &&
      scfCompatibleVersion (iVersion, scfInterfaceTraits<iRefTracker>::GetVersion ()))
    {
      refTracker->IncRef ();
      return static_cast<iRefTracker*> (refTracker);
    }
  }
#endif

  return scfImplementation<csSCF>::QueryInterface (iInterfaceID, iVersion);
}

void csSCF::ScanPluginsInt (csPathsList const* pluginPaths,
                            const char* context)
{
  if (pluginPaths)
  {
    // Search plugins in pluginpaths
    csRef<iStringArray> plugins;

    size_t i, j;
    for (i = 0; i < pluginPaths->Length(); i++)
    {
      csPathsList::Entry const& pathrec = (*pluginPaths)[i];
      if (IsVerbose(SCF_VERBOSE_PLUGIN_SCAN))
      {
        char const* x = scannedDirs.Contains(pathrec.path) ? "re-" : "";
        csPrintfErr("SCF_NOTIFY: %sscanning plugin directory: %s "
          "(context `%s'; recursive %s)\n", x, pathrec.path.GetData(),
          GetContextName(pathrec.type),
          pathrec.scanRecursive ? "yes" : "no");
      }

      if (plugins)
        plugins->Empty();

      csRef<iStringArray> messages =
        csScanPluginDir (pathrec.path, plugins, pathrec.scanRecursive);
      scannedDirs.Request(pathrec.path);

      if ((messages != 0) && (messages->GetSize () > 0))
      {
        csPrintfErr("SCF_WARNING: the following issue(s) arose while "
          "scanning '%s':", pathrec.path.GetData());
        for (j = 0; j < messages->GetSize (); j++)
          csPrintfErr(" %s\n", messages->Get (j));
      }

      csRef<iDocument> metadata;
      csRef<iString> msg;
      for (j = 0; j < plugins->GetSize (); j++)
      {
        char const* plugin = plugins->Get(j);
        msg = csGetPluginMetadata (plugin, metadata);
        if (msg != 0)
        {
          csPrintfErr("SCF_ERROR: metadata retrieval error for %s: %s\n",
            plugin, msg->GetData ());
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
          RegisterClasses(plugin, metadata, 
          context ? context : pathrec.type.GetData());
      }
    }
  }
}

static unsigned int parse_verbosity(int argc, const char* const argv[])
{
  csVerbosityParser p(csParseVerbosity(argc, argv));
  unsigned int v = SCF_VERBOSE_NONE;
  if (p.Enabled("scf.plugin.scan"    )) v |= SCF_VERBOSE_PLUGIN_SCAN;
  if (p.Enabled("scf.plugin.load"    )) v |= SCF_VERBOSE_PLUGIN_LOAD;
  if (p.Enabled("scf.plugin.register")) v |= SCF_VERBOSE_PLUGIN_REGISTER;
  if (p.Enabled("scf.class.register" )) v |= SCF_VERBOSE_CLASS_REGISTER;
  return v;
}

void scfInitialize (csPathsList const* pluginPaths, unsigned int verbose)
{
#ifndef CS_PLATFORM_WIN32
  /* Required so non-Win32 csPrintf() picks up the right char set
     Done early here instead of in csPlatformStartup because SCF may already
     use csPrintf() */
  setlocale (LC_CTYPE, "");
#endif

  if (!PrivateSCF)
    PrivateSCF = new csSCF (verbose);
  else if (verbose != SCF_VERBOSE_NONE)
    PrivateSCF->SetVerbose(verbose | PrivateSCF->GetVerbose());
  PrivateSCF->ScanPluginsInt (pluginPaths, 0);
}

void scfInitialize (int argc, const char* const argv[],
		    bool scanDefaultPluginPaths)
{
  unsigned int const verbosity = parse_verbosity(argc, argv);
  if (!scanDefaultPluginPaths)
    scfInitialize (0, verbosity);
  else
  {
    csPathsList* pluginPaths = csGetPluginPaths (argv[0]);
    scfInitialize (pluginPaths, verbosity);
    delete pluginPaths;
  }
}

struct scfStaticClass
{
  scfFactoryFunc func;
  const char *iClassID;
  const char *Description;
  const char *Dependencies;
};
struct scfStaticFactoryFunc
{
  scfFactoryFunc func;
  const char *FactClass;
};

/* Since the order of static symbol initialization is undeterminated,
 * use this wrapper to instantiate a csArray<> when needed.
 */
template<typename T>
class StaticArrayWrapper
{
public:
  typedef csArray<T> ArrayType; 
private:
  ArrayType* array;
  ArrayType& GetArray()
  {
    if (!array) array = new ArrayType;
    return *array;
  }
public:
  ~StaticArrayWrapper() { delete array; }
  size_t Length() { return array ? array->GetSize () : 0; }
  T& operator [] (size_t n) { return GetArray()[n]; }
  size_t Push (T const& what) { return GetArray().Push (what); }
};

static StaticArrayWrapper<scfStaticClass> staticClasses;
static StaticArrayWrapper<const char*> staticXml;
static StaticArrayWrapper<scfStaticFactoryFunc> staticFactoryFuncs;

void scfRegisterStaticClass (scfFactoryFunc func, const char *iClassID, 
			     const char *Description, 
			     const char *Dependencies)
{
  scfStaticClass c;
  c.func = func;
  c.iClassID = iClassID;
  c.Description = Description;
  c.Dependencies = Dependencies;
  staticClasses.Push (c);
}

void scfRegisterStaticClasses (char const* xml)
{
  staticXml.Push (xml);
}

void scfRegisterStaticFactoryFunc (scfFactoryFunc func, const char *FactClass)
{
  scfStaticFactoryFunc ff;
  ff.func = func;
  ff.FactClass = FactClass;
  staticFactoryFuncs.Push (ff);
}

csSCF::csSCF (unsigned int v) : scfImplementation<csSCF> (this), verbose(v)
#ifdef CS_REF_TRACKER
  ,refTracker(0)
#endif  
{
  SCF = PrivateSCF = this;
#if defined(CS_DEBUG) || defined (CS_MEMORY_TRACKER)
  object_reg = 0;
#endif

#ifdef CS_REF_TRACKER
  refTracker = new csRefTracker();
  if (!classIDs)
    classIDs = new csStringHash;
#endif

  if (!ClassRegistry)
    ClassRegistry = new scfClassRegistry ();

  if (!LibraryRegistry)
    LibraryRegistry = new scfLibraryVector ();
  if (!libraryNames)
    libraryNames = new csStringSet;


  staticContextID = contexts.Request (SCF_STATIC_CLASS_CONTEXT);

  size_t i;
  for (i = 0; i < staticClasses.Length(); i++)
  {
    const scfStaticClass& c = staticClasses[i];
    RegisterClass (c.func, c.iClassID, c.Description, c.Dependencies,
      SCF_STATIC_CLASS_CONTEXT);
  }
  for (i = 0; i < staticXml.Length(); i++)
  {
    RegisterClasses (staticXml[i], SCF_STATIC_CLASS_CONTEXT);
  }
  for (i = 0; i < staticFactoryFuncs.Length(); i++)
  {
    const scfStaticFactoryFunc& ff = staticFactoryFuncs[i];
    RegisterFactoryFunc (ff.func, ff.FactClass);
  }
}

csSCF::~csSCF ()
{
  delete ClassRegistry;
  ClassRegistry = 0;
  SortClassRegistry = false;
  UnloadUnusedModules ();
  delete LibraryRegistry;
  LibraryRegistry = 0;
  delete libraryNames;
  libraryNames = 0;

#ifdef CS_REF_TRACKER
  refTracker->Report ();
  delete refTracker;
  delete classIDs; classIDs = 0;
#endif
#ifdef LAZY_UNLOAD
  for (size_t i = 0; i < lazyUnloadLibs.GetSize(); i++)
    csUnloadLibrary (lazyUnloadLibs[i]);
#endif
  SCF = PrivateSCF = 0;
}
void csSCF::RegisterClasses (char const* xml, const char* context)
{
  csMemFile file(xml, strlen(xml));
  csTinyDocumentSystem docsys;
  csRef<iDocument> doc = docsys.CreateDocument();
  if (doc->Parse(&file, true) == 0)
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
	  csPrintfErr("SCF_ERROR: missing <scf> node in metadata for %s "
	    "in context `%s'\n", pluginPath != 0 ? pluginPath : "{unknown}",
	    GetContextName(context));
      }
      else
        csPrintfErr("SCF_ERROR: missing root <plugin> node in metadata "
	  "for %s in context `%s'\n",
	  pluginPath != 0 ? pluginPath : "{unknown}", GetContextName(context));
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
  bool const seen = pluginPath != 0 && libraryNames->Contains(pluginPath);

  if (IsVerbose(SCF_VERBOSE_PLUGIN_REGISTER))
  {
    char const* s = pluginPath != 0 ? pluginPath : "{unknown}";
    char const* c = GetContextName(context);
    if (!seen)
      csPrintfErr("SCF_NOTIFY: registering plugin %s in context `%s'\n", s, c);
    else
      csPrintfErr("SCF_NOTIFY: ignoring duplicate plugin registration %s "
        "in context `%s'\n", s, c);
  }

  if (seen)
    return;			// *** RETURN: Do not re-register ***

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

uint64 scfImplementationHelper::stats[scfImplementationHelper::scfstatsNum];
CS::Threading::Mutex scfImplementationHelper::statsLock;

class scfImplementationHelperXS : public scfImplementationHelper
{
public:
  static void DumpStats ()
  {
#ifdef SCF_TRACK_STATS
    csPrintf ("Total SCF objects created: %llu\n", stats[scfstatTotal]);
    csPrintf (" SCF-parented:             %llu\n", stats[scfstatParented]);
    csPrintf (" Weak-Referenced:          %llu\n", stats[scfstatWeakreffed]);
    csPrintf (" Metadata requested:       %llu\n", stats[scfstatMetadata]);
    csPrintf ("\n");
    csPrintf ("IncRef calls:              %llu\n", stats[scfstatIncRef]);
    csPrintf ("DecRef calls:              %llu\n", stats[scfstatDecRef]);
#endif
  }
};

void csSCF::Finish ()
{
  delete this;
  scfImplementationHelperXS::DumpStats();
}

iBase *csSCF::CreateInstance (const char *iClassID)
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);

  // Pre-sort class registry for doing binary searches
  if (SortClassRegistry)
  {
    ClassRegistry->Sort();
    SortClassRegistry = false;
  }

  size_t idx = ClassRegistry->FindClass(iClassID, true);
  iBase *object = 0;

  if (idx != (size_t)-1)
  {
    iFactory *cf = (iFactory *)ClassRegistry->Get (idx);
    object = cf->CreateInstance ();

    if (!object)
      csPrintfErr("SCF_WARNING: factory returned a null instance for %s\n"
        "\tif error messages are not self explanatory, recompile CS with "
        "CS_DEBUG\n", iClassID);
 
  } /* endif */

  UnloadUnusedModules ();

  return object;
}

void csSCF::UnloadUnusedModules ()
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  for (size_t i = LibraryRegistry->GetSize (); i > 0; i--)
  {
    scfSharedLibrary *sl = (scfSharedLibrary *)LibraryRegistry->Get (i - 1);
    sl->TryUnload ();
  }
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
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  size_t idx;
  csStringID contextID = 
    context ? contexts.Request (context) : csInvalidStringID;

  if (IsVerbose(SCF_VERBOSE_CLASS_REGISTER))
    csPrintfErr("SCF_NOTIFY: registering class %s in context `%s' (from %s)\n",
      iClassID, GetContextName(context), iLibraryName);

  if ((idx = ClassRegistry->FindClass(iClassID)) != (size_t)-1)
  {
    scfFactory *cf = (scfFactory *)ClassRegistry->Get (idx);
    if (ContextClash (cf->classContext, contextID))
    {
      csPrintfErr("SCF_WARNING: class %s (from %s) has already been "
        "registered in the same context `%s' (in %s)\n",
        iClassID, iLibraryName, GetContextName(context),
	get_library_name(cf->LibraryName));
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
	csPrintfErr("SCF_NOTIFY: class %s (from %s) has already been "
	  "registered in a different context: '%s' vs. '%s' (from %s); this "
	  "message appears only in debug builds\n",
	  iClassID, iLibraryName, GetContextName(context),
	  GetContextName(cf->classContext), get_library_name(cf->LibraryName));
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
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  size_t idx;
  csStringID contextID = 
    context ? contexts.Request (context) : csInvalidStringID;

  if (IsVerbose(SCF_VERBOSE_CLASS_REGISTER))
    csPrintfErr("SCF_NOTIFY: registering class %s in context `%s' "
      "(statically linked)\n", iClassID, GetContextName(context));

  if ((idx = ClassRegistry->FindClass(iClassID)) != (size_t)-1)
  {
    scfFactory *cf = (scfFactory *)ClassRegistry->Get (idx);
    if (ContextClash (cf->classContext, contextID))
    {
      csPrintfErr("SCF_WARNING: class %s (statically linked) has already been "
	"registered in the same context `%s' (from %s)\n",
	iClassID, GetContextName(context), get_library_name(cf->LibraryName));
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
	csPrintfErr("SCF_NOTIFY: class %s (statically linked) has already "
	  "been registered in a different context: '%s' vs. '%s' (from %s); "
	  "this message appears only in debug builds\n",
	  iClassID, GetContextName(context), GetContextName(cf->classContext),
	  get_library_name(cf->LibraryName));
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
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  for (size_t i = 0, n = ClassRegistry->GetSize (); i < n; i++)
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
  CS::Threading::RecursiveMutexScopedLock lock (mutex);

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
  csPathsList paths;
  paths.AddUniqueExpanded (path, recursive);
  ScanPluginsInt (&paths, context);
}

bool csSCF::RegisterPlugin (const char* path)
{
  csRef<iDocument> metadata;
  csRef<iString> msg;
  if (IsVerbose(SCF_VERBOSE_PLUGIN_REGISTER))
    csPrintfErr("SCF_NOTIFY: registering plugin %s (no context)\n", path);

  if ((msg = csGetPluginMetadata (path, metadata)) != 0)
  {
    csPrintfErr("SCF_ERROR: couldn't retrieve metadata for '%s': %s\n", 
      path, msg->GetData ());
    return false;
  }

  RegisterClasses (path, metadata);
  return true;
}

const char *csSCF::GetClassDescription (const char *iClassID)
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);

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
  CS::Threading::RecursiveMutexScopedLock lock (mutex);

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
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  size_t idx = ClassRegistry->FindClass(iClassID);
  if (idx != (size_t)-1)
  {
    scfFactory *cf = ClassRegistry->Get (idx);
    if (cf->LibraryName != csInvalidStringID)
      csGetPluginMetadata (get_library_name(cf->LibraryName), metadata);
  }
  return metadata;
}

bool csSCF::ClassRegistered (const char *iClassID)
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  return (ClassRegistry->FindClass(iClassID) != csArrayItemNotFound);
}


char const* csSCF::GetInterfaceName (scfInterfaceID i) const
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  return InterfaceRegistry.Request (i);
}

scfInterfaceID csSCF::GetInterfaceID (const char *iInterface)
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  return (scfInterfaceID)InterfaceRegistry.Request (iInterface);
}

csRef<iStringArray> csSCF::QueryClassList (char const* pattern)
{
  iStringArray* v = new scfStringArray();

  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  size_t const rlen = ClassRegistry->GetSize ();
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
