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
ARRAY_CHANGE_ALL_TEMPLATE(csVector3)

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

// Swig 1.3.24 doesn't handle pointer default args well unless we tell it
// to use an alternate way for those functions
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
%ignore csOBB::Diameter;
%include "csgeom/obb.h"

%ignore csSegment2::Start (); // Non-const.
%ignore csSegment2::End ();   // Non-const.
%ignore csSegment3::Start (); // Non-const.
%ignore csSegment3::End ();   // Non-const.
%include "csgeom/segment.h"

/*Ignore some deprecated functions*/
%ignore csPath::GetPointCount;
%ignore csPath::GetTimeValue;
%ignore csPath::SetTimeValues;
%ignore csPath::GetTimeValues;
%include "csgeom/path.h"
%template(pycsTriangleMesh) scfImplementation1<csTriangleMesh, iTriangleMesh>;
%template(pycsTriangleMeshBox) scfImplementation1<csTriangleMeshBox, iTriangleMesh>;
%template (scfTriangleMeshPointer) scfImplementation1<csTriangleMeshPointer,iTriangleMesh >;
%include "csgeom/trimesh.h"

%ignore csArray<csArray<int> >::Contains;
%template(csIntArray) csArray<int>;
%template(csIntArrayArray) csArray<csArray<int> >;
ARRAY_OBJECT_FUNCTIONS(csArray<int>,int)
ARRAY_OBJECT_FUNCTIONS(csArray<csArray<int> >,csArray<int>)
%newobject csTriangleMeshTools::CalculateVertexConnections;
%include "csgeom/trimeshtools.h"

%include "csgeom/spline.h"

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

  csVector3 Cross (const csVector3& other) const { return *self % other; }
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

/* POST */
#ifndef SWIGIMPORTED
#undef APPLY_FOR_ALL_INTERFACES_POST
#define APPLY_FOR_ALL_INTERFACES_POST
#endif

%include "bindings/common/basepost.i"

#ifndef SWIGIMPORTED
cs_apply_all_interfaces
#endif

cs_lang_include(csgeompost.i)

