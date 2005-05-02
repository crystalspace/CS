/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_CSSYSDEF_H__
#define __CS_CSSYSDEF_H__

#define CSDEF_FRIEND
#include "csdef.h"
#undef CSDEF_FRIEND

/** \file
  This file should be #included before any other Crystal Space header files. It
  sets up a compilation environment which smooths over differences between
  platforms, allowing the same code to compile cleanly over a variety of
  operating systems and build tools. It also provides a number of utility
  macros useful to projects utilizing Crystal Space and to the Crystal Space
  code itself.
*/

/*
 * Pull in platform-specific overrides of the requested functionality.
 */
#include "csutil/csosdefs.h"

// Defaults for platforms that do not define their own.
#ifndef CS_EXPORT_SYM_DLL
#  ifdef CS_HAVE_GCC_VISIBILITY
#    define CS_EXPORT_SYM_DLL  __attribute__ ((visibility ("default")))
#  else
#    define CS_EXPORT_SYM_DLL
#  endif
#endif
#ifndef CS_IMPORT_SYM_DLL
#  define CS_IMPORT_SYM_DLL extern
#endif
#ifndef CS_EXPORT_SYM
#  if defined(CS_HAVE_GCC_VISIBILITY) && defined(CS_BUILD_SHARED_LIBS)
#    define CS_EXPORT_SYM  __attribute__ ((visibility ("default")))
#  else
#    define CS_EXPORT_SYM
#  endif
#endif
#ifndef CS_IMPORT_SYM
#  define CS_IMPORT_SYM
#endif

#include "csextern.h"

/*
 * Default definitions for requested functionality.  Platform-specific
 * configuration files may override these.
 */

#ifndef CS_FORCEINLINE
#define CS_FORCEINLINE inline
#endif

/**\def CS_MAXPATHLEN
 * Maximum length of a filesystem pathname. Useful for declaring character
 * buffers for calls to system functions which return a pathname in the buffer.
 */
#ifndef CS_MAXPATHLEN
#define CS_MAXPATHLEN 1024
#endif
#include <stdio.h>
#ifdef CS_HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

/**\def CS_ALLOC_STACK_ARRAY(type, var, size)
 * Dynamic stack memory allocation.
 * \param type Type of the array elements.
 * \param var Name of the array to be allocated.
 * \param size Number of elements to be allocated.
 */
#ifdef CS_COMPILER_GCC
// In GCC we are able to declare stack vars of dynamic size directly
#  define CS_ALLOC_STACK_ARRAY(type, var, size) \
     type var [size]
#else
#  include <malloc.h>
#  define CS_ALLOC_STACK_ARRAY(type, var, size) \
     type *var = (type *)alloca ((size) * sizeof (type))
#endif

/**\def CS_TEMP_DIR
 * Directory for temporary files
 */
#ifndef CS_TEMP_DIR
#  if defined(CS_PLATFORM_UNIX)
#    define CS_TEMP_DIR "/tmp/"
#  else
#    define CS_TEMP_DIR ""
#  endif
#endif

/**\def CS_TEMP_FILE
 * Name for temporary files
 */
#ifndef CS_TEMP_FILE
#  if defined(CS_PLATFORM_UNIX)
#    define CS_TEMP_FILE "cs%lud.tmp", (unsigned long)getpid()
#  else
#    define CS_TEMP_FILE "$cs$.tmp"
#  endif
#endif

#ifdef CS_USE_CUSTOM_ISDIR
static inline bool isdir (const char *path, struct dirent *de)
{
  int pathlen = strlen (path);
  char* fullname = new char[pathlen + 2 + strlen (de->d_name)];
  memcpy (fullname, path, pathlen + 1);
  if ((pathlen) && (fullname[pathlen-1] != CS_PATH_SEPARATOR))
  {
    fullname[pathlen++] = CS_PATH_SEPARATOR;
    fullname[pathlen] = 0;
  }
  strcat (&fullname [pathlen], de->d_name);
  struct stat st;
  stat (fullname, &st);
  delete[] fullname;
  return ((st.st_mode & S_IFMT) == S_IFDIR);
}
#endif

/**\def CS_HAVE_POSIX_MMAP
 * Platforms which support POSIX mmap() should #define CS_HAVE_POSIX_MMAP. This
 * can be done via the platform-specific csosdef.h or via the configure script.
 * Doing so will declare a POSIX mmap()-suitable csMemMapInfo structure. The
 * build process on such platforms must also arrange to have
 * CS/libs/csutil/generic/mmap.cpp incorporated into the csutil library.
 */
#ifdef CS_HAVE_POSIX_MMAP

#ifndef CS_HAVE_MEMORY_MAPPED_IO
#define CS_HAVE_MEMORY_MAPPED_IO
#endif

/// POSIX-specific memory mapped I/O platform dependent structure.
struct csMemMapInfo
{          
  /// Handle to the mapped file.
  int hMappedFile;
  /// Base pointer to the data.
  unsigned char *data;
  /// File size.
  unsigned int file_size;
  /// Close file descriptor (handle) when unmapping.
  bool close;
};

#endif // CS_HAVE_POSIX_MMAP

/**
 * The CS_HEADER_GLOBAL() macro composes a pathname from two components and
 * wraps the path in `<' and `>'.  This macro is useful in cases where one does
 * not have the option of augmenting the preprocessor's header search path,
 * even though the include path for some header file may vary from platform to
 * platform.  For instance, on many platforms OpenGL headers are in a `GL'
 * directory, whereas on other platforms they are in an `OpenGL' directory.  As
 * an example, in the first case, the platform might define the preprocessor
 * macro GLPATH with the value `GL', and in the second case GLPATH would be
 * given the value `OpenGL'.  To actually include an OpenGL header, such as
 * gl.h, the following code would be used:
 * <pre>
 * #include CS_HEADER_GLOBAL(GLPATH,gl.h)
 * </pre>
 */
#define CS_HEADER_GLOBAL(X,Y) CS_HEADER_GLOBAL_COMPOSE(X,Y)
#define CS_HEADER_GLOBAL_COMPOSE(X,Y) <X/Y>

/**
 * The CS_HEADER_LOCAL() macro composes a pathname from two components and
 * wraps the path in double-quotes.  This macro is useful in cases where one
 * does not have the option of augmenting the preprocessor's header search
 * path, even though the include path for some header file may vary from
 * platform to platform.  For example, assuming that the preprocessor macro
 * UTILPATH is defined with some platform-specific value, to actually include a
 * header, such as util.h, the following code would be used:
 * <pre>
 * #include CS_HEADER_LOCAL(UTILPATH,util.h)
 * </pre>
 */
#define CS_HEADER_LOCAL(X,Y) CS_HEADER_LOCAL_COMPOSE1(X,Y)
#define CS_HEADER_LOCAL_COMPOSE1(X,Y) CS_HEADER_LOCAL_COMPOSE2(X/Y)
#define CS_HEADER_LOCAL_COMPOSE2(X) #X


/**\def CS_EXPORTED_FUNCTION
 * \internal A macro to export a function from a shared library.
 * Some platforms may need to override this.  For instance, Windows requires
 * extra `__declspec' goop when exporting a function from a plug-in module.
 */
#if !defined(CS_EXPORTED_FUNCTION)
#  define CS_EXPORTED_FUNCTION extern "C" CS_EXPORT_SYM_DLL
#endif

/**\def CS_EXPORTED_NAME(Prefix, Suffix)
 * \internal A macro used to build exported function names.
 * Usually "Prefix" is derived from shared library name, thus for each library
 * we'll have different exported names.  This prevents naming collisions when
 * static linking is used, and on platforms where plug-in symbols are exported
 * by default.  However, this may be bad for platforms which need to build
 * special export-tables on-the-fly at compile-time since distinct names make
 * the job more difficult.  Such platforms may need to override the default
 * expansion of this macro to use only the `Suffix' and ignore the `Prefix'
 * when composing the name.
 */
#if !defined(CS_EXPORTED_NAME)
#  define CS_EXPORTED_NAME(Prefix, Suffix) Prefix ## Suffix
#endif

#ifndef CS_IMPLEMENT_PLATFORM_PLUGIN
#  define CS_IMPLEMENT_PLATFORM_PLUGIN
#endif

#ifndef CS_IMPLEMENT_PLATFORM_APPLICATION
#  define CS_IMPLEMENT_PLATFORM_APPLICATION
#endif

/**\def CS_INITIALIZE_PLATFORM_APPLICATION
 * Perform platform-specific application initializations.
 * This macro should be invoked very near to the "beginning" of the 
 * application.
 * \remark NB: It is invoked in csInitializer::CreateEnvironment().
 */
#ifndef CS_INITIALIZE_PLATFORM_APPLICATION
#  define CS_INITIALIZE_PLATFORM_APPLICATION /* */
/*
  This definition may seem odd, but it's here for doxygen's sake, which
  apparently fails to document empty macro definitions.
 */
#endif

typedef void (*csStaticVarCleanupFN) (void (*p)());
extern csStaticVarCleanupFN csStaticVarCleanup;

#ifndef CS_IMPLEMENT_STATIC_VARIABLE_REGISTRATION
#  define CS_IMPLEMENT_STATIC_VARIABLE_REGISTRATION(Name)              \
void Name (void (*p)())                                                \
{                                                                      \
  static void (**a)() = 0;                                             \
  static int lastEntry = 0;                                            \
  static int maxEntries = 0;                                           \
                                                                       \
  if (p != 0)                                                          \
  {                                                                    \
    if (lastEntry >= maxEntries)                                       \
    {                                                                  \
      maxEntries += 10;                                                \
      if (a == 0)                                                      \
        a = (void (**)())malloc(maxEntries * sizeof(void*));           \
      else                                                             \
        a = (void (**)())realloc(a, maxEntries * sizeof(void*));       \
    }                                                                  \
    a[lastEntry++] = p;                                                \
  }                                                                    \
  else if (a != 0)                                                     \
  {                                                                    \
    for (int i = lastEntry - 1; i >= 0; i--)                           \
      a[i] ();                                                         \
    free (a);                                                          \
    a = 0;                                                             \
    lastEntry = 0;                                                     \
    maxEntries = 0;                                                    \
  }                                                                    \
}
#endif

#ifndef CS_DEFINE_STATIC_VARIABLE_REGISTRATION
#  define CS_DEFINE_STATIC_VARIABLE_REGISTRATION(func) \
    csStaticVarCleanupFN csStaticVarCleanup = &func
#endif

#ifndef CS_DECLARE_STATIC_VARIABLE_REGISTRATION
#  define CS_DECLARE_STATIC_VARIABLE_REGISTRATION(func) \
    void func (void (*p)())
#endif

#ifndef CS_DECLARE_DEFAULT_STATIC_VARIABLE_REGISTRATION
#  define CS_DECLARE_DEFAULT_STATIC_VARIABLE_REGISTRATION		\
    CS_CRYSTALSPACE_EXPORT 						\
    CS_DECLARE_STATIC_VARIABLE_REGISTRATION (csStaticVarCleanup_csutil);
#endif

/* scfStaticallyLinked - Flag indicating whether external linkage was used when 
 * building the application. Determines whether SCF scans for plugins at 
 * startup.
 */
/**\def CS_DEFINE_STATICALLY_LINKED_FLAG
 * Define the scfStaticallyLinked variable.
 */
#if defined(CS_BUILD_SHARED_LIBS)
#  define CS_DEFINE_STATICALLY_LINKED_FLAG
#elif defined(CS_STATIC_LINKED)
#  define CS_DEFINE_STATICALLY_LINKED_FLAG  bool scfStaticallyLinked = true;
#else
#  define CS_DEFINE_STATICALLY_LINKED_FLAG  bool scfStaticallyLinked = false;
#endif


/**\def CS_IMPLEMENT_FOREIGN_DLL
 * The CS_IMPLEMENT_FOREIGN_DLL macro should be placed at the global scope in
 * exactly one compilation unit comprising a foreign (non-Crystal Space)
 * module.  For maximum portability, each such module should employ this macro.
 * This is useful for situations in which a dynamic load library (DLL) is being
 * built for some other facility. Obvious examples are pure extension modules
 * for Python, Perl, and Java. For Crystal Space plugins, instead use
 * CS_IMPLEMENT_PLUGIN.  Platforms may override the definition of this macro in
 * order to augment the implementation of the foreign module with any special
 * implementation details required by the platform.
 */
#ifndef CS_IMPLEMENT_FOREIGN_DLL
#  if defined(CS_BUILD_SHARED_LIBS)
#    define CS_IMPLEMENT_FOREIGN_DLL					    \
       CS_IMPLEMENT_STATIC_VARIABLE_REGISTRATION(csStaticVarCleanup_local); \
       CS_DEFINE_STATICALLY_LINKED_FLAG					    \
       CS_DEFINE_STATIC_VARIABLE_REGISTRATION (csStaticVarCleanup_local);
#  else
#    define CS_IMPLEMENT_FOREIGN_DLL					    \
       CS_DECLARE_DEFAULT_STATIC_VARIABLE_REGISTRATION			    \
       CS_DEFINE_STATICALLY_LINKED_FLAG					    \
       CS_DEFINE_STATIC_VARIABLE_REGISTRATION (csStaticVarCleanup_csutil);
#  endif
#endif

/**\def CS_IMPLEMENT_PLUGIN
 * The CS_IMPLEMENT_PLUGIN macro should be placed at the global scope in
 * exactly one compilation unit comprising a plugin module.  For maximum
 * portability, each plugin module must employ this macro.  Platforms may
 * override the definition of this macro in order to augment the implementation
 * of the plugin module with any special implementation details required by the
 * platform.
 */
#if defined(CS_STATIC_LINKED)

#  ifndef CS_IMPLEMENT_PLUGIN
#  define CS_IMPLEMENT_PLUGIN        					\
          CS_IMPLEMENT_PLATFORM_PLUGIN 
#  endif

#elif !defined(CS_BUILD_SHARED_LIBS)

#  ifndef CS_IMPLEMENT_PLUGIN
#  define CS_IMPLEMENT_PLUGIN        					\
          CS_IMPLEMENT_PLATFORM_PLUGIN 					\
	  CS_DEFINE_STATICALLY_LINKED_FLAG				\
	  CS_DECLARE_DEFAULT_STATIC_VARIABLE_REGISTRATION		\
	  CS_DEFINE_STATIC_VARIABLE_REGISTRATION (csStaticVarCleanup_csutil);
#  endif

#else

#  ifndef CS_IMPLEMENT_PLUGIN
#  define CS_IMPLEMENT_PLUGIN						\
   CS_DEFINE_STATICALLY_LINKED_FLAG					\
   CS_IMPLEMENT_STATIC_VARIABLE_REGISTRATION(csStaticVarCleanup_local)	\
   CS_DEFINE_STATIC_VARIABLE_REGISTRATION (csStaticVarCleanup_local);	\
   CS_IMPLEMENT_PLATFORM_PLUGIN 
#  endif

#endif

/**\def CS_IMPLEMENT_APPLICATION
 * The CS_IMPLEMENT_APPLICATION macro should be placed at the global scope in
 * exactly one compilation unit comprising an application.  For maximum
 * portability, each application should employ this macro.  Platforms may
 * override the definition of this macro in order to augment the implementation
 * of an application with any special implementation details required by the
 * platform.
 */
#ifndef CS_IMPLEMENT_APPLICATION
#  define CS_IMPLEMENT_APPLICATION       				\
  CS_DECLARE_DEFAULT_STATIC_VARIABLE_REGISTRATION			\
  CS_DEFINE_STATICALLY_LINKED_FLAG					\
  CS_DEFINE_STATIC_VARIABLE_REGISTRATION (csStaticVarCleanup_csutil);	\
  CS_IMPLEMENT_PLATFORM_APPLICATION 
#endif

/**\def CS_REGISTER_STATIC_FOR_DESTRUCTION
 * Register a method that will destruct one static variable.
 */
#ifndef CS_REGISTER_STATIC_FOR_DESTRUCTION
#define CS_REGISTER_STATIC_FOR_DESTRUCTION(getterFunc)\
        csStaticVarCleanup (getterFunc);
#endif

/**\def CS_STATIC_VARIABLE_CLEANUP
 * Invoke the function that will call all destruction functions
 */
#ifndef CS_STATIC_VARIABLE_CLEANUP
#define CS_STATIC_VARIABLE_CLEANUP  \
        csStaticVarCleanup (0);
#endif

/**\def CS_IMPLEMENT_STATIC_VAR(getterFunc,Type,initParam,kill_how)
 * Create a global variable thats created on demand. Create a Getter function
 * to access the variable and a destruction function. The Getter function will
 * register the destruction function on first invocation. Example:
 * <pre>
 * CS_IMPLEMENT_STATIC_VAR (GetVertexPool, csVertexPool,)
 * </pre>
 * This will give you a global function GetVertexPool that returns a pointer to
 * a static variable.
 */

#ifndef CS_IMPLEMENT_STATIC_VAR_EXT
#define CS_IMPLEMENT_STATIC_VAR_EXT(getterFunc,Type,initParam,kill_how) \
extern "C" {                                                            \
static Type* getterFunc ();                                             \
static void getterFunc ## _kill ();					\
static void getterFunc ## _kill_array ();				\
void getterFunc ## _kill ()                                      	\
{                                                                       \
  (void)(&getterFunc ## _kill_array);					\
  delete getterFunc ();                                                 \
}                                                                       \
void getterFunc ## _kill_array ()                                	\
{                                                                       \
  (void)(&getterFunc ## _kill);						\
  delete [] getterFunc ();                                              \
}                                                                       \
Type* getterFunc ()                                                     \
{                                                                       \
  static Type *v=0;                                                     \
  if (!v)                                                               \
  {                                                                     \
    v = new Type initParam;                                             \
    csStaticVarCleanup (getterFunc ## kill_how);        		\
  }                                                                     \
  return v;                                                             \
}                                                                       \
}
#endif

#ifndef CS_IMPLEMENT_STATIC_VAR
#define CS_IMPLEMENT_STATIC_VAR(getterFunc,Type,initParam) \
 CS_IMPLEMENT_STATIC_VAR_EXT(getterFunc,Type,initParam,_kill)    
#endif

#ifndef CS_IMPLEMENT_STATIC_VAR_ARRAY
#define CS_IMPLEMENT_STATIC_VAR_ARRAY(getterFunc,Type,initParam) \
 CS_IMPLEMENT_STATIC_VAR_EXT(getterFunc,Type,initParam,_kill_array)    
#endif

/**\def CS_DECLARE_STATIC_CLASSVAR(var,getterFunc,Type)
 * Declare a static variable inside a class. This will also declare a Getter
 * function.  Example:
 * <pre>
 * CS_DECLARE_STATIC_CLASSVAR (pool, GetVertexPool, csVertexPool)
 * </pre>
 */
#ifndef CS_DECLARE_STATIC_CLASSVAR
#define CS_DECLARE_STATIC_CLASSVAR(var,getterFunc,Type)       \
static Type *var;                                             \
static Type *getterFunc ();                                   
#endif

#ifndef CS_DECLARE_STATIC_CLASSVAR_REF
#define CS_DECLARE_STATIC_CLASSVAR_REF(var,getterFunc,Type)   \
static Type *var;                                             \
static Type &getterFunc ();                                   
#endif

/**\def CS_IMPLEMENT_STATIC_CLASSVAR(Class,var,getterFunc,Type,initParam)
 * Create the static class variable that has been declared with
 * CS_DECLARE_STATIC_CLASSVAR.  This will also create the Getter function and
 * the destruction function.  The destruction function will be registered upon
 * the first invocation of the Getter function.  Example:
 * <pre>
 * CS_IMPLEMENT_STATIC_CLASSVAR (csPolygon2D, pool, GetVertexPool,
 *                               csVertexPool,)
 * </pre>
 */
#ifndef CS_IMPLEMENT_STATIC_CLASSVAR_EXT
#define CS_IMPLEMENT_STATIC_CLASSVAR_EXT(Class,var,getterFunc,Type,initParam,\
  kill_how)                                                    	\
Type *Class::var = 0;                                          	\
extern "C" {                                                   	\
static void Class ## _ ## getterFunc ## _kill ();              	\
static void Class ## _ ## getterFunc ## _kill_array ();        	\
void Class ## _ ## getterFunc ## _kill ()               	\
{                                                              	\
  delete Class::getterFunc ();                                 	\
  (void)(&Class ## _ ## getterFunc ## _kill_array);		\
}                                                              	\
void Class ## _ ## getterFunc ## _kill_array ()         	\
{                                                              	\
  delete [] Class::getterFunc ();                              	\
  (void)(&Class ## _ ## getterFunc ## _kill);			\
}                                                              	\
}                                                              	\
Type* Class::getterFunc ()                                     	\
{                                                              	\
  if (!var)                                                    	\
  {                                                            	\
    var = new Type initParam;                                  	\
    csStaticVarCleanup (Class ## _ ## getterFunc ## kill_how); 	\
  }                                                            	\
  return var;                                                  	\
}
#endif

#ifndef CS_IMPLEMENT_STATIC_CLASSVAR
#define CS_IMPLEMENT_STATIC_CLASSVAR(Class,var,getterFunc,Type,initParam) \
  CS_IMPLEMENT_STATIC_CLASSVAR_EXT(Class,var,getterFunc,Type,initParam,_kill)
#endif

#ifndef CS_IMPLEMENT_STATIC_CLASSVAR_ARRAY
#define CS_IMPLEMENT_STATIC_CLASSVAR_ARRAY(Class,var,getterFunc,Type,\
  initParam) \
  CS_IMPLEMENT_STATIC_CLASSVAR_EXT(Class,var,getterFunc,Type,initParam,\
    _kill_array)
#endif

#ifndef CS_IMPLEMENT_STATIC_CLASSVAR_REF_EXT
#define CS_IMPLEMENT_STATIC_CLASSVAR_REF_EXT(Class,var,getterFunc,Type,\
  initParam,kill_how) \
Type *Class::var = 0;                                          \
extern "C" {                                                   \
static void Class ## _ ## getterFunc ## _kill ();              \
static void Class ## _ ## getterFunc ## _kill_array ();        \
void Class ## _ ## getterFunc ## _kill ()                      \
{                                                              \
  (void) Class ## _ ## getterFunc ## _kill_array;              \
  delete &Class::getterFunc ();                                \
}                                                              \
void Class ## _ ## getterFunc ## _kill_array ()                \
{                                                              \
  (void) Class ## _ ## getterFunc ## _kill;                    \
  delete [] &Class::getterFunc ();                             \
}                                                              \
}                                                              \
Type &Class::getterFunc ()                                     \
{                                                              \
  if (!var)                                                    \
  {                                                            \
    var = new Type initParam;                                  \
    csStaticVarCleanup (Class ## _ ## getterFunc ## kill_how); \
  }                                                            \
  return *var;                                                 \
}
#endif

#ifndef CS_IMPLEMENT_STATIC_CLASSVAR_REF
#define CS_IMPLEMENT_STATIC_CLASSVAR_REF(Class,var,getterFunc,Type,initParam)\
  CS_IMPLEMENT_STATIC_CLASSVAR_REF_EXT(Class,var,getterFunc,Type,\
    initParam,_kill)
#endif

#ifndef CS_IMPLEMENT_STATIC_CLASSVAR_REF_ARRAY
#define CS_IMPLEMENT_STATIC_CLASSVAR_REF_ARRAY(Class,var,getterFunc,Type,\
  initParam) \
  CS_IMPLEMENT_STATIC_CLASSVAR_REF_EXT(Class,var,getterFunc,Type,initParam,\
    _kill_array)
#endif

// The following define should only be enabled if you have defined
// a special version of overloaded new that accepts two additional
// parameters: a (void*) pointing to the filename and an int with the
// line number. This is typically used for memory debugging.
// In csutil/memdebug.cpp there is a memory debugger which can (optionally)
// use this feature. Note that if CS_EXTENSIVE_MEMDEBUG is enabled while
// the memory debugger is not the memory debugger will still provide the
// needed overloaded operators so you can leave CS_EXTENSIVE_MEMDEBUG on in
// that case and the only overhead will be a little more arguments to 'new'.
// Do not enable CS_EXTENSIVE_MEMDEBUG if your platform or your own code
// defines its own 'new' operator, since this version will interfere with your
// own.
// CS_MEMORY_TRACKER is treated like CS_EXTENSIVE_MEMDEBUG here.
// Same for CS_REF_TRACKER.
#ifndef CS_DEBUG
#  undef CS_EXTENSIVE_MEMDEBUG
#  undef CS_REF_TRACKER
#else
#  if defined(CS_EXTENSIVE_MEMDEBUG) && defined(CS_MEMORY_TRACKER)
#    error Do not use CS_EXTENSIVE_MEMDEBUG and CS_MEMORY_TRACKER together!
#  endif
#endif
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
extern void* operator new (size_t s, void* filename, int line);
extern void* operator new[] (size_t s, void* filename, int line);
#define CS_EXTENSIVE_MEMDEBUG_NEW new ((void*)__FILE__, __LINE__)
#define new CS_EXTENSIVE_MEMDEBUG_NEW
#endif

#ifdef CS_DEBUG
#  if !defined (DEBUG_BREAK)
#    if defined (CS_PROCESSOR_X86)
#      if defined (CS_COMPILER_GCC)
#        define DEBUG_BREAK asm ("int $3")
#      else
#        define DEBUG_BREAK _asm int 3
#      endif
#    else
#      define DEBUG_BREAK { static int x = 0; x /= x; }
#    endif
#  endif
#  if !defined (CS_ASSERT)
#    if defined (CS_COMPILER_MSVC)
#      define CS_ASSERT(x) assert(x)
#    else
#      include <stdio.h>
#      define CS_ASSERT(x) \
         if (!(x)) \
         { \
           fprintf (stderr, __FILE__ ":%d: failed assertion '%s'\n", \
             int(__LINE__), #x); \
           DEBUG_BREAK; \
         }
#    endif
#  endif
#  if !defined (CS_ASSERT_MSG)
#      define CS_ASSERT_MSG(msg,x) CS_ASSERT(((msg) && (x)))
#  endif
#else
#  undef  DEBUG_BREAK
#  define DEBUG_BREAK
#  undef  CS_ASSERT
#  define CS_ASSERT(x)
#  undef  CS_ASSERT_MSG
#  define CS_ASSERT_MSG(m,x)
#endif

/**\def CS_DEPRECATED_METHOD
 * Use the CS_DEPRECATED_METHOD macro in front of method declarations to
 * indicate that they are deprecated. Example:
 * \code
 * struct iFoo : iBase {
 *   CS_DEPRECATED_METHOD virtual void Plankton() const = 0;
 * }
 * \endcode
 * Compilers which are capable of flagging deprecation will exhibit a warning
 * when it encounters client code invoking methods so tagged.
 */
#ifndef CS_DEPRECATED_METHOD
#  if defined(CS_COMPILER_MSVC)
#    define CS_DEPRECATED_METHOD		/*__declspec(deprecated)*/
      /* Disabled: Unfortunately, MSVC is overzealous with warnings; 
	 it even emits one when a deprecated method is overridden, e.g. when 
	 implementing an interface method. */
#  else
#    define CS_DEPRECATED_METHOD
#  endif
#endif

/**\def CS_DEPRECATED_TYPE
 * Use the CS_DEPRECATED_TYPE macro after type declarations to
 * indicate that they are deprecated. Example:
 * \code
 * typedef csFoo csBar CS_DEPRECATED_TYPE;
 * \endcode
 * Compilers which are capable of flagging deprecation will exhibit a warning
 * when it encounters client code using types so tagged.
 */
#ifndef CS_DEPRECATED_TYPE
#  if defined(CS_COMPILER_MSVC)
#    define CS_DEPRECATED_TYPE
#  else
#    define CS_DEPRECATED_TYPE
#  endif
#endif

// Check if the csosdefs.h defined either CS_LITTLE_ENDIAN or CS_BIG_ENDIAN
#if !defined (CS_LITTLE_ENDIAN) && !defined (CS_BIG_ENDIAN)
#  error No CS_XXX_ENDIAN macro defined in your OS-specific csosdefs.h!
#endif

/*
 * This is a bit of overkill but if you're sure your CPU doesn't require
 * strict alignment add your CPU to the !defined below to get slightly
 * smaller and faster code in some cases.
 *
 * @@@ In the future, this should be moved to csconfig.h and determined as
 * part of the configuration process.
 */
#if !defined (CS_PROCESSOR_X86)
#  define CS_STRICT_ALIGNMENT
#endif

// Adjust some definitions contained in csconfig.h
#if !defined (CS_PROCESSOR_X86) || !defined (CS_HAVE_NASM)
#  undef CS_HAVE_MMX
#  undef CS_HAVE_NASM
#endif

// Use special knowledge of IEEE float format in some cases for CPU's that are
// known to support it
#if !defined (CS_IEEE_DOUBLE_FORMAT)
#  if defined (CS_PROCESSOR_X86) || \
      defined (CS_PROCESSOR_POWERPC) || \
      defined (CS_PROCESSOR_M68K)
#    define CS_IEEE_DOUBLE_FORMAT
#  endif
#endif

// gcc can perform usefull checking for printf/scanf format strings, just add
// this define at the end of the function declaration
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#  define CS_GNUC_PRINTF(format_idx, arg_idx) \
     __attribute__((format (__printf__, format_idx, arg_idx)))
#  define CS_GNUC_SCANF(format_idx, arg_idx) \
     __attribute__((format (__scanf__, format_idx, arg_idx)))
#else
#  define CS_GNUC_PRINTF(format_idx, arg_idx)
#  define CS_GNUC_SCANF(format_idx, arg_idx)
#endif

// Remove __attribute__ on non GNUC compilers.
#ifndef __GNUC__
#define __attribute__(x)
#endif

// Support for alignment and packing of structures.
#if !defined(CS_STRUCT_ALIGN_4BYTE_BEGIN)
#  if defined(__GNUC__) && defined(CS_STRICT_ALIGNMENT)
#    define CS_STRUCT_ALIGN_4BYTE_BEGIN
#    define CS_STRUCT_ALIGN_4BYTE_END __attribute__ ((aligned(4)))
#  else
#    define CS_STRUCT_ALIGN_4BYTE_BEGIN
#    define CS_STRUCT_ALIGN_4BYTE_END
#  endif
#endif

// Macro used to define static implicit pointer conversion function.
// Only use within a class declaration.
#ifndef _CS_IMPLICITPTRCAST_NAME
#  define _CS_IMPLICITPTRCAST_NAME __ImplicitPtrCast
#endif
/**
 * Implements a static member function for a class which can be used to
 * perform implicit pointer casts.
 * \param classname Name of the class that the macro is being used in.
 * \remarks
 * This macro is intended to support typecasting within macros, allowing the
 * compiler to provide a more descriptive error message. Use
 * CS_IMPLEMENT_IMPLICIT_PTR_CAST() in the declaration of the class and
 * CS_IMPLICIT_PTR_CAST() in the macro declaration.
 * \par Example:
 * \code
 * struct iObjectRegistry : public iBase
 * {
 *   // Allow implicit casts through static function.
 *   CS_IMPLEMENT_IMPLICIT_PTR_CAST(iObjectRegistry);
 *   ...
 * }
 *
 * #define CS_QUERY_REGISTRY_TAG(Reg, Tag) \
 *  csPtr<iBase> (CS_IMPLICIT_PTR_CAST(iObjectRegistry, Reg)->Get (Tag))
 * \endcode
 */
#define CS_IMPLEMENT_IMPLICIT_PTR_CAST(classname) \
  inline static classname* _CS_IMPLICITPTRCAST_NAME (classname* ptr) \
  { \
    return ptr;\
  }

/**
 * Perform a compiler implicit cast of a pointer to another pointer type
 * using a static member function declared with the
 * \c CS_IMPLEMENT_IMPLICIT_PTR_CAST macro.
 * \param classname Name of the class to convert to
 * \param ptr Pointer to be convereted into 
 * \see CS_IMPLEMENT_IMPLICIT_PTR_CAST
 */
#define CS_IMPLICIT_PTR_CAST(classname, ptr) \
  (classname::_CS_IMPLICITPTRCAST_NAME(ptr))

/**\def CS_VA_COPY(dest, src)
 * Copies the state of a va_list value.
 */
#ifdef CS_HAVE_VA_COPY
#  define CS_VA_COPY(dest, src)		va_copy(dest, src)
#else
#  ifdef CS_HAVE___VA_COPY
#    define CS_VA_COPY(dest, src)	__va_copy(dest, src)
#  else
#    define CS_VA_COPY(dest, src)	dest = src;
#  endif
#endif

/**\def CS_FUNCTION_NAME
 * Macro that resolves to a compiler-specific variable or string that contains 
 * the name of the current function.
 */
#if defined(CS_COMPILER_GCC)
#  define CS_FUNCTION_NAME		__PRETTY_FUNCTION__
#elif defined(__FUNCTION__)
#  define CS_FUNCTION_NAME		__FUNCTION__
#else
#  define CS_FUNCTION_NAME		"<?\?\?>"
#endif

#endif // __CS_CSSYSDEF_H__
