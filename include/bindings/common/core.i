/*
    Copyright (C) 2003 Rene Jager <renej_frog@users.sourceforge.net>

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

/*
  SWIG interface for Crystal Space

  This file includes scripting language specific SWIG files.  Search for
  'pythpre' (pre-include phase) and 'pythpost' (post-include phase) for places
  where to include files for new scripting languages.

  Python (renej_frog@users.sourceforge.net):
    Used for the new "cspython" plugin based on python 2.2 including almost
    complete access to CS.

    Additionally, provides CS as a python 2.2 module name "cspace".  To use
    this, make sure cspace.py and _cspace.so in directory
    $CRYSTAL/scripts/python can be found by python; and don't forget to set the
    CRYSTAL environment variable.  See $CRYSTAL/scripts/python/pysimpcd.py for
    an example of usage.

  Perl (oktal@gmx.co.uk):
    Used for the "csperl5" plugin based on perl 5.8 including access to most
    parts of CS (although some ealier versions are probably supported).

    Additionally, provices CS as a perl 5.8 module name "cspace".  Make sure
    the directory containing cspace.pm and cspace.so, $CRYSTAL/scripts/perl5,
    is in perl's @INC array.

  Java (renej_frog@users.sourceforge.net):
    A wrapper for using CS from Java is provided: crystalspace.jar and
    libcsjava.so.

  Note:
    Tested with swig 1.3.17 and up, swig 1.1 won't work!

  Thanks to:
    Norman Kramer <norman@users.sourceforge.net> who made me think
      about sequence of declarations in SWIG file.
    Mark Gossage <mark@gossage.cjb.net> who made me think about
      preventing handling of smart pointers by SWIG to reduce
      size of generated code.
    Mat Sutcliffe <oktal@gmx.co.uk> for input, Perl includes,
      and wrapping SCF and other macros.
    Eric Sunshine <sunshine@sunshineco.com> for all the hours spent tracking
      down and fixing bugs.
*/

/*
  SWIG's preprocessor can be used to "replace" a macro by a function in the
  target scripting language using the following:

  In C header file:

    #define X(arg) ...

  which as a function would have the prototype:

    int X (int i);

  Now in the SWIG *.i file do:

    #define _X(a) X(a)
    #undef X
    int _X (int i);

  and there will be a "X" function in the scripting language available.
*/

//#define USE_DIRECTORS
/*
#ifdef USE_DIRECTORS
%module(directors="1") cspace
#else
%module cspace
#endif
*/

%include "csconfig.h"
%include "cstypes.h"

// Ignored macros.
#undef CS_STRUCT_ALIGN_4BYTE_BEGIN
#define CS_STRUCT_ALIGN_4BYTE_BEGIN
#undef CS_STRUCT_ALIGN_4BYTE_END
#define CS_STRUCT_ALIGN_4BYTE_END
#undef CS_GNUC_PRINTF
#define CS_GNUC_PRINTF(format_idx, arg_idx)
#undef CS_GNUC_SCANF
#define CS_GNUC_SCANF(format_idx, arg_idx)
#undef CS_GNUC_WPRINTF
#define CS_GNUC_WPRINTF(format_idx, arg_idx)
#undef CS_GNUC_WSCANF
#define CS_GNUC_WSCANF(format_idx, arg_idx)
#undef CS_DECLARE_STATIC_CLASSVAR
#define CS_DECLARE_STATIC_CLASSVAR(a, b, c)
#undef CS_FORCEINLINE
#define CS_FORCEINLINE
#undef CS_FORCEINLINE_TEMPLATEMETHOD
#define CS_FORCEINLINE_TEMPLATEMETHOD
#undef CS_EXPORT_SYM
#define CS_EXPORT_SYM
#undef CS_IMPORT_SYM
#define CS_IMPORT_SYM
#undef CS_CRYSTALSPACE_EXPORT
#define CS_CRYSTALSPACE_EXPORT
#undef CS_LEAKGUARD_DECLARE
#define CS_LEAKGUARD_DECLARE(m)
#undef CS_DEPRECATED_METHOD
#define CS_DEPRECATED_METHOD
#undef CS_DEPRECATED_METHOD_MSG
#define CS_DEPRECATED_METHOD_MSG(msg)
#undef CS_DEPRECATED_VAR
#define CS_DEPRECATED_VAR(decl) decl
#undef CS_DEPRECATED_VAR_MSG
#define CS_DEPRECATED_VAR_MSG(msg,decl) decl
#undef CS_DEPRECATED_TYPE
#define CS_DEPRECATED_TYPE
#undef CS_DEPRECATED_TYPE_MSG
#define CS_DEPRECATED_TYPE_MSG(msg)
#undef CS_PURE_METHOD
#define CS_PURE_METHOD
#undef CS_CONST_METHOD
#define CS_CONST_METHOD
#undef CS_COMPILE_ASSERT
#define CS_COMPILE_ASSERT(x)
#undef CS_VISIBILITY_DEFAULT
#define CS_VISIBILITY_DEFAULT

// Compatibility macros.
#define typename_qualifier typename

/* For debugging: If you need to debug the build commands for the scripting
 * modules, or if you need to debug or test certain small portions of the
 * entire scripting support but despair of having to wait for the lengthy
 * compilations to complete (for instance, 30 minutes to build csperl5 on
 * Sunshine's computer), then you can define either of these macros.  Define
 * CS_MINI_SWIG to avoid publishing most of the CS interfaces except for
 * csInitializer, object registry, VFS, SCF, event queue & handlers, 2D & 3D
 * drivers, and font & font server.  This should cut compilation time down
 * considerably (to 6 minutes, for example, for csperl).  If this is still too
 * long, however, then define CS_MICRO_SWIG, which will publish only
 * csInitializer, object registry, SCF, and VFS.  This is about the bare
 * minimum of exports which are still useful for testing various bits of
 * functionality, and should reduce compilation time as much as possible (under
 * 2 minutes for csperl, for instance).  CS_MICRO_SWIG implies CS_MINI_SWIG.
 */
#undef CS_MINI_SWIG
#undef CS_MICRO_SWIG

#ifdef CS_MICRO_SWIG
#define CS_MINI_SWIG
#endif

#ifdef SWIGPERL5
%include "bindings/perl/perl1st.i"
#endif

%{
#include "csgeom.h"
#include "cstool/initapp.h"
#include "csutil.h"
%}

%include "bindings/common/allinterfaces.i"

/* All this functions will are redefined later to always return
   script objects with the proper interfaces. 
   script definitions for all these is at common/scfsugar.i */
/*%ignore iObject::GetChild (const char *Name);*/

%rename (GetChildByName) iObject::GetChild(const char *Name) const;
%ignore iObject::GetChild (int iInterfaceID, int iVersion,const char *Name = 0) const;
%ignore iObject::GetChild (int iInterfaceID, int iVersion,const char *Name, bool FirstName) const;
%rename (LoadPluginAlways) LoadPlugin (const char *classID,bool init = true, bool report = true);
%ignore iPluginManager::QueryPlugin (const char *iInterface, int iVersion);
%ignore iPluginManager::QueryPlugin (const char* classID,
        const char *iInterface, int iVersion);
%ignore iObjectRegistry::Get (char const* tag);
%ignore iObjectRegistry::Get (char const* tag, scfInterfaceID id, int version);
%ignore iObjectRegistry::Get (scfInterfaceID id, int version);
%ignore iBase::QueryInterface(scfInterfaceID iInterfaceID, int iVersion);
%ignore iBase::~iBase(); // We replace iBase dtor with one that calls DecRef().
                         // Swig already knows not to delete an SCF pointer.
%include "typemaps.i"

// The following list all kown arguments that are actually (also) outputs.
// It is possible that some of the output arguments should be INOUT instead
// of OUTPUT.

// output arguments
%apply unsigned long { utf32_char };
%apply unsigned long { uint32 };
%apply unsigned long { uint };
%apply unsigned char * OUTPUT { uint8 & red };
%apply unsigned char * OUTPUT { uint8 & green };
%apply unsigned char * OUTPUT { uint8 & blue };
%apply unsigned char * OUTPUT { uint8 & oR };
%apply unsigned char * OUTPUT { uint8 & oG };
%apply unsigned char * OUTPUT { uint8 & oB };
%apply int * OUTPUT { int & red };
%apply int * OUTPUT { int & green };
%apply int * OUTPUT { int & blue };
%apply int * OUTPUT { int & r };
%apply int * OUTPUT { int & g };
%apply int * OUTPUT { int & b };
%apply int * OUTPUT { int & mw };
%apply int * OUTPUT { int & mh };
%apply int * OUTPUT { int & oW };
%apply int * OUTPUT { int & oH };
%apply int * OUTPUT { int & oR };
%apply int * OUTPUT { int & oG };
%apply int * OUTPUT { int & oB };
%apply int * OUTPUT { int & w };
%apply int * OUTPUT { int & h };
%apply int * OUTPUT { int & bw };
%apply int * OUTPUT { int & bh };
%apply int * OUTPUT { int & ClientW };
%apply int * OUTPUT { int & ClientH };
%apply int * OUTPUT { int & totw };
%apply int * OUTPUT { int & toth };
%apply int * OUTPUT { int & sugw };
%apply int * OUTPUT { int & sugh };
%apply int * OUTPUT { int & adv };
%apply int * OUTPUT { int & left };
%apply int * OUTPUT { int & top };
%apply int * OUTPUT { int & width };
%apply int * OUTPUT { int & height };
%apply int * OUTPUT { int & desc };
%apply int * OUTPUT { int & nr };
%apply int * OUTPUT { int & key };
%apply int * OUTPUT { int & shift };
%apply int * OUTPUT { int & button };
%apply int * OUTPUT { int & ver };
%apply int * OUTPUT { int & scfInterfaceID };
%apply int * OUTPUT { int & a };
%apply int * OUTPUT { int & x };
%apply int * OUTPUT { int & y };
%apply int * OUTPUT { int & oFontSize };
%apply int * OUTPUT { int & oUnderlinePos };
%apply int * OUTPUT { int & newX };
%apply int * OUTPUT { int & newY };
%apply int * OUTPUT { int & newH };
%apply int * OUTPUT { int & newW };
%apply int * OUTPUT { int & xmin };
%apply int * OUTPUT { int & ymin };
%apply int * OUTPUT { int & xmax };
%apply int * OUTPUT { int & ymax };
%apply int * OUTPUT { int & theRow };
%apply int * OUTPUT { int & theCol };
%apply int * OUTPUT { int & row };
%apply int * OUTPUT { int & col };
%apply int * OUTPUT { int & newHeight };
%apply int * OUTPUT { int & newWidth };
%apply int * OUTPUT { int & oColor };
%apply int * OUTPUT { int & val };
%apply int * OUTPUT { int * polygon_idx };
%apply float * OUTPUT { float & oDiffuse };
%apply float * OUTPUT { float & oAmbient };
%apply float * OUTPUT { float & oReflection };
%apply float * OUTPUT { float & red };
%apply float * OUTPUT { float & green };
%apply float * OUTPUT { float & blue };
%apply float * OUTPUT { float & a };
%apply float * OUTPUT { float & b };
%apply float * OUTPUT { float & x1 };
%apply float * OUTPUT { float & y1 };
%apply float * OUTPUT { float & x2 };
%apply float * OUTPUT { float & y2 };
%apply float * OUTPUT { float & oH };
%apply float * OUTPUT { float & oS };
%apply float * OUTPUT { float & oR };
%apply float * OUTPUT { float & oG };
%apply float * OUTPUT { float & oB };
%apply float * OUTPUT { float & r };
%apply float * OUTPUT { float & g };
%apply float * OUTPUT { float & b };
%apply float * OUTPUT { float & l };
%apply float * OUTPUT { float & h };
%apply float * OUTPUT { float & s };
%apply float * OUTPUT { float & start };
%apply float * OUTPUT { float & dist };
%apply float * OUTPUT { float & w };
%apply float * OUTPUT { float & ra };
%apply float * OUTPUT { float & rb };
%apply float * OUTPUT { float * pr }; // iMeshObject::HitBeam*().
%apply float * OUTPUT { float & max };
%apply float * OUTPUT { float & min };

// input/output arguments
%apply bool * INOUT { bool & mirror };
%apply int * INOUT { int & maxcolors };
%apply float * INOUT { float & iR };
%apply float * INOUT { float & iG };
%apply float * INOUT { float & iB };


%define CS_WRAP_PTR_IMPLEMENT(PtrName)
%immutable PtrName::Type;
%inline %{

  // This pointer wrapper can be used to prevent code-bloat by macros
  // acting as template functions.  Examples are SCF_QUERY_INTERFACE()
  // and CS_QUERY_REGISTRY().  Also note that CS should never need to
  // use virtual inheritance as long as it has SCF.
  //
  // Ref - A managed reference to the iBase pointer of the wrapped
  //    interface.
  // Type
  //    The SCF interface name which this pointer represents (for
  //    instance, "iEngine").
  // Version - The version of the interface this pointer represents.

  struct PtrName
  {
    csRef<iBase> Ref;
    const char *Type;
    scfInterfaceVersion Version;
    PtrName (const char *t, scfInterfaceVersion v, csPtr<iBase> r)
    : Ref (r), Type (t), Version(v) {}
    PtrName (const char *t, scfInterfaceVersion v, csRef<iBase> r)
    : Ref (r), Type (t), Version(v) {}
    PtrName (const PtrName &p)
    : Ref (p.Ref), Type (p.Type), Version(p.Version) {}
  };

%}
%enddef
CS_WRAP_PTR_IMPLEMENT(csWrapPtr)
// Macro's expected in rest of this file: ignored by default.
// When overriding in language-specific files, first #undef and then
// re-%define them.
#define LANG_FUNCTIONS
#define TYPEMAP_OUT_csRef(T)
#define TYPEMAP_OUT_csPtr(T)
#define TYPEMAP_OUT_csRefArray(T)
#define TYPEMAP_OUT_csWrapPtr
#define TYPEMAP_IN_ARRAY_CNT_PTR(a,b)
#define TYPEMAP_IN_ARRAY_PTR_CNT(a,b) 
#define TYPEMAP_OUTARG_ARRAY_PTR_CNT(a,b,c)
#define TYPEMAP_ARGOUT_PTR(T)
#define APPLY_TYPEMAP_ARGOUT_PTR(T,Args)
#define BUFFER_RW_FUNCTIONS(classname,datafunc,countfunc,ElmtType,BufGetter)
#define ITERATOR_FUNCTIONS(T)
#define ARRAY_OBJECT_FUNCTIONS(classname,typename)
#define LIST_OBJECT_FUNCTIONS(classname,typename)
#define SET_OBJECT_FUNCTIONS(classname,typename)
#define DEPRECATED_METHOD(classname,method,replacement)
#define TYPEMAP_STRING(Type)
#define TYPEMAP_STRING_PTR(Type,Type2)
#define LANG_POST(T)

#if defined(SWIGPYTHON)
  %include "bindings/python/pythpre.i"
#elif defined(SWIGPERL5)
  %include "bindings/perl/perlpre.i"
#elif defined(SWIGRUBY)
  %include "bindings/ruby/rubypre.i"
#elif defined(SWIGTCL8)
  %include "bindings/tcl/tclpre.i"
#elif defined(SWIGJAVA)
  %include "bindings/java/javapre.i"
#elif defined(SWIGLUA)
  %include "bindings/lua/luapre.i"
#endif

%define INTERFACE_POST(T)
  %extend T
  {
    static int scfGetVersion() { return scfInterfaceTraits<T>::GetVersion(); }
    virtual ~T() { if (self) self->DecRef (); }
  }
  LANG_POST(T)
%enddef

%define INLINE_FUNCTIONS
%inline %{
/* Funtions to set the modules global SCF pointer, this is needed
   when working on a pure scripting environment, as then this code
   lives in a non-cs dll, thus the pointer isnt initialized
   by cs itself, and scf stuff wont work unless the pointer is
   initialized manually. Use it after CreateEnvironment call. */
void SetSCFPointer(iSCF* pscf)
{
  iSCF::SCF = pscf;
}

iSCF* GetSCFPointer()
{
  return iSCF::SCF;
}
%}
LANG_FUNCTIONS
%enddef

// Handle arrays as input arguments.
// The second parameter is for putting in an extra dereference operator if the
// array is an array of objects. If it is an array of _pointers to_ objects,
// then the second parameter should be empty. This is the inverse of the
// third parameter in the TYPEMAP_OUTARG_ARRAY_... macros below.
TYPEMAP_IN_ARRAY_CNT_PTR((int num_layers, iTextureWrapper ** wrappers), /**/)
TYPEMAP_IN_ARRAY_PTR_CNT((csVector2 * InPolygon, int InCount), *)
TYPEMAP_IN_ARRAY_PTR_CNT((csVector3* vertices, int num_vertices), *)
TYPEMAP_IN_ARRAY_PTR_CNT((csVector3* vertices, int num), *)

// Handle arrays as output arguments.
// The third parameter is for putting in an extra dereference operator if the
// array is an array of _pointers to_ objects. If it is just array of objects,
// then the third parameter should be empty. This is the inverse of the
// second parameter in the TYPEMAP_IN_ARRAY_... macros above.
TYPEMAP_OUTARG_ARRAY_PTR_CNT(
  (csVector2 * OutPolygon, int & OutCount),
  new csVector2[MAX_OUTPUT_VERTICES], /**/
)

%ignore csPtr::csPtr;
%ignore csRef::csRef;
%ignore csRef::~csRef;
%ignore csRef::operator =;
%ignore csRef::operator *;

%include "csutil/ref.h"

%define INTERFACE_PRE(T)
  %nodefault T;
  TYPEMAP_OUT_csRef(T)
  TYPEMAP_OUT_csPtr(T)
  TYPEMAP_OUT_csRefArray(T)
%enddef

#undef INTERFACE_APPLY
#define INTERFACE_APPLY(x) INTERFACE_PRE(x)
APPLY_FOR_ALL_INTERFACES

TYPEMAP_OUT_csWrapPtr

// Inclusion of CS headers.  The sequence of %include-ing the CS headers can be
// crucial!  The scheme is as follows: %ignore'd functions and types are placed
// before actual inclusion, as are "local" %typemap's (like default).  After
// %include-ing the header resets can be done; for example resetting the
// %typemap(default). The %extend-ing and extra code takes place after all
// %include's are done, mentioning the header(s) it is related to.

%ignore CS::Memory::CustomAllocated::operator new;
%ignore CS::Memory::CustomAllocated::operator new[];
%ignore CS::Memory::CustomAllocated::operator delete;
%ignore CS::Memory::CustomAllocated::operator delete[];
%ignore CS::Memory::CustomAllocatedDerived::operator new;
%ignore CS::Memory::CustomAllocatedDerived::operator new[];
%ignore CS::Memory::CustomAllocatedDerived::operator delete;
%ignore CS::Memory::CustomAllocatedDerived::operator delete[];
%ignore CS::Memory::CustomAllocatedDerivedVirtual::operator new;
%ignore CS::Memory::CustomAllocatedDerivedVirtual::operator new[];
%ignore CS::Memory::CustomAllocatedDerivedVirtual::operator delete;
%ignore CS::Memory::CustomAllocatedDerivedVirtual::operator delete[];
%include "csutil/customallocated.h"

%include "iutil/array.h"
%ignore csOrdering;
%ignore csArrayCmp;
%ignore csArrayElementHandler;
%ignore csArrayMemoryAllocator;
%ignore csSafeCopyArrayMemoryAllocator;
%ignore csSafeCopyArray;
%ignore csArray::Capacity;
%ignore csArray::DefaultCompare;
%ignore csArray::Delete;
%ignore csArray::DeleteFast;
%ignore csArray::Find;
%ignore csArray::FindKey;
%ignore csArray::FindSortedKey;
%ignore csArray::GetExtend;
%ignore csArray::GetIndex;
%ignore csArray::GetIterator;
%ignore csArray::GetReverseIterator;
%ignore csArray::InitRegion;
%ignore csArray::InsertSorted;
%ignore csArray::Iterator;
%ignore csArray::ReverseIterator;
%ignore csArray::Length;
%ignore csArray::PushSmart;
%ignore csArray::Section;
%ignore csArray::SetCapacity;
%ignore csArray::SetLength;
%ignore csArray::SetSize;
%ignore csArray::ShrinkBestFit;
%ignore csArray::Sort;
%ignore csArray::TransferTo;
%ignore csArray::operator=;
%ignore csArray::operator[];
/* The following is a bit ugly but otherwise there is no way pass the
   necessary directives to swig between template declarations.        */
template <typename Threshold = csArrayThresholdVariable>
class csArrayCapacityLinear : public Threshold {
	public:
	csArrayCapacityLinear ();
	csArrayCapacityLinear (const Threshold& threshold);
	csArrayCapacityLinear (const size_t x);
	bool IsCapacityExcessive (size_t capacity, size_t count) const;
	size_t GetCapacity (size_t count) const;
};
struct csArrayThresholdVariable {
	public:
	csArrayThresholdVariable (size_t in_threshold = 0);
	size_t GetThreshold() const;
};
%template(csArrayThresholdVariableCapacityLinear) 
csArrayCapacityLinear<csArrayThresholdVariable >;
%ignore csArrayCapacityLinear;
%ignore csArrayThresholdVariable;
%include "csutil/array.h"
%define ARRAY_CHANGE_ALL_TEMPLATE(typename)
%template (typename ## ArrayReadOnly) iArrayReadOnly<typename >;
%template (typename ## ArrayChangeElements) iArrayChangeElements<typename >;
%template (typename ## ArrayChangeAll) iArrayChangeAll<typename >;
%enddef
%define ARRAY_CHANGE_ALL_TEMPLATE_PTR(typename)
%template (typename ## ArrayReadOnly) iArrayReadOnly<typename* >;
%template (typename ## ArrayChangeElements) iArrayChangeElements<typename* >;
%template (typename ## ArrayChangeAll) iArrayChangeAll<typename* >;
%enddef
#%ignore csDirtyAccessArray::Length;
%include "csutil/dirtyaccessarray.h"
// Provide some useful default instantiations
%template (Vector2Array) csArray<csVector2>;
%template (Vector2DirtyAccessArray) csDirtyAccessArray<csVector2>;
%template (Vector3Array) csArray<csVector3>;
%template (Vector3DirtyAccessArray) csDirtyAccessArray<csVector3>;
%template (Vector4Array) csArray<csVector4>;
%template (Vector4DirtyAccessArray) csDirtyAccessArray<csVector4>;
%template (UIntArray) csArray<unsigned int>;
%template (UIntDirtyAccessArray) csDirtyAccessArray<unsigned int>;

%ignore scfInitialize;
%immutable iSCF::SCF;
%inline %{
void SetCoreSCFPointer(iSCF *scf_pointer)
{
  iSCF::SCF = scf_pointer;
}
%}
%immutable scfInterfaceMetadata::interfaceName;
%include "csutil/scf_interface.h"
%include "csutil/scf.h"

// hand made scf template wrappers
%include "bindings/common/scf.i"

// Needed to resolve THREADED_INTERFACE macros
%include "iutil/threadmanager.h"
%include "iutil/dbghelp.h"
%include "iutil/cmdline.h"

%ignore operator* (const csColor &, float);
%ignore operator* (float, const csColor &);
%ignore operator* (const csColor &, const csColor &);
%ignore operator/ (const csColor &, float);
%ignore operator+ (const csColor &, const csColor &);
%ignore operator- (const csColor &, const csColor &);
%include "csutil/cscolor.h"

%include "csutil/cmdhelp.h"
%include "csutil/comparator.h"

%include "csutil/flags.h"

%ignore CS::Utility::StringSet::GlobalIterator;
%ignore CS::Utility::StringSet::GetIterator;
%ignore CS::Utility::GetIterator;
DEPRECATED_METHOD(iStringArray,Length,GetSize);
DEPRECATED_METHOD(iStringArray,DeleteAll,Empty);
/* %apply unsigned long { csStringID }; */
%include "iutil/strset.h"
%include "csutil/strset.h"
DEPRECATED_METHOD(CS::Utility::StringSet,Clear,Empty);
%ignore csSet::GlobalIterator;
%ignore csSet::GetIterator;
%include "csutil/set.h"
/* helper macro to declare csSet templated classes */
%define SET_HELPER(typename)
%template (typename ## Set) csSet<typename>;
SET_OBJECT_FUNCTIONS(csSet<typename>,typename)
%enddef
SET_HELPER(csStringID)

%include "bindings/common/csstring.i"

%include "csutil/refcount.h"

%ignore csGetPlatformConfig;
%ignore csPrintfV;
%ignore csFPrintfV;
%ignore csPrintfErrV;
%include "csutil/sysfunc.h"

%ignore csInitializer::RequestPlugins(iObjectRegistry*, ...);
%ignore csInitializer::RequestPluginsV;
%rename (_RequestPlugins) csInitializer::RequestPlugins(iObjectRegistry*,
  csArray<csPluginRequest> const&);
%rename (_CreateEnvironment) csInitializer::CreateEnvironment(int,char const *const []);
%rename (_CreateEnvironment2) csInitializer::CreateEnvironment(int,char const *const [],bool);

%ignore csInitializer::SetupEventHandler (iObjectRegistry*, csEventHandlerFunc,
  const csEventID events[]);
%ignore csInitializer::SetupEventHandler (iObjectRegistry*, csEventHandlerFunc);
%rename (_SetupEventHandler) csInitializer::SetupEventHandler (iObjectRegistry*,
  iEventHandler *, const csEventID[]);
%typemap(default) const char * configName { $1 = 0; }
// Swig 1.3.24 doesn't handle pointer default args well unless we tell it
// to use an alternate way for that function
%feature("compactdefaultargs") csInitializer::SetupConfigManager;
%include "cstool/initapp.h"
%typemap(default) const char * configName;

%ignore csArray<csPluginRequest>::Capacity;
%ignore csArray<csPluginRequest>::DefaultCompare;
%ignore csArray<csPluginRequest>::Delete;
%ignore csArray<csPluginRequest>::DeleteAll;
%ignore csArray<csPluginRequest>::DeleteFast;
%ignore csArray<csPluginRequest>::Find;
%ignore csArray<csPluginRequest>::FindKey;
%ignore csArray<csPluginRequest>::FindSortedKey;
%ignore csArray<csPluginRequest>::Get(size_t); // Non-const.
%ignore csArray<csPluginRequest>::GetExtend;
%ignore csArray<csPluginRequest>::GetIndex;
%ignore csArray<csPluginRequest>::GetIterator;
%ignore csArray<csPluginRequest>::GetReverseIterator;
%ignore csArray<csPluginRequest>::InitRegion;
%ignore csArray<csPluginRequest>::InsertSorted;
%ignore csArray<csPluginRequest>::Iterator;
%ignore csArray<csPluginRequest>::ReverseIterator;
%ignore csArray<csPluginRequest>::PushSmart;
%ignore csArray<csPluginRequest>::Put;
%ignore csArray<csPluginRequest>::Section;
%ignore csArray<csPluginRequest>::SetCapacity;
%ignore csArray<csPluginRequest>::SetLength;
%ignore csArray<csPluginRequest>::SetSize;
%ignore csArray<csPluginRequest>::ShrinkBestFit;
%ignore csArray<csPluginRequest>::Sort;
%ignore csArray<csPluginRequest>::Top();       // Non-const.
%ignore csArray<csPluginRequest>::TransferTo;
%ignore csArray<csPluginRequest>::operator=;
%ignore csArray<csPluginRequest>::operator[];
%template(csPluginRequestArray) csArray<csPluginRequest>;

%ignore iReporter::ReportV;
%ignore csReporterHelper::ReportV;
%include "ivaria/reporter.h"

%include "iutil/comp.h"
%include "iutil/cache.h"
%include "iutil/vfs.h"
%include "iutil/dbghelp.h"
%include "iutil/object.h"
%extend iObjectIterator
{
  ITERATOR_FUNCTIONS(iObjectIterator)
}
%ignore CS_QUERY_REGISTRY_TAG_is_deprecated;
%include "iutil/objreg.h"
%include "iutil/virtclk.h"

%rename(AddInt8) iEvent::Add(const char *, int8);
%rename(AddInt16) iEvent::Add(const char *, int16);
%rename(AddInt32) iEvent::Add(const char *, int32);
%rename(AddUInt8) iEvent::Add(const char *, uint8);
%rename(AddUInt16) iEvent::Add(const char *, uint16);
%rename(AddUInt32) iEvent::Add(const char *, uint32);
%rename(AddFloat) iEvent::Add(const char *, float);
%rename(AddDouble) iEvent::Add(const char *, double);
%rename(AddString) iEvent::Add(const char *, char *);
%rename(AddBool) iEvent::Add(const char *, bool);
%rename(AddVoidPtr) iEvent::Add(const char *, void *, size_t);
%rename(RetrieveInt8) iEvent::Retrieve(const char *, int8 &) const;
%rename(RetrieveInt16) iEvent::Retrieve(const char *, int16 &) const;
%rename(RetrieveInt32) iEvent::Retrieve(const char *, int32 &) const;
%rename(RetrieveUInt8) iEvent::Retrieve(const char *, uint8 &) const;
%rename(RetrieveUInt16) iEvent::Retrieve(const char *, uint16 &) const;
%rename(RetrieveUInt32) iEvent::Retrieve(const char *, uint32 &) const;
%rename(RetrieveFloat) iEvent::Retrieve(const char *, float &) const;
%rename(RetrieveDouble) iEvent::Retrieve(const char *, double &) const;
// workaround RetrieveString as following line gives problems with swig 1.3.28
//%rename(RetrieveString) iEvent::Retrieve(const char *, const char *&) const;
%ignore iEvent::Retrieve(const char *, const char *&) const;
%rename(RetrieveBool) iEvent::Retrieve(const char *, bool &) const;
%rename(RetrieveVoidPtr) iEvent::Retrieve(const char*, void**, size_t&) const;
#pragma SWIG nowarn=312; // nested union not supported

%ignore csJoystickEventHelper::GetX;
%ignore csJoystickEventHelper::GetY;
%include "iutil/event.h"

%ignore csMouseEventHelper::GetModifiers(iEvent const *);
%ignore csJoystickEventHelper::GetModifiers(iEvent const *);

TYPEMAP_ARGOUT_PTR(csKeyModifiers)
APPLY_TYPEMAP_ARGOUT_PTR(csKeyModifiers,csKeyModifiers& modifiers)

/* this one is for preventing swig from generating int32_t* for int32*
   (trips compilation on msvc) */
typedef int int32_t;

TYPEMAP_ARGOUT_PTR(csKeyEventData)
APPLY_TYPEMAP_ARGOUT_PTR(csKeyEventData,csKeyEventData& data)
TYPEMAP_ARGOUT_PTR(csMouseEventData)
APPLY_TYPEMAP_ARGOUT_PTR(csMouseEventData,csMouseEventData& data)
TYPEMAP_ARGOUT_PTR(csJoystickEventData)
APPLY_TYPEMAP_ARGOUT_PTR(csJoystickEventData,csJoystickEventData& data)
TYPEMAP_ARGOUT_PTR(csCommandEventData)
APPLY_TYPEMAP_ARGOUT_PTR(csCommandEventData,csCommandEventData& data)

%include "csutil/event.h"
%extend iEvent {
	csEventError RetrieveString(const char *name, char *&v)
	{
		return self->Retrieve(name,(const char *&)v);
	}
}

%include "iutil/evdefs.h"
%include "iutil/eventq.h"
%ignore csStrKey::operator const char*;
%ignore csHash::ConstGlobalIterator;
%ignore csHash::GlobalIterator;
%ignore csHash::Iterator;
%ignore csHash::PutFirst;
%ignore csHash::GetIterator;
%ignore csHash::DeleteElement;
%ignore csHash::Get (const K& key, T& fallback);
%ignore csHash::csHash (const csHash<T> &o);
%ignore csHash::operator [];

/* We have to declare this here, otherwise swig gets confused about a
forward declaration of the csHash template without default parameters */
template <class T, class K = unsigned int,
  class ArrayMemoryAlloc = CS::Memory::AllocatorMalloc,
  class ArrayElementHandler = csArrayElementHandler<
    CS::Container::HashElement<T, K> > > class csHash;

%include "csutil/hash.h"
%include "iutil/eventnames.h"
%include "csutil/eventnames.h"
%include "iutil/eventh.h"
%include "iutil/plugin.h"
%extend iPluginIterator
{
  ITERATOR_FUNCTIONS(iPluginIterator)
}

%template(scfObject) scfImplementation1<csObject,iObject >;
%include "csutil/csobject.h"

%include "igeom/clip2d.h"
%include "igeom/path.h"

%template(scfPath) scfImplementationExt1<csPath,csObject,iPath >;
#ifndef CS_SWIG_PUBLISH_IGENERAL_FACTORY_STATE_ARRAYS
%ignore iTriangleMesh::GetTriangles;
%ignore iTriangleMesh::GetVertices;
#endif
%include "igeom/trimesh.h"


%include "iutil/csinput.h"
%include "iutil/cfgfile.h"
%include "iutil/cfgmgr.h"
%include "iutil/stringarray.h"
ARRAY_OBJECT_FUNCTIONS(iStringArray,const char *)
%include "iutil/document.h"
%newobject iDocument::Write();
/* extension that will just return the written string */
%extend iDocument
{
  scfString *Write()
  {
     scfString *dest_str = new scfString();
     self->Write(dest_str);
     return dest_str;
  }
}

%template(scfConfigFile) scfImplementation1<csConfigFile,iConfigFile >;
%include "csutil/cfgfile.h"
%include "csutil/radixsort.h"

%extend iDocumentAttributeIterator
{
  ITERATOR_FUNCTIONS(iDocumentAttributeIterator)
}

%extend iDocumentNodeIterator
{
  ITERATOR_FUNCTIONS(iDocumentNodeIterator)
}

%include "csutil/xmltiny.h"

%ignore iDataBuffer::GetInt8;
%include "iutil/databuff.h"
BUFFER_RW_FUNCTIONS(iDataBuffer,GetData,GetSize,
                char,AsBuffer)

%ignore iBase::~iBase(); // We replace iBase dtor with one that calls DecRef().
			 // Swig already knows not to delete an SCF pointer.

%extend iTriangleMesh
{
  csVector3 *GetVertexByIndex(int index)
  { return &(self->GetVertices()[index]); }

  csTriangle *GetTriangleByIndex(int index)
  { return &(self->GetTriangles()[index]); }
}

// iutil/csinput.h
%extend iKeyboardDriver
{
  bool GetKeyState (const char * key)
  { return self->GetKeyState ((int) key[0]); }
}

// iutil/event.h
%extend iEvent
{
  const csMouseEventData Mouse;
  const csJoystickEventData Joystick;
  const csCommandEventData Command;
}

// iutil/event.h
%{
  /// note that these values are only valid until the next call.
  csMouseEventData * iEvent_Mouse_get (iEvent * event)
  { 
    static csMouseEventData p; 
    if(!csMouseEventHelper::GetEventData(event, p)) return 0;
    return &p; 
  }
  csJoystickEventData * iEvent_Joystick_get (iEvent * event)
  { 
    static csJoystickEventData p; 
    if(!csJoystickEventHelper::GetEventData(event, p)) return 0;
    return &p; 
  }
  csCommandEventData * iEvent_Command_get (iEvent * event)
  { 
    static csCommandEventData p; 
    if(!csCommandEventHelper::GetEventData(event, p)) return 0;
    return &p; 
  }
%}


// iutil/evdefs.h
#define _CSKEY_SHIFT_NUM(n) CSKEY_SHIFT_NUM(n)
#undef CSKEY_SHIFT_NUM
int CSKEY_SHIFT_NUM(int n);
#define _CSKEY_CTRL_NUM(n) CSKEY_CTRL_NUM(n)
#undef CSKEY_CTRL_NUM
int CSKEY_CTRL_NUM(int n);
#define _CSKEY_ALT_NUM(n) CSKEY_ALT_NUM(n)
#undef CSKEY_ALT_NUM
int CSKEY_ALT_NUM(int n);

#define _CSKEY_SPECIAL(n) CSKEY_SPECIAL(n)
#undef CSKEY_SPECIAL
int CSKEY_SPECIAL(int n);

#define _CSKEY_SPECIAL_NUM(code) CSKEY_SPECIAL_NUM(code)
#undef CSKEY_SPECIAL_NUM
int CSKEY_SPECIAL_NUM(int code);

#define _CSKEY_MODIFIER(type,num) CSKEY_MODIFIER(type,num)
#undef CSKEY_MODIFIER
int CSKEY_MODIFIER(int type,int num);

// csutil/eventnames.h
#define _CS_IS_KEYBOARD_EVENT(reg,e) CS_IS_KEYBOARD_EVENT(reg,e)
#undef CS_IS_KEYBOARD_EVENT
bool _CS_IS_KEYBOARD_EVENT (iObjectRegistry *,const iEvent &);
#define _CS_IS_MOUSE_EVENT(reg,e) CS_IS_MOUSE_EVENT(reg,e)
#undef CS_IS_MOUSE_EVENT
bool _CS_IS_MOUSE_EVENT (iObjectRegistry *,const iEvent &);
#define _CS_IS_JOYSTICK_EVENT(reg,e) CS_IS_JOYSTICK_EVENT(reg,e)
#undef CS_IS_JOYSTICK_EVENT
bool _CS_IS_JOYSTICK_EVENT (iObjectRegistry *,const iEvent &);
#define _CS_IS_INPUT_EVENT(reg,e) CS_IS_INPUT_EVENT(reg,e)
#undef CS_IS_INPUT_EVENT
bool _CS_IS_INPUT_EVENT (iObjectRegistry *,const iEvent &);


/*
 New Macros for to use instead of event type masks
 XXX still many missing here!!!
*/
#define _csevAllEvents(reg) csevAllEvents(reg)
#undef csevAllEvents
csEventID _csevAllEvents (iObjectRegistry *);
#define _csevFrame(reg) csevFrame(reg)
#undef csevFrame
csEventID _csevFrame (iObjectRegistry *);
#define _csevInput(reg) csevInput(reg)
#undef csevInput
csEventID _csevInput (iObjectRegistry *);
#define _csevQuit(reg) csevQuit(reg)
#undef csevQuit
csEventID _csevQuit (iObjectRegistry *);

/* Canvas */
#define _csevCanvasClose(reg, g2d) csevCanvasClose(reg, g2d)
#undef csevCanvasClose
csEventID csevCanvasClose(iObjectRegistry*, iGraphics2D*);
#define _csevCanvasExposed(reg, g2d) csevCanvasExposed(reg, g2d)
#undef csevCanvasExposed
csEventID csevCanvasExposed(iObjectRegistry*, iGraphics2D*);
#define _csevCanvasExposed(reg, g2d) csevCanvasExposed(reg, g2d)
#undef csevCanvasExposed
csEventID csevCanvasExposed(iObjectRegistry*, iGraphics2D*);
#define _csevCanvasHidden(reg, g2d) csevCanvasHidden(reg, g2d)
#undef csevCanvasHidden
csEventID csevCanvasHidden(iObjectRegistry*, iGraphics2D*);
#define _csevCanvasResize(reg, g2d) csevCanvasResize(reg, g2d)
#undef csevCanvasResize
csEventID csevCanvasResize(iObjectRegistry*, iGraphics2D*);
#define _csevFocusChanged(reg) csevFocusChanged(reg)
#undef csevFocusChanged
csEventID csevFocusChanged(iObjectRegistry*);
#define _csevFocusGained(reg) csevFocusGained(reg)
#undef csevFocusGained
csEventID csevFocusGained(iObjectRegistry*);
#define _csevFocusLost(reg) csevFocusLost(reg)
#undef csevFocusLost
csEventID csevFocusLost(iObjectRegistry*);

/* Keyboard */
#define _csevKeyboardEvent(reg) csevKeyboardEvent(reg)
#undef csevKeyboardEvent
csEventID _csevKeyboardEvent (iObjectRegistry *);
#define _csevKeyboardDown(reg) csevKeyboardDown(reg)
#undef csevKeyboardDown
csEventID _csevKeyboardDown (iObjectRegistry *);
#define _csevKeyboardUp(reg) csevKeyboardUp(reg)
#undef csevKeyboardUp
csEventID _csevKeyboardUp (iObjectRegistry *);

/* Mouse */
#define _csevMouseEvent(reg) csevMouseEvent(reg)
#undef csevMouseEvent
csEventID _csevMouseEvent (iObjectRegistry *);
#define _csevMouseButton(reg,x) csevMouseButton(reg,x)
#undef csevMouseButton
csEventID _csevMouseButton (iObjectRegistry *,uint x);
#define _csevMouseUp(reg,x) csevMouseUp(reg,x)
#undef csevMouseUp
csEventID _csevMouseUp (iObjectRegistry *,uint x);
#define _csevMouseDown(reg,x) csevMouseDown(reg,x)
#undef csevMouseDown
csEventID _csevMouseDown (iObjectRegistry *,uint x);
#define _csevMouseClick(reg,x) csevMouseClick(reg,x)
#undef csevMouseClick
csEventID _csevMouseClick (iObjectRegistry *,uint x);
#define _csevMouseDoubleClick(reg,x) csevMouseDoubleClick(reg,x)
#undef csevMouseDoubleClick
csEventID _csevMouseDoubleClick (iObjectRegistry *,uint x);
#define _csevMouseMove(reg,x) csevMouseMove(reg,x)
#undef csevMouseMove
csEventID _csevMouseMove (iObjectRegistry *,uint x);

/* Joystick */
#define _csevJoystickEvent(reg) csevJoystickEvent(reg)
#undef csevJoystickEvent
csEventID _csevJoystickEvent (iObjectRegistry *);

%include "csutil/eventhandlers.h"
%include "csutil/common_handlers.h"

// csutil/cscolor.h
%extend csColor
{
  csColor operator + (const csColor & c) const { return *self + c; }
  csColor operator - (const csColor & c) const { return *self - c; }
}

// functions for returning wrapped iBase objects.
%include "bindings/common/scfsugar.i"

/* POST */
#ifndef SWIGIMPORTED
#undef APPLY_FOR_ALL_INTERFACES_POST
#define APPLY_FOR_ALL_INTERFACES_POST CORE_APPLY_FOR_EACH_INTERFACE
#endif

%include "bindings/common/basepost.i"

#ifndef SWIGIMPORTED
cs_apply_all_interfaces
#endif

cs_lang_include(corepost.i)


