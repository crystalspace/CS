#ifdef SWIG_CSHARP

#ifndef CS_MINI_SWIG
%extend csTransform
{
  static csMatrix3 mulmat1 (const csMatrix3& m, const csTransform& t)
  { return m * t; }
  static csMatrix3 mulmat2 (const csTransform& t, const csMatrix3& m)
  { return t * m; }
}
%extend csReversibleTransform
{
  static csTransform mulrev (const csTransform& t1,
                             const csReversibleTransform& t2)
  { return t1 * t2; } 
}
#endif // CS_MINI_SWIG

#endif //SWIG_CSHARP
