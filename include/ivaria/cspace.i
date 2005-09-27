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

// Ignored macro's.
#define CS_STRUCT_ALIGN_4BYTE_BEGIN
#define CS_STRUCT_ALIGN_4BYTE_END
#define CS_GNUC_PRINTF(format_idx, arg_idx)
#define CS_GNUC_SCANF(format_idx, arg_idx)
#define CS_DECLARE_STATIC_CLASSVAR(a, b, c)
#define CS_SPECIALIZE_TEMPLATE template<>
#define CS_FORCEINLINE
#define CS_EXPORT_SYM
#define CS_IMPORT_SYM
#define CS_CRYSTALSPACE_EXPORT
#define CS_LEAKGUARD_DECLARE(m)
#define CS_DEPRECATED_METHOD
#define CS_DEPRECATED_TYPE
#define CS_PURE_METHOD
#define CS_CONST_METHOD

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
%include "ivaria/perl1st.i"
#endif

%{
#include "crystalspace.h"
%}

// The following list holds all the interfaces that are handled correctly.
// If you have problems, first check if the interface in question is in this
// list. Please keep the list sorted alphabetically.
#ifndef CS_MINI_SWIG
%define APPLY_FOR_EACH_INTERFACE
  INTERFACE_APPLY(iAws)
  INTERFACE_APPLY(iAwsKey)
  INTERFACE_APPLY(iAudioStream)
  INTERFACE_APPLY(iBase)
  INTERFACE_APPLY(iBallState)
  INTERFACE_APPLY(iBinaryLoaderPlugin)
  INTERFACE_APPLY(iBodyGroup)
  INTERFACE_APPLY(iCamera)
  INTERFACE_APPLY(iCameraPosition)
  INTERFACE_APPLY(iCacheManager)
  INTERFACE_APPLY(iCollider)
  INTERFACE_APPLY(iCollideSystem)
  INTERFACE_APPLY(iCommandLineParser)
  INTERFACE_APPLY(iComponent)
  INTERFACE_APPLY(iConfigFile)
  INTERFACE_APPLY(iConfigIterator)
  INTERFACE_APPLY(iConfigManager)
  INTERFACE_APPLY(iDataBuffer)
  INTERFACE_APPLY(iDebugHelper)
  INTERFACE_APPLY(iDocument)
  INTERFACE_APPLY(iDocumentSystem)
  INTERFACE_APPLY(iDynamics)
  INTERFACE_APPLY(iDynamicSystem)
  INTERFACE_APPLY(iEngine)
  INTERFACE_APPLY(iEvent)
  INTERFACE_APPLY(iEventHandler)
  INTERFACE_APPLY(iEventQueue)
  INTERFACE_APPLY(iFactory)
  INTERFACE_APPLY(iFile)
  INTERFACE_APPLY(iFont)
  INTERFACE_APPLY(iFontServer)
  INTERFACE_APPLY(iFrustumView)
  INTERFACE_APPLY(iFrustumViewUserdata)
  INTERFACE_APPLY(iGeneralFactoryState)
  INTERFACE_APPLY(iGeneralMeshState)
  INTERFACE_APPLY(iGraphics3D)
  INTERFACE_APPLY(iGraphics2D)
  INTERFACE_APPLY(iHalo)
  INTERFACE_APPLY(iImage)
  INTERFACE_APPLY(iImageIO)
  INTERFACE_APPLY(iJoint)
  INTERFACE_APPLY(iODEDynamicState)
  INTERFACE_APPLY(iODEDynamicSystemState)
  INTERFACE_APPLY(iODEJointState)
  INTERFACE_APPLY(iODESliderJoint)
  INTERFACE_APPLY(iODEUniversalJoint)
  INTERFACE_APPLY(iODEAMotorJoint)
  INTERFACE_APPLY(iODEHingeJoint)
  INTERFACE_APPLY(iODEBallJoint)
  INTERFACE_APPLY(iKeyboardDriver)
  INTERFACE_APPLY(iLight)
  INTERFACE_APPLY(iLightList)
  INTERFACE_APPLY(iLoader)
  INTERFACE_APPLY(iLoaderPlugin)
  INTERFACE_APPLY(iMapNode)
  INTERFACE_APPLY(iMaterial)
  INTERFACE_APPLY(iMaterialWrapper)
  INTERFACE_APPLY(iMeshFactoryWrapper)
  INTERFACE_APPLY(iMeshObject)
  INTERFACE_APPLY(iMeshObjectFactory)
  INTERFACE_APPLY(iMeshObjectType)
  INTERFACE_APPLY(iMeshWrapper)
  INTERFACE_APPLY(iMeshWrapperIterator)
  INTERFACE_APPLY(iModelConverter)
  INTERFACE_APPLY(iMouseDriver)
  INTERFACE_APPLY(iMovable)
  INTERFACE_APPLY(iMovableListener)
  INTERFACE_APPLY(iMovieRecorder)
  INTERFACE_APPLY(iObject)
  INTERFACE_APPLY(iObjectModel)
  INTERFACE_APPLY(iObjectModelListener)
  INTERFACE_APPLY(iObjectRegistry)
  INTERFACE_APPLY(iPath)
  INTERFACE_APPLY(iPluginManager)
  INTERFACE_APPLY(iPolygonMesh)
  INTERFACE_APPLY(iPortal)
  INTERFACE_APPLY(iPortalContainer)
  INTERFACE_APPLY(iReporter)
  INTERFACE_APPLY(iReporterIterator)
  INTERFACE_APPLY(iReporterListener)
  INTERFACE_APPLY(iSCF)
  INTERFACE_APPLY(iScript)
  INTERFACE_APPLY(iScriptObject)
  INTERFACE_APPLY(iSimpleFormerState)
  INTERFACE_APPLY(iSector)
  INTERFACE_APPLY(iSectorList)
  INTERFACE_APPLY(iShaderManager)
  INTERFACE_APPLY(iShaderVariableContext)
  INTERFACE_APPLY(iSoundHandle)
  INTERFACE_APPLY(iSoundLoader)
  INTERFACE_APPLY(iSoundRender)
  INTERFACE_APPLY(iSoundWrapper)
  INTERFACE_APPLY(iSoundDriver)
  INTERFACE_APPLY(iSoundSource)
  INTERFACE_APPLY(iSoundListener)
  INTERFACE_APPLY(iSprite2DState)
  INTERFACE_APPLY(iSprite3DState)
  INTERFACE_APPLY(iSpriteCal3DState)
  INTERFACE_APPLY(iStandardReporterListener)
  INTERFACE_APPLY(iStream)
  INTERFACE_APPLY(iStreamIterator)
  INTERFACE_APPLY(iStreamFormat)
  INTERFACE_APPLY(iString)
  INTERFACE_APPLY(iStringArray)
  INTERFACE_APPLY(iStringSet)
  INTERFACE_APPLY(iTerrainFactoryState)
  INTERFACE_APPLY(iTerrainObjectState)
  INTERFACE_APPLY(iTerraFormer)
  INTERFACE_APPLY(iTerraSampler)
  INTERFACE_APPLY(iTextureHandle)
  INTERFACE_APPLY(iTextureList)
  INTERFACE_APPLY(iTextureManager)
  INTERFACE_APPLY(iTextureWrapper)
  INTERFACE_APPLY(iThingState)
  INTERFACE_APPLY(iVFS)
  INTERFACE_APPLY(iVideoStream)
  INTERFACE_APPLY(iView)
  INTERFACE_APPLY(iVirtualClock)
  INTERFACE_APPLY(iVisibilityCuller)
%enddef
#else // CS_MINI_SWIG
#ifndef CS_MICRO_SWIG
%define APPLY_FOR_EACH_INTERFACE
  INTERFACE_APPLY(iBase)
  INTERFACE_APPLY(iEvent)
  INTERFACE_APPLY(iEventHandler)
  INTERFACE_APPLY(iEventQueue)
  INTERFACE_APPLY(iFont)
  INTERFACE_APPLY(iFontServer)
  INTERFACE_APPLY(iGraphics3D)
  INTERFACE_APPLY(iGraphics2D)
  INTERFACE_APPLY(iObjectRegistry)
  INTERFACE_APPLY(iSCF)
  INTERFACE_APPLY(iVFS)
%enddef
#else // CS_MICRO_SWIG
%define APPLY_FOR_EACH_INTERFACE
  INTERFACE_APPLY(iBase)
  INTERFACE_APPLY(iObjectRegistry)
  INTERFACE_APPLY(iSCF)
  INTERFACE_APPLY(iVFS)
%enddef
#endif // CS_MICRO_SWIG
#endif // CS_MINI_SWIG

%include "typemaps.i"

// The following list all kown arguments that are actually (also) outputs.
// It is possible that some of the output arguments should be INOUT instead
// of OUTPUT.

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

// input/output arguments
%apply bool * INOUT { bool & mirror };
%apply int * INOUT { int & maxcolors };
%apply float * INOUT { float & iR };
%apply float * INOUT { float & iG };
%apply float * INOUT { float & iB };

%include "cstypes.h"

%immutable csWrapPtr::Type;
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

  struct csWrapPtr
  {
    csRef<iBase> Ref;
    const char *Type;
    scfInterfaceVersion Version;
    csWrapPtr (const char *t, scfInterfaceVersion v, iBase *r)
    : Ref (r), Type (t), Version(v) {}
    csWrapPtr (const char *t, scfInterfaceVersion v, csPtr<iBase> r)
    : Ref (r), Type (t), Version(v) {}
    csWrapPtr (const char *t, scfInterfaceVersion v, csRef<iBase> r)
    : Ref (r), Type (t), Version(v) {}
    csWrapPtr (const csWrapPtr &p)
    : Ref (p.Ref), Type (p.Type), Version(p.Version) {}
  };

%}

// Macro's expected in rest of this file: ignored by default.
// When overriding in language-specific files, first #undef and then
// re-%define them.
#define TYPEMAP_OUT_csRef(T)
#define TYPEMAP_OUT_csPtr(T)
#define TYPEMAP_OUT_csRefArray(T)
#define TYPEMAP_OUT_csWrapPtr
#define TYPEMAP_IN_ARRAY_CNT_PTR(a,b)
#define TYPEMAP_IN_ARRAY_PTR_CNT(a,b) 
#define TYPEMAP_OUTARG_ARRAY_PTR_CNT(a,b,c)

#if defined(SWIGPYTHON)
  %include "ivaria/pythpre.i"
#elif defined(SWIGPERL5)
  %include "ivaria/perlpre.i"
#elif defined(SWIGRUBY)
  %include "ivaria/rubypre.i"
#elif defined(SWIGTCL8)
  %include "ivaria/tclpre.i"
#elif defined(SWIGJAVA)
  %include "ivaria/javapre.i"
#endif

// Handle arrays as input arguments.
TYPEMAP_IN_ARRAY_CNT_PTR((int num_layers, iTextureWrapper ** wrappers), /**/)
TYPEMAP_IN_ARRAY_PTR_CNT((csVector2 * InPolygon, int InCount), *)
TYPEMAP_IN_ARRAY_PTR_CNT((csVector3* vertices, int num_vertices), *)
TYPEMAP_IN_ARRAY_PTR_CNT((csVector3* vertices, int num), *)

// Handle arrays as output arguments.
TYPEMAP_OUTARG_ARRAY_PTR_CNT(
  (csVector2 * OutPolygon, int & OutCount),
  new csVector2[MAX_OUTPUT_VERTICES], *
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
APPLY_FOR_EACH_INTERFACE

TYPEMAP_OUT_csWrapPtr

// Inclusion of CS headers.  The sequence of %include-ing the CS headers can be
// crucial!  The scheme is as follows: %ignore'd functions and types are placed
// before actual inclusion, as are "local" %typemap's (like default).  After
// %include-ing the header resets can be done; for example resetting the
// %typemap(default). The %extend-ing and extra code takes place after all
// %include's are done, mentioning the header(s) it is related to.

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
%include "csutil/array.h"

%ignore scfInitialize;
%include "csutil/scf_interface.h"
%include "csutil/scf.h"

#ifndef CS_MINI_SWIG
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

%ignore csStringSet::GlobalIterator;
%ignore csStringSet::GetIterator;
%include "csutil/strset.h"

%ignore iString::SetCapacity;
%ignore iString::GetCapacity;
%ignore iString::SetGrowsBy;
%ignore iString::GetGrowsBy;
%ignore iString::Truncate;
%ignore iString::Reclaim;
%ignore iString::ShrinkBestFit;
%ignore iString::Clear;
%ignore iString::Empty;
%ignore iString::Clone;
%ignore iString::GetData (); // Non-const.
%ignore iString::Length;
%ignore iString::IsEmpty;
%ignore iString::SetAt;
%ignore iString::GetAt;
%ignore iString::Insert;
%ignore iString::Overwrite;
%ignore iString::Append;
%ignore iString::Slice;
%ignore iString::SubString;
%ignore iString::FindFirst;
%ignore iString::FindLast;
%ignore iString::Find;
%ignore iString::ReplaceAll;
%ignore iString::Format;
%ignore iString::FormatV;
%ignore iString::Replace;
%ignore iString::Compare;
%ignore iString::CompareNoCase;
%ignore iString::Downcase;
%ignore iString::Upcase;
%ignore iString::operator+=;
%ignore iString::operator+;
%ignore iString::operator==;
%ignore iString::operator [] (size_t);
%ignore iString::operator [] (size_t) const;
%ignore iString::operator char const*;
%include "iutil/string.h"
#endif // CS_MINI_SWIG

%ignore csStringBase;
%ignore csStringBase::operator [] (size_t);
%ignore csStringBase::operator [] (size_t) const;
%ignore csStringFast;
%ignore csString::csString (size_t);
%ignore csString::csString (char);
%ignore csString::csString (unsigned char);
%ignore csString::SetCapacity;
%ignore csString::GetCapacity;
%ignore csString::SetGrowsBy;
%ignore csString::GetGrowsBy;
%ignore csString::SetGrowsExponentially;
%ignore csString::GetGrowsExponentially;
%ignore csString::Truncate;
%ignore csString::Free;
%ignore csString::Reclaim;
%ignore csString::ShrinkBestFit;
%ignore csString::Clear;
%ignore csString::Empty;
%ignore csString::SetAt;
%ignore csString::GetAt;
%ignore csString::DeleteAt;
%ignore csString::Insert;
%ignore csString::Overwrite;
%ignore csString::Append;
%ignore csString::AppendFmt;
%ignore csString::AppendFmtV;
%ignore csString::Slice;
%ignore csString::SubString;
%ignore csString::FindFirst;
%ignore csString::FindLast;
%ignore csString::Find;
%ignore csString::ReplaceAll;
%ignore csString::Replace;
%ignore csString::Compare;
%ignore csString::CompareNoCase;
%ignore csString::Clone;
%ignore csString::LTrim;
%ignore csString::RTrim;
%ignore csString::Trim;
%ignore csString::Collapse;
%ignore csString::Format;
%ignore csString::FormatV;
%ignore csString::PadLeft;
%ignore csString::PadRight;
%ignore csString::PadCenter;
%ignore csString::GetData (); // Non-const.
%ignore csString::Detach;
%ignore csString::Upcase;
%ignore csString::Downcase;
%ignore csString::operator=;
%ignore csString::operator+=;
%ignore csString::operator+;
%ignore csString::operator==;
%ignore csString::operator!=;
%ignore csString::operator [] (size_t);
%ignore csString::operator [] (size_t) const;
%ignore csString::operator const char*;
%ignore operator+ (const char*, const csString&);
%ignore operator+ (const csString&, const char*);
%ignore operator<<;
%include "csutil/csstring.h"

#ifndef CS_MINI_SWIG
%ignore csVector2::operator+ (const csVector2 &, const csVector2 &);
%ignore csVector2::operator- (const csVector2 &, const csVector2 &);
%ignore csVector2::operator* (const csVector2 &, const csVector2 &);
%ignore csVector2::operator* (const csVector2 &, float);
%ignore csVector2::operator* (float, const csVector2 &);
%ignore csVector2::operator/ (const csVector2 &, float);
%ignore csVector2::operator== (const csVector2 &, const csVector2 &);
%ignore csVector2::operator!= (const csVector2 &, const csVector2 &);
%ignore csVector2::operator< (const csVector2 &, float);
%ignore csVector2::operator> (float, const csVector2 &);
%ignore csVector2::operator[];
%ignore csVector2::Norm (const csVector2 &);
%include "csgeom/vector2.h"

%ignore csVector3::operator+ (const csVector3 &, const csVector3 &);
%ignore csVector3::operator+ (const csDVector3 &, const csVector3 &);
%ignore csVector3::operator+ (const csVector3 &, const csDVector3 &);
%ignore csVector3::operator- (const csVector3 &, const csVector3 &);
%ignore csVector3::operator- (const csDVector3 &, const csVector3 &);
%ignore csVector3::operator- (const csVector3 &, const csDVector3 &);
%ignore csVector3::operator* (const csVector3 &, const csVector3 &);
%ignore csVector3::operator% (const csVector3 &, const csVector3 &);
%ignore csVector3::operator* (const csVector3 &, float);
%ignore csVector3::operator* (float, const csVector3 &);
%ignore csVector3::operator* (const csVector3 &, double);
%ignore csVector3::operator* (double, const csVector3 &);
%ignore csVector3::operator* (const csVector3 &, int);
%ignore csVector3::operator* (int, const csVector3 &);
%ignore csVector3::operator/ (const csVector3 &, float);
%ignore csVector3::operator/ (const csVector3 &, double);
%ignore csVector3::operator/ (const csVector3 &, int);
%ignore csVector3::operator== (const csVector3 &, const csVector3 &);
%ignore csVector3::operator!= (const csVector3 &, const csVector3 &);
%ignore csVector3::operator>> (const csVector3 &, const csVector3 &);
%ignore csVector3::operator<< (const csVector3 &, const csVector3 &);
%ignore csVector3::operator< (const csVector3 &, float);
%ignore csVector3::operator> (float, const csVector3 &);
%ignore csVector3::operator[];
%ignore csVector3::Norm (const csVector3 &);
%ignore csVector3::Unit (const csVector3 &);
%include "csgeom/vector3.h"

%ignore csMatrix2::operator+ (const csMatrix2 &, const csMatrix2 &);
%ignore csMatrix2::operator- (const csMatrix2 &, const csMatrix2 &);
%ignore csMatrix2::operator* (const csMatrix2 &, const csMatrix2 &);
%ignore csMatrix2::operator* (const csMatrix2 &, const csVector2 &);
%ignore csMatrix2::operator* (const csMatrix2 &, float);
%ignore csMatrix2::operator* (float, const csMatrix2 &);
%ignore csMatrix2::operator/ (const csMatrix2 &, float);
%include "csgeom/matrix2.h"

%ignore csMatrix3::operator+ (const csMatrix3 &, const csMatrix3 &);
%ignore csMatrix3::operator- (const csMatrix3 &, const csMatrix3 &);
%ignore csMatrix3::operator* (const csMatrix3 &, const csMatrix3 &);
%ignore csMatrix3::operator* (const csMatrix3 &, const csVector3 &);
%ignore csMatrix3::operator* (const csMatrix3 &, float);
%ignore csMatrix3::operator* (float, const csMatrix3 &);
%ignore csMatrix3::operator/ (const csMatrix3 &, float);
%ignore csMatrix3::operator== (const csMatrix3 &, const csMatrix3 &);
%ignore csMatrix3::operator!= (const csMatrix3 &, const csMatrix3 &);
%ignore csMatrix3::operator< (const csMatrix3 &, float);
%ignore csMatrix3::operator> (float, const csMatrix3 &);
%include "csgeom/matrix3.h"

%ignore csTransform::operator* (const csVector3 &, const csTransform &);
%ignore csTransform::operator* (const csTransform &, const csVector3 &);
%ignore csTransform::operator*= (csVector3 &, const csTransform &);
%ignore csTransform::operator* (const csPlane3 &, const csTransform &);
%ignore csTransform::operator* (const csTransform &, const csPlane3 &);
%ignore csTransform::operator*= (csPlane3 &, const csTransform &);
%ignore csTransform::operator* (const csSphere &, const csTransform &);
%ignore csTransform::operator* (const csTransform &, const csSphere &);
%ignore csTransform::operator*= (csSphere &, const csTransform &);
%ignore csTransform::operator* (const csMatrix3 &, const csTransform &);
%ignore csTransform::operator* (const csTransform &, const csMatrix3 &);
%ignore csTransform::operator*= (csMatrix3 &, const csTransform &);
%ignore csTransform::operator*
  (const csTransform &, const csReversibleTransform &);
%ignore csReversibleTransform::operator/
  (const csVector3 &, const csReversibleTransform &);
%ignore csReversibleTransform::operator/=
  (csVector3 &, const csReversibleTransform &);
%ignore csReversibleTransform::operator/
  (const csPlane3 &, const csReversibleTransform &);
%ignore csReversibleTransform::operator/=
  (csPlane3 &, const csReversibleTransform &);
%ignore csReversibleTransform::operator/
  (const csSphere &, const csReversibleTransform &);
%ignore csReversibleTransform::operator*=
  (csReversibleTransform &, const csReversibleTransform &);
%ignore csReversibleTransform::operator*
  (const csReversibleTransform &, const csReversibleTransform &);
%ignore csReversibleTransform::operator*
  (const csTransform &, const csReversibleTransform &);
%ignore csReversibleTransform::operator/=
  (csReversibleTransform &, const csReversibleTransform &);
%ignore csReversibleTransform::operator/
  (const csReversibleTransform &, const csReversibleTransform &);
%include "csgeom/transfrm.h"

%ignore csSphere::operator+ (const csSphere &, const csSphere &);
%ignore csSphere::GetCenter (); // Non-const.
%ignore csEllipsoid::GetCenter (); // Non-const.
%ignore csEllipsoid:: GetRadius (); // Non-const.
%include "csgeom/sphere.h"

%ignore csPlane2::A ();
%ignore csPlane2::B ();
%ignore csPlane2::C ();
%include "csgeom/plane2.h"

%ignore csPlane3::A ();
%ignore csPlane3::B ();
%ignore csPlane3::C ();
%ignore csPlane3::D ();
%ignore csPlane3::Normal (); // Non-const.
%include "csgeom/plane3.h"

%include "csgeom/math2d.h"

%ignore csPoly2D::operator[];
%ignore csPoly2D::GetVertices (); // Non-const.
%include "csgeom/poly2d.h"

// Swig 1.3.24 doesn't handle pointer default args well unless we tell it
// to use an alternate way for those functions
%feature("compactdefaultargs") csIntersect3::BoxSegment;
%include "csgeom/math3d.h"

%ignore csPoly3D::operator[];
%ignore csPoly3D::GetVertices (); // Non-const.
%include "csgeom/poly3d.h"

%include "csgeom/tri.h"

%include "csgeom/csrect.h"
%include "csgeom/csrectrg.h"

%ignore csQuaternion::operator+ (const csQuaternion &, const csQuaternion &);
%ignore csQuaternion::operator- (const csQuaternion &, const csQuaternion &);
%ignore csQuaternion::operator* (const csQuaternion &, const csQuaternion &);
%include "csgeom/quaterni.h"

%include "csgeom/spline.h"
%include "csgeom/cspoint.h"

%ignore csBox2::operator+ (const csBox2& box1, const csBox2& box2);
%ignore csBox2::operator+ (const csBox2& box, const csVector2& point);
%ignore csBox2::operator* (const csBox2& box1, const csBox2& box2);
%ignore csBox2::operator== (const csBox2& box1, const csBox2& box2);
%ignore csBox2::operator!= (const csBox2& box1, const csBox2& box2);
%ignore csBox2::operator< (const csBox2& box1, const csBox2& box2);
%ignore csBox2::operator> (const csBox2& box1, const csBox2& box2);
%ignore csBox2::operator< (const csVector2& point, const csBox2& box);
%ignore csBox3::operator+ (const csBox3& box1, const csBox3& box2);
%ignore csBox3::operator+ (const csBox3& box, const csVector3& point);
%ignore csBox3::operator* (const csBox3& box1, const csBox3& box2);
%ignore csBox3::operator== (const csBox3& box1, const csBox3& box2);
%ignore csBox3::operator!= (const csBox3& box1, const csBox3& box2);
%ignore csBox3::operator< (const csBox3& box1, const csBox3& box2);
%ignore csBox3::operator> (const csBox3& box1, const csBox3& box2);
%ignore csBox3::operator< (const csVector3& point, const csBox3& box);
%include "csgeom/box.h"

%ignore csSegment2::Start (); // Non-const.
%ignore csSegment2::End ();   // Non-const.
%ignore csSegment3::Start (); // Non-const.
%ignore csSegment3::End ();   // Non-const.
%include "csgeom/segment.h"

%rename(asRGBcolor) csRGBpixel::operator csRGBcolor;
%include "csgfx/rgbpixel.h"
#endif // CS_MINI_SWIG

%ignore csGetPlatformConfig;
%ignore csPrintfV;
%ignore csFPrintfV;
%ignore csPrintfErrV;
%include "csutil/sysfunc.h"

%ignore csInitializer::RequestPlugins(iObjectRegistry*, ...);
%ignore csInitializer::RequestPluginsV;
%rename (_RequestPlugins) csInitializer::RequestPlugins(iObjectRegistry*,
  csArray<csPluginRequest> const&);
%ignore csInitializer::SetupEventHandler(iObjectRegistry*, csEventHandlerFunc,
  unsigned int);
%rename(_SetupEventHandler) csInitializer::SetupEventHandler(iObjectRegistry*,
  iEventHandler *, unsigned int);
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
%ignore csArray<csPluginRequest>::InitRegion;
%ignore csArray<csPluginRequest>::InsertSorted;
%ignore csArray<csPluginRequest>::Iterator;
%ignore csArray<csPluginRequest>::Iterator;
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

#ifndef CS_MINI_SWIG
%include "iaws/aws.h"

%include "igeom/clip2d.h"
%include "igeom/objmodel.h"
%include "igeom/path.h"
%include "igeom/polymesh.h"
%include "csgeom/path.h"
%include "csgeom/polymesh.h"
%include "csgeom/spline.h"

%include "iengine/fview.h"
%include "iengine/light.h"
%include "iengine/sector.h"
%include "iengine/engine.h"

%ignore iCamera::GetTransform (); // Non-const.
%ignore iCamera::Perspective (const csVector3&, csVector2&) const;
%ignore iCamera::InvPerspective (const csVector2&, float, csVector3&) const;
%include "iengine/camera.h"

%include "iengine/campos.h"
%include "iengine/texture.h"
%include "iengine/material.h"

// Swig 1.3.24 doesn't handle pointer default args well unless we tell it
// to use an alternate way for that function
%feature("compactdefaultargs") HitBeamObject;
%ignore iMeshWrapper::HitBeamBBox (const csVector3&, const csVector3&,
  csVector3&, float*);
%ignore iMeshWrapper::HitBeamOutline (const csVector3&, const csVector3&,
  csVector3&, float*);
%ignore iMeshWrapper::HitBeam (const csVector3&, const csVector3&,
  csVector3&, float*);
%ignore iMeshWrapper::HitBeam (const csVector3&, const csVector3&,
  csVector3&, float*);
%ignore iMeshWrapper::GetRadius (csVector3&, csVector3&) const;
%ignore iMeshWrapper::GetWorldBoundingBox (csBox3&) const;
%ignore iMeshWrapper::GetTransformedBoundingBox (
  const csReversibleTransform&, csBox3&);
%ignore iMeshWrapper::GetScreenBoundingBox (iCamera*, csBox2&, csBox3&);
%include "iengine/mesh.h"

%include "iengine/movable.h"
%include "iengine/region.h"
// Swig 1.3.24 doesn't handle pointer default args well unless we tell it
// to use an alternate way for that function
%feature("compactdefaultargs") IntersectSegment;
%include "iengine/viscull.h"
%include "iengine/portal.h"
%include "iengine/portalcontainer.h"

#ifndef CS_SWIG_PUBLISH_IGENERAL_FACTORY_STATE_ARRAYS
%ignore iGeneralFactoryState::GetVertices;
%ignore iGeneralFactoryState::GetTexels;
%ignore iGeneralFactoryState::GetNormals;
%ignore iGeneralFactoryState::GetTriangles;
%ignore iGeneralFactoryState::GetColors;
#endif
%include "imesh/genmesh.h"

%ignore iSprite2DState::GetVertices;
%include "imesh/sprite2d.h"
%include "imesh/sprite3d.h"
%include "imesh/spritecal3d.h"
%include "imesh/mdlconv.h"
// Swig 1.3.24 doesn't handle pointer default args well unless we tell it
// to use an alternate way for that function
%feature("compactdefaultargs") HitBeamObject;
%include "imesh/object.h"
%include "imesh/ball.h"
%include "imesh/thing.h"
%include "imesh/terrain.h"

%include "imap/loader.h"
%include "imap/reader.h"
%include "imap/saver.h"

%include "isound/handle.h"
%include "isound/loader.h"
%include "isound/renderer.h"
%include "isound/wrapper.h"
%include "isound/driver.h"
%include "isound/source.h"
%include "isound/listener.h"

%include "iutil/comp.h"
%include "iutil/cache.h"
#endif // CS_MINI_SWIG
%include "iutil/vfs.h"
#ifndef CS_MINI_SWIG
%include "iutil/dbghelp.h"
%include "iutil/object.h"
%include "iutil/strset.h"
#endif // CS_MINI_SWIG
%include "iutil/objreg.h"
#ifndef CS_MINI_SWIG
%include "iutil/virtclk.h"
#endif // CS_MINI_SWIG

#ifndef CS_MICRO_SWIG
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
%rename(RetrieveInt32) iEvent::Retrieve(const char *, int32 &, bool) const;
%rename(RetrieveUInt8) iEvent::Retrieve(const char *, uint8 &) const;
%rename(RetrieveUInt16) iEvent::Retrieve(const char *, uint16 &) const;
%rename(RetrieveUInt32) iEvent::Retrieve(const char *, uint32 &) const;
%rename(RetrieveFloat) iEvent::Retrieve(const char *, float &) const;
%rename(RetrieveDouble) iEvent::Retrieve(const char *, double &) const;
%rename(RetrieveString) iEvent::Retrieve(const char *, char **) const;
%rename(RetrieveBool) iEvent::Retrieve(const char *, bool &) const;
%rename(RetrieveVoidPtr) iEvent::Retrieve(const char*, void**, size_t&) const;
#pragma SWIG nowarn=312; // nested union not supported

%ignore csJoystickEventHelper::GetX;
%ignore csJoystickEventHelper::GetY;
%include "iutil/event.h"
%include "csutil/event.h"

%include "iutil/evdefs.h"
%include "iutil/eventq.h"
%include "iutil/eventh.h"
%include "iutil/plugin.h"
#endif // CS_MICRO_SWIG

#ifndef CS_MINI_SWIG
%include "iutil/csinput.h"
%include "iutil/cfgfile.h"
%include "iutil/cfgmgr.h"
%include "iutil/stringarray.h"
%include "iutil/document.h"

%include "csutil/xmltiny.h"

%ignore iDataBuffer::GetInt8;
%include "iutil/databuff.h"
#endif // CS_MINI_SWIG

#ifndef CS_MICRO_SWIG
%ignore iGraphics2D::PerformExtensionV;
%rename(GetRGBA) iGraphics2D::GetRGB(int, int&, int&, int&, int&);
%include "ivideo/graph2d.h"
%include "ivideo/graph3d.h"
%include "ivideo/cursor.h"

%ignore iNativeWindowManager::AlertV;
%include "ivideo/natwin.h"

%ignore GetGlyphSize(uint8, int &, int &);
%ignore GetGlyphBitmap(uint8, int &, int &);
%ignore GetGlyphAlphaBitmap(uint8, int &, int &);
%ignore GetDimensions(char const *, int &, int &);
%include "ivideo/fontserv.h"
#endif // CS_MICRO_SWIG

#ifndef CS_MINI_SWIG
%include "ivideo/halo.h"
%include "ivideo/shader/shader.h"

%rename(GetKeyColorStatus) iTextureHandle::GetKeyColor() const;
%include "ivideo/texture.h"

%include "ivideo/txtmgr.h"
%include "ivideo/material.h"

%immutable csStreamDescription::name;
%include "ivideo/codec.h"

%include "igraphic/image.h"

%immutable csImageIOFileFormatDescription::mime;
%immutable csImageIOFileFormatDescription::subtype;
%include "igraphic/imageio.h"

%ignore iReporter::ReportV;
%ignore csReporterHelper::ReportV;
%include "ivaria/reporter.h"

%ignore iConsoleOutput::PutTextV;
%ignore iConsoleOutput::PerformExtensionV;
%include "ivaria/conout.h"

%include "ivaria/stdrep.h"
%include "ivaria/view.h"
%include "ivaria/collider.h"
%include "ivaria/dynamics.h"
%include "ivaria/ode.h"
%include "ivaria/engseq.h"
%include "ivaria/movierecorder.h"
%include "ivaria/mapnode.h"

%rename(IntCall) *::Call(const char*, int&, const char*, ...);
%rename(FloatCall) *::Call(const char*, float&, const char*, ...);
%rename(DoubleCall) *::Call(const char*, double&, const char*, ...);
%rename(StringCall) *::Call(const char*, char**, const char*, ...);
%rename(ObjectCall) *::Call(const char*,csRef<iScriptObject>&,const char*,...);
%rename(StoreInt) iScript::Store(const char*, int);
%rename(StoreFloat) iScript::Store(const char*, float);
%rename(StoreDouble) iScript::Store(const char*, double);
%rename(StoreString) iScript::Store(const char*, const char*);
%rename(StoreObject) iScript::Store(const char*, iStringObject*);
%rename(RetrieveInt) iScript::Retrieve(const char*, int);
%rename(RetrieveFloat) iScript::Retrieve(const char*, float&);
%rename(RetrieveDouble) iScript::Retrieve(const char*, double&);
%rename(RetrieveString) iScript::Retrieve(const char*, char**);
%rename(RetrieveObject) iScript::Retrieve(const char*, csRef<iStringObject>&);
%rename(SetInt) iScriptObject::Set(const char*, int);
%rename(SetFloat) iScriptObject::Set(const char*, float);
%rename(SetDouble) iScriptObject::Set(const char*, double);
%rename(SetString) iScriptObject::Set(const char*, const char*);
%rename(SetObject) iScriptObject::Set(const char*, iStringObject*);
%rename(GetInt) iScriptObject::Get(const char*, int);
%rename(GetFloat) iScriptObject::Get(const char*, float&);
%rename(GetDouble) iScriptObject::Get(const char*, double&);
%rename(GetString) iScriptObject::Get(const char*, char**);
%rename(GetObject) iScriptObject::Get(const char*, csRef<iStringObject>&);
%include "ivaria/script.h"
%include "ivaria/simpleformer.h"
%include "ivaria/terraform.h"

%include "csutil/csobject.h"

%ignore csColliderHelper::TraceBeam (iCollideSystem*, iSector*,
  const csVector3&, const csVector3&, bool, csIntersectingTriangle&,
  csVector3&, iMeshWrapper**);
%include "cstool/collider.h"
%include "cstool/csview.h"
%include "cstool/csfxscr.h"

%ignore csPixmap;
%rename(csPixmap) csSimplePixmap;
%include "cstool/cspixmap.h"

#endif // CS_MINI_SWIG

%define INTERFACE_POST(T)
  %extend T
  {
    virtual ~T() { if (self) self->DecRef(); }
    static int scfGetVersion() { return scfInterfaceTraits<T>::GetVersion(); }
  }
%enddef

#undef INTERFACE_APPLY
#define INTERFACE_APPLY(x) INTERFACE_POST(x)
APPLY_FOR_EACH_INTERFACE

#ifndef CS_MINI_SWIG
// imesh/sprite2d.h
%extend iSprite2DState
{
  csSprite2DVertex* GetVertexByIndex(int index)
  { return &(self->GetVertices()[index]); }

  int GetVertexCount()
  { return self->GetVertices().Length(); }
}

// imesh/genmesh.h
%extend iGeneralFactoryState
{
  csVector3 *GetVertexByIndex(int index)
  { return &(self->GetVertices()[index]); }

  csVector2 *GetTexelByIndex(int index)
  { return &(self->GetTexels()[index]); }

  csVector3 *GetNormalByIndex(int index)
  { return &(self->GetNormals()[index]); }

  csTriangle *GetTriangleByIndex(int index)
  { return &(self->GetTriangles()[index]); }

  csColor *GetColorByIndex(int index)
  { return &(self->GetColors()[index]); }
}

// iaws/aws.h
%extend iAws
{
  bool SetupCanvas (iGraphics2D *g2d=0, iGraphics3D *g3d=0)
  { return self->SetupCanvas(0, g2d, g3d); }
}

// iutil/csinput.h
%extend iKeyboardDriver
{
  bool GetKeyState (const char * key)
  { return self->GetKeyState ((int) key[0]); }
}
#endif // CS_MINI_SWIG

#ifndef CS_MICRO_SWIG
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
#define _CS_IS_KEYBOARD_EVENT(e) CS_IS_KEYBOARD_EVENT(e)
#undef CS_IS_KEYBOARD_EVENT
bool _CS_IS_KEYBOARD_EVENT (const iEvent &);
#define _CS_IS_MOUSE_EVENT(e) CS_IS_MOUSE_EVENT(e)
#undef CS_IS_MOUSE_EVENT
bool _CS_IS_MOUSE_EVENT (const iEvent &);
#define _CS_IS_JOYSTICK_EVENT(e) CS_IS_JOYSTICK_EVENT(e)
#undef CS_IS_JOYSTICK_EVENT
bool _CS_IS_JOYSTICK_EVENT (const iEvent &);
#define _CS_IS_INPUT_EVENT(e) CS_IS_INPUT_EVENT(e)
#undef CS_IS_INPUT_EVENT
bool _CS_IS_INPUT_EVENT (const iEvent &);

// iutil/objreg.h
#define _CS_QUERY_REGISTRY_TAG(a, b) CS_QUERY_REGISTRY_TAG(a, b)
#undef CS_QUERY_REGISTRY_TAG
csPtr<iBase> _CS_QUERY_REGISTRY_TAG (iObjectRegistry *, const char *);

// iutil/plugin.h
#define _CS_LOAD_PLUGIN_ALWAYS(a, b) CS_LOAD_PLUGIN_ALWAYS(a, b)
#undef CS_LOAD_PLUGIN_ALWAYS
csPtr<iBase> _CS_LOAD_PLUGIN_ALWAYS (iPluginManager *, const char *);
#endif // CS_MICRO_SWIG

#ifndef CS_MINI_SWIG
// ivaria/collider.h
%extend iCollideSystem
{
  csCollisionPair * GetCollisionPairByIndex (int index)
  { return self->GetCollisionPairs() + index; }
}
#endif // CS_MINI_SWIG

#ifndef CS_MICRO_SWIG
// ivideo/graph3d.h
#define _CS_FX_SETALPHA(a) CS_FX_SETALPHA(a)
#undef CS_FX_SETALPHA
uint _CS_FX_SETALPHA (uint);
#define _CS_FX_SETALPHA_INT(a) CS_FX_SETALPHA_INT(a)
#undef CS_FX_SETALPHA_INT
uint _CS_FX_SETALPHA_INT (uint);
#endif // CS_MICRO_SWIG

#ifndef CS_MINI_SWIG
// csgeom/vector2.h csgeom/vector3.h
%define VECTOR_OBJECT_FUNCTIONS(V)
  V operator + (const V & v) const { return *self + v; }
  V operator - (const V & v) const { return *self - v; }
  float operator * (const V & v) const { return *self * v; }
  V operator * (float f) const { return *self * f; }
  V operator / (float f) const { return *self * f; }
  bool operator == (const V & v) const { return *self == v; }
  bool operator != (const V & v) const { return *self != v; }
  bool operator < (float f) const { return *self < f; }
  bool operator > (float f) const { return f > *self; }
%enddef

// csgeom/vector2.h
%extend csVector2
{
  VECTOR_OBJECT_FUNCTIONS(csVector2)
}

// csgeom/vector3.h
%extend csVector3
{
  VECTOR_OBJECT_FUNCTIONS(csVector3)
  csVector3& operator *=(const csTransform& t) { return *self *= t; }
  csVector3& operator /=(const csReversibleTransform& t) { return *self /= t; }
  csVector3 operator /(const csReversibleTransform& t) { return *self / t; }
  csVector3 project(const csVector3& what) const { return what << *self; }
}

%define BOX_OBJECT_FUNCTIONS(B)
  B operator + (const B & b) const { return *self + b; }
  B operator * (const B & b ) const { return *self * b; }
  bool operator != (const B & b ) const { return *self != b; }
  bool operator < (const B & b ) const { return *self < b; }
  bool operator > (const B & b ) const { return b > *self; }
%enddef

//csgeom/box.h
%extend csBox2
{
  BOX_OBJECT_FUNCTIONS(csBox2)
  csBox2 operator + (const csVector2 & point) const { return *self + point; }
  bool operator < ( const csVector2 & point ) const { return point < *self; }
}

%extend csBox3
{
  BOX_OBJECT_FUNCTIONS(csBox3)
  csBox3 operator + (const csVector3 & point) const { return *self + point; }
  bool operator < ( const csVector3 & point ) const { return point < *self; }

}

// csgeom/plane3.h
%extend csPlane3
{
  csPlane3& operator *=(const csTransform& t) { return *self *= t; }
  csPlane3& operator /=(const csReversibleTransform& t) { return *self /= t; }
  csPlane3 operator /(const csReversibleTransform& t) { return *self / t; }
}

// csgeom/sphere.h
%extend csSphere
{
  csSphere & operator *= (const csTransform & t) { return *self *= t; }
  csSphere operator / (const csReversibleTransform & t) { return *self / t; }
}

// csgeom/matrix3.h
%extend csMatrix3
{
  csMatrix3 operator + (const csMatrix3 & m) { return *self + m; }
  csMatrix3 operator - (const csMatrix3 & m) { return *self - m; }
  csMatrix3 operator * (const csMatrix3 & m) { return *self * m; }
  csVector3 operator * (const csVector3 & v) { return *self * v; }
  csMatrix3 operator * (float f) { return *self * f; }
  csMatrix3 operator / (float f) { return *self / f; }
  bool operator == (const csMatrix3 & m) const { return *self == m; }
  bool operator != (const csMatrix3 & m) const { return *self != m; }
  bool operator < (float f) const { return *self < f; }
  csMatrix3 operator * (const csTransform & t) const { return *self * t; }
  csMatrix3 & operator *= (const csTransform & t) { return *self *= t; }
}

// csgeom/transfrm.h
%extend csTransform
{
  csVector3 operator * (const csVector3 & v) const { return *self * v; } 
  csPlane3 operator * (const csPlane3 & p) const { return *self * p; } 
  csSphere operator * (const csSphere & s) const { return *self * s; } 
  csMatrix3 operator * (const csMatrix3 & m) const { return *self * m; } 
  csTransform operator * (const csReversibleTransform & t) const
    { return *self * t; } 
}

// csgeom/transfrm.h
%extend csReversibleTransform
{
  csReversibleTransform & operator *= (const csReversibleTransform & t)
    { return *self *= t; }
  csReversibleTransform operator * (const csReversibleTransform & t)
    { return *self * t; }
  csReversibleTransform & operator /= (const csReversibleTransform & t)
    { return *self /= t; }
  csReversibleTransform operator / (const csReversibleTransform & t)
    { return *self / t; }
}

// csutil/cscolor.h
%extend csColor
{
  csColor operator + (const csColor & c) const { return *self + c; }
  csColor operator - (const csColor & c) const { return *self - c; }
}

// csgeom/quaterni.h
%extend csQuaternion
{
  csQuaternion operator + (const csQuaternion& q) { return *self + q; }
  csQuaternion operator - (const csQuaternion& q) { return *self - q; }
  csQuaternion operator * (const csQuaternion& q) { return *self * q; }
}
#endif // CS_MINI_SWIG

/****************************************************************************
 * These functions are replacements for CS's macros of the same names.
 * These functions can be wrapped by Swig but the macros can't.
 ****************************************************************************/
%inline %{
#undef SCF_QUERY_INTERFACE
#undef SCF_QUERY_INTERFACE_SAFE
#undef CS_QUERY_REGISTRY
#undef CS_QUERY_REGISTRY_TAG_INTERFACE
#undef CS_QUERY_PLUGIN_CLASS
#undef CS_LOAD_PLUGIN
#undef CS_GET_CHILD_OBJECT
#undef CS_GET_NAMED_CHILD_OBJECT
#undef CS_GET_FIRST_NAMED_CHILD_OBJECT

csWrapPtr CS_QUERY_REGISTRY (iObjectRegistry *reg, const char *iface,
  int iface_ver)
{
  iBase *b = reg->Get(iface, iSCF::SCF->GetInterfaceID (iface), iface_ver);
  return csWrapPtr (iface, iface_ver, b);
}

csWrapPtr CS_QUERY_REGISTRY_TAG_INTERFACE (iObjectRegistry *reg,
  const char *tag, const char *iface, int iface_ver)
{
  iBase *b = reg->Get(tag, iSCF::SCF->GetInterfaceID (iface), iface_ver);
  return csWrapPtr (iface, iface_ver, b);
}

csWrapPtr SCF_QUERY_INTERFACE (iBase *obj, const char *iface, int iface_ver)
{
  // This call to QueryInterface ensures that IncRef is called and that
  // the object supports the interface.  However, for type safety and
  // object layout reasons the void pointer returned by QueryInterface
  // can't be wrapped inside the csWrapPtr so obj must be wrapped.
  if (obj->QueryInterface(iSCF::SCF->GetInterfaceID(iface), iface_ver))
    return csWrapPtr (iface, iface_ver, obj);
  else
    return csWrapPtr (iface, iface_ver, 0);
}

csWrapPtr SCF_QUERY_INTERFACE_SAFE (iBase *obj, const char *iface,
  int iface_ver)
{
  if (!obj)
    return csWrapPtr (iface, iface_ver, 0);

  // This call to QueryInterface ensures that IncRef is called and that
  // the object supports the interface.  However, for type safety and
  // object layout reasons the void pointer returned by QueryInterface
  // can't be wrapped inside the csWrapPtr so obj must be wrapped.
  if (obj->QueryInterface(iSCF::SCF->GetInterfaceID(iface), iface_ver))
    return csWrapPtr (iface, iface_ver, obj);
  else
    return csWrapPtr (iface, iface_ver, 0);
}

csWrapPtr CS_QUERY_PLUGIN_CLASS (iPluginManager *obj, const char *id,
  const char *iface, int iface_ver)
{
  return csWrapPtr (iface, iface_ver, obj->QueryPlugin (id, iface, iface_ver));
}

csWrapPtr CS_LOAD_PLUGIN (iPluginManager *obj, const char *id,
  const char *iface, int iface_ver)
{
  return csWrapPtr (iface, iface_ver, obj->LoadPlugin(id));
}

csWrapPtr CS_GET_CHILD_OBJECT (iObject *obj, const char *iface, int iface_ver)
{
  return csWrapPtr (iface, iface_ver, obj->GetChild(iSCF::SCF->GetInterfaceID (iface),
    iface_ver));
}

csWrapPtr CS_GET_NAMED_CHILD_OBJECT (iObject *obj, const char *iface,
  int iface_ver, const char *name)
{
  return csWrapPtr (iface, iface_ver, obj->GetChild(iSCF::SCF->GetInterfaceID (iface),
    iface_ver, name));
}

csWrapPtr CS_GET_FIRST_NAMED_CHILD_OBJECT (iObject *obj, const char *iface,
  int iface_ver, const char *name)
{
  return csWrapPtr (iface, iface_ver, obj->GetChild(iSCF::SCF->GetInterfaceID (iface),
    iface_ver, name, true));
}
%}

#if defined(SWIGPYTHON)
  %include "ivaria/pythpost.i"
#elif defined(SWIGPERL5)
  %include "ivaria/perlpost.i"
#elif defined(SWIGRUBY)
  %include "ivaria/rubypost.i"
#elif defined(SWIGTCL8)
  %include "ivaria/tclpost.i"
#elif defined(SWIGJAVA)
  %include "ivaria/javapost.i"
#endif
