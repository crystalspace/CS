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

    PLEASE USE 8-SPACE TAB WIDTH AT LEAST WHILE EDITING THIS FILE!

*/

// We'll use static_cast<> instead of old-style casts to protect
// from declaring unimplemented interfaces in SCF classes, but
// unfortunately we need to avoid static_cast <> () on NeXT
#include "def.h"

/**
 * We'll use structs for defining interfaces, as the simplest choice
 * (because struct members are public by default)
 */
#define scfInterface struct

/// Use this macro to construct interface version numbers
#define SCF_VERSION(Major,Minor,Micro)	\
  ((Major << 24) | (Minor << 16) | Micro)

/// SCF_INTERFACE can be used as a shorter way to define an interface;
/// you should specify interface name and major, minor and micro version
/// components. This way:
/// <pre>
/// SCF_INTERFACE (iSomething, 0, 0, 1) : public iBase
/// {
///   ...
/// </pre>
#define SCF_INTERFACE(Name,Major,Minor,Micro) \
  const int VERSION_##Name = SCF_VERSION (Major, Minor, Micro); \
  struct Name

/**
 * This is the basic interface: all other interfaces should be
 * derived from this one, this will allow us to always use at least
 * some minimal functionality given any interface pointer.
 */
SCF_INTERFACE (iBase, 0, 0, 1)
{
  /// Increment the number of references to this object
  virtual void IncRef () = 0;
  /// Decrement the reference count
  virtual void DecRef () = 0;
  /// Query a particular interface embedded into this object
  virtual void *QueryInterface (const char *iInterfaceID, int iVersion) = 0;
};

/**
 * The following macro should be embedded into any SCF-capable class definition
 * to declare the minimal functionality required by iBase interface
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
 */
#define CONSTRUCT_IBASE(Parent)						\
  scfRefCount = 0; scfParent = Parent;

/**
 * The CONSTRUCT_EMBEDDED_IBASE macro should be invoked inside the
 * constructor of an exported class that has exported embedded interfaces
 * (not inside the constructor of the embedded interface)
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
    CHKB (delete this);							\
}									\
void *Class::QueryInterface (const char *iInterfaceID, int iVersion)	\
{

/**
 * IMPLEMENT_EMBEDDED_IBASE should be used to implement embedded
 * interfaces derived from iBase. It differs from IMPLEMENT_IBASE
 * because embedded interface doesn't have to delete the object
 * when its reference count reaches zero.
 */
#define IMPLEMENT_EMBEDDED_IBASE(Class)					\
void Class::IncRef ()							\
{									\
  if (scfParent)							\
    scfParent->IncRef ();						\
  scfRefCount++;							\
}									\
void Class::DecRef ()							\
{									\
  scfRefCount--;							\
  if (scfParent)							\
    scfParent->DecRef ();						\
}									\
void *Class::QueryInterface (const char *iInterfaceID, int iVersion)	\
{

/**
 * This macro is used to finish a IMPLEMENT_IBASE definition
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
    Object->IncRef();							\
    return STATIC_CAST (Interface *) (Object);				\
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
  CHK (void *ret = new Class (iParent));				\
  return ret;								\
}

/**
 * The shared library loader expects an array of such structures
 * to be exported from each shared library. Usually this is done by
 * implementing a exported function that returns a pointer to that table.
 */
struct scfClassInfo
{
  char *ClassID;
  char *Description;
  void *(*Factory) (iBase *iParent);
};

/**
 * The following set of macros are used to define the table that contains
 * information about all classes exported from a shared library. This table
 * is used with both static and dynamic class linking.
 */

/// Define the start of class export table
#define EXPORT_CLASS_TABLE(LibraryName)					\
EXPORTED_FUNCTION (scfClassInfo *EXPORTED_NAME (LibraryName,_GetClassTable)) ()	\
{									\
  static scfClassInfo ExportClassTable [] =				\
  {

/// Add information about a exported class into the table
#define EXPORT_CLASS(Class, ClassID, Description)			\
    { ClassID, Description, Create_##Class },

/// Finish the definition of exported class table
#define EXPORT_CLASS_TABLE_END						\
    { 0, 0, 0 }								\
  };									\
  return ExportClassTable;						\
}

/**
 * When SCF classes are statically linked (vs dynamic linking) they should
 * be referenced from somewhere inside your program, otherwise the static
 * libraries won't be linked into the static executable. For this you can
 * use the following macro.
 */
#define REGISTER_STATIC_LIBRARY(LibraryName)				\
  extern "C" scfClassInfo *LibraryName##_GetClassTable ();		\
  class __##LibraryName##_Init						\
  {									\
  public:								\
    __##LibraryName##_Init ()						\
    { scfRegisterClassList (LibraryName##_GetClassTable ()); }		\
  } __##LibraryName##_dummy;

/**
 * This macro is similar to above, but registers a single class.
 * You also should provide a ClassID and a description since a valid
 * scfClassInfo structure should be created.
 */
#define REGISTER_STATIC_CLASS(Class,ClassID,Description)		\
  extern void *Create_##Class (iBase *);				\
  static scfClassInfo Class##_ClassInfo =				\
  { ClassID, Description, Create_##Class };				\
  class __##Class##_Init						\
  {									\
  public:								\
    __##Class##_Init ()							\
    { scfRegisterClass (&Class##_ClassInfo); }				\
  } __##Class##_dummy;

//---------------------------------------------- Class factory interface -----//

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
SCF_INTERFACE (iFactory, 0, 0, 1) : public iBase
{
  /// Create a insance of class this factory represents
  virtual void *CreateInstance () = 0;
  /// Try to unload class module (i.e. shared module)
  virtual void TryUnload () = 0;
  /// Query class description string
  virtual const char *QueryDescription () = 0;
};

//------------------------------------------------ Client-side functions -----//

// We'll use csIniFile to read SCF.CFG
class csIniFile;

/// Handy macro to create an instance of a shared class
#define CREATE_INSTANCE(ClassID,Interface)				\
  (Interface *)scfCreateInstance (ClassID, #Interface, VERSION_##Interface)
/// Shortcut macro to query given interface from given object
#define QUERY_INTERFACE(Object,Interface)				\
  (Interface *)Object->QueryInterface (#Interface, VERSION_##Interface)

/**
 * This function should be called to initialize client SCF library.
 * <p>
 * If you provide a iConfig object, the SCF-related registry section
 * will be read from it. It is legal to call scfInitialize more than once
 * (possibly providing a different iConfig object each time). If you
 * don't specify this parameter, this argument is ignored.
 */
extern void scfInitialize (csIniFile *iConfig = 0);
/**
 * And this function should be called to finish working with SCF
 * (this won't free shared objects but they couldn't be used anymore
 * because this will do a forced free of all loaded shared libraries)
 */
extern void scfFinish ();

/// Create an instance of a class that supports given interface
extern void *scfCreateInstance (const char *iClassID, const char *iInterfaceID,
  int iVersion);
/// Query the description of a class: AT LEAST ONE OBJECT OF THIS CLASS SHOULD EXIST
extern const char *scfGetClassDescription (const char *iClassID);
/// Unload all unused shared libraries (also called inside scfCreateInstance)
extern void scfUnloadUnusedModules ();
/// Register a single dynamic class (implemented in a shared library)
extern bool scfRegisterClass (const char *iClassID, const char *iLibraryName);
/// Register a single static class (that is, implemented in SCF client module
extern bool scfRegisterClass (scfClassInfo *iClassInfo);
/// Register a set of static classes (used with static linking)
extern bool scfRegisterClassList (scfClassInfo *iClassInfo);
/// This function should be called to deregister a class at run-time
extern bool scfUnregisterClass (char *iClassID);
/// This function checks whenever an interface is compatible with given version
static inline bool scfCompatibleVersion (int iVersion, int iItfVersion)
{
  return ((iVersion & 0xff000000) == (iItfVersion & 0xff000000))
      && ((iVersion & 0x00ffffff) <= (iItfVersion & 0x00ffffff));
}

//--------------------------------------------- System-dependent defines -----//

/// A macro to declare a symbol that should be exported from shared libraries
#if defined (OS_WIN32) || defined (OS_BE)
#  define EXPORTED_FUNCTION(f) extern "C" __declspec(dllexport) f
#elif defined (OS_MACOS)
#  define EXPORTED_FUNCTION(f) extern "C" __declspec(export) f
#else
#  define EXPORTED_FUNCTION(f) extern "C" f
#endif

/**
 * A macro used to build exported function names.
 * Usually "Prefix" is derived from shared library name, thus for each library
 * we'll have different exported names. While this is good for static linking
 * and for shared libraries on NeXTStep, this is bad for some platforms that
 * will need to build on-the-fly at compile-time some additional files
 * (such as exported functions table). Because of this, on these platforms
 * we drop the module-dependent prefix in the case we use dynamic linking.
 */
#if !defined (CS_STATIC_LINKING) && defined (OS_AMIGA)
#  define EXPORTED_NAME(Prefix, Suffix) Suffix
#else
#  define EXPORTED_NAME(Prefix, Suffix) Prefix ## Suffix
#endif

#endif // __CSSCF_H__
