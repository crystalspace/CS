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

#ifdef USE_DIRECTORS
%module(directors="1") cspace
#else
%module cspace
#endif

%include "csconfig.h"
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

%import "crystalspace.h"
%{
#include "crystalspace.h"
%}

%include "typemaps.i"

/* The following list all kown arguments that are actually (also) outputs.
   It is possible that some of the output arguments should be INOUT instead
   of OUTPUT.								    */

// output arguments
%apply unsigned long { utf32_char };
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

%include "cstypes.h"

%immutable csWrapPtr::Type;
%inline %{

  /* This pointer wrapper can be used to prevent code-bloat by macros
   *  acting as template functions.  Examples are SCF_QUERY_INTERFACE()
   * and CS_QUERY_REGISTRY().  Also note that CS should never need to
   * use virtual inheritance as long as it has SCF.
   *
   * Ref - A managed reference to the iBase pointer of the wrapped
   *   interface.
   * Type
   *   The SCF interface name which this pointer represents (for
   *   instance, "iEngine").
   * Version - The version of the interface this pointer represents.
   */
  struct csWrapPtr
  {
    csRef<iBase> Ref;
    const char *Type;
    scfInterfaceVersion Version;
    csWrapPtr (const char *t, scfInterfaceVersion v, csPtr<iBase> r)
    : Ref (r), Type (t), Version(v) {}
    csWrapPtr (const char *t, scfInterfaceVersion v, csRef<iBase> r)
    : Ref (r), Type (t), Version(v) {}
    csWrapPtr (const csWrapPtr &p)
    : Ref (p.Ref), Type (p.Type), Version(p.Version) {}
  };

%}

/* Macro's expected in rest of this file: ignored by default.
   When overriding in language-specific files, first #undef and then
   re-%define them.							    */
#define TYPEMAP_OUT_csRef(T)
#define TYPEMAP_OUT_csPtr(T)
#define TYPEMAP_OUT_csRefArray(T)
#define TYPEMAP_OUT_csWrapPtr
#define TYPEMAP_IN_ARRAY_CNT_PTR(a,b)
#define TYPEMAP_IN_ARRAY_PTR_CNT(a,b) 
#define TYPEMAP_OUTARG_ARRAY_PTR_CNT(a,b,c)
#define TYPEMAP_ARGOUT_PTR(T)
#define APPLY_TYPEMAP_ARGOUT_PTR(T,Args)
#define ITERATOR_FUNCTIONS(T)
#define ARRAY_OBJECT_FUNCTIONS(classname,typename)
#define LIST_OBJECT_FUNCTIONS(classname,typename)
#define SET_OBJECT_FUNCTIONS(classname,typename)

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

/* Handle arrays as input arguments.
   The second parameter is for putting in an extra dereference operator if the
   array is an array of objects. If it is an array of _pointers to_ objects,
   then the second parameter should be empty. This is the inverse of the
   third parameter in the TYPEMAP_OUTARG_ARRAY_... macros below.	    */
TYPEMAP_IN_ARRAY_CNT_PTR((int num_layers, iTextureWrapper ** wrappers), /**/)
TYPEMAP_IN_ARRAY_PTR_CNT((csVector2 * InPolygon, int InCount), *)
TYPEMAP_IN_ARRAY_PTR_CNT((csVector3* vertices, int num_vertices), *)
TYPEMAP_IN_ARRAY_PTR_CNT((csVector3* vertices, int num), *)

/* Handle arrays as output arguments.
   The third parameter is for putting in an extra dereference operator if the
   array is an array of _pointers to_ objects. If it is just array of objects,
   then the third parameter should be empty. This is the inverse of the
   second parameter in the TYPEMAP_IN_ARRAY_... macros above.		    */
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
%ignore csArray::DeleteAll;
%ignore csArray::DeleteFast;
%ignore csArray::Find;
%ignore csArray::FindKey;
%ignore csArray::FindSortedKey;
%ignore csArray::GetExtend;
%ignore csArray::GetIndex;
%ignore csArray::GetIterator;
%ignore csArray::InitRegion;
%ignore csArray::InsertSorted;
%ignore csArray::Iterator;
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
%ignore csArray::Iterator;
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

%ignore scfInitialize;
%immutable iSCF::SCF;
%include "csutil/scf_interface.h"
%include "csutil/scf.h"

// hand made scf template wrappers
%include "bindings/scf.i"

%define INTERFACE_PRE(T)
  %nodefault T;
  TYPEMAP_OUT_csRef(T)
  TYPEMAP_OUT_csPtr(T)
  TYPEMAP_OUT_csRefArray(T)
%enddef

#undef INTERFACE_APPLY
#define INTERFACE_APPLY(x) INTERFACE_PRE(x)
APPLY_FOR_ALL_INTERFACES_PRE

TYPEMAP_OUT_csWrapPtr

