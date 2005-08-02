/*
    Crystal Space Shared Class Facility (SCF)
    Copyright (C) 1999 by Andrew Zabolotny
              (C) 2005 by Marten Svanfeldt
              (C) 2005 by Michael Adams

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

#include "csextern.h"

#include "csutil/array.h"
#include "csutil/ref.h"

/**\file
 * Crystal Space Shared Class Facility (SCF)
 */

/**
 * \addtogroup scf
 * @{ */

class csPathsList;


// INTERFACE DEFINITIONS
#include "csutil/scf_interface.h"
// NEW STYLE IMPLEMENTATION
//#include "csutil/scf_implementation.h"


//-- Helper macros

/**\def SCF_TRACE(x)
 * Macro for typing debug strings: Add #define SCF_DEBUG at the top
 * of modules you want to track miscelaneous SCF activity and recompile.
 */
#ifdef SCF_DEBUG
#  define SCF_TRACE(x)							\
   {									\
     printf ("SCF [%s:%d]:\n", __FILE__, (int)__LINE__);		\
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


#ifdef CS_REF_TRACKER
 #include <typeinfo>
 #include "iutil/string.h"
 #include "iutil/databuff.h"
 #define CS_TYPENAME(x)		    typeid(x).name()
 /* @@@ HACK: Force an AddAlias() call for every contained interface
  * However, when iSCF::SCF == 0, don't call QI to prevent interface ID 
  * resolution (which will fail).
  */
 #define SCF_INIT_TRACKER_ALIASES    \
  if (iSCF::SCF != 0) QueryInterface ((scfInterfaceID)-1, -1);
#else
 #define CS_TYPENAME(x)		    0
 #define SCF_INIT_TRACKER_ALIASES
#endif

//-- Old style implementation
/**
 * This macro should be embedded into any SCF-capable class definition
 * to declare the minimal functionality required by iBase interface.
 */
#define SCF_DECLARE_IBASE						\
  int scfRefCount;		/* Reference counter */			\
  csArray<iBase**>* scfWeakRefOwners;					\
  void scfRemoveRefOwners ();						\
  SCF_DECLARE_EMBEDDED_IBASE (iBase)

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
  virtual void AddRefOwner (iBase** ref_owner);				\
  virtual void RemoveRefOwner (iBase** ref_owner);			\
  virtual void *QueryInterface (scfInterfaceID iInterfaceID, int iVersion)

/**
 * The SCF_CONSTRUCT_IBASE macro should be invoked inside the constructor
 * of a class (not inside an embedded interface). Normally each
 * constructor should accept an iBase* parameter (that is passed by
 * scfCreateInstance function) which should be passed to this macro.
 * The macro will zero the reference count and initialize the pointer
 * to the parent object.  If the object is unparented (a common case),
 * it is okay to use null as the argument to this macro.
 */
#define SCF_CONSTRUCT_IBASE(Parent)					\
  csRefTrackerAccess::TrackConstruction (this);				\
  csRefTrackerAccess::SetDescription (this, CS_TYPENAME(*this));	\
  scfRefCount = 1;							\
  scfWeakRefOwners = 0;							\
  scfParent = Parent; if (scfParent) scfParent->IncRef();		\
  SCF_INIT_TRACKER_ALIASES 						

/**
 * The SCF_CONSTRUCT_EMBEDDED_IBASE macro should be invoked inside the
 * constructor of a class that has an embedded interface
 * (not inside the constructor of the embedded interface).
 * The macro will initialize the pointer to the parent object
 * (to the object this one is embedded into).  The argument to this macro
 * is the name of the parent's instance variable by which the embedded
 * object is known (typically something like `scfiFooBar').
 */
#define SCF_CONSTRUCT_EMBEDDED_IBASE(Interface)				\
  Interface.scfParent = this;						\
  csRefTrackerAccess::AddAlias (&Interface, this);

/**
 * The SCF_DESTRUCT_IBASE macro should be invoked inside the destructor
 * of a class (not inside an embedded interface).  It reverses the
 * initialization performed by the SCF_CONSTRUCT_IBASE() macro.
 */
#define SCF_DESTRUCT_IBASE()						\
  csRefTrackerAccess::TrackDestruction (this, scfRefCount);		\
  scfRemoveRefOwners ();

/**
 * The SCF_DESTRUCT_EMBEDDED_IBASE macro should be invoked inside the
 * destructor of a class that has an embedded interface (not inside the
 * destructor of the embedded interface).  It reverses the initialization
 * performed by the SCF_CONSTRUCT_EMBEDDED_IBASE() macro.
 */
#define SCF_DESTRUCT_EMBEDDED_IBASE(Interface)				\
  csRefTrackerAccess::RemoveAlias (&Interface, this);			\
  Interface.scfParent = 0;

/**
 * The SCF_IMPLEMENT_IBASE_INCREF() macro implements the IncRef() method for a
 * class in a C++ source module.  Typically, this macro is automatically
 * employed by the SCF_IMPLEMENT_IBASE() convenience macro.
 */
#define SCF_IMPLEMENT_IBASE_INCREF(Class)				\
void Class::IncRef ()							\
{									\
  SCF_TRACE (("  (%s *)%p->IncRef (%d)\n", #Class, this, scfRefCount + 1));\
  csRefTrackerAccess::TrackIncRef (this, scfRefCount);			\
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
  csRefTrackerAccess::TrackDecRef (this, scfRefCount);			\
  if (scfRefCount == 1)							\
  {									\
    SCF_TRACE ((" delete (%s *)%p\n", #Class, this));			\
    scfRemoveRefOwners ();						\
    if (scfParent)							\
      scfParent->DecRef ();						\
    delete this;							\
    return;								\
  }									\
  scfRefCount--;							\
}

/**
 * The SCF_IMPLEMENT_IBASE_REMOVE_REF_OWNERS() macro implements the
 * scfRemoveRefOwners() method for a class in a C++ source module.  Typically,
 * this macro is automatically employed by the SCF_IMPLEMENT_IBASE()
 * convenience macro.
 */
#define SCF_IMPLEMENT_IBASE_REMOVE_REF_OWNERS(Class)			\
void Class::scfRemoveRefOwners ()					\
{									\
  if (!scfWeakRefOwners) return;					\
  for (size_t i = 0 ; i < scfWeakRefOwners->Length () ; i++)		\
  {									\
    iBase** p = (*scfWeakRefOwners)[i];					\
    *p = 0;								\
  }									\
  delete scfWeakRefOwners;						\
  scfWeakRefOwners = 0;							\
}

/**
 * The SCF_IMPLEMENT_IBASE_REFOWNER() macro implements the AddRefOwner()
 * and RemoveRefOwner() for a weak reference.
 */
#define SCF_IMPLEMENT_IBASE_REFOWNER(Class)				\
void Class::AddRefOwner (iBase** ref_owner)				\
{									\
  if (!scfWeakRefOwners)						\
    scfWeakRefOwners = new csArray<iBase**> (0, 4);			\
  scfWeakRefOwners->InsertSorted (ref_owner);				\
}									\
void Class::RemoveRefOwner (iBase** ref_owner)				\
{									\
  if (!scfWeakRefOwners)						\
    return;								\
  size_t index = scfWeakRefOwners->FindSortedKey (			\
    csArrayCmp<iBase**, iBase**> (ref_owner)); 				\
  if (index != csArrayItemNotFound) scfWeakRefOwners->DeleteIndex (	\
    index); 								\
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
  SCF_TRACE (("  (%s *)%p->QueryInterface (%lu, %08X)\n",		\
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
  SCF_IMPLEMENT_IBASE_REFOWNER(Class)					\
  SCF_IMPLEMENT_IBASE_REMOVE_REF_OWNERS(Class)				\
  SCF_IMPLEMENT_IBASE_QUERY(Class)

/**
 * The SCF_IMPLEMENT_IBASE_END macro is used to finish an SCF_IMPLEMENT_IBASE
 * definition
 */
#define SCF_IMPLEMENT_IBASE_END						\
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
 * The SCF_IMPLEMENT_EMBEDDED_IBASE_REFOWNER() macro implements the
 * AddRefOwner() and RemoveRefOwner() for a weak reference.
 */
#define SCF_IMPLEMENT_EMBEDDED_IBASE_REFOWNER(Class)			\
void Class::AddRefOwner (iBase** ref_owner)				\
{									\
  scfParent->AddRefOwner (ref_owner);					\
}									\
void Class::RemoveRefOwner (iBase** ref_owner)				\
{									\
  scfParent->RemoveRefOwner (ref_owner);				\
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
  SCF_TRACE (("  (%s *)%p->QueryInterface (%lu, %08X)\n",		\
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
  SCF_IMPLEMENT_EMBEDDED_IBASE_REFOWNER(Class)				\
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
  csRefTrackerAccess::AddAlias (CS_STATIC_CAST(Interface*, this), this);\
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
  if (iInterfaceID == scfInterfaceTraits<Interface>::GetID() &&		\
    scfCompatibleVersion (iVersion, scfInterfaceTraits<Interface>::GetVersion())) \
  {									\
    (Object)->IncRef ();						\
    return CS_STATIC_CAST(Interface*, Object);				\
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
  virtual void AddRefOwner (iBase** ref_owner);				\
  virtual void RemoveRefOwner (iBase** ref_owner);			\
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
 * The SCF_IMPLEMENT_IBASE_EXT_REFOWNER() macro implements the
 * AddRefOwner() and RemoveRefOwner() for a weak reference.
 */
#define SCF_IMPLEMENT_IBASE_EXT_REFOWNER(Class)			\
void Class::AddRefOwner (iBase** ref_owner)				\
{									\
  __scf_superclass::AddRefOwner (ref_owner);				\
}									\
void Class::RemoveRefOwner (iBase** ref_owner)				\
{									\
  __scf_superclass::RemoveRefOwner (ref_owner);				\
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
  SCF_IMPLEMENT_IBASE_EXT_REFOWNER(Class)				\
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
#ifdef CS_MEMORY_TRACKER
// This special version of SCF_IMPLEMENT_FACTORY_INIT will make sure that
// the memory tracker for this plugin is implemented.
#define SCF_IMPLEMENT_FACTORY_INIT(Class)				\
static inline void Class ## _scfUnitInitialize(iSCF* SCF)		\
{									\
  iSCF::SCF = SCF;							\
  extern void mtiRegisterModule (char*);				\
  mtiRegisterModule (#Class);						\
}									\
CS_EXPORTED_FUNCTION							\
void CS_EXPORTED_NAME(Class,_scfInitialize)(iSCF* SCF)			\
{ Class ## _scfUnitInitialize(SCF); }
#else
#define SCF_IMPLEMENT_FACTORY_INIT(Class)				\
static inline void Class ## _scfUnitInitialize(iSCF* SCF)		\
{ iSCF::SCF = SCF; }							\
CS_EXPORTED_FUNCTION							\
void CS_EXPORTED_NAME(Class,_scfInitialize)(iSCF* SCF)			\
{ Class ## _scfUnitInitialize(SCF); }
#endif

/**
 * The SCF_IMPLEMENT_FACTORY_FINIS macro defines finalization code for a plugin
 * module.  As with SCF_IMPLEMENT_FACTORY_INIT, only one instance of this
 * function will be invoked to finalize the module.
 */
#define SCF_IMPLEMENT_FACTORY_FINIS(Class)				\
CS_EXPORTED_FUNCTION							\
void CS_EXPORTED_NAME(Class,_scfFinalize)()				\
{									\
CS_STATIC_VARIABLE_CLEANUP						\
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
  iBase *ret = new Class (iParent);					\
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
 * Define the C++ class needed to register an SCF class, but don't do any
 * automatic registration.
 */
#define SCF_DEFINE_FACTORY_FUNC_REGISTRATION(Class)			\
  CS_EXPORTED_FUNCTION void* CS_EXPORTED_NAME(Class,_Create)(iBase*);	\
  class Class##_StaticInit						\
  {									\
  public:								\
    Class##_StaticInit()						\
    {									\
      scfInitialize(0);							\
      iSCF::SCF->RegisterFactoryFunc(CS_EXPORTED_NAME(Class,_Create),#Class); \
    }									\
  };

/**
 * Register a statically linked plugin. The _static version of the plugin 
 * needs to be linked in, too.
 */
#define SCF_USE_STATIC_PLUGIN(Module)					\
  namespace csStaticPluginInit						\
  {									\
    class Module { public: Module(); };					\
    Module Module##_StaticInit;						\
  }

/**
 * Used in conjunction with SCF_REGISTER_STATIC_LIBRARY to ensure that a
 * reference to the class(es) registered via SCF_REGISTER_STATIC_LIBRARY are
 * actually linked into the application.  Invoke this macro once for each
 * \<implementation\> node mentioned in the MetaInfo registered with
 * SCF_REGISTER_STATIC_LIBRARY.  Invocations of this macro must appear after
 * the the invocation of SCF_REGISTER_STATIC_LIBRARY.
 */
#define SCF_REGISTER_FACTORY_FUNC(Class)				\
  SCF_DEFINE_FACTORY_FUNC_REGISTRATION(Class)				\
  Class##_StaticInit Class##_static_init__;

//--------------------------------------------- Class factory interface -----//

/**
 * iFactory is an interface that is used to create instances of shared classes.
 * Any object supports the iFactory interface; a QueryInterface about iFactory
 * will return a valid pointer to the factory that was used to create that
 * object. Thus you can clone objects without even knowing their types.
 * <p>
 * NOTE: Currently you cannot add factories to the class factory list since it
 * is internally maintained by SCF. That is, you can use an existing factory
 * but cannot create objects that implements this interface (well, you can
 * but its pointless since you won't be able to add it to the factory list).
 * Instead, you should register new class factories through the normal class
 * registration mechanism.
 */
struct iFactory : public iBase
{
  /// Create a instance of class this factory represents.
  virtual iBase *CreateInstance () = 0;
  /// Try to unload class module (i.e. shared module).
  virtual void TryUnload () = 0;
  /// Query class description string.
  virtual const char *QueryDescription () = 0;
  /// Query class dependency strings.
  virtual const char *QueryDependencies () = 0;
  /// Query class ID
  virtual const char *QueryClassID () = 0;
  /// Query library module name.
  virtual const char *QueryModuleName () = 0;
};

//----------------------------------------------- Client-side functions -----//

/**
 * SCF verbosity flags. For use with scfInitialize(). Combine with bitwise-or
 * to select more than one.
 */
enum
{
  SCF_VERBOSE_NONE            = 0,      ///< No diagnostic information.
  SCF_VERBOSE_PLUGIN_SCAN     = 1 << 0, ///< Directories scanned for plugins.
  SCF_VERBOSE_PLUGIN_LOAD     = 1 << 1, ///< Plugins loaded and unloaded.
  SCF_VERBOSE_PLUGIN_REGISTER = 1 << 2, ///< Plugins discovered and registered.
  SCF_VERBOSE_CLASS_REGISTER  = 1 << 3, ///< Classes registered within plugins.
  SCF_VERBOSE_ALL             = ~0      ///< All diagnostic information.
};

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
 * \param verbose One or more of the \c SCF_VERBOSE_FOO flags combined with
 *   bitwise-or which control SCF verbosity.
 * \remark The path list is ignored for static builds.
 */
CS_CRYSTALSPACE_EXPORT void scfInitialize(csPathsList const* pluginPaths,
  unsigned int verbose = SCF_VERBOSE_NONE);

/**
 * This function should be called to initialize client SCF library.
 * It uses the default plugin paths provided by csGetPluginPaths().
 */
CS_CRYSTALSPACE_EXPORT void scfInitialize(int argc, const char* const argv[]);


//---------- IMPLEMENTATION OF HELPER FUNCTIONS



/**
 * Helper function around iBase::QueryInterface
 */
template<class Interface, class ClassPtr>
inline csPtr<Interface> scfQueryInterface (ClassPtr object)
{
  Interface *x = (Interface*)object->QueryInterface (
    scfInterfaceTraits<Interface>::GetID (),
    scfInterfaceTraits<Interface>::GetVersion ());
  return csPtr<Interface> (x);
}

//Compatibility macro for scfQueryInterface
#define SCF_QUERY_INTERFACE(Object,Interface) \
  scfQueryInterface<Interface> (Object)

/**
 * Helper function around iBase::QueryInterface which also 
 * does null-check of object.
 */
template<class Interface, class ClassPtr>
inline csPtr<Interface> scfQueryInterfaceSafe (ClassPtr object)
{
  if (object == 0) return csPtr<Interface> (0);

  Interface *x = (Interface*)object->QueryInterface (
    scfInterfaceTraits<Interface>::GetID (),
    scfInterfaceTraits<Interface>::GetVersion ());
  return csPtr<Interface> (x);
}
//Compatibility macro for scfQueryInterfaceSafe
#define SCF_QUERY_INTERFACE_SAFE(Object,Interface) \
  scfQueryInterfaceSafe<Interface> (Object)

/**
 * Handy function to create an instance of a shared class.
 */
template<class Interface>
inline csPtr<Interface> scfCreateInstance (char const * const ClassID)
{
  iBase *base = iSCF::SCF->CreateInstance (ClassID);

  if (base == 0) return csPtr<Interface> (0);

  Interface *x = (Interface*)base->QueryInterface (
    scfInterfaceTraits<Interface>::GetID (),
    scfInterfaceTraits<Interface>::GetVersion ());

  if (x) base->DecRef (); //release our base interface
  return csPtr<Interface> (x);
}

// Compatibility macro for scfCreateInstance function
#define SCF_CREATE_INSTANCE(ClassID,Interface) \
  scfCreateInstance<Interface> (ClassID)

// Give versions to above declared classes.
SCF_VERSION (iFactory, 0, 0, 2);

// A bit hacky.
#include "csutil/reftrackeraccess.h"

/** @} */

#endif // __CSSCF_H__
