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

#ifndef __CSSCF_H__
#define __CSSCF_H__

/*

    PLEASE USE 8-SPACE TAB WIDTH AT LEAST WHILE EDITING THIS FILE!

*/

/**
 * We'll use structs for defining interfaces, as the simplest choice
 * (because struct members are public by default)
 */
#define csInterface struct

/// Use this macro to construct interface version numbers
#define SCF_VERSION(Major,Minor,Micro)	\
  ((Major << 24) | (Minor << 16) | Micro)

/// Current version of IBase interface
#define VERSION_IBase		SCF_VERSION (0, 0, 1)

/**
 * This is the basic interface: all other interfaces should be
 * derived from this one, this will allow us to always use at least
 * some minimal functionality given any interface pointer.
 */
csInterface IBase
{
  /// Increment the number of references to this object
  virtual void AddRef () = 0;
  /// Decrement the reference count
  virtual void Release () = 0;
  /// Query a particular interface embedded into this object
  virtual IBase *QueryInterface (const char *iItfName, int iVersion) = 0;
};

/**
 * The following macro should be embedded into any SCF-capable class definition
 * to declare the minimal functionality required by IBase interface
 */
#define DECLARE_IBASE							\
  DECLARE_EMBEDDED_IBASE (IBase)

/**
 * DECLARE_EMBEDDED_IBASE is used to declare the methods of IBase inside
 * an embedded class that is exposed via QueryInterface...
 */
#define DECLARE_EMBEDDED_IBASE(OuterClass)				\
public:									\
  int scfRefCount;		/* Reference counter */			\
  OuterClass *scfParent;	/* The parent object */			\
  virtual void AddRef ();						\
  virtual void Release ();						\
  virtual IBase *QueryInterface (const char *iItfName, int iVersion);

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
void ClassName::AddRef ()						\
{									\
  scfParent->AddRef ();							\
  scfRefCount++;							\
}									\
void ClassName::Release ()						\
{									\
  scfRefCount--;							\
  scfParent->Release ();						\
  if (!scfRefCount)							\
    delete this;							\
}									\
IBase *ClassName::QueryInterface (const char *iItfName, int iVersion)	\
{									\
  IMPLEMENTS_INTERFACE (IBase)

/**
 * IMPLEMENT_EMBEDDED_IBASE should be used to implement embedded
 * interfaces derived from IBase. It differs from IMPLEMENT_IBASE
 * because embedded interface doesn't have to delete the object
 * when its reference count reaches zero.
 */
#define IMPLEMENT_EMBEDDED_IBASE(ClassName)				\
void ClassName::AddRef ()						\
{									\
  scfParent->AddRef ();							\
  scfRefCount++;							\
}									\
void ClassName::Release ()						\
{									\
  scfRefCount--;							\
  scfParent->Release ();						\
}									\
IBase *ClassName::QueryInterface (const char *iItfName, int iVersion)	\
{									\
  IMPLEMENTS_INTERFACE (IBase)

/**
 * This macro is used to finish a IMPLEMENT_IBASE definition
 */
#define IMPLEMENT_IBASE_END						\
  return NULL;								\
}

/**
 * The IMPLEMENT_INTERFACE macro is used inside QueryInterface function
 * to check if user requested a specific interface, whenever requested
 * version of the interface correspond to the version we have and to
 * return a pointer to that interface if everything is correct.
 */
#define IMPLEMENTS_INTERFACE(Interface)					\
  if (scfCompatibleVersion (iVersion, VERSION_##Interface)		\
   && !strcmp (iItfName, #Interface))					\
  {									\
    AddRef ();								\
    return this;							\
  }

/**
 * IMPLEMENT_EMBEDDED_INTERFACE is same as IMPLEMENT_INTERFACE but is used
 * when class implements the interface as an embedded member.
 */
#define IMPLEMENTS_EMBEDDED_INTERFACE(Interface)			\
  if (scfCompatibleVersion (iVersion, VERSION_##Interface)		\
   && !strcmp (iItfName, #Interface))					\
  {									\
    scf##Interface.AddRef();						\
    return &scf##Interface;						\
  }

#ifndef CS_STATIC_LINKED

/**
 * The IMPLEMENT_FACTORY macro is used to define a factory for one of
 * exported classes. You can define the function manually, of course,
 * if the constructor for your class has some specific constructor
 * arguments (that is, more than one IBase* argument).
 */
#define IMPLEMENT_FACTORY(Class)					\
EXPORTED_ENTRY IBase *Create_##Class (IBase *iParent)			\
{									\
  return new Class (iParent);						\
}

#else // CS_STATIC_LINKED

#define IMPLEMENT_FACTORY(Class)					\
static IBase *Create_##Class (IBase *iParent)				\
{									\
  return new Class (iParent);						\
}									\
void Register_##Class ()						\
{									\
  scfRegisterClass (#Class, Create_##Class);				\
}									\
void Unregister_##Class ()						\
{									\
  scfUnregisterClass (#Class);						\
}

#endif // CS_STATIC_LINKED

/**
 * Since static shared classes aren't directly referenced from anywhere
 * in the main engine code, we should make at least one reference from
 * somewhere if we use static linking: this forces linker to link in
 * all static shared class libraries, otherwise they will be quietly
 * ignored, but application will fail at run time.
 * <p>
 * The macros below expects two external functions to be defined
 * inside the shared class library called
 * <pre>
 *   void ClassName##Register ()
 * </pre>
 * and
 * <pre>
 *   void ClassName##Unregister ()
 * </pre>
 * their function should be obvious from their names. If you use
 * the EXPORT_CLASS macros (as usual), they will define these functions
 * automatically (if static linkage is selected).
 */

#if !defined (CS_STATIC_LINKED)
#define REGISTER_SCF_CLASS(Class)
#else
#define REGISTER_SCF_CLASS(Class)					\
  class __##Class##Initializer						\
  {									\
  public:								\
    __##Class##Initializer ()						\
    {									\
      extern void Register_##Class ();					\
      Register_##Class ();						\
    }									\
    ~__##Class##Initializer ()						\
    {									\
      extern void Unregister_##Class ();				\
      Unregister_##Class ();						\
    }									\
  } __##Class##Reference;
#endif // CS_STATIC_LINKED

//------------------------------------------------ Client-side functions -----//

/// Handy macro to create an instance of a shared class
#define CREATE_INSTANCE(Class,Interface)				\
  (Interface *)scfCreateInstance (#Class, #Interface, VERSION_##Interface)
/// Shortcut macro to query given interface from given object
#define QUERY_INTERFACE(Object,Interface)				\
  (Interface *)Object->QueryInterface (#Interface, VERSION_##Interface)

/// This function should be called to initialize client SCF library
extern void scfInitialize (/* csVFS *Vfs */);
/**
 * And this function should be called to finish working with SCF
 * (this won't free shared objects but they couldn't be used anymore)
 */
extern void scfFinish ();

/// Create an instance of a class that supports given interface
extern IBase *scfCreateInstance (const char *iClassName, const char *iItfName,
  int iVersion);
/// Unload all unused shared libraries (also called inside scfCreateInstance)
extern void scfUnloadUnusedModules ();
/// This function should be called to register a static class at run-time
extern bool scfRegisterClass (char *iClassName,
  IBase *(*iCreateInstance) (IBase *iParent));
/// This function should be called to deregister a static class at run-time
extern bool scfUnregisterClass (char *iClassName);
/// This function checks whenever an interface is compatible with given version
inline bool scfCompatibleVersion (int iVersion, int iItfVersion)
{
  return ((iVersion & 0xff000000) == (iItfVersion & 0xff000000))
      && ((iVersion & 0x00ffffff) <= (iItfVersion & 0x00ffffff));
}

//--------------------------------------------- System-dependent defines -----//

#if defined (OS_WIN32) || defined (OS_BE)
#  define EXPORTED_ENTRY extern "C" __declspec(dllexport)
#else
#  define EXPORTED_ENTRY extern "C"
#endif

#endif // __CSSCF_H__
