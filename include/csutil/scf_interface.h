/*
  Crystal Space Shared Class Facility (SCF)
  This header contains the parts of SCF that is needed for defining
  new interfaces.

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

#ifndef __CSUTIL_SCF_INTERFACE_H__
#define __CSUTIL_SCF_INTERFACE_H__

#include "csextern.h"

// -- Forward declarations
struct iDocument;
#if defined(CS_DEBUG) || defined(CS_MEMORY_TRACKER)
  struct iObjectRegistry;
#endif
template<class T>
class csRef;
struct iStringArray;

/**\file
 * Crystal Space Shared Class Facility (SCF) - interface creation support
 */

/**
 * \addtogroup scf
 * @{ */

/**
 * Type of registered interface handle used by iBase::QueryInterface().
 */
typedef unsigned long scfInterfaceID;

/**
 * Type of interface version used by iBase::QueryInterface().
 */
typedef int scfInterfaceVersion;

// -- Some helpers needed below
/**
* SCF_INTERFACE can be used to define an interface's version number;
* you should specify interface name and major, minor and micro version
* components. This way:
* <pre>
* struct iSomething : public iBase
* {
* public:
*   SCF_INTERFACE(iSomething, 0, 0, 1);
*   ...
* };
* </pre>
*/
#define SCF_INTERFACE(Name,Major,Minor,Micro)             \
struct InterfaceTraits {                                  \
  static CS_FORCEINLINE scfInterfaceVersion GetVersion()          \
  { return SCF_CONSTRUCT_VERSION(Major, Minor, Micro); }  \
  static CS_FORCEINLINE char const * GetName() { return #Name; }  \
}


/// Use this macro to construct interface version numbers.
#define SCF_CONSTRUCT_VERSION(Major,Minor,Micro)          \
  ((Major << 24) | (Minor << 16) | Micro)


/**
 * This function checks whenever an interface is compatible with given version.
 * SCF uses the following comparison criteria: if the major version numbers
 * are equal and required minor and micro version is less or equal than target
 * version minor and micro numbers, the versions are considered compatible.
 */
static CS_FORCEINLINE bool scfCompatibleVersion (int iVersion, int iItfVersion)
{
  return ((iVersion & 0xff000000) == (iItfVersion & 0xff000000))
    && ((iVersion & 0x00ffffff) <= (iItfVersion & 0x00ffffff));
}

// -- The main two SCF interfaces, iBase and iSCF

/**
 * This is the basic interface: all other interfaces should be
 * derived from this one, this will allow us to always use at least
 * some minimal functionality given any interface pointer.
 */
struct iBase
{
  // Jorrit: removed the code below as it causes 'pure virtual' method
  // calls to happen upon destruction.
  //protected:
  //// Needed for GCC4. Otherwise emits a flood of "virtual functions but
  //// non-virtual destructor" warnings.
  //virtual ~iBase() {}
public:
  SCF_INTERFACE(iBase, 1, 0, 0);
  /// Increment the number of references to this object.
  virtual void IncRef () = 0;
  /// Decrement the reference count.
  virtual void DecRef () = 0;
  /// Get the ref count (only for debugging).
  virtual int GetRefCount () = 0;
  /**
   * Query a particular interface implemented by this object. You are 
   * _not_ allowed to cast this to anything but a pointer to this interface
   * (not even iBase).
   * Use scfQueryInterface<interface> instead of using this method directly.
   */
  virtual void *QueryInterface (scfInterfaceID iInterfaceID, int iVersion) = 0;
  /// For weak references: add a reference owner.
  virtual void AddRefOwner (iBase** ref_owner) = 0;
  /// For weak references: remove a reference owner.
  virtual void RemoveRefOwner (iBase** ref_owner) = 0;
};

/// Type of factory function which creates an instance of an SCF class.
typedef iBase* (*scfFactoryFunc)(iBase*);

/**
 * iSCF is the interface that allows using SCF functions from shared classes.
 * Since there should be just one instance of SCF kernel, the shared classes
 * should not use scfXXX functions directly; instead they should obtain a
 * pointer to an iSCF object and work through that pointer.
 */
struct iSCF : public iBase
{
  SCF_INTERFACE(iSCF, 2, 0,0);
  /**
   * This is the global instance of iSCF.  On most platforms, this variable is
   * module-global; for instance, the application has an iSCF::SCF variable,
   * and each plugin module has an iSCF::SCF variable, all of which point at
   * the same shared instance of iSCF.  On other platforms, though, the
   * variable might truly be global, in which case the variable itself is
   * shared by application and all plugin modules.  In actual practice,
   * however, whether the variable's scope is global or only module-global,
   * makes no difference since clients access the shared instance uniformly as
   * iSCF::SCF.
   */
  static CS_CRYSTALSPACE_EXPORT iSCF* SCF;

#if defined(CS_DEBUG) || defined(CS_MEMORY_TRACKER)
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
   * The returned pointer will be to a iBase, and thus you need to use
   * scfQueryInterface if you want to get a specific interface.
   */
  virtual iBase *CreateInstance (const char *iClassID) = 0;

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
   * Given a registered class name, returns the meta information associated
   * with the plugin module in which the class is implemented.  Since the meta
   * information associated with a plugin is extensible, plugin authors are
   * free to attach any additional information they desire, beyond that which
   * is used by SCF itself.  This function provides a way for clients to access
   * the additional meta information which plugin authors might choose to
   * publish.
   * <p>
   * If the specified class is not implemented by a plugin module (for
   * instance, it might be implemented directly by the application), or if
   * plugin module is lacking meta information for some reason, then the
   * returned csRef<> will be <em>invalid</em>.  You should check for this
   * condition by invoking csRef<>::IsValid() or simply by using the returned
   * reference in a boolean conditional expression.
   * <p>
   * Note that it is possible for a single plugin module to export multiple,
   * named SCF classes.  The meta information returned by this function belongs
   * to the plugin itself, not to any individual class exported by that plugin.
   * Therefore, if you invoke this method twice for two different classes, and
   * those classes are exported by the same plugin, then the same meta
   * information will be returned by both queries.
   * <p>
   * If you know the physical path of a plugin, then you can instead invoke
   * csGetPluginMetadata() (csutil/csshlib.h) to retrieve its meta information.
   */
  virtual csRef<iDocument> GetPluginMetadata (char const *iClassID) = 0;

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
   * with an implementation name (the value in the \<implementation\> node of
   * the meta information; also the name of the iFactoryClass in
   * RegisterClass).  Returns true upon sucess, or false if the class does not
   * exist or already has an associated creation function.
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
  * Return the name of an interface given an interface ID.  If the ID is
  * unknown, null is returned.
  */
  virtual char const* GetInterfaceName (scfInterfaceID) const = 0;

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
   * value is a list of strings.  If no class names match the pattern string,
   * then the returned list is empty.
   */
  virtual csRef<iStringArray> QueryClassList (char const* pattern) = 0;

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


//-- Interface traits

/**
 * Interface information query class.  This template class allows you to 
 * query static information about SCF interfaces, such as an interface's 
 * current version number.  For example, to find out the version number 
 * of the iFooBar SCF interface, you would invoke 
 * scfInterfaceTraits<iFooBar>::GetVersion().
 * For old-style interfaces it is specialized through SCF_VERSION macro,
 * with new style interfaces no specialization is needed.
 */
template <class Interface> 
class scfInterfaceTraits
{
public:
  /**
   * Retrieve the interface's current version number.
   */
  static scfInterfaceVersion GetVersion ()
  {
    return Interface::InterfaceTraits::GetVersion ();
  }

  /**
   * Retrieve the interface's identifier.  This is a unique identifier by
   * which SCF recognizes the interface.  Although human's prefer to identify
   * interfaces symbolically via name, SCF perfers to identify them, for
   * performance reasons, by scfInterfaceID, which is typically a small
   * integer.
   */
  static scfInterfaceID GetID ()
  {
    scfInterfaceID& ID = GetMyID ();
    if (ID == (scfInterfaceID)(-1))
    {
      ID = iSCF::SCF->GetInterfaceID (GetName ());
      csStaticVarCleanup (CleanupID);
    }
    return ID;
  }

  /**
   * Retrieve the interface's name as a string.
   */
  static CS_FORCEINLINE char const* GetName ()
  { 
    return Interface::InterfaceTraits::GetName ();
  }
private:
  // This idiom is a Meyers singleton
  static CS_FORCEINLINE scfInterfaceID& GetMyID ()
  {
    static scfInterfaceID ID = (scfInterfaceID)-1;
    return ID;
  }
  static void CleanupID ()
  {
    GetMyID () = (scfInterfaceID)-1;
  }
};

/**
 * FOR COMPATIBILITY!
 * SCF_VERSION can be used to define an interface's version number;
 * you should specify interface name and major, minor and micro version
 * components. This way:
 * <pre>
 * SCF_VERSION (iSomething, 0, 0, 1);
 * struct iSomething : public iBase
 *   ...
 * };
 * </pre>
 * Notice that SCF_VERSION cannot be used on interfaces in namespaces
 */
#define SCF_VERSION(Name,Major,Minor,Micro)                \
struct Name;                                               \
CS_SPECIALIZE_TEMPLATE                                     \
class scfInterfaceTraits<Name>                             \
{                                                          \
public:                                                    \
  static scfInterfaceVersion GetVersion()                  \
  { return SCF_CONSTRUCT_VERSION(Major, Minor, Micro); }   \
  static char const* GetName ()                            \
  { return #Name; }                                        \
  static scfInterfaceID GetID ()                           \
  { scfInterfaceID& ID = GetMyID ();                       \
    if (ID == (scfInterfaceID)(-1))                        \
    { ID = iSCF::SCF->GetInterfaceID (GetName ());         \
      csStaticVarCleanup (CleanupID);    }                 \
    return ID;                                             \
  }                                                        \
private:                                                   \
  static scfInterfaceID& GetMyID ()                        \
  { static scfInterfaceID ID = (scfInterfaceID)-1; return ID; } \
  static void CleanupID ()                                 \
  { GetMyID () = (scfInterfaceID)-1; }                     \
}

/** @} */

#endif

