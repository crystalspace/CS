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


  // Perform the scalar divition
  public static csVector2 operator/(csVector2 v, float s)
  {
    return new csVector2(v.x/s, v.y/s);
  }

  // Are the two vectors equal?
  public static bool operator==(csVector2 v1, csVector2 v2)
  {
    return v1.x==v2.x && v1.y==v2.y;
  }

  // Are the two vectors differents?
  public static bool operator!=(csVector2 v1, csVector2 v2)
  {
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
  public static csVector3 operator*(csVector3 v, int s)
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
  public static bool operator==(csVector3 v1, csVector3 v2)
  {
    return v1.x==v2.x && v1.y==v2.y && v1.z==v2.z;
  }

  // Are the two vectors differents?
  public static bool operator!=(csVector3 v1, csVector3 v2)
  {
    return v1.x!=v2.x || v1.y!=v2.y || v1.z!=v2.z;
  }

  public static bool operator<(csVector3 v, float epsilon)
  {
    return Math.Abs(v.x) < epsilon && Math.Abs(v.y) < epsilon 
           && Math.Abs(v.z) < epsilon;
  }

  public static bool operator<(csVector3 v, float epsilon)
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
    return new csMatrix2(m.m11 * s, m.m12 * s
			 m.m21 * s, m.m22 * s);
  }

  public static csMatrix2 operator*(float s, csMatrix2 m)
  {
    return new csMatrix2(m.m11 * s, m.m12 * s
			 m.m21 * s, m.m22 * s);
  }

  /// Multiply a vector by a matrix (transform it).
  public static csVector2 operator* (csMatrix2 m, csVector2 v)
  {
    return csVector2 (m.m11*v.x + m.m12*v.y, m.m21*v.x + m.m22*v.y);
  }

  public static csMatrix2 operator/(csMatrix2 m, float s)
  {
    return new csMatrix2(m.m11 / s, m.m12 / s,
			 m.m21 / s, m.m22 / s);
  }

  public static bool operator==(csMatrix2 m1, csMatrix2 m2)
  {
    return m1.m11 == m2.m11 && m1.m12 == m2.m12
	&& m1.m21 == m2.m21 && m1.m22 == m2.m22;
  }

  public static bool operator!=(csMatrix2 m1, csMatrix2 m2)
  {
    return m1.m11 != m2.m11 || m1.m12 != m2.m12
	|| m1.m21 != m2.m21 || m1.m22 != m2.m22;
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
			 m1.m31 + m2.m31, m1.m32 + m2.m32, m1.m33 + m2.m33,)
  }

  public static csMatrix3 operator-(csMatrix3 m1, csMatrix3 m2)
  {
    return new csMatrix3(m1.m11 - m2.m11, m1.m12 - m2.m12, m1.m13 - m2.m13,
			 m1.m21 - m2.m21, m1.m22 - m2.m22, m1.m23 - m2.m23,
			 m1.m31 - m2.m31, m1.m32 - m2.m32, m1.m33 - m2.m33,)
  }

  public static bool operator==(csMatrix3 m1, csMatrix3 m2)
  {
    return m1.m11==m2.m11 && m1.m12==m2.12 && m1.m13==m2.13
	&& m1.m21==m2.m21 && m1.m22==m2.22 && m1.m23==m2.23
	&& m1.m31==m2.m31 && m1.m32==m2.32 && m1.m33==m2.33;
  }

  public static bool operator!=(csMatrix3 m1, csMatrix3 m2)
  {
    return m1.m11!=m2.m11 || m1.m12!=m2.12 || m1.m13!=m2.13
	|| m1.m21!=m2.m21 || m1.m22!=m2.22 || m1.m23!=m2.23
	|| m1.m31!=m2.m31 || m1.m32!=m2.32 || m1.m33!=m2.33;
  }

  public static csVector3 operator* (csMatrix3 m, csVector3 v)
  {
    return csVector3 (m.m11*v.x + m.m12*v.y + m.m13*v.z,
                      m.m21*v.x + m.m22*v.y + m.m23*v.z,
                      m.m31*v.x + m.m32*v.y + m.m33*v.z);
  }
%}
%enddef
MATRIX3_CSHARP_CODE

#endif // CS_MINI_SWIG

#endif //SWIGCSHARP
