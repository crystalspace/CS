/*
    Crystal Space Shared Class Facility (SCF)
    Copyright (C) 1999 by Andrew Zabolotny

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

#ifndef __CSSCF_H__
#define __CSSCF_H__

/*
    PLEASE USE 8-SPACE TAB WIDTH WHEN EDITING THIS FILE!
*/

/**
 * Macro for typing debug strings: Add #define SCF_DEBUG at the top
 * of modules you want to track miscelaneous SCF activity and recompile.
 */
#ifdef SCF_DEBUG
#  define SCF_TRACE(x)							\
   {									\
     printf ("SCF [%s:%d]:\n", __FILE__, __LINE__);			\
     printf x; SCF_PRINT_CALL_ADDRESS					\
   }
#else
#  define SCF_TRACE(x)
#endif

/**
 * Macro for getting the address we were called from (stack backtracing).
 * This works ONLY For GCC >= 2.8.0
 */
#if (__GNUC__ >= 2) && (__GNUC_MINOR__ >= 8)
#  define SCF_PRINT_CALL_ADDRESS					\
   printf ("  Called from address %p\n", __builtin_return_address (0));
#else
#  define SCF_PRINT_CALL_ADDRESS
#endif

/// Use this macro to construct interface version numbers.
#define SCF_CONSTRUCT_VERSION(Major,Minor,Micro)			\
  ((Major << 24) | (Minor << 16) | Micro)

/**
 * SCF_VERSION can be used as a shorter way to define an interface version;
 * you should specify interface name and major, minor and micro version
 * components. This way:
 * <pre>
 * SCF_VERSION (iSomething, 0, 0, 1);
 * struct iSomething : public iBase
 * {
 *   ...
 * };
 * </pre>
 */
#define SCF_VERSION(Name,Major,Minor,Micro)				\
  const int VERSION_##Name = SCF_CONSTRUCT_VERSION (Major, Minor, Micro)

SCF_VERSION (iBase, 0, 0, 1);

/**
 * This is the basic interface: all other interfaces should be
 * derived from this one, this will allow us to always use at least
 * some minimal functionality given any interface pointer.
 */
struct iBase
{
  /// Increment the number of references to this object.
  virtual void IncRef () = 0;
  /// Decrement the reference count.
  virtual void DecRef () = 0;
  /// Query a particular interface embedded into this object.
  virtual void *QueryInterface (const char *iInterfaceID, int iVersion) = 0;
  /**
   * Query a particular interface embedded into an object.
   * This version will test if 'ibase' is NULL.
   */
  static void* QueryInterfaceSafe (iBase* ibase, const char* iInterfaceID,
  	int iVersion)
  {
    if (ibase == NULL) return NULL;
    else return ibase->QueryInterface (iInterfaceID, iVersion);
  }
};

/// This macro should make use of IncRef() safer.
#define INC_REF(ptr) {if (ptr) {ptr->IncRef();}}

/// This macro should make use of DecRef() safer.
#define DEC_REF(ptr) {if (ptr) {ptr->DecRef();}}


/**
 * This macro should be embedded into any SCF-capable class definition
 * to declare the minimal functionality required by iBase interface.
 */
#define DECLARE_IBASE							\
  DECLARE_EMBEDDED_IBASE (iBase)

/**
 * DECLARE_EMBEDDED_IBASE is used to declare the methods of iBase inside
 * an embedded class that is exposed via QueryInterface...
 */
#define DECLARE_EMBEDDED_IBASE(OuterClass)				\
public:									\
  int scfRefCount;		/* Reference counter */			\
  OuterClass *scfParent;	/* The parent object */			\
  virtual void IncRef ();						\
  virtual void DecRef ();						\
  virtual void *QueryInterface (const char *iInterfaceID, int iVersion)

/**
 * The CONSTRUCT_IBASE macro should be invoked inside the constructor
 * of an exported class (not inside an embedded interface). Normally each
 * constructor should accept an iBase* parameter (that is passed by
 * scfCreateInstance function) which should be passed to this macro.
 * The macro will zero the reference count and initialize the pointer
 * to the parent object.
 */
#define CONSTRUCT_IBASE(Parent)						\
  scfRefCount = 1; scfParent = Parent; if (scfParent) scfParent->IncRef();

/**
 * The CONSTRUCT_EMBEDDED_IBASE macro should be invoked inside the
 * constructor of an exported class that has exported embedded interfaces
 * (not inside the constructor of the embedded interface).
 * The macro will zero the reference count and initialize the pointer
 * to the parent object (to the object this one is embedded into).
 */
#define CONSTRUCT_EMBEDDED_IBASE(Interface)				\
  Interface.scfRefCount = 0; Interface.scfParent = this;

/**
 * The following macro should be used within the C++ source module that
 * implements a interface derived from iBase. Of course, you can still
 * implement those methods manually, if you desire ...
 */
#define IMPLEMENT_IBASE(Class)						\
void Class::IncRef ()							\
{									\
  SCF_TRACE (("  (%s *)%p->IncRef (%d)\n", #Class, this, scfRefCount + 1));\
  if (scfParent)							\
    scfParent->IncRef ();						\
  scfRefCount++;							\
}									\
void Class::DecRef ()							\
{									\
  scfRefCount--;							\
  if (scfParent)							\
    scfParent->DecRef ();						\
  if (scfRefCount <= 0)							\
  {									\
    SCF_TRACE (("  delete (%s *)%p\n", #Class, this));			\
    delete this;							\
  }									\
  else									\
    SCF_TRACE (("  (%s *)%p->DecRef (%d)\n", #Class, this, scfRefCount));\
}									\
void *Class::QueryInterface (const char *iInterfaceID, int iVersion)	\
{									\
  SCF_TRACE (("  (%s *)%p->QueryInterface (%s, %08X)\n",		\
    #Class, this, iInterfaceID, iVersion));				\

/**
 * IMPLEMENT_EMBEDDED_IBASE should be used to implement embedded
 * interfaces derived from iBase. It differs from IMPLEMENT_IBASE
 * because embedded interface doesn't have to delete the object
 * when its reference count reaches zero.
 */
#define IMPLEMENT_EMBEDDED_IBASE(Class)					\
void Class::IncRef ()							\
{									\
  SCF_TRACE (("  (%s *)%p->IncRef (%d)\n", #Class, this, scfRefCount + 1));\
  if (scfParent)							\
    scfParent->IncRef ();						\
  scfRefCount++;							\
}									\
void Class::DecRef ()							\
{									\
  scfRefCount--;							\
  if (scfParent)							\
    scfParent->DecRef ();						\
  SCF_TRACE (("  (%s *)%p->DecRef (%d)\n", #Class, this, scfRefCount));	\
}									\
void *Class::QueryInterface (const char *iInterfaceID, int iVersion)	\
{									\
  SCF_TRACE (("  (%s *)%p->QueryInterface (%s, %08X)\n",		\
    #Class, this, iInterfaceID, iVersion));				\

/**
 * This macro is used to finish an IMPLEMENT_IBASE definition
 */
#define IMPLEMENT_IBASE_END						\
  return scfParent ?							\
    scfParent->QueryInterface (iInterfaceID, iVersion) : NULL;		\
}

/// Same as IMPLEMENT_IBASE_END
#define IMPLEMENT_EMBEDDED_IBASE_END					\
  IMPLEMENT_IBASE_END

/**
 * The IMPLEMENT_INTERFACE macro is used inside QueryInterface function
 * to check if user requested a specific interface, whenever requested
 * version of the interface correspond to the version we have and to
 * return a pointer to that interface if everything is correct.
 */
#define IMPLEMENTS_INTERFACE(Interface)					\
  IMPLEMENTS_INTERFACE_COMMON (Interface, this)

/**
 * IMPLEMENT_EMBEDDED_INTERFACE is same as IMPLEMENT_INTERFACE but is used
 * when class implements the interface as an embedded member.
 */
#define IMPLEMENTS_EMBEDDED_INTERFACE(Interface)			\
  IMPLEMENTS_INTERFACE_COMMON (Interface, (&scf##Interface))

/**
 * This is a common macro used in all IMPLEMENTS_XXX_INTERFACE macros
 */
#define IMPLEMENTS_INTERFACE_COMMON(Interface,Object)			\
  if (scfCompatibleVersion (iVersion, VERSION_##Interface)		\
   && !strcmp (iInterfaceID, #Interface))				\
  {									\
    (Object)->IncRef ();						\
    return STATIC_CAST(Interface*, Object);				\
  }

/**
 * The following macro is used in "expansion SCF classes". An expansion class
 * is a class that extends the functionality of another SCF class. For example,
 * suppose a class TheWolf that implements the iWolf interface. Separately
 * it is a useful class per se, but if you want to implement an additional
 * class TheDog that is a subclass of TheWolf and which implements an
 * additional interface iDog in theory you should just override the
 * QueryInterface method and return the corresponding pointer when asked.
 * The following macro makes such overrides simpler to write.
 */
#define DECLARE_IBASE_EXT(ParentClass)					\
  typedef ParentClass __scf_superclass;					\
  virtual void IncRef ();						\
  virtual void DecRef ();						\
  virtual void *QueryInterface (const char *iInterfaceID, int iVersion)

/**
 * This macro implements same functionality as IMPLEMENT_IBASE
 * except that it should be used for expansion SCF classes.
 */
#define IMPLEMENT_IBASE_EXT(Class)					\
void Class::IncRef ()							\
{									\
  __scf_superclass::IncRef ();						\
}									\
void Class::DecRef ()							\
{									\
  __scf_superclass::DecRef ();						\
}									\
void *Class::QueryInterface (const char *iInterfaceID, int iVersion)	\
{

/**
 * This macro implements same functionality as IMPLEMENT_IBASE_END
 * except that it is used for expansion SCF classes.
 */
#define IMPLEMENT_IBASE_EXT_END						\
  return __scf_superclass::QueryInterface (iInterfaceID, iVersion);	\
}

/**
 * The IMPLEMENT_FACTORY macro is used to define a factory for one of
 * exported classes. You can define the function manually, of course,
 * if the constructor for your class has some specific constructor
 * arguments (that is, more than one iBase* argument).
 */
#define IMPLEMENT_FACTORY(Class)					\
void *Create_##Class (iBase *iParent)					\
{									\
  void *ret = new Class (iParent);					\
  SCF_TRACE (("  %p = new %s ()\n", ret, #Class));			\
  return ret;								\
}

/**
 * The DECLARE_FACTORY macro is used to provide a forward definition 
 * if IMPLEMENT_FACTORY is declared in another file.
 */
#define DECLARE_FACTORY(Class)  void *Create_##Class (iBase *iParent);

/**
 * The shared library loader expects an array of such structures
 * to be exported from each shared library. Usually this is done by
 * implementing a exported function that returns a pointer to that table.
 */
struct scfClassInfo
{
  /// This is the classID.
  char *ClassID;
  /// This is the description of given class.
  char *Description;
  /**
   * An optional comma-separated list of ClassIDs that would be better
   * to load before this. This is a free-format string and its the
   * responsability of each application to query, parse and use it.
   */
  char *Dependencies;
  /// Class factory function.
  void *(*Factory) (iBase *iParent);
};

/*
 * The following set of macros are used to define the table that contains
 * information about all classes exported from a shared library. This table
 * is used with both static and dynamic class linking.
 */

/**
 * Define the start of class export table.
 * Any module that exports a number of SCF classes can define a table
 * with a list ot all classes exported from this module. The LibraryName
 * parameter is used to construct the name of the table variable; usually
 * the table is returned by a function called LibraryName_scfInitialize.
 * This function also initializes the global (iSCF::SCF) pointer for each
 * module.
 */
#define EXPORT_CLASS_TABLE(LibraryName)					\
SCF_EXPORT_FUNCTION scfClassInfo*					\
SCF_EXPORTED_NAME(LibraryName,_scfInitialize)(iSCF *SCF)		\
{									\
  static scfClassInfo ExportClassTable [] =				\
  {

/** Add information about a exported class into the table. */
#define EXPORT_CLASS(Class, ClassID, Description)			\
    { ClassID, Description, NULL, Create_##Class },

/** Add information about an exported class and dependency info into table. */
#define EXPORT_CLASS_DEP(Class, ClassID, Description, Dependencies)	\
    { ClassID, Description, Dependencies, Create_##Class },

/** Finish the definition of exported class table. */
#define EXPORT_CLASS_TABLE_END						\
    { 0, 0, 0, 0 }							\
  };									\
  iSCF::SCF = SCF;							\
  return ExportClassTable;						\
}

/**
 * Automatically register a static library with SCF during startup.
 * When SCF classes are statically linked (vs dynamic linking) they should
 * be referenced from somewhere inside your program, otherwise the static
 * libraries won't be linked into the static executable. This macro defines
 * a dummy variable that registers given library during initialization.
 */
#define REGISTER_STATIC_LIBRARY(LibraryName)				\
  extern "C" scfClassInfo *LibraryName##_scfInitialize (iSCF*);		\
  class __##LibraryName##_Init						\
  {									\
  public:								\
    __##LibraryName##_Init ()						\
    { if (!iSCF::SCF) scfInitialize ();					\
      iSCF::SCF->RegisterClassList (LibraryName##_scfInitialize (iSCF::SCF)); }	\
  } __##LibraryName##_dummy;

/**
 * This macro is similar to REGISTER_STATIC_LIBRARY, but registers a
 * single class. You also should provide a ClassID and a description
 * since a valid scfClassInfo structure should be created.
 */
#define REGISTER_STATIC_CLASS(Class,ClassID,Description)		\
  REGISTER_STATIC_CLASS_DEP (Class,ClassID,Description,NULL);

/**
 * This is similar to REGISTER_STATIC_CLASS except that you can provide
 * an additional argument specifying the class dependencies.
 */
#define REGISTER_STATIC_CLASS_DEP(Class,ClassID,Description,Dependency)	\
  extern void *Create_##Class (iBase *);				\
  static scfClassInfo Class##_ClassInfo =				\
  { ClassID, Description, Dependency, Create_##Class };			\
  class __##Class##_Init						\
  {									\
  public:								\
    __##Class##_Init ()							\
    { if (!iSCF::SCF) scfInitialize ();					\
      iSCF::SCF->RegisterStaticClass (&Class##_ClassInfo); }		\
  } __##Class##_dummy;

//--------------------------------------------- Class factory interface -----//

SCF_VERSION (iFactory, 0, 0, 1);

/**
 * iFactory is a interface that is used to create instances of shared classes.
 * Any object supports the iFactory interface; a QueryInterface about iFactory
 * will return a valid pointer to the factory that was used to create that
 * object. Thus you can clone objects without even knowing their types.
 * <p>
 * NOTE: Currently you cannot add factories to the class factory list
 * internally maintained by SCF. That is, you can use an existing factory
 * but cannot create objects that implements this interface (well, you can
 * but its pointless since you won't be able to add it to the factory list).
 * Instead, you should register new class factories using scfRegisterClass ().
 */
struct iFactory : public iBase
{
  /// Create a insance of class this factory represents.
  virtual void *CreateInstance () = 0;
  /// Try to unload class module (i.e. shared module).
  virtual void TryUnload () = 0;
  /// Query class description string.
  virtual const char *QueryDescription () = 0;
  /// Query class dependency strings.
  virtual const char *QueryDependencies () = 0;
  /// Query class ID
  virtual const char *QueryClassID () = 0;
};

//----------------------------------------------- Client-side functions -----//

// We'll use iConfigFile to read `scf.cfg'.
struct iConfigFile;

/**
 * Handy macro to create an instance of a shared class.
 * This is a simple wrapper around scfCreateInstance.
 */
#define CREATE_INSTANCE(ClassID,Interface)				\
  (Interface *)iSCF::SCF->CreateInstance (ClassID, #Interface, VERSION_##Interface)

/**
 * Shortcut macro to query given interface from given object.
 * This is a wrapper around iBase::QueryInterface method.
 */
#define QUERY_INTERFACE(Object,Interface)				\
  (Interface *)(Object)->QueryInterface (#Interface, VERSION_##Interface)

/**
 * Shortcut macro to query given interface from given object.
 * This is a wrapper around iBase::QueryInterface method.
 * This version tests if Object is NULL and will return NULL in that case.
 */
#define QUERY_INTERFACE_SAFE(Object,Interface)				\
  (Interface *)(iBase::QueryInterfaceSafe ((Object), #Interface, VERSION_##Interface))

/**
 * This function should be called to initialize client SCF library.
 * If you provide a iConfig object, the SCF-related registry section
 * will be read from it. It is legal to call scfInitialize more than once
 * (possibly providing a different iConfig object each time). If you
 * don't specify this parameter, this argument is ignored.
 */
extern void scfInitialize (iConfigFile *iConfig = 0);

/**
 * This function checks whenever an interface is compatible with given version.
 * SCF uses the following comparison criteria: if the major version numbers
 * are equal and required minor and micro version is less or equal than target
 * version minor and micro numbers, the versions are considered compatible.
 */
static inline bool scfCompatibleVersion (int iVersion, int iItfVersion)
{
  return ((iVersion & 0xff000000) == (iItfVersion & 0xff000000))
      && ((iVersion & 0x00ffffff) <= (iItfVersion & 0x00ffffff));
}

SCF_VERSION (iSCF, 0, 0, 1);

/**
 * iSCF is the interface that allows using SCF functions from shared classes.
 * Since there should be just one instance of SCF kernel, the shared classes
 * should not use scfXXX functions directly; instead they should obtain a
 * pointer to an iSCF object and work through that pointer.
 */
struct iSCF : public iBase
{
  /// This is the global instance of iSCF
  static iSCF *SCF;

  /**
   * Read additional class descriptions from the given config file. This does
   * the same as additional calls to scfInitialize ().
   */
  virtual void RegisterConfigClassList (iConfigFile *Config) = 0;

  /**
   * Check whenever the class is present in SCF registry.
   * You can use this function to check whenever a class instance creation
   * failed because the class is not present at all in the class registry,
   * or it just doesn't support the requested interface.
   */
  virtual bool ClassRegistered (const char *iClassID) = 0;

  /**
   * Create an instance of a class that supports given interface.
   * The function returns NULL either if such a class ID is not found in
   * class registry, or a object of given class does not support given
   * interface or supports an incompatible version of given interface.
   * If you want to make a difference between these error conditions,
   * you can check whenever such a class exists using scfClassRegistered()
   * function.
   * <p>
   * If you specify NULL as iInterfaceID, you'll receive a pointer to the
   * basic interface, no matter what it is. <b>The reference count will be zero
   * thus you should increment it yourself if you use this approach.</b>
   * You can treat the pointer returned just as an iBase*, not more.
   * If you need more, do QueryInterface() on received pointer (this will also
   * increment the reference counter).
   */
  virtual void *CreateInstance (const char *iClassID,
	const char *iInterfaceID, int iVersion) = 0;

  /**
   * Query the description of a class.
   * NOTE: At least one instance of this class should exist, or the class
   * should be a static class. Otherwise the function will return NULL
   */
  virtual const char *GetClassDescription (const char *iClassID) = 0;

  /**
   * Query the dependency list for a class.
   * The format of dependency string is implementation-specific, SCF itself
   * does not make any assumptions about the format of the string.
   */
  virtual const char *GetClassDependencies (const char *iClassID) = 0;

  /**
   * Unload all unused shared libraries (also called inside scfCreateInstance).
   * If you want to be sure that all unused shared libraries are unloaded,
   * call this function. It is automatically invoked inside scfCreateInstance(),
   * thus it is called from time to time if you constantly create new objects.
   */
  virtual void UnloadUnusedModules () = 0;
  
  /**
   * Register a single dynamic class (implemented in a shared library).
   * This function tells SCF kernel that a specific class is implemented within
   * a specific shared library. There can be multiple classes within a single
   * shared library. You also can provide an application-specific dependency
   * list.
   */
  virtual bool RegisterClass (const char *iClassID,
	const char *iLibraryName, const char *Dependencies = NULL) = 0;

  /**
   * Register a single static class (that is, implemented in SCF client module).
   * This function is similar to scfRegisterClass but is intended to be used
   * with statically linked classes (that is, not located in a shared library)
   */
  virtual bool RegisterStaticClass (scfClassInfo *iClassInfo) = 0;

  /**
   * Register a set of static classes (used with static linking).
   * If you design a SCF module that contains a number of SCF classes, and you
   * want that module to be usable when using either static and dynamic linkage,
   * you can use scfRegisterClassList (or the SCF_REGISTER_STATIC_LIBRARY macro)
   * to register the export class table with the SCF kernel.
   */
  virtual bool RegisterClassList (scfClassInfo *iClassInfo) = 0;

  /**
   * This function should be called to deregister a class at run-time.
   * By calling this function you will remove the description of a class,
   * no matter whenever it is statically or dynamically linked, from the
   * SCF registry.
   */
  virtual bool UnregisterClass (char *iClassID) = 0;

  /**
   * This function should be called to finish working with SCF.
   * (this won't free shared objects but they couldn't be used anymore
   * because this will do a forced free of all loaded shared libraries).
   */
  virtual void Finish () = 0;
};

//-------------------------------------------- System-dependent defines -----//

/*
 * A macro to export a function from a shared library.
 * Some platforms may need to override this.  For instance, Windows requires
 * extra `__declspec' goop when exporting a function from a plug-in module.
 */
#if !defined(SCF_EXPORT_FUNCTION)
#  define SCF_EXPORT_FUNCTION extern "C"
#endif

/*
 * A macro used to build exported function names.
 * Usually "Prefix" is derived from shared library name, thus for each library
 * we'll have different exported names.  This prevents naming collisions when
 * static linking is used, and on platforms where plug-in symbols are exported
 * by default.  However, this may be bad for platforms which need to build
 * special export-tables on-the-fly at compile-time since distinct names make
 * the job more difficult.  Such platforms may need to override the default
 * expansion of this macro to use only the `Suffix' and ignore the `Prefix'
 * when composing the name.
 */
#if !defined(SCF_EXPORTED_NAME)
#  define SCF_EXPORTED_NAME(Prefix, Suffix) Prefix ## Suffix
#endif

#endif // __CSSCF_H__
