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

/**
 * We'll use structs for defining interfaces, as the simplest choice
 * (because struct members are public by default)
 */
#define scfInterface struct

/// Use this macro to construct interface version numbers
#define SCF_VERSION(Major,Minor,Micro)	\
  ((Major << 24) | (Minor << 16) | Micro)

/// Current version of IBase interface
#define VERSION_IBase SCF_VERSION (0, 0, 1)

/**
 * This is the basic interface: all other interfaces should be
 * derived from this one, this will allow us to always use at least
 * some minimal functionality given any interface pointer.
 */
scfInterface IBase
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
 * to declare the minimal functionality required by IBase interface
 */
#define DECLARE_IBASE							\
  DECLARE_EMBEDDED_IBASE (IBase);

/**
 * DECLARE_EMBEDDED_IBASE is used to declare the methods of IBase inside
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
 * constructor should accept an IBase* parameter (that is passed by
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
 * implements a interface derived from IBase. Of course, you can still
 * implement those methods manually, if you desire ...
 */
#define IMPLEMENT_IBASE(ClassName)					\
void ClassName::IncRef ()						\
{									\
  scfParent->IncRef ();							\
  scfRefCount++;							\
}									\
void ClassName::DecRef ()						\
{									\
  scfRefCount--;							\
  scfParent->DecRef ();							\
  if (scfRefCount <= 0)							\
    delete this;							\
}									\
void *ClassName::QueryInterface (const char *iInterfaceID, int iVersion)\
{

/**
 * IMPLEMENT_EMBEDDED_IBASE should be used to implement embedded
 * interfaces derived from IBase. It differs from IMPLEMENT_IBASE
 * because embedded interface doesn't have to delete the object
 * when its reference count reaches zero.
 */
#define IMPLEMENT_EMBEDDED_IBASE(ClassName)				\
void ClassName::IncRef ()						\
{									\
  scfParent->IncRef ();							\
  scfRefCount++;							\
}									\
void ClassName::DecRef ()						\
{									\
  scfRefCount--;							\
  scfParent->DecRef ();							\
}									\
void *ClassName::QueryInterface (const char *iInterfaceID, int iVersion)\
{

/**
 * This macro is used to finish a IMPLEMENT_IBASE definition
 */
#define IMPLEMENT_IBASE_END						\
  return scfParent->QueryInterface (iInterfaceID, iVersion);		\
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
  IMPLEMENTS_INTERFACE_COMMON (Interface, this, Interface)

/**
 * IMPLEMENT_EMBEDDED_INTERFACE is same as IMPLEMENT_INTERFACE but is used
 * when class implements the interface as an embedded member.
 */
#define IMPLEMENTS_EMBEDDED_INTERFACE(Interface)			\
  IMPLEMENTS_INTERFACE_COMMON (Interface, (&scf##Interface), Interface)

/**
 * This is a common macro used in all IMPLEMENTS_XXX_INTERFACE macros
 */
#define IMPLEMENTS_INTERFACE_COMMON(Interface,Object,ParentClass)	\
  if (scfCompatibleVersion (iVersion, VERSION_##Interface)		\
   && !strcmp (iInterfaceID, #Interface))				\
  {									\
    Object->IncRef();							\
    return (Interface *)(ParentClass *)Object;				\
  }

/**
 * The IMPLEMENT_FACTORY macro is used to define a factory for one of
 * exported classes. You can define the function manually, of course,
 * if the constructor for your class has some specific constructor
 * arguments (that is, more than one IBase* argument).
 */
#define IMPLEMENT_FACTORY(Class)					\
void *Create_##Class (IBase *iParent)					\
{									\
  return new Class (iParent);						\
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
  void *(*Factory) (IBase *iParent);
};

/**
 * The following set of macros are used to define the table that contains
 * information about all classes exported from a shared library. This table
 * is used with both static and dynamic class linking.
 */

/// Define the start of class export table
#define EXPORT_CLASS_TABLE(LibraryName)					\
EXPORTED_FUNCTION (scfClassInfo) *LibraryName##_GetClassTable ()	\
{									\
  static scfClassInfo ExportClassTable [] =				\
  {

/// Add information about a exported class into the table
#define EXPORT_CLASS(ClassID, Description)				\
    { #ClassID, Description, Create_##ClassID },

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

//---------------------------------------------- Class factory interface -----//

/// Current version of IFactory interface
#define VERSION_IFactory SCF_VERSION (0, 0, 1)

/**
 * IFactory is a interface that is used to create instances of shared classes.
 * Any object supports the IFactory interface; a QueryInterface about IFactory
 * will return a valid pointer to the factory that was used to create that
 * object. Thus you can clone objects without even knowing their types.
 * <p>
 * NOTE: Currently you cannot add factories to the class factory list
 * internally maintained by SCF. That is, you can use an existing factory
 * but cannot create objects that implements this interface (well, you can
 * but its pointless since you won't be able to add it to the factory list).
 * Instead, you should register new class factories using scfRegisterClass ().
 */
scfInterface IFactory : public IBase
{
  /// Create a insance of class this factory represents
  virtual void *CreateInstance () = 0;
  /// Try to unload class module (i.e. shared module)
  virtual void TryUnload () = 0;
};

//------------------------------------------------ Client-side functions -----//

// We'll use csIniFile to read SCF.CFG
class csIniFile;

/// Handy macro to create an instance of a shared class
#define CREATE_INSTANCE(Class,Interface)				\
  (Interface *)scfCreateInstance (Class, #Interface, VERSION_##Interface)
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

//----------------------------------------------------- faster class IDs -----//

/**
 * If you have lots, I mean REALLY LOTS of classes, you could find that using
 * strcmp() for finding classes is slow. In this case you can use scfClassID
 * structures as a faster alternative. If first character of a iClassID is
 * less than space ' ', the identifier is considered a numerical ID rather
 * than a string. In this case comparison is made just as if class ID would
 * be a usual long integer. The first three bytes are guaranteed to be
 * non-zero, and fourth is guaranteed to be zero. This way, we can use
 * usual string functions with it (such as strnew(), strcmp() etc).
 * Since first byte is in the range 1..32, we have 32*255*255 = 2080800
 * of possible IDs for machines with sizeof (long) == 4.
 */
typedef char scfClassID [sizeof (unsigned long)];

/// Call this function to get a new unique class ID
extern scfClassID &scfGenerateUniqueID ();

/// Check if the identifier is a fast class ID
static inline bool scfIsFastClassID (const char *iID)
{
  return (*iID <= ' ');
}

/// Compare two fast class IDs: you SHOULD be sure they are indeed fast class IDs
static inline bool scfEqualFastClassID (const char *iID1, const char *iID2)
{
  return (*(unsigned long *)iID1) == (*(unsigned long *)iID2);
}

//--------------------------------------------- System-dependent defines -----//

#if defined (OS_WIN32) || defined (OS_BE)
#  define EXPORTED_FUNCTION(f) extern "C" __declspec(dllexport) f
#else
#  define EXPORTED_FUNCTION(f) extern "C" f
#endif

#endif // __CSSCF_H__
