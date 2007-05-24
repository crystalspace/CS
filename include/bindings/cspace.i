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

%module cspace

%include "bindings/allinterfaces.i"

#undef APPLY_FOR_ALL_INTERFACES
#define APPLY_FOR_ALL_INTERFACES CORE_APPLY_FOR_EACH_INTERFACE

/****************************************************************************/
%include "bindings/basepre.i"
/****************************************************************************/

/* Inclusion of CS headers.  The sequence of %include-ing the CS headers can be
   crucial!  The scheme is as follows: %ignore'd functions and types are placed
   before actual inclusion, as are "local" %typemap's (like default).  After
   %include-ing the header resets can be done; for example resetting the
   %typemap(default). The %extend-ing and extra code takes place after all
   %include's are done, mentioning the header(s) it is related to.	    */

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
%ignore csSet::GlobalIterator;
%ignore csSet::GetIterator;
%include "csutil/set.h"
// helper macro to declare csSet templated classes
%define SET_HELPER(typename)
%template (typename ## Set) csSet<typename>;
SET_OBJECT_FUNCTIONS(csSet<typename>,typename)
%enddef
SET_HELPER(csStringID)
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

%ignore csStringBase;
%ignore csStringBase::operator [] (size_t);
%ignore csStringBase::operator [] (size_t) const;
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

%include "csutil/refcount.h"
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

template<typename T> struct csVector4T {
    T x,y,z,w;
};
%warnfilter(302) csVector4T; // csVector4T redefined
%ignore csVector4T::operator[];
%template(csVector4Float) csVector4T<float >;
%include "csgeom/vector4.h"

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

/* Swig 1.3.24 doesn't handle pointer default args well unless we tell it
   to use an alternate way for those functions				    */
%feature("compactdefaultargs") csIntersect3::BoxSegment;
%include "csgeom/math3d.h"

%ignore csPoly3D::operator[];
%ignore csPoly3D::GetVertices (); // Non-const.
%include "csgeom/poly3d.h"

namespace CS
{
  template<typename T> struct TriangleT
  { 
    T a, b, c;
    void Set (const T& _a, const T& _b, const T& _c);
  };
}
%template(TriangleInt) CS::TriangleT<int >;
%warnfilter(302) TriangleT; // redefined
%ignore CS::TriangleT::operator[];
%include "csgeom/tri.h"

%ignore csRect::AddAdjanced; // Deprecated misspelling.
%include "csgeom/csrect.h"
%include "csgeom/csrectrg.h"

%ignore csQuaternion::operator+ (const csQuaternion &, const csQuaternion &);
%ignore csQuaternion::operator- (const csQuaternion &, const csQuaternion &);
%ignore csQuaternion::operator* (const csQuaternion &, const csQuaternion &);
%include "csgeom/quaternion.h"

%include "csgeom/spline.h"

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

/*
%rename(asRGBcolor) csRGBpixel::operator csRGBcolor;
%include "csgfx/rgbpixel.h"
%ignore ShaderVarName;
%include "csgfx/shadervar.h"
%template(csShaderVariableArrayReadOnly) iArrayReadOnly<csShaderVariable * >;
%template(csShaderVariableArrayChangeElements) 
iArrayChangeElements<csShaderVariable * >;
%template(csShaderVariableArray) iArrayChangeAll<csShaderVariable * >;
*/

%ignore csGetPlatformConfig;
%ignore csPrintfV;
%ignore csFPrintfV;
%ignore csPrintfErrV;
%include "csutil/sysfunc.h"

%ignore csInitializer::RequestPlugins(iObjectRegistry*, ...);
%ignore csInitializer::RequestPluginsV;
%rename (_RequestPlugins) csInitializer::RequestPlugins(iObjectRegistry*,
  csArray<csPluginRequest> const&);

%ignore csInitializer::SetupEventHandler (iObjectRegistry*, csEventHandlerFunc,
  const csEventID events[]);
%ignore csInitializer::SetupEventHandler (iObjectRegistry*, csEventHandlerFunc);
%rename (_SetupEventHandler) csInitializer::SetupEventHandler (iObjectRegistry*,
  iEventHandler *, const csEventID[]);
%typemap(default) const char * configName { $1 = 0; }
/* Swig 1.3.24 doesn't handle pointer default args well unless we tell it
   to use an alternate way for that function				    */
%feature("compactdefaultargs") csInitializer::SetupConfigManager;
%include "cstool/initapp.h"
%typemap(default) const char * configName;

%include "csutil/customallocated.h"
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
/*

%ignore iPen::Rotate;
*/
%include "igeom/clip2d.h"
%include "igeom/path.h"
%template(scfPath) scfImplementation1<csPath,iPath >;
#ifndef CS_SWIG_PUBLISH_IGENERAL_FACTORY_STATE_ARRAYS
%ignore iPolygonMesh::GetTriangles;
%ignore iPolygonMesh::GetVertices;
%ignore iPolygonMesh::GetPolygons;
%ignore iTriangleMesh::GetTriangles;
%ignore iTriangleMesh::GetVertices;
#endif
%include "igeom/polymesh.h"
%include "igeom/trimesh.h"
// Ignore some deprecated functions
%ignore csPath::GetPointCount;
%ignore csPath::GetTimeValue;
%ignore csPath::SetTimeValues;
%ignore csPath::GetTimeValues;
%include "csgeom/path.h"
%template(pycsPolygonMesh) scfImplementation1<csPolygonMesh, iPolygonMesh>;
%template(pycsPolygonMeshBox) scfImplementation1<csPolygonMeshBox, iPolygonMesh>;
%template(pycsTriangleMesh) scfImplementation1<csTriangleMesh, iTriangleMesh>;
%template(pycsTriangleMeshBox) scfImplementation1<csTriangleMeshBox, iTriangleMesh>;
%include "csgeom/polymesh.h"
%include "csgeom/trimesh.h"

%ignore csArray<csArray<int> >::Contains;
%template(csIntArray) csArray<int>;
%template(csIntArrayArray) csArray<csArray<int> >;
ARRAY_OBJECT_FUNCTIONS(csArray<int>,int)
ARRAY_OBJECT_FUNCTIONS(csArray<csArray<int> >,csArray<int>)
%newobject csPolygonMeshTools::CalculateVertexConnections;
%newobject csTriangleMeshTools::CalculateVertexConnections;
%include "csgeom/pmtools.h"
%include "csgeom/trimeshtools.h"
%include "csgeom/spline.h"

/*
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
%template(iSceneNodeArrayReadOnly) iArrayReadOnly<iSceneNode * >;
%include "iengine/scenenode.h"

*/
/* Swig 1.3.24 doesn't handle pointer default args well unless we tell it
   to use an alternate way for that function				    */
/*
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
*/
/* Swig 1.3.24 doesn't handle pointer default args well unless we tell it
   to use an alternate way for that function				    */
/*
%feature("compactdefaultargs") IntersectSegment;
%include "iengine/viscull.h"
%include "iengine/portal.h"
%include "iengine/portalcontainer.h"

%extend iVisibilityObjectIterator {
  ITERATOR_FUNCTIONS(iVisibilityObjectIterator)
}
%extend iLightIterator {
  ITERATOR_FUNCTIONS(iLightIterator)
}
%extend iSectorIterator {
  ITERATOR_FUNCTIONS(iSectorIterator)
}
%extend iMeshWrapperIterator {
  ITERATOR_FUNCTIONS(iMeshWrapperIterator)
}
*/
%ignore iReporter::ReportV;
%ignore csReporterHelper::ReportV;
%include "ivaria/reporter.h"

#if defined(SWIGPYTHON)
%pythoncode %{
  csReport = csReporterHelper.Report
%}
#endif

/* %include "bindings/imesh.i" */
/* %include "bindings/imap.i" */
/* %include "bindings/isndsys.i" */

%include "iutil/comp.h"
%include "iutil/cache.h"
%include "iutil/vfs.h"
%include "iutil/dbghelp.h"
%include "iutil/object.h"
%include "iutil/strset.h"
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
%include "csutil/hash.h"
%include "iutil/eventnames.h"
%include "csutil/eventnames.h"
%include "iutil/eventh.h"
%include "iutil/plugin.h"

%include "iutil/csinput.h"
%include "iutil/cfgfile.h"
%include "iutil/cfgmgr.h"
%include "iutil/stringarray.h"
%include "iutil/document.h"

#if defined(SWIGPYTHON)
%include "bindings/python/pyeventh.i"
%pythoncode %{
  def _csInitializer_RequestPlugins (reg, plugins):
    """Replacement of C++ version."""
    def _get_tuple (x):
      if callable(x):
        return tuple(x())
      else:
        return tuple(x)
    requests = csPluginRequestArray()
    for cls, intf, ident, ver in map(
        lambda x: _get_tuple(x), plugins):
      requests.Push(csPluginRequest(
        csString(cls), csString(intf), ident, ver))
    return csInitializer._RequestPlugins(reg, requests)

  csInitializer.RequestPlugins = staticmethod(_csInitializer_RequestPlugins)
%}
%pythoncode %{
  def _csInitializer_SetupEventHandler (reg, obj,
      eventids=None):
    """Replacement of C++ versions."""
    if callable(obj):
      # obj is a function
      hdlr = _EventHandlerFuncWrapper(obj)
      hdlr.thisown = 1
    else:
      # assume it is a iEventHandler
      hdlr = obj
    if eventids==None:
      eventids=[csevFrame(reg), csevInput(reg), csevKeyboardEvent(reg), \
                csevMouseEvent(reg), csevQuit(reg), CS_EVENTLIST_END]
    return csInitializer._SetupEventHandler(reg, hdlr, eventids)

  csInitializer.SetupEventHandler = \
    staticmethod(_csInitializer_SetupEventHandler)
%}
#endif

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
/*
%ignore iGraphics2D::PerformExtensionV;
%ignore iGraphics3D::PerformExtensionV;
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

%include "ivideo/halo.h"
%include "ivideo/shader/shader.h"

%rename(GetKeyColorStatus) iTextureHandle::GetKeyColor() const;
%include "ivideo/texture.h"

%include "ivideo/txtmgr.h"
%include "ivideo/material.h"
*/
%include "igraphic/image.h"
/*
%template(csImageBaseBase) scfImplementation1<csImageBase, iImage>;
%include "csgfx/imagebase.h"
%template(csImageMemoryBase) scfImplementationExt0<csImageMemory, csImageBase>;
%include "csgfx/imagememory.h"
*/
%immutable csImageIOFileFormatDescription::mime;
%immutable csImageIOFileFormatDescription::subtype;
%template (csImageIOFileFormatDescriptions) csArray<csImageIOFileFormatDescription const*>;
%include "igraphic/imageio.h"

%template(pycsObject) scfImplementation1<csObject,iObject >;
%include "csutil/csobject.h"
/*
%ignore csColliderHelper::TraceBeam (iCollideSystem*, iSector*,
  const csVector3&, const csVector3&, bool, csIntersectingTriangle&,
  csVector3&, iMeshWrapper**);
%template(pycsColliderWrapper) scfImplementationExt1<csColliderWrapper,csObject,scfFakeInterface<csColliderWrapper> >;
%include "cstool/collider.h"
%include "cstool/csview.h"
%include "cstool/csfxscr.h"

%include "cstool/cspixmap.h"
%include "cstool/enginetools.h"

%include "cstool/pen.h"
*/

/****************************************************************************/
%include "bindings/basepost.i"
/****************************************************************************/

%extend iPolygonMesh
{
  csVector3 *GetVertexByIndex(int index)
  { return &(self->GetVertices()[index]); }

  csMeshedPolygon *GetPolygonByIndex(int index)
  { return &(self->GetPolygons()[index]); }

  csTriangle *GetTriangleByIndex(int index)
  { return &(self->GetTriangles()[index]); }
}

%extend iTriangleMesh
{
  csVector3 *GetVertexByIndex(int index)
  { return &(self->GetVertices()[index]); }

  csTriangle *GetTriangleByIndex(int index)
  { return &(self->GetTriangles()[index]); }
}

%extend csMeshedPolygon
{
  int GetVertexByIndex(int index)
  { return self->vertices[index]; }
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

/* Process */
#define _csevProcess(reg) csevProcess(reg)
#undef csevProcess
csEventID _csevProcess (iObjectRegistry *);
#define _csevPreProcess(reg) csevPreProcess(reg)
#undef csevPreProcess
csEventID _csevPreProcess (iObjectRegistry *);
#define _csevPostProcess(reg) csevPostProcess(reg)
#undef csevPostProcess
csEventID _csevPostProcess (iObjectRegistry *);
#define _csevFinalProcess(reg) csevFinalProcess(reg)
#undef csevFinalProcess
csEventID _csevFinalProcess (iObjectRegistry *);

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

// iutil/plugin.h
#define _CS_LOAD_PLUGIN_ALWAYS(a, b) CS_LOAD_PLUGIN_ALWAYS(a, b)
#undef CS_LOAD_PLUGIN_ALWAYS
csPtr<iBase> _CS_LOAD_PLUGIN_ALWAYS (iPluginManager *, const char *);

/*
// ivideo/graph3d.h
#define _CS_FX_SETALPHA(a) CS_FX_SETALPHA(a)
#undef CS_FX_SETALPHA
uint _CS_FX_SETALPHA (uint);
#define _CS_FX_SETALPHA_INT(a) CS_FX_SETALPHA_INT(a)
#undef CS_FX_SETALPHA_INT
uint _CS_FX_SETALPHA_INT (uint);
*/

// csgeom/vector2.h csgeom/vector3.h
%define VECTOR_OBJECT_FUNCTIONS(V)
  V operator + (const V & v) const { return *self + v; }
  V operator - (const V & v) const { return *self - v; }
  float operator * (const V & v) const { return *self * v; }
  V operator * (float f) const { return *self * f; }
  V operator / (float f) const { return *self / f; }
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
// csgeom/vector4.h
%extend csVector4
{
  VECTOR_OBJECT_FUNCTIONS(csVector4)
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

%include "cstool/primitives.h"
/*
%extend csSimpleRenderMesh
{
  void SetWithGenmeshFactory(iGeneralFactoryState *factory) 
  {
    self->vertices = factory->GetVertices(); 
    self->vertexCount = factory->GetVertexCount(); 
    self->indices = (uint *)factory->GetTriangles();
    self->indexCount = factory->GetTriangleCount()*3; 
    self->texcoords = factory->GetTexels();
  }
}
*/
/* List Methods */
/*
LIST_OBJECT_FUNCTIONS(iMeshList,iMeshWrapper)
LIST_OBJECT_FUNCTIONS(iMeshFactoryList,iMeshFactoryWrapper)
LIST_OBJECT_FUNCTIONS(iMaterialList,iMaterialWrapper)
LIST_OBJECT_FUNCTIONS(iRegionList,iRegion)
LIST_OBJECT_FUNCTIONS(iLightList,iLight)
LIST_OBJECT_FUNCTIONS(iCameraPositionList,iCameraPosition)
LIST_OBJECT_FUNCTIONS(iSectorList,iSector)
LIST_OBJECT_FUNCTIONS(iTextureList,iTextureWrapper)
// Not wrapping yet:
//LIST_OBJECT_FUNCTIONS(iCollectionList,iCollection) 
//LIST_OBJECT_FUNCTIONS(iSharedVariableList,iSharedVariable)
*/

#if defined(SWIGPYTHON)
%pythoncode %{

  CS_VEC_FORWARD = csVector3(0,0,1)
  CS_VEC_BACKWARD = csVector3(0,0,-1)
  CS_VEC_RIGHT = csVector3(1,0,0)
  CS_VEC_LEFT = csVector3(-1,0,0)
  CS_VEC_UP = csVector3(0,1,0)
  CS_VEC_DOWN = csVector3(0,-1,0)
  CS_VEC_ROT_RIGHT = csVector3(0,1,0)
  CS_VEC_ROT_LEFT = csVector3(0,-1,0)
  CS_VEC_TILT_RIGHT = -csVector3(0,0,1)
  CS_VEC_TILT_LEFT = -csVector3(0,0,-1) 
  CS_VEC_TILT_UP = -csVector3(1,0,0)
  CS_VEC_TILT_DOWN = -csVector3(-1,0,0)

%}
#endif

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
  csPtr<iBase> b (reg->Get(iface, iSCF::SCF->GetInterfaceID(iface), iface_ver));
  return csWrapPtr (iface, iface_ver, b);
}

csWrapPtr CS_QUERY_REGISTRY_TAG_INTERFACE (iObjectRegistry *reg,
  const char *tag, const char *iface, int iface_ver)
{
  csPtr<iBase> b (reg->Get(tag, iSCF::SCF->GetInterfaceID(iface), iface_ver));
  return csWrapPtr (iface, iface_ver, b);
}

csWrapPtr SCF_QUERY_INTERFACE (iBase *obj, const char *iface, int iface_ver)
{
  // This call to QueryInterface ensures that IncRef is called and that
  // the object supports the interface.  However, for type safety and
  // object layout reasons the void pointer returned by QueryInterface
  // can't be wrapped inside the csWrapPtr so obj must be wrapped.
  if (obj->QueryInterface(iSCF::SCF->GetInterfaceID(iface), iface_ver))
    return csWrapPtr (iface, iface_ver, csPtr<iBase> (obj));
  else
    return csWrapPtr (iface, iface_ver, csPtr<iBase> (0));
}

csWrapPtr SCF_QUERY_INTERFACE_SAFE (iBase *obj, const char *iface,
  int iface_ver)
{
  if (!obj)
    return csWrapPtr (iface, iface_ver, csPtr<iBase> (0));

  // This call to QueryInterface ensures that IncRef is called and that
  // the object supports the interface.  However, for type safety and
  // object layout reasons the void pointer returned by QueryInterface
  // can't be wrapped inside the csWrapPtr so obj must be wrapped.
  if (obj->QueryInterface(iSCF::SCF->GetInterfaceID(iface), iface_ver))
    return csWrapPtr (iface, iface_ver, csPtr<iBase> (obj));
  else
    return csWrapPtr (iface, iface_ver, csPtr<iBase> (0));
}

csWrapPtr CS_QUERY_PLUGIN_CLASS (iPluginManager *obj, const char *id,
  const char *iface, int iface_ver)
{
  return csWrapPtr (iface, iface_ver,
    csPtr<iBase> (obj->QueryPlugin (id, iface, iface_ver)));
}

csWrapPtr CS_LOAD_PLUGIN (iPluginManager *obj, const char *id,
  const char *iface, int iface_ver)
{
  return csWrapPtr (iface, iface_ver, csPtr<iBase> (obj->LoadPlugin (id)));
}

csWrapPtr CS_GET_CHILD_OBJECT (iObject *obj, const char *iface, int iface_ver)
{
  return csWrapPtr (iface, iface_ver, csRef<iBase> (
    obj->GetChild(iSCF::SCF->GetInterfaceID (iface), iface_ver)));
}

csWrapPtr CS_GET_NAMED_CHILD_OBJECT (iObject *obj, const char *iface,
  int iface_ver, const char *name)
{
  return csWrapPtr (iface, iface_ver, csRef<iBase> (
    obj->GetChild(iSCF::SCF->GetInterfaceID (iface), iface_ver, name)));
}

csWrapPtr CS_GET_FIRST_NAMED_CHILD_OBJECT (iObject *obj, const char *iface,
  int iface_ver, const char *name)
{
  return csWrapPtr (iface, iface_ver, csRef<iBase> (
    obj->GetChild(iSCF::SCF->GetInterfaceID (iface), iface_ver, name, true)));
}
%}

