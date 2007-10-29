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
%}
%enddef
VECTOR3_CSHARP_CODE

#endif // CS_MINI_SWIG

#endif //SWIGCSHARP
