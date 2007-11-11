#ifdef SWIGCSHARP

#ifndef CS_MINI_SWIG

%define VECTOR2_CSHARP_CODE
%typemap(cscode) csVector2
%{
  // Perform the addition of two vectors
  public static csVector2 operator+(csVector2 v1, csVector2 v2)
  {
    return new csVector2(v1.x+v2.x, v1.y+v2.y);
  }

  // Perform the substraction of two vectors
  public static csVector2 operator-(csVector2 v1, csVector2 v2)
  {
    return new csVector2(v1.x-v2.x, v1.y-v2.y);
  }

  // Perform the dot product between two vectors
  public static float operator*(csVector2 v1, csVector2 v2)
  {
    return v1.x*v2.x + v1.y*v2.y;
  }

  // Perform the scalar product
  public static csVector2 operator*(csVector2 v, float s)
  {
    return new csVector2(v.x*s, v.y*s);
  }

  // Perform the scalar product
  public static csVector2 operator*(float s, csVector2 v)
  {
    return new csVector2(v.x*s, v.y*s);
  }

  // Perform the scalar divition
  public static csVector2 operator/(csVector2 v, float s)
  {
    return new csVector2(v.x/s, v.y/s);
  }

  // Are the two vectors equal?
  public new static bool Equals(object vo1, object vo2)
  {
    if(!(vo1 is csVector2) || !(vo2 is csVector2))
      return false;
 
    csVector2 v1 = (csVector2)vo1;
    csVector2 v2 = (csVector2)vo2;

    // If one of the objects is null, then compares by reference
    if (vo1 == null || vo2 == null)
      return vo1 == vo2;
    else
      return v1.x==v2.x && v1.y==v2.y;
  }

  public new static int GetHashCode(object vo)
  {
    // This probably isn't the best form for calculate the hash code
    csVector2 v = (csVector2) vo;
    return (int)((v.x.GetHashCode() & 0xff00ff) | ((v.y.GetHashCode() & 0xff00ff)<<8));
  }

  public override bool Equals(object o)
  {
    return csVector2.Equals(this, o);
  }

  public override int GetHashCode()
  {
    return csVector2.GetHashCode(this);
  }

  public static bool operator==(csVector2 v1, csVector2 v2)
  {
    // If one of the objects is null, then compares by reference
    if ((object)v1 == null || (object)v2 == null)
      return (object)v1 == (object)v2;
    else
      return v1.x==v2.x && v1.y==v2.y;
  }


  // Are the two vectors differents?
  public static bool operator!=(csVector2 v1, csVector2 v2)
  {
    // If one of the objects is null, then compares by reference
    if ((object)v1 == null || (object)v2 == null)
      return (object)v1 != (object)v2;
    else
      return v1.x!=v2.x || v1.y!=v2.y;
  }

  // Perform the comparation between each vector member and a epsilon value
  public static bool operator<(csVector2 v, float epsilon)
  {
    return Math.Abs(v.x)<epsilon && Math.Abs(v.y)<epsilon;
  }
    
  // Perform the comparation between each vector member and a epsilon value
  public static bool operator>(csVector2 v, float epsilon)
  {
    return Math.Abs(v.x)>epsilon && Math.Abs(v.y)>epsilon;
  }
%}
%enddef
VECTOR2_CSHARP_CODE

%define VECTOR3_CSHARP_CODE
%typemap(cscode) csVector3
%{

  // Perform the addition of two vectors
  public static csVector3 operator+(csVector3 v1, csVector3 v2)
  {
    return new csVector3(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z);
  }

  // Perform the substraction of two vectors
  public static csVector3 operator-(csVector3 v1, csVector3 v2)
  {
    return new csVector3(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z);
  }

  // Perform the dot product between two vectors
  public static float operator*(csVector3 v1, csVector3 v2)
  {
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
  }
    
  // Perform the cross product between two vectors
  public static csVector3 operator%(csVector3 v1, csVector3 v2)
  {
   return new csVector3(v1.y*v2.z - v1.z*v2.y,
		        v1.z*v2.x - v1.x*v2.z,
		        v1.x*v2.y - v1.x*v2.y);
  }

  // Perform the scalar product
  public static csVector3 operator*(csVector3 v, float s)
  {
    return new csVector3(v.x*s, v.y*s, v.z*s);
  }

  // Perform the scalar product
  public static csVector3 operator*(float s, csVector3 v)
  {
    return new csVector3(v.x*s, v.y*s, v.z*s);
  }
  // Perform the scalar product
  public static csVector3 operator*(csVector3 v, int s)
  {
    return new csVector3(v.x*s, v.y*s, v.z*s);
  }

  // Perform the scalar product
  public static csVector3 operator*(int s, csVector3 v)
  {
    return new csVector3(v.x*s, v.y*s, v.z*s);
  }

  // Perform the scalar divition
  public static csVector3 operator/(csVector3 v, float s)
  {
    return new csVector3(v.x/s, v.y/s, v.z/s);
  }

  // Perform the scalar divition
  public static csVector3 operator/(csVector3 v, int s)
  {
    return new csVector3(v.x/s, v.y/s, v.z/s);
  }

  // Are the two vectors equal?
  public new static bool Equals(object vo1, object vo2)
  {
    if(!(vo1 is csVector3) || !(vo2 is csVector3))
      return false;
 
    csVector3 v1 = (csVector3)vo1;
    csVector3 v2 = (csVector3)vo2;

    // If one of the objects is null, then compares by reference
    if (vo1 == null || vo2 == null)
      return vo1 == vo2;
    else
      return v1.x==v2.x && v1.y==v2.y;
  }

  public override bool Equals(object o)
  {
    return csVector3.Equals(this, o);
  }

  public override int GetHashCode()
  {
    return csVector3.GetHashCode(this);
  }

  public new static int GetHashCode(object vo)
  {
    // This probably isn't the best form for calculate the hash code
    csVector3 v = (csVector3)vo;
    return (int)((v.x.GetHashCode() & 0xfff) | ((v.y.GetHashCode() & 0xfff)<<12) | (v.y.GetHashCode() &0xff000000));
  }

  // Are the two vectors equal?
  public static bool operator==(csVector3 v1, csVector3 v2)
  {
    // If one of the objects is null, then compares by reference
    if ((object)v1 == null || (object)v2 == null)
      return (object)v1 == (object)v2;
    else
      return v1.x==v2.x && v1.y==v2.y && v1.z==v2.z;
  }

  // Are the two vectors differents?
  public static bool operator!=(csVector3 v1, csVector3 v2)
  {
    // If one of the objects is null, then compares by reference
    if ((object)v1 == null || (object)v2 == null)
      return (object)v1 != (object)v2;
    else
      return v1.x!=v2.x || v1.y!=v2.y || v1.z!=v2.z;
  }

  public static bool operator<(csVector3 v, float epsilon)
  {
    return Math.Abs(v.x) < epsilon && Math.Abs(v.y) < epsilon 
           && Math.Abs(v.z) < epsilon;
  }

  public static bool operator>(csVector3 v, float epsilon)
  {
    return Math.Abs(v.x) > epsilon && Math.Abs(v.y) > epsilon 
           && Math.Abs(v.z) > epsilon;
  }
%}
%enddef
VECTOR3_CSHARP_CODE

%define MATRIX2_CSHARP_CODE
%typemap (cscode) csMatrix2
%{
  public static csMatrix2 operator+(csMatrix2 m1, csMatrix2 m2)
  {
    return new csMatrix2(m1.m11 + m2.m11, m1.m12 + m2.m12,
			 m1.m21 + m2.m21, m1.m22 + m2.m22);
  }

  public static csMatrix2 operator-(csMatrix2 m1, csMatrix2 m2)
  {
    return new csMatrix2(m1.m11 - m2.m11, m1.m12 - m2.m12,
			 m1.m21 - m2.m21, m1.m22 - m2.m22);
  }

  public static csMatrix2 operator*(csMatrix2 m1, csMatrix2 m2)
  {
    return new csMatrix2(m1.m11 * m2.m11 + m1.m12 * m2.m21,
			 m1.m11 * m2.m12 + m1.m12 * m2.m22,
			 m1.m21 * m2.m11 + m1.m22 * m2.m21,
			 m1.m21 * m2.m12 + m1.m22 * m2.m22);
  }

  public static csMatrix2 operator*(csMatrix2 m, float s)
  {
    return new csMatrix2(m.m11 * s, m.m12 * s,
			 m.m21 * s, m.m22 * s);
  }

  public static csMatrix2 operator*(float s, csMatrix2 m)
  {
    return new csMatrix2(m.m11 * s, m.m12 * s,
			 m.m21 * s, m.m22 * s);
  }

  /// Multiply a vector by a matrix (transform it).
  public static csVector2 operator* (csMatrix2 m, csVector2 v)
  {
    return new csVector2 (m.m11*v.x + m.m12*v.y, m.m21*v.x + m.m22*v.y);
  }

  public static csMatrix2 operator/(csMatrix2 m, float s)
  {
    return new csMatrix2(m.m11 / s, m.m12 / s,
			 m.m21 / s, m.m22 / s);
  }

  public new static bool Equals(object mo1, object mo2)
  {
    if(!(mo1 is csMatrix2) || !(mo2 is csMatrix2))
      return false;
    csMatrix2 m1 = (csMatrix2) mo1;
    csMatrix2 m2 = (csMatrix2) mo2;

     // If one of the objects is null, then compares by reference
    if (mo1 == null || mo2 == null)
      return mo1 != mo2;
    else
      return m1.m11 == m2.m11 && m1.m12 == m2.m12
	&& m1.m21 == m2.m21 && m1.m22 == m2.m22;
  }

  public new static int GetHashCode(object mo)
  {
    csMatrix3 m = (csMatrix3) mo;

    return (int)((m.m11.GetHashCode()&0x000000ff) | (m.m12.GetHashCode()&0x0000ff00) 
	 | (m.m21.GetHashCode()&0x00ff0000) | (m.m22.GetHashCode()&0xff000000));
  }

  public override bool Equals(object o)
  {
    return csMatrix2.Equals(this, o);
  }

  public override int GetHashCode()
  {
    return csMatrix2.GetHashCode(this);
  }

  public static bool operator==(csMatrix2 m1, csMatrix2 m2)
  {
     // If one of the objects is null, then compares by reference
    if ((object)m1 == null || (object)m2 == null)
      return (object)m1 == (object)m2;
    else
      return m1.m11 == m2.m11 && m1.m12 == m2.m12
	&& m1.m21 == m2.m21 && m1.m22 == m2.m22;
  }

  public static bool operator!=(csMatrix2 m1, csMatrix2 m2)
  {
     // If one of the objects is null, then compares by reference
    if ((object)m1 == null || (object)m2 == null)
      return (object)m1 != (object)m2;
    else
      return m1.m11 != m2.m11 || m1.m12 != m2.m12
	|| m1.m21 != m2.m21 || m1.m22 != m2.m22;
  }

  public static bool operator<(csMatrix2 m, float e)
  {
    return Math.Abs(m.m11) < e && Math.Abs(m.m12) < e
	&& Math.Abs(m.m21) < e && Math.Abs(m.m22) < e;
  }

  public static bool operator>(csMatrix2 m, float e)
  {
    return Math.Abs(m.m11) > e && Math.Abs(m.m12) > e
	&& Math.Abs(m.m21) > e && Math.Abs(m.m22) > e;
  }
%}
%enddef
MATRIX2_CSHARP_CODE

%define MATRIX3_CSHARP_CODE
%typemap (cscode) csMatrix3
%{
  public static csMatrix3 operator+(csMatrix3 m1, csMatrix3 m2)
  {
    return new csMatrix3(m1.m11 + m2.m11, m1.m12 + m2.m12, m1.m13 + m2.m13,
			 m1.m21 + m2.m21, m1.m22 + m2.m22, m1.m23 + m2.m23,
			 m1.m31 + m2.m31, m1.m32 + m2.m32, m1.m33 + m2.m33);
  }

  public static csMatrix3 operator-(csMatrix3 m1, csMatrix3 m2)
  {
    return new csMatrix3(m1.m11 - m2.m11, m1.m12 - m2.m12, m1.m13 - m2.m13,
			 m1.m21 - m2.m21, m1.m22 - m2.m22, m1.m23 - m2.m23,
			 m1.m31 - m2.m31, m1.m32 - m2.m32, m1.m33 - m2.m33);
  }

  public new static bool Equals(object mo1, object mo2)
  {
    if(!(mo1 is csMatrix3) || !(mo2 is csMatrix3))
      return false;
    csMatrix3 m1 = (csMatrix3) mo1;
    csMatrix3 m2 = (csMatrix3) mo2;

     // If one of the objects is null, then compares by reference
    if (mo1 == null || mo2 == null)
      return mo1 == mo2;
    else
      return m1.m11==m2.m11 && m1.m12==m2.m12 && m1.m13==m2.m13
	&& m1.m21==m2.m21 && m1.m22==m2.m22 && m1.m23==m2.m23
	&& m1.m31==m2.m31 && m1.m32==m2.m32 && m1.m33==m2.m33;
  }

  public new static int GetHashCode(object mo)
  {
    csMatrix3 m = (csMatrix3) mo;

    return (int)((m.m11.GetHashCode()&0x0000000f) | (m.m12.GetHashCode()&0x000000f0) | (m.m13.GetHashCode()&0x00000f00) 
	 | (m.m21.GetHashCode()&0x0000f000) | (m.m22.GetHashCode()&0x0000f000) | (m.m23.GetHashCode()&0x000f0000)
	 | (m.m31.GetHashCode()&0x00f00000) | (m.m32.GetHashCode()&0x0f000000) | (m.m33.GetHashCode()&0xf0000000));
  }

  public override bool Equals(object o)
  {
    return csMatrix3.Equals(this, o);
  }

  public override int GetHashCode()
  {
    return csMatrix3.GetHashCode(this);
  }

  public static bool operator==(csMatrix3 m1, csMatrix3 m2)
  {
     // If one of the objects is null, then compares by reference
    if ((object)m1 == null || (object)m2 == null)
      return (object)m1 == (object)m2;
    else
      return m1.m11==m2.m11 && m1.m12==m2.m12 && m1.m13==m2.m13
	&& m1.m21==m2.m21 && m1.m22==m2.m22 && m1.m23==m2.m23
	&& m1.m31==m2.m31 && m1.m32==m2.m32 && m1.m33==m2.m33;
  }

  public static bool operator!=(csMatrix3 m1, csMatrix3 m2)
  {
     // If one of the objects is null, then compares by reference
    if ((object)m1 == null || (object)m2 == null)
      return (object)m1 != (object)m2;
    else
      return m1.m11!=m2.m11 || m1.m12!=m2.m12 || m1.m13!=m2.m13
	|| m1.m21!=m2.m21 || m1.m22!=m2.m22 || m1.m23!=m2.m23
	|| m1.m31!=m2.m31 || m1.m32!=m2.m32 || m1.m33!=m2.m33;
  }

  public static csVector3 operator* (csMatrix3 m, csVector3 v)
  {
    return new csVector3 (m.m11*v.x + m.m12*v.y + m.m13*v.z,
                      m.m21*v.x + m.m22*v.y + m.m23*v.z,
                      m.m31*v.x + m.m32*v.y + m.m33*v.z);
  }

  public static csMatrix3 operator*(csMatrix3 m1, csMatrix3 m2)
  {
    return new csMatrix3 (
      m1.m11 * m2.m11 + m1.m12 * m2.m21 + m1.m13 * m2.m31,
      m1.m11 * m2.m12 + m1.m12 * m2.m22 + m1.m13 * m2.m32,
      m1.m11 * m2.m13 + m1.m12 * m2.m23 + m1.m13 * m2.m33,
      m1.m21 * m2.m11 + m1.m22 * m2.m21 + m1.m23 * m2.m31,
      m1.m21 * m2.m12 + m1.m22 * m2.m22 + m1.m23 * m2.m32,
      m1.m21 * m2.m13 + m1.m22 * m2.m23 + m1.m23 * m2.m33,
      m1.m31 * m2.m11 + m1.m32 * m2.m21 + m1.m33 * m2.m31,
      m1.m31 * m2.m12 + m1.m32 * m2.m22 + m1.m33 * m2.m32,
      m1.m31 * m2.m13 + m1.m32 * m2.m23 + m1.m33 * m2.m33);
  }

  public static csMatrix3 operator*(csMatrix3 m, float s)
  {
    return new csMatrix3(m.m11 * s, m.m12 * s, m.m13 * s,
			 m.m21 * s, m.m22 * s, m.m23 * s,
			 m.m31 * s, m.m32 * s, m.m33 * s);
  }

  public static csMatrix3 operator*(float s, csMatrix3 m)
  {
    return new csMatrix3(m.m11 * s, m.m12 * s, m.m13 * s,
			 m.m21 * s, m.m22 * s, m.m23 * s,
			 m.m31 * s, m.m32 * s, m.m33 * s);
  }

  public static csMatrix3 operator/(csMatrix3 m, float s)
  {
    return new csMatrix3(m.m11 / s, m.m12 / s, m.m13 / s,
			 m.m21 / s, m.m22 / s, m.m23 / s,
			 m.m31 / s, m.m32 / s, m.m33 / s);
  }

  public static bool operator<(csMatrix3 m, float e)
  {
    return Math.Abs(m.m11) < e && Math.Abs(m.m12) < e && Math.Abs(m.m13) < e
	&& Math.Abs(m.m21) < e && Math.Abs(m.m22) < e && Math.Abs(m.m23) < e
	&& Math.Abs(m.m31) < e && Math.Abs(m.m32) < e && Math.Abs(m.m33) < e;
  }

  public static bool operator>(csMatrix3 m, float e)
  {
    return Math.Abs(m.m11) > e && Math.Abs(m.m12) > e && Math.Abs(m.m13) > e
	&& Math.Abs(m.m21) > e && Math.Abs(m.m22) > e && Math.Abs(m.m23) > e
	&& Math.Abs(m.m31) > e && Math.Abs(m.m32) > e && Math.Abs(m.m33) > e;
  }
%}
%enddef
MATRIX3_CSHARP_CODE

%define TRANSFORM_CSHARP_CODE
%typemap (cscode) csTransform
%{
  
  public static csVector3 operator*(csVector3 v, csTransform t)
  {
    return t.Other2This(v);
  }

  public static csVector3 operator*(csTransform t, csVector3 v)
  {
    return t.Other2This(v);
  }

  public static csPlane3 operator*(csTransform t, csPlane3 p)
  {
    return t.Other2This(p);
  }

  public static csPlane3 operator*(csPlane3 p, csTransform t)
  {
    return t.Other2This(p);
  }

  public static csSphere operator*(csTransform t, csSphere s)
  {
    return t.Other2This(s);
  }

  public static csSphere operator*(csSphere s, csTransform t)
  {
    return t.Other2This(s);
  }

  public static csMatrix3 operator*(csMatrix3 m, csTransform t)
  {
    return m*t.GetO2T();
  }

  public static csMatrix3 operator*(csTransform t, csMatrix3 m)
  {
    return t.GetO2T()*m;
  }

%}
%enddef
TRANSFORM_CSHARP_CODE

%define REVERSIBLE_TRANSFORM_CSHARP_CODE
%typemap (cscode) csReversibleTransform
%{
  
  public static csVector3 operator/(csVector3 v, csReversibleTransform t)
  {
    return t.This2Other(v);
  }

  public static csVector3 operator/(csReversibleTransform t, csVector3 v)
  {
    return t.This2Other(v);
  }

  public static csPlane3 operator/(csReversibleTransform t, csPlane3 p)
  {
    return t.This2Other(p);
  }

  public static csPlane3 operator/(csPlane3 p, csReversibleTransform t)
  {
    return t.This2Other(p);
  }

  public static csSphere operator/(csReversibleTransform t, csSphere s)
  {
    return t.This2Other(s);
  }

  public static csSphere operator/(csSphere s, csReversibleTransform t)
  {
    return t.This2Other(s);
  }

  public static csMatrix3 operator/(csMatrix3 m, csReversibleTransform t)
  {
    return m*t.GetT2O();
  }

  public static csMatrix3 operator/(csReversibleTransform t, csMatrix3 m)
  {
    return t.GetT2O()*m;
  }
%}
%enddef
REVERSIBLE_TRANSFORM_CSHARP_CODE

%define SPHERE_CSHARP_CODE
%typemap (cscode) csSphere
%{
  public static csSphere operator+(csSphere s1, csSphere s2)
  {
    csSphere ret = new csSphere(s1.GetCenter(), s1.GetRadius());
    ret.Union(s2.GetCenter(), s2.GetRadius());
    return ret;
  }
%}
%enddef
SPHERE_CSHARP_CODE

%define QUATERNION_CSHARP_CODE
%typemap (cscode) csQuaternion
%{
  public static csQuaternion operator+(csQuaternion q1, csQuaternion q2)
  {
    return new csQuaternion(q1.v + q2.v, q1.w + q2.w);
  }

  public static csQuaternion operator-(csQuaternion q1, csQuaternion q2)
  {
    return new csQuaternion(q1.v - q2.v, q1.w - q2.w);
  }

  public static csQuaternion operator*(csQuaternion q1, csQuaternion q2)
  {
    return new csQuaternion(q1.v*q2.w + q1.w*q2.v + q1.v%q2.v,
			    q1.w*q2.w - q1.v*q2.v);
  }

  public static csQuaternion operator*(csQuaternion q, float s)
  {
    return new csQuaternion(q.v*s, q.w*s);
  }

  public static csQuaternion operator*(float s, csQuaternion q)
  {
    return new csQuaternion(s*q.v, s*q.w);
  }

  public static csQuaternion operator/(csQuaternion q, float s)
  {
    float invS = 1.0f / s;
    return new csQuaternion(q.v * invS, q.w * invS);
  }

  public static csQuaternion operator/(float s, csQuaternion q)
  {
    float invS = 1.0f / s;
    return new csQuaternion(q.v * invS, q.w * invS);
  }
%}
%enddef
QUATERNION_CSHARP_CODE

%define BOX2_CSHARP_CODE
%typemap (cscode) csBox2
%{

  ///<comment>
  /// Represents minimal part of the box.
  ///<comment>
  public csVector2 BoxMin
  {
    get
    {
      return new csVector2(MinX(), MinY());
    }
    set
    {
      SetMin(0, value.x);
      SetMin(1, value.y);
    }
  }

  ///<comment>
  /// Represents maximal part of the box.
  ///<comment>
  public csVector2 BoxMax
  {
    get
    {
      return new csVector2(MaxX(), MaxY());
    }
    set
    {
      SetMax(0, value.x);
      SetMax(1, value.y);
    }
  }

  public static csBox2 operator+(csBox2 b1, csBox2 b2)
  {
    return new csBox2(Math.Min(b1.MinX(), b2.MinX()),
		      Math.Min(b1.MinY(), b2.MinY()),
		      Math.Max(b1.MaxX(), b2.MaxX()),
		      Math.Max(b1.MaxY(), b2.MaxY()));
  }

  public static csBox2 operator+(csBox2 b, csVector2 p)
  {
    return new csBox2(Math.Min(b.MinX(), p.x),
		      Math.Min(b.MinY(), p.y),
		      Math.Max(b.MaxX(), p.x),
		      Math.Max(b.MaxY(), p.y));
  }

  public static csBox2 operator*(csBox2 b1, csBox2 b2)
  {
    return new csBox2(Math.Max(b1.MinX(), b2.MinX()),
		      Math.Max(b1.MinY(), b2.MinY()),
		      Math.Min(b1.MaxX(), b2.MaxX()),
		      Math.Min(b1.MaxY(), b2.MaxY()));
  }

  public new static bool Equals(object bo1, object bo2)
  {
    if(!(bo1 is csBox2) || !(bo2 is csBox2))
      return false;
    csBox2 b1 = (csBox2) bo1;
    csBox2 b2 = (csBox2) bo2;

    // If one of the objects is null, then compares by reference
    if ((object)b1 == null || (object)b2 == null)
      return b1 != b2;
    else
      return b1.MinX()==b2.MinX() &&
	   b1.MinY()==b2.MinY() &&
	   b1.MaxX()==b2.MaxX() &&
	   b1.MaxY()==b2.MaxY();
  }

  public new static int GetHashCode(object bo)
  {
    csBox2 b = (csBox2) bo;

    return (int)((b.BoxMin.GetHashCode()&0x0000ffff) |(b.BoxMax.GetHashCode()&0xffff0000));
  }

  public override bool Equals(object o)
  {
    return csBox2.Equals(this, o);
  }

  public override int GetHashCode()
  {
    return csBox2.GetHashCode(this);
  }

  public static bool operator==(csBox2 b1, csBox2 b2)
  {
    // If one of the objects is null, then compares by reference
    if ((object)b1 == null || (object)b2 == null)
      return (object)b1 != (object)b2;
    else
      return b1.MinX()==b2.MinX() &&
	   b1.MinY()==b2.MinY() &&
	   b1.MaxX()==b2.MaxX() &&
	   b1.MaxY()==b2.MaxY();
  }

  public static bool operator!=(csBox2 b1, csBox2 b2)
  {
    // If one of the objects is null, then compares by reference
    if ((object)b1 == null || (object)b2 == null)
      return (object)b1 != (object)b2;
    else
      return b1.MinX()!=b2.MinX() ||
	   b1.MinY()!=b2.MinY() ||
	   b1.MaxX()!=b2.MaxX() ||
	   b1.MaxY()!=b2.MaxY();
  }

  public static bool operator<(csBox2 b1, csBox2 b2)
  {
    return b1.MinX() >= b2.MinX() &&
	   b1.MinY() >= b2.MinY() &&
	   b1.MaxX() <= b2.MaxX() &&
	   b1.MaxY() <= b2.MaxY();
  }

  public static bool operator>(csBox2 b1, csBox2 b2)
  {
    return b2.MinX() >= b1.MinX() &&
	   b2.MinY() >= b1.MinY() &&
	   b2.MaxX() <= b1.MaxX() &&
	   b2.MaxY() <= b1.MaxY();
  }

  public static bool operator<(csVector2 p, csBox2 b)
  {
    return p.x >= b.MinX() && p.x <= b.MaxX() &&
	   p.y >= b.MinY() && p.y <= b.MaxY();
  }

  public static bool operator>(csVector2 p, csBox2 b)
  {
    return b.MinX() >= p.x && b.MaxX() <= p.x &&
	   b.MinY() >= p.y && b.MaxY() <= p.y;
  }
%}
%enddef
BOX2_CSHARP_CODE

%define BOX3_CSHARP_CODE
%typemap (cscode) csBox3
%{
  ///<comment>
  /// Represents minimal part of the box.
  ///<comment>
  public csVector3 BoxMin
  {
    get
    {
      return new csVector3(MinX(), MinY(), MinZ());
    }
    set
    {
      SetMin(0, value.x);
      SetMin(1, value.y);
      SetMin(2, value.z);
    }
  }

  ///<comment>
  /// Represents maximal part of the box.
  ///<comment>
  public csVector3 BoxMax
  {
    get
    {
      return new csVector3(MaxX(), MaxY(), MaxZ());
    }
    set
    {
      SetMax(0, value.x);
      SetMax(1, value.y);
      SetMax(2, value.z);
    }
  }

  public new static bool Equals(object bo1, object bo2)
  {
    if(!(bo1 is csBox3) || !(bo2 is csBox3))
      return false;
    csBox3 b1 = (csBox3) bo1;
    csBox3 b2 = (csBox3) bo2;

    // If one of the objects is null, then compares by reference
    if (bo1 == null || bo2 == null)
      return bo1 != bo2;
    else
      return b1.MinX()==b2.MinX() &&
	   b1.MinY()==b2.MinY() &&
	   b1.MaxX()==b2.MaxX() &&
	   b1.MaxY()==b2.MaxY();
  }

  public new static int GetHashCode(object bo)
  {
    csBox3 b = (csBox3) bo;

    return (int)((b.BoxMin.GetHashCode()&0x0000ffff) |(b.BoxMax.GetHashCode()&0xffff0000));
  }

  public override bool Equals(object o)
  {
    return csBox3.Equals(this, o);
  }

  public override int GetHashCode()
  {
    return csBox3.GetHashCode(this);
  }

  public static csBox3 operator+(csBox3 b1, csBox3 b2)
  {
    return new csBox3(Math.Min(b1.MinX(), b2.MinX()),
		      Math.Min(b1.MinY(), b2.MinY()),
		      Math.Min(b1.MinZ(), b2.MinZ()),
		      Math.Max(b1.MaxX(), b2.MaxX()),
		      Math.Max(b1.MaxY(), b2.MaxY()),
		      Math.Max(b1.MaxZ(), b2.MaxZ()));
  }

  public static csBox3 operator+(csBox3 b, csVector3 p)
  {
    return new csBox3(Math.Min(b.MinX(), p.x),
		      Math.Min(b.MinY(), p.y),
		      Math.Min(b.MinZ(), p.z),
		      Math.Max(b.MaxX(), p.x),
		      Math.Max(b.MaxY(), p.y),
		      Math.Max(b.MaxZ(), p.z));
  }

  public static csBox3 operator*(csBox3 b1, csBox3 b2)
  {
    return new csBox3(Math.Max(b1.MinX(), b2.MinX()),
		      Math.Max(b1.MinY(), b2.MinY()),
		      Math.Max(b1.MinZ(), b2.MinZ()),
		      Math.Min(b1.MaxX(), b2.MaxX()),
		      Math.Min(b1.MaxY(), b2.MaxY()),
		      Math.Min(b1.MaxZ(), b2.MaxZ()));
  }

  public static bool operator==(csBox3 b1, csBox3 b2)
  {
    // If one of the objects is null, then compares by reference
    if ((object)b1 == null || (object)b2 == null)
      return (object)b1 != (object)b2;
    else
      return b1.MinX()==b2.MinX() &&
	   b1.MinY()==b2.MinY() &&
	   b1.MinZ()==b2.MinZ() &&
	   b1.MaxX()==b2.MaxX() &&
	   b1.MaxY()==b2.MaxY() &&
	   b1.MaxZ()==b2.MaxZ();
  }

  public static bool operator!=(csBox3 b1, csBox3 b2)
  {
    // If one of the objects is null, then compares by reference
    if ((object)b1 == null || (object)b2 == null)
      return (object)b1 != (object)b2;
    else
      return b1.MinX()!=b2.MinX() ||
	   b1.MinY()!=b2.MinY() ||
	   b1.MinZ()!=b2.MinZ() ||
	   b1.MaxX()!=b2.MaxX() ||
	   b1.MaxY()!=b2.MaxY() ||
	   b1.MaxZ()!=b2.MaxZ();
  }

  public static bool operator<(csBox3 b1, csBox3 b2)
  {
    return b1.MinX() >= b2.MinX() &&
	   b1.MinY() >= b2.MinY() &&
	   b1.MinZ() >= b2.MinZ() &&
	   b1.MaxX() <= b2.MaxX() &&
	   b1.MaxY() <= b2.MaxY() &&
	   b1.MaxZ() <= b2.MaxZ();
  }

  public static bool operator>(csBox3 b1, csBox3 b2)
  {
    return b2.MinX() >= b1.MinX() &&
	   b2.MinY() >= b1.MinY() &&
	   b2.MinZ() >= b1.MinZ() &&
	   b2.MaxX() <= b1.MaxX() &&
	   b2.MaxY() <= b1.MaxY() &&
	   b2.MaxZ() <= b1.MaxZ();
  }

  public static bool operator<(csVector3 p, csBox3 b)
  {
    return p.x >= b.MinX() && p.x <= b.MaxX() &&
	   p.y >= b.MinY() && p.y <= b.MaxY() &&
	   p.z >= b.MinZ() && p.z <= b.MaxZ();
  }

  public static bool operator>(csVector3 p, csBox3 b)
  {
    return b.MinX() >= p.x && b.MaxX() <= p.x &&
	   b.MinY() >= p.y && b.MaxY() <= p.y &&
  	   b.MinZ() >= p.z && b.MaxZ() <= p.z;
  }
%}
%enddef
BOX3_CSHARP_CODE

#endif // CS_MINI_SWIG

#endif //SWIGCSHARP
