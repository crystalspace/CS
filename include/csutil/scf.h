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

/**\file
 * Crystal Space Shared Class Facility (SCF)
 */

/** 
 * \addtogroup scf
 * @{ */

class csPluginPaths;

#include "csutil/ref.h"
#include "csutil/array.h"

/**
 * Type of registered interface handle used by iBase::QueryInterface().
 */
typedef unsigned long scfInterfaceID;

/**\def SCF_TRACE(x)
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

/**\def SCF_PRINT_CALL_ADDRESS
 * Macro for getting the address we were called from (stack backtracing).
 * This works ONLY For GCC >= 2.8.0
 */
#if (__GNUC__ >= 3) || ((__GNUC__ >= 2) && (__GNUC_MINOR__ >= 8))
#  define SCF_PRINT_CALL_ADDRESS					\
   printf ("  Called from address %p\n", __builtin_return_address (0));
#else
#  define SCF_PRINT_CALL_ADDRESS
#endif

/// Use this macro to construct interface version numbers.
#define SCF_CONSTRUCT_VERSION(Major,Minor,Micro)			\
  ((Major << 24) | (Minor << 16) | Micro)

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
  /// Get the ref count (only for debugging).
  virtual int GetRefCount () = 0;
  /// Query a particular interface implemented by this object.
  virtual void *QueryInterface (scfInterfaceID iInterfaceID, int iVersion) = 0;
  /**
   * Query a particular interface implemented by an object.
   * This version will test if 'ibase' is 0.
   */
  static void* QueryInterfaceSafe (iBase* ibase, scfInterfaceID iInterfaceID,
  	int iVersion)
  {
    if (ibase == 0) return 0;
    else return ibase->QueryInterface (iInterfaceID, iVersion);
  }
};

/// Checks for null pointer before calling IncRef().
#define SCF_INC_REF(ptr) {if (ptr) {ptr->IncRef();}}

/// Checks for null pointer before calling DecRef().
#define SCF_DEC_REF(ptr) {if (ptr) {ptr->DecRef();}}

/**
 * This macro should be used to change the reference
 * inside a variable. The old reference will be DecRef()'ed
 * and the new reference will be IncRef()'ed.
 */
#define SCF_SET_REF(var,ref) \
  { \
    if (ref) ref->IncRef (); \
    if (var) var->DecRef (); \
    var = ref; \
  }

/**
 * This macro should be embedded into any SCF-capable class definition
 * to declare the minimal functionality required by iBase interface.
 */
#define SCF_DECLARE_IBASE						\
  int scfRefCount;		/* Reference counter */			\
  SCF_DECLARE_EMBEDDED_IBASE (iBase)

/**
 * This macro should be embedded into any SCF-capable class definition
 * to declare the minimal functionality required by iBase interface.
 * This is a special version which supports weak references (the
 * csWeakRef<T> class).
 */
#define SCF_DECLARE_IBASE_WEAK(Class)					\
  csArray<Class**> weak_ref_owners;					\
  virtual void AddRefOwner (Class** ref_owner);				\
  virtual void RemoveRefOwner (Class** ref_owner);			\
  virtual void RemoveRefOwners ();					\
  SCF_DECLARE_IBASE

/**
 * SCF_DECLARE_EMBEDDED_IBASE is used to declare the methods of iBase inside
 * an embedded class that is exposed via QueryInterface...
 */
#define SCF_DECLARE_EMBEDDED_IBASE(OuterClass)				\
public:									\
  OuterClass *scfParent;	/* The parent object */			\
  virtual void IncRef ();						\
  virtual void DecRef ();						\
  virtual int GetRefCount ();						\
  virtual void *QueryInterface (scfInterfaceID iInterfaceID, int iVersion)

/**
 * The SCF_CONSTRUCT_IBASE macro should be invoked inside the constructor
 * of an exported class (not inside an embedded interface). Normally each
 * constructor should accept an iBase* parameter (that is passed by
 * scfCreateInstance function) which should be passed to this macro.
 * The macro will zero the reference count and initialize the pointer
 * to the parent object.
 */
#define SCF_CONSTRUCT_IBASE(Parent)					\
  scfRefCount = 1; scfParent = Parent; if (scfParent) scfParent->IncRef();

/**
 * The SCF_CONSTRUCT_EMBEDDED_IBASE macro should be invoked inside the
 * constructor of an exported class that has exported embedded interfaces
 * (not inside the constructor of the embedded interface).
 * The macro will and initialize the pointer to the parent object
 * (to the object this one is embedded into).
 */
#define SCF_CONSTRUCT_EMBEDDED_IBASE(Interface)				\
  Interface.scfParent = this;

/**
 * The SCF_IMPLEMENT_IBASE_INCREF() macro implements the IncRef() method for a
 * class in a C++ source module.  Typically, this macro is automatically
 * employed by the SCF_IMPLEMENT_IBASE() convenience macro.
 */
#define SCF_IMPLEMENT_IBASE_INCREF(Class)				\
void Class::IncRef ()							\
{									\
  SCF_TRACE (("  (%s *)%p->IncRef (%d)\n", #Class, this, scfRefCount + 1));\
  scfRefCount++;							\
}

/**
 * The SCF_IMPLEMENT_IBASE_DECREF() macro implements the DecRef() method for a
 * class in a C++ source module.  Typically, this macro is automatically
 * employed by the SCF_IMPLEMENT_IBASE() convenience macro.
 * <p>
 * A note about the implementation: We do the "if" before the "scRefCount--"
 * to make sure that calling Inc/DecRef doesn't result in a 2nd delete
 */
#define SCF_IMPLEMENT_IBASE_DECREF(Class)				\
void Class::DecRef ()							\
{									\
  if (scfRefCount == 1)							\
  {									\
    SCF_TRACE ((" delete (%s *)%p\n", #Class, this));			\
    if (scfParent)							\
      scfParent->DecRef ();						\
    delete this;							\
    return;								\
  }									\
  scfRefCount--;							\
}

/**
 * The SCF_IMPLEMENT_IBASE_WEAK_DECREF() macro implements the DecRef() method
 * for a class in a C++ source module.  Typically, this macro is automatically
 * employed by the SCF_IMPLEMENT_IBASE_WEAK() convenience macro.
 * <p>
 * This version clears the pointers for all weak reference owners.
 */
#define SCF_IMPLEMENT_IBASE_WEAK_DECREF(Class)				\
void Class::DecRef ()							\
{									\
  if (scfRefCount == 1)							\
  {									\
    RemoveRefOwners ();							\
    SCF_TRACE ((" delete (%s *)%p\n", #Class, this));			\
    if (scfParent)							\
      scfParent->DecRef ();						\
    delete this;							\
    return;								\
  }									\
  scfRefCount--;							\
}									\
void Class::RemoveRefOwners ()						\
{									\
  for (int i = 0 ; i < weak_ref_owners.Length () ; i++)			\
  {									\
    Class** p = weak_ref_owners[i];					\
    *p = 0;								\
  }									\
  weak_ref_owners.DeleteAll ();						\
}

/**
 * The SCF_IMPLEMENT_IBASE_WEAK_REFOWNER() macro implements the AddRefOwner()
 * and RemoveRefOwner() for a weak reference.
 */
#define SCF_IMPLEMENT_IBASE_WEAK_REFOWNER(Class)			\
void Class::AddRefOwner (Class** ref_owner)				\
{									\
  weak_ref_owners.Push (ref_owner);					\
}									\
void Class::RemoveRefOwner (Class** ref_owner)				\
{									\
  weak_ref_owners.Delete (ref_owner);					\
}

/**
 * The SCF_IMPLEMENT_IBASE_GETREFCOUNT() macro implements GetRefCount()
 * for a class in a C++ source module.
 */
#define SCF_IMPLEMENT_IBASE_GETREFCOUNT(Class)				\
int Class::GetRefCount ()						\
{									\
  return scfRefCount;							\
}

/**
 * The SCF_IMPLEMENT_IBASE_QUERY() macro implements the opening boilerplate for
 * the QueryInterface() method for a class in a C++ source module.  Typically,
 * this macro is automatically employed by the SCF_IMPLEMENT_IBASE()
 * convenience macro.
 */
#define SCF_IMPLEMENT_IBASE_QUERY(Class)				\
void *Class::QueryInterface (scfInterfaceID iInterfaceID, int iVersion)	\
{									\
  SCF_TRACE (("  (%s *)%p->QueryInterface (%u, %08X)\n",		\
    #Class, this, iInterfaceID, iVersion));

/**
 * The SCF_IMPLEMENT_IBASE_QUERY_END macro implements the closing boilerplate
 * for the QueryInterface() method for a class in a C++ source module.
 * Typically, this macro is automatically employed by the
 * SCF_IMPLEMENT_IBASE_END convenience macro.
 */
#define SCF_IMPLEMENT_IBASE_QUERY_END					\
  return scfParent ?							\
    scfParent->QueryInterface (iInterfaceID, iVersion) : 0;		\
}

/**
 * The SCF_IMPLEMENT_IBASE() macro should be used within the C++ source module
 * that implements a interface derived from iBase.  Of course, you can still
 * implement those methods manually, if you desire ...
 */
#define SCF_IMPLEMENT_IBASE(Class)					\
  SCF_IMPLEMENT_IBASE_INCREF(Class)					\
  SCF_IMPLEMENT_IBASE_DECREF(Class)					\
  SCF_IMPLEMENT_IBASE_GETREFCOUNT(Class)				\
  SCF_IMPLEMENT_IBASE_QUERY(Class)

/**
 * The SCF_IMPLEMENT_IBASE_WEAK() macro should be used within the C++ source
 * module that implements a interface derived from iBase.  Of course, you can
 * still implement those methods manually, if you desire ...
 * This version is for use with weak reference counting.
 */
#define SCF_IMPLEMENT_IBASE_WEAK(Class)					\
  SCF_IMPLEMENT_IBASE_INCREF(Class)					\
  SCF_IMPLEMENT_IBASE_WEAK_DECREF(Class)				\
  SCF_IMPLEMENT_IBASE_WEAK_REFOWNER(Class)				\
  SCF_IMPLEMENT_IBASE_GETREFCOUNT(Class)				\
  SCF_IMPLEMENT_IBASE_QUERY(Class)

/**
 * The SCF_IMPLEMENT_IBASE_END macro is used to finish an SCF_IMPLEMENT_IBASE
 * definition
 */
#define SCF_IMPLEMENT_IBASE_END						\
  SCF_IMPLEMENT_IBASE_QUERY_END

/**
 * The SCF_IMPLEMENT_IBASE_WEAK_END macro is used to finish an
 * SCF_IMPLEMENT_IBASE_WEAK definition
 */
#define SCF_IMPLEMENT_IBASE_WEAK_END					\
  SCF_IMPLEMENT_IBASE_QUERY_END

/**
 * The SCF_IMPLEMENT_EMBEDDED_IBASE_INCREF() macro implements the IncRef()
 * method for an embedded class in a C++ source module.  Typically, this macro
 * is automatically employed by the SCF_IMPLEMENT_EMBEDDED_IBASE() convenience
 * macro.
 */
#define SCF_IMPLEMENT_EMBEDDED_IBASE_INCREF(Class)			\
void Class::IncRef ()							\
{									\
  SCF_TRACE (("  (%s *)%p->IncRef (%d)\n", #Class, this,		\
    scfParent->GetRefCount () + 1));					\
  scfParent->IncRef ();							\
}

/**
 * The SCF_IMPLEMENT_EMBEDDED_IBASE_DECREF() macro implements the DecRef()
 * method for an embedded class in a C++ source module.  Typically, this macro
 * is automatically employed by the SCF_IMPLEMENT_EMBEDDED_IBASE() convenience
 * macro.
 */
#define SCF_IMPLEMENT_EMBEDDED_IBASE_DECREF(Class)			\
void Class::DecRef ()							\
{									\
  SCF_TRACE (("  (%s *)%p->DecRef (%d)\n", #Class, this,                \
	      scfParent->GetRefCount ()-1));				\
  scfParent->DecRef ();							\
}

/**
 * The SCF_IMPLEMENT_EMBEDDED_IBASE_GETREFCOUNT() macro implements
 * the GetRefCount() method for an embedded class in a C++ source module.
 */
#define SCF_IMPLEMENT_EMBEDDED_IBASE_GETREFCOUNT(Class)			\
int Class::GetRefCount ()						\
{									\
  return scfParent->GetRefCount ();					\
}

/**
 * The SCF_IMPLEMENT_EMBEDDED_IBASE_QUERY() macro implements the opening
 * boilerplate for the QueryInterface() method for an embedded class in a C++
 * source module.  Typically, this macro is automatically employed by the
 * SCF_IMPLEMENT_EMBEDDED_IBASE() convenience macro.
 */
#define SCF_IMPLEMENT_EMBEDDED_IBASE_QUERY(Class)			\
void *Class::QueryInterface (scfInterfaceID iInterfaceID, int iVersion)	\
{									\
  SCF_TRACE (("  (%s *)%p->QueryInterface (%u, %08X)\n",		\
    #Class, this, iInterfaceID, iVersion));

/**
 * The SCF_IMPLEMENT_EMBEDDED_IBASE_QUERY_END macro implements the closing
 * boilerplate for the QueryInterface() method for a class in an embedded C++
 * source module.  Typically, this macro is automatically employed by the
 * SCF_IMPLEMENT_EMBEDDED_IBASE_END convenience macro.
 */
#define SCF_IMPLEMENT_EMBEDDED_IBASE_QUERY_END				\
  return scfParent->QueryInterface (iInterfaceID, iVersion);		\
}

/**
 * SCF_IMPLEMENT_EMBEDDED_IBASE should be used to implement embedded interfaces
 * derived from iBase.  It differs from SCF_IMPLEMENT_IBASE because embedded
 * interface don't have reference counts themselves, but instead use the
 * reference count of their parent object.
 */
#define SCF_IMPLEMENT_EMBEDDED_IBASE(Class)				\
  SCF_IMPLEMENT_EMBEDDED_IBASE_INCREF(Class)				\
  SCF_IMPLEMENT_EMBEDDED_IBASE_DECREF(Class)				\
  SCF_IMPLEMENT_EMBEDDED_IBASE_GETREFCOUNT(Class)			\
  SCF_IMPLEMENT_EMBEDDED_IBASE_QUERY(Class)

/**
 * The SCF_IMPLEMENT_EMBEDDED_IBASE_END macro is used to finish an
 * SCF_IMPLEMENT_EMBEDDED_IBASE definition
 */
#define SCF_IMPLEMENT_EMBEDDED_IBASE_END				\
  SCF_IMPLEMENT_EMBEDDED_IBASE_QUERY_END

/**
 * The IMPLEMENT_INTERFACE macro is used inside QueryInterface function
 * to check if user requested a specific interface, whenever requested
 * version of the interface correspond to the version we have and to
 * return a pointer to that interface if everything is correct.
 */
#define SCF_IMPLEMENTS_INTERFACE(Interface)				\
  SCF_IMPLEMENTS_INTERFACE_COMMON (Interface, this)

/**
 * IMPLEMENT_EMBEDDED_INTERFACE is same as IMPLEMENT_INTERFACE but is used
 * when class implements the interface as an embedded member.
 */
#define SCF_IMPLEMENTS_EMBEDDED_INTERFACE(Interface)			\
  SCF_IMPLEMENTS_INTERFACE_COMMON (Interface, (&scf##Interface))

/**
 * This is a common macro used in all IMPLEMENTS_XXX_INTERFACE macros
 */
#define SCF_IMPLEMENTS_INTERFACE_COMMON(Interface,Object)		\
  static scfInterfaceID Interface##_scfID = (scfInterfaceID)-1;		\
  if (Interface##_scfID == (scfInterfaceID)-1)				\
    Interface##_scfID = iSCF::SCF->GetInterfaceID (#Interface);		\
  if (iInterfaceID == Interface##_scfID &&				\
    scfCompatibleVersion (iVersion, Interface##_VERSION))		\
  {									\
    (Object)->IncRef ();						\
    return STATIC_CAST(Interface*, Object);				\
  }

/**
 * The following macro is used in "expansion SCF classes".  An expansion class
 * is a class that extends the functionality of another SCF class.  For
 * example, suppose a class TheWolf that implements the iWolf interface.
 * Separately it is a useful class per se, but if you want to implement an
 * additional class TheDog that is a subclass of TheWolf and which implements
 * an additional interface iDog in theory you should just override the
 * QueryInterface method and return the corresponding pointer when asked.  The
 * following macro makes such overrides simpler to write.
 */
#define SCF_DECLARE_IBASE_EXT(ParentClass)				\
  typedef ParentClass __scf_superclass;					\
  virtual void IncRef ();						\
  virtual void DecRef ();						\
  virtual int GetRefCount ();						\
  virtual void *QueryInterface (scfInterfaceID iInterfaceID, int iVersion)

/**
 * The SCF_IMPLEMENT_IBASE_EXT_INCREF() macro implements the IncRef() method
 * for a class extending another SCF class in a C++ source module.  Typically,
 * this macro is automatically employed by the SCF_IMPLEMENT_IBASE_EXT()
 * convenience macro.
 */
#define SCF_IMPLEMENT_IBASE_EXT_INCREF(Class)				\
void Class::IncRef ()							\
{									\
  __scf_superclass::IncRef ();						\
}

/**
 * The SCF_IMPLEMENT_IBASE_EXT_DECREF() macro implements the DecRef() method
 * for a class extending another SCF class in a C++ source module.  Typically,
 * this macro is automatically employed by the SCF_IMPLEMENT_IBASE_EXT()
 * convenience macro.
 */
#define SCF_IMPLEMENT_IBASE_EXT_DECREF(Class)				\
void Class::DecRef ()							\
{									\
  __scf_superclass::DecRef ();						\
}

/**
 * The SCF_IMPLEMENT_IBASE_EXT_GETREFCOUNT() macro implements the GetRefCount()
 * method for a class extending another SCF class in a C++ source module.
 * Typically, this macro is automatically employed by the
 * SCF_IMPLEMENT_IBASE_EXT() convenience macro.
 */
#define SCF_IMPLEMENT_IBASE_EXT_GETREFCOUNT(Class)			\
int Class::GetRefCount ()						\
{									\
  return __scf_superclass::GetRefCount ();				\
}

/**
 * The SCF_IMPLEMENT_IBASE_EXT_QUERY() macro implements the opening boilerplate
 * for the QueryInterface() method for a class extending another SCF class in a
 * C++ source module.  Typically, this macro is automatically employed by the
 * SCF_IMPLEMENT_IBASE_EXT() convenience macro.
 */
#define SCF_IMPLEMENT_IBASE_EXT_QUERY(Class)				\
void *Class::QueryInterface (scfInterfaceID iInterfaceID, int iVersion)	\
{

/**
 * The SCF_IMPLEMENT_IBASE_EXT_QUERY_END macro implements the closing
 * boilerplate for the QueryInterface() method for a class extending another
 * SCF class in a C++ source module.  Typically, this macro is automatically
 * employed by the SCF_IMPLEMENT_IBASE_EXT_END convenience macro.
 */
#define SCF_IMPLEMENT_IBASE_EXT_QUERY_END				\
  return __scf_superclass::QueryInterface (iInterfaceID, iVersion);	\
}

/**
 * This macro implements same functionality as SCF_IMPLEMENT_IBASE
 * except that it should be used for expansion SCF classes.
 */
#define SCF_IMPLEMENT_IBASE_EXT(Class)					\
  SCF_IMPLEMENT_IBASE_EXT_INCREF(Class)					\
  SCF_IMPLEMENT_IBASE_EXT_DECREF(Class)					\
  SCF_IMPLEMENT_IBASE_EXT_GETREFCOUNT(Class)				\
  SCF_IMPLEMENT_IBASE_EXT_QUERY(Class)

/**
 * This macro implements same functionality as SCF_IMPLEMENT_IBASE_END
 * except that it is used for expansion SCF classes.
 */
#define SCF_IMPLEMENT_IBASE_EXT_END					\
  SCF_IMPLEMENT_IBASE_EXT_QUERY_END

/**
 * The SCF_IMPLEMENT_FACTORY_INIT macro defines initialization code for a
 * plugin module.  This function should set the plugin-global iSCF::SCF
 * variable, and otherwise initialize the plugin module.  Although a version of
 * this function will be created for each SCF factory exported by the plugin,
 * SCF will call one, and only one, to perform the plugin initialization.  The
 * choice of which function will be invoked to initialize the plugin is an SCF
 * implementation detail.  You should not attempt to predict which
 * class_scfInitialize() function will be used, nor should use try to sway
 * SCF's choice.  Implementation note: There are some rare instances where a
 * particularly picky (and probably buggy) compiler does not allow C++
 * expressions within a function declared `extern "C"'.  For this reason, the
 * iSCF::SCF variable is instead initialized in the Class_scfUnitInitialize()
 * function which is not qualified as `extern "C"'.
 */
#define SCF_IMPLEMENT_FACTORY_INIT(Class)				\
static inline void Class ## _scfUnitInitialize(iSCF* SCF)		\
{ iSCF::SCF = SCF; }							\
CS_EXPORTED_FUNCTION							\
void CS_EXPORTED_NAME(Class,_scfInitialize)(iSCF* SCF)			\
{ Class ## _scfUnitInitialize(SCF); }

/**
 * The SCF_IMPLEMENT_FACTORY_FINIS macro defines finalization code for a plugin
 * module.  As with SCF_IMPLEMENT_FACTORY_INIT, only one instance of this
 * function will be invoked to finalize the module.
 */
#define SCF_IMPLEMENT_FACTORY_FINIS(Class)				\
CS_DECLARE_STATIC_VARIABLE_CLEANUP					\
CS_EXPORTED_FUNCTION							\
void CS_EXPORTED_NAME(Class,_scfFinalize)()				\
{									\
CS_STATIC_VARIABLE_CLEANUP						\
iSCF::SCF = 0;								\
}

/**
 * The SCF_IMPLEMENT_FACTORY_CREATE macro is used to define a factory for one
 * of exported classes.  You can define the function manually, of course, if
 * the constructor for your class has some specific constructor arguments (that
 * is, more than one iBase* argument).
 */
#define SCF_IMPLEMENT_FACTORY_CREATE(Class)				\
CS_EXPORTED_FUNCTION 							\
void* CS_EXPORTED_NAME(Class,_Create)(iBase *iParent)			\
{									\
  void *ret = new Class (iParent);					\
  SCF_TRACE (("  %p = new %s ()\n", ret, #Class));			\
  return ret;								\
}

/**
 * The SCF_IMPLEMENT_FACTORY macro is used to define a factory for one of
 * exported classes. You can define the function manually, of course,
 * if the constructor for your class has some specific constructor
 * arguments (that is, more than one iBase* argument).
 */
#define SCF_IMPLEMENT_FACTORY(Class)					\
  SCF_IMPLEMENT_FACTORY_INIT(Class)					\
  SCF_IMPLEMENT_FACTORY_FINIS(Class)					\
  SCF_IMPLEMENT_FACTORY_CREATE(Class)

#define SCF_STATIC_CLASS_CONTEXT      "*static*"

/**
 * Automatically register a built-in class with SCF during startup.  When SCF
 * classes are statically linked (vs dynamic linking) they should be referenced
 * from somewhere inside your program, otherwise the static libraries won't be
 * linked into the static executable.  This macro defines a dummy variable that
 * registers the class during initialization and ensures that it gets linked
 * into the program
 */
#define SCF_REGISTER_STATIC_CLASS(Class,Ident,Desc,Dep)			\
  CS_EXPORTED_FUNCTION void* CS_EXPORTED_NAME(Class,_Create)(iBase*);	\
  class Class##_StaticInit__						\
  {									\
  public:								\
    Class##_StaticInit__()						\
    {									\
      scfInitialize(0);							\
      iSCF::SCF->RegisterClass(						\
        CS_EXPORTED_NAME(Class,_Create), Ident, Desc, Dep,		\
	SCF_STATIC_CLASS_CONTEXT);					\
    }									\
  } Class##_static_init__;

/**
 * Automatically register a static library with SCF during startup.  Employ
 * this macro along with one or more invocations of SCF_REGISTER_FACTORY_FUNC.
 */
#define SCF_REGISTER_STATIC_LIBRARY(Module, MetaInfo)			\
  class Module##_StaticInit						\
  {									\
  public:								\
    Module##_StaticInit()						\
    {									\
      scfInitialize(0);							\
      iSCF::SCF->RegisterClasses(MetaInfo, SCF_STATIC_CLASS_CONTEXT);	\
    }									\
  } Module##_static_init__;

/**
 * Used in conjunction with SCF_REGISTER_STATIC_LIBRARY to ensure that a
 * reference to the class(s) registered via SCF_REGISTER_STATIC_LIBRARY are
 * actually linked into the application.  Invoke this macro once for each
 * <implementation> mentioned in the MetaInfo registered with
 * SCF_REGISTER_STATIC_LIBRARY.  Invocations of this macro must appear after
 * the the invocation of SCF_REGISTER_STATIC_LIBRARY.
 */
#define SCF_REGISTER_FACTORY_FUNC(Class)				\
  CS_EXPORTED_FUNCTION void* CS_EXPORTED_NAME(Class,_Create)(iBase*);	\
  class Class##_StaticInit						\
  {									\
  public:								\
    Class##_StaticInit()						\
    {									\
      scfInitialize(0);							\
      iSCF::SCF->RegisterFactoryFunc(CS_EXPORTED_NAME(Class,_Create),#Class); \
    }									\
  } Class##_static_init__;

//--------------------------------------------- Class factory interface -----//

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
 * Instead, you should register new class factories through the normal class
 * registration mechanism.
 */
struct iFactory : public iBase
{
  /// Create a instance of class this factory represents.
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

struct iDocument;
struct iStringArray;

/// Type of factory function which creates an instance of an SCF class.
typedef void* (*scfFactoryFunc)(iBase*);

/**
 * Handy macro to create an instance of a shared class.
 * This is a simple wrapper around scfCreateInstance.
 */
#define SCF_CREATE_INSTANCE(ClassID,Interface)				\
  (Interface *)iSCF::SCF->CreateInstance (				\
  ClassID, #Interface, Interface##_VERSION)

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
  const int Name##_VERSION = SCF_CONSTRUCT_VERSION (Major, Minor, Micro); \
inline static scfInterfaceID Name##_scfGetID ()				\
{									\
  static scfInterfaceID ID = (scfInterfaceID)-1;			\
  if (ID == (scfInterfaceID)(-1))					\
    ID = iSCF::SCF->GetInterfaceID (#Name);				\
  return ID;								\
}

/**
 * Shortcut macro to query given interface from given object.
 * This is a wrapper around iBase::QueryInterface method.
 */
#define SCF_QUERY_INTERFACE(Object,Interface)				\
  csPtr<Interface> ((Interface *)(Object)->QueryInterface (		\
  Interface##_scfGetID (), Interface##_VERSION))

/**
 * Shortcut macro to query given interface from given object.
 * This is a wrapper around iBase::QueryInterface method.
 * This version tests if Object is 0 and will return 0 in that case.
 */
#define SCF_QUERY_INTERFACE_SAFE(Object,Interface)			\
  csPtr<Interface> ((Interface *)(iBase::QueryInterfaceSafe ((Object),	\
  Interface##_scfGetID (), Interface##_VERSION)))

/**
 * This function should be called to initialize client SCF library.  
 * If a number of plugin paths are provided, the directories will be
 * scanned for plugins and their SCF-related registry data will be retrieved.
 * The root node within the registry data document should be named "plugin", 
 * and the SCF-related information should be in a child node of the root 
 * named "scf".
 * It is legal to call scfInitialize more than once (possibly providing a
 * different set of directories each time).  
 * \param pluginPaths Directories that will be scanned for plugins. If this
 *   parameter is 0, the paths returned by csGetPluginPaths() will be scanned.
 * \remark The path list is ignored for static builds. 
 */
extern void scfInitialize (csPluginPaths* pluginPaths);

/**
 * This function should be called to initialize client SCF library.  
 * It uses the default plugin paths provided by csGetPluginPaths().
 */
extern void scfInitialize (int argc, const char* const argv[]);

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

#ifdef CS_DEBUG
  struct iObjectRegistry;
#endif

/**
 * iSCF is the interface that allows using SCF functions from shared classes.
 * Since there should be just one instance of SCF kernel, the shared classes
 * should not use scfXXX functions directly; instead they should obtain a
 * pointer to an iSCF object and work through that pointer.
 */
struct iSCF : public iBase
{
  /**
   * This is the global instance of iSCF.  On most platforms, this instance is
   * module-global (for instance, the application has an iSCF::SCF variable,
   * and each plugin module has an iSCF::SCF variable).
   */
  static iSCF* SCF;

#ifdef CS_DEBUG
  // This is EXTREMELY dirty but I see no other solution for now.
  // For debugging reasons I must have a global (global over the application
  // and all plugins)pointer to the object registry. I have no other
  // global object to tag this pointer on that except for iSCF.
  // This pointer is only here in debug mode though. That ensures that it
  // cannot be misused in real code.
  // If you know another solution for this problem? This global pointer
  // will be used by csDebuggingGraph in csutil.
  iObjectRegistry* object_reg;
#endif

  /**
   * Read additional class descriptions from the given iDocument.  
   */
  virtual void RegisterClasses (iDocument* metadata,
    const char* context = 0) = 0;

  /**
   * A convenience wrapper for RegisterClasses(iDocument).  Assumes that the
   * string input argument is XML, which it wraps in an iDocument and then
   * passes to RegisterClasses(iDocument).
   */
  virtual void RegisterClasses (char const* xml,
    const char* context = 0) = 0;

  /**
   * Read additional class descriptions from the given iDocument.  
   */
  virtual void RegisterClasses (const char* pluginPath, 
    iDocument* metadata, const char* context = 0) = 0;

  /**
   * Check whenever the class is present in SCF registry.
   * You can use this function to check whenever a class instance creation
   * failed because the class is not present at all in the class registry,
   * or it just doesn't support the requested interface.
   */
  virtual bool ClassRegistered (const char *iClassID) = 0;

  /**
   * Create an instance of a class that supports given interface.  The function
   * returns 0 either if such a class ID is not found in class registry, or
   * a object of given class does not support given interface or supports an
   * incompatible version of given interface.  If you want to make a difference
   * between these error conditions, you can check whenever such a class exists
   * using scfClassRegistered() function.
   * <p>
   * If you specify 0 as iInterfaceID, you'll receive a pointer to the basic
   * interface, no matter what it is.  <b>The reference count will be zero thus
   * you should increment it yourself if you use this approach.</b> You can
   * treat the pointer returned just as an iBase*, not more.  If you need more,
   * do QueryInterface() on received pointer (this will also increment the
   * reference counter).
   */
  virtual void *CreateInstance (const char *iClassID,
	const char *iInterface, int iVersion) = 0;

  /**
   * Query the description of a class.
   * NOTE: At least one instance of this class should exist, or the class
   * should be a static class. Otherwise the function will return 0
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
   * If you want to be sure that all unused shared libraries are unloaded, call
   * this function.  It is automatically invoked inside scfCreateInstance(),
   * thus it is called from time to time if you constantly create new objects.
   */
  virtual void UnloadUnusedModules () = 0;

  /**
   * Register a single dynamic class (implemented in a shared library).  This
   * function tells SCF kernel that a specific class is implemented within a
   * specific shared library.  There can be multiple classes within a single
   * shared library.  You also can provide an application-specific dependency
   * list. 'context' is an information about the source of the plugin. It
   * primarily affects whether a class conflict is reported. If the same
   * class already exists in the same context, a warning is emitted; if it's
   * in a different context, only there is a notification only in debug mode. 
   */
  virtual bool RegisterClass (const char *iClassID,
	const char *iLibraryName, const char *iFactoryClass,
	const char *Description, const char *Dependencies = 0,
	const char* context = 0) = 0;

  /**
   * Register a single dynamic class.  This function tells SCF kernel that a
   * specific class is implemented within a specific module (typically a static
   * library, as opposed to a plugin module).  You also can provide an
   * application-specific dependency list.
   */
  virtual bool RegisterClass (scfFactoryFunc, const char *iClassID,
	const char *Description, const char *Dependencies = 0,
	const char* context = 0) = 0;

  /**
   * Associate a factory function (the function which instantiates a class)
   * with an implementation name (the value in the <implementation> node of the
   * meta info; also the name of the iFactoryClass in RegisterClass).  Returns
   * true upon sucess, or false if the class does not exist or already has an
   * associated creation function.
   */
  virtual bool RegisterFactoryFunc (scfFactoryFunc, const char *FactClass) = 0;

  /**
   * This function should be called to deregister a class at run-time.
   * By calling this function you will remove the description of a class,
   * no matter whenever it is statically or dynamically linked, from the
   * SCF registry.
   */
  virtual bool UnregisterClass (const char *iClassID) = 0;

  /**
   * Return the interface ID number that belongs to the given interface.
   * If the interface is unknown, a new ID is allocated. This number can be
   * used to quickly determine whether two interfaces are equal.
   */
  virtual scfInterfaceID GetInterfaceID (const char *iInterface) = 0;

  /**
   * This function should be called to finish working with SCF.
   * This will not free shared objects but they should not be used anymore
   * after calling this function since this will do a forced free of all loaded
   * shared libraries.
   */
  virtual void Finish () = 0;

  /**
   * Retrieve a list of class names whose prefix matches a pattern string.  For
   * example, QueryClassList("crystalspace.sound.loader.") will return a list
   * of class names which begin with the string "crystalspace.sound.loader.".
   * If pattern is zero length or the null pointer, then all registered class
   * names are returned.  If any class names match the pattern, then the return
   * value is a list strings.  If no class names match the pattern string, then
   * the returned list is empty.  It is the caller's responsibility to invoke
   * DecRef() on the returned list when the list is no longer needed.
   */
  virtual iStringArray* QueryClassList (char const* pattern) = 0;

  /**
   * Scan a specified native path for plugins and auto-register them.
   */
  virtual void ScanPluginsPath (const char* path, bool recursive = false,
    const char* context = 0) = 0;
    
  /**
   * Register a single plugin.
   * \param path (Almost) fully qualified native path to the plugin binary. 
   *   'Almost' because it doesn't have to be the actual binary - it is 
   *   sufficient if the file name suffix is ".csplugin", no matter what
   *   the real extension for binaries on a platform is or whether there
   *   actually is an external .csplugin file.
   * \return Whether loading of the plugin was successful.
   */
  virtual bool RegisterPlugin (const char* path) = 0;
};

SCF_VERSION (iFactory, 0, 0, 1);
SCF_VERSION (iBase, 0, 1, 0);
SCF_VERSION (iSCF, 0, 2, 0);

/* @} */

#endif // __CSSCF_H__
