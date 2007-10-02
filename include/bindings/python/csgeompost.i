%define VECTOR_PYTHON_OBJECT_FUNCTIONS(N)
  csVector ## N __rmul__ (float f) const { return f * *self; }
  float __abs__ () const { return self->Norm(); }
%enddef

%extend csVector2
{
  VECTOR_PYTHON_OBJECT_FUNCTIONS(2)

  float __getitem__ (int i) const { return i ? self->y : self->x; }
  void __setitem__ (int i, float v) { if (i) self->y = v; else self->x = v; }
}

%extend csVector3
{
  VECTOR_PYTHON_OBJECT_FUNCTIONS(3)

  float __getitem__ (int i) const { return self->operator[](i); }
  void __setitem__ (int i, float v) { self->operator[](i) = v; }
  bool __nonzero__ () const { return !self->IsZero(); }
}

%extend csVector4
{
  VECTOR_PYTHON_OBJECT_FUNCTIONS(4)

  float __getitem__ (int i) const { return self->operator[](i); }
  void __setitem__ (int i, float v) { self->operator[](i) = v; }
  bool __nonzero__ () const { return !self->IsZero(); }
}

%extend csTriangle
{
  int __getitem__ (int i) const { return self->operator[](i); }
  void __setitem__ (int i, int v) { self->operator[](i) = v; }
  bool __nonzero__ () const { return !(self->a||self->b||self->c); }
}

%extend csMatrix3
{
  csMatrix3 __rmul__ (float f) { return f * *self; }
}

%define CSPOLY_PYTHON_OBJECT_FUNCTIONS(N)
  csVector ## N & __getitem__ (int i) { return self->operator[](i); }
  %pythoncode %{
    def __setitem__ (self, i, v):
      own_v = self.__getitem__(i)
      for i in range(N):
        own_v[i] = v[i]
  %}
%enddef

%extend csPoly3D
{
  CSPOLY_PYTHON_OBJECT_FUNCTIONS(3)
}

%extend csPoly2D
{
  CSPOLY_PYTHON_OBJECT_FUNCTIONS(2)
}

%extend csTransform
{
  csVector3 __rmul__ (const csVector3 & v) const { return v * *self; }
  csPlane3 __rmul__ (const csPlane3 & p) const { return p * *self; }
  csSphere __rmul__ (const csSphere & s) const { return s * *self; }
}

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

