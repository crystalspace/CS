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

#ifndef CS_TYPENAME
  #ifdef CS_REF_TRACKER
   #include <typeinfo>
   #define CS_TYPENAME(x)		    typeid(x).name()
  #else
   #define CS_TYPENAME(x)		    0
  #endif
#endif

// INTERFACE DEFINITIONS
#include "csutil/scf_interface.h"
// NEW STYLE IMPLEMENTATION
//#include "csutil/scf_implementation.h"


//-- Helper macros

/**\def SCF_TRACE(x)
 * Macro for typing debug strings: Add \#define SCF_DEBUG at the top
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
#include "memdebug.h" // needed for mtiRegisterModule
// This special version of SCF_IMPLEMENT_FACTORY_INIT will make sure that
// the memory tracker for this plugin is implemented.
#define SCF_IMPLEMENT_FACTORY_INIT(Class)				\
static inline void Class ## _scfUnitInitialize(iSCF* SCF)		\
{									\
  iSCF::SCF = SCF;							\
  CS::Debug::MemTracker::RegisterModule (#Class);			\
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
iBase* CS_EXPORTED_NAME(Class,_Create)(iBase *iParent)			\
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
  CS_EXPORTED_FUNCTION iBase* CS_EXPORTED_NAME(Class,_Create)(iBase*);	\
  class Class##_StaticInit__						\
  {									\
  public:								\
    Class##_StaticInit__()						\
    {									\
      scfRegisterStaticClass(						\
        CS_EXPORTED_NAME(Class,_Create), Ident, Desc, Dep);		\
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
      scfRegisterStaticClasses (MetaInfo);				\
    }									\
  } Module##_static_init__;

/**
 * Define the C++ class needed to register an SCF class, but don't do any
 * automatic registration.
 */
#define SCF_DEFINE_FACTORY_FUNC_REGISTRATION(Class)			\
  CS_EXPORTED_FUNCTION iBase* CS_EXPORTED_NAME(Class,_Create)(iBase*);	\
  class Class##_StaticInit						\
  {									\
  public:								\
    Class##_StaticInit()						\
    {									\
      scfRegisterStaticFactoryFunc (CS_EXPORTED_NAME(Class,_Create),	\
	#Class); 							\
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
// Give versions to above declared classes.
SCF_VERSION (iFactory, 0, 0, 2);

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
 */
CS_CRYSTALSPACE_EXPORT void scfInitialize(csPathsList const* pluginPaths,
  unsigned int verbose = SCF_VERBOSE_NONE);

/**
 * This function should be called to initialize client SCF library.
 * If \a scanDefaultPluginPaths is true the default plugin paths provided by
 * csGetPluginPaths() are scanned. Otherwise, no plugin scanning is done.
 * In this case the csPathsList* version of scfInitialize() should be used if
 * some paths should be scanned for plugins.
 */
CS_CRYSTALSPACE_EXPORT void scfInitialize(int argc, const char* const argv[],
  bool scanDefaultPluginPaths = true);

//@{
/**
 * Register a static class.
 * This needs extra handling since they require automatic registration even
 * when SCF is initialized after it was possibly shut down previously.
 * The static class info is stored and read out later when SCF is initialized.
 */
CS_CRYSTALSPACE_EXPORT void scfRegisterStaticClass (scfFactoryFunc, 
  const char *iClassID, const char *Description, 
  const char *Dependencies = 0);
CS_CRYSTALSPACE_EXPORT void scfRegisterStaticClasses (char const* xml);
CS_CRYSTALSPACE_EXPORT void scfRegisterStaticFactoryFunc (scfFactoryFunc, 
  const char *FactClass);
//@}
  
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
// Save a QI for 'identity' queries 
/*
   However, this does not fly on all compilers.
   Known working:
     gcc 4.1.2
   Known NOT working:
     gcc 3.4.2
 */
#if (!defined(__GNUC__) || (__GNUC__ >= 4))
template<class Interface>
inline csPtr<Interface> scfQueryInterface (Interface* object)
{
  object->IncRef ();
  return csPtr<Interface> (object);
}
#endif

/**
 * Helper function around iBase::QueryInterface which also 
 * does null-check of object.
 */
template<class Interface, class ClassPtr>
inline csPtr<Interface> scfQueryInterfaceSafe (ClassPtr object)
{
  if (object == 0) return csPtr<Interface> (0);
  return scfQueryInterface<Interface> (object);
}

/**
 * Handy function to create an instance of a shared class.
 */
template<class Interface>
inline csPtr<Interface> scfCreateInstance (char const * const ClassID)
{
  csRef<iBase> base = csPtr<iBase> (iSCF::SCF->CreateInstance (ClassID));
  return scfQueryInterfaceSafe<Interface> (base);
}



// A bit hacky.
#include "csutil/reftrackeraccess.h"

/** @} */

#endif // __CSSCF_H__
