%define VECTOR_PYTHON_OBJECT_FUNCTIONS(N)
  csVector ## N __rmul__ (float f) const { return f * *self; }
  float __abs__ () const { return self->Norm(); }
%enddef

%extend csVector2
{
  VECTOR_PYTHON_OBJECT_FUNCTIONS(2)

  float __getitem__ (int i) const { return i ? self->y : self->x; }
  void __setitem__ (int i, float v) { if (i) self->y = v; else self->x = v; }
  %pythoncode %{
    def __str__(self):
      return "%f,%f"%(self.x,self.y)
    def __repr__(self):
      return "cspace.csVector2(%s)"%(self)
  %}
}

%extend csVector3
{
  VECTOR_PYTHON_OBJECT_FUNCTIONS(3)

  float __getitem__ (int i) const { return self->operator[](i); }
  void __setitem__ (int i, float v) { self->operator[](i) = v; }
  bool __nonzero__ () const { return !self->IsZero(); }
  %pythoncode %{
    def __str__(self):
      return "%f,%f,%f"%(self.x,self.y,self.z)
    def __repr__(self):
      return "cspace.csVector3(%s)"%(self)
  %}
}

%extend csVector4
{
  VECTOR_PYTHON_OBJECT_FUNCTIONS(4)

  float __getitem__ (int i) const { return self->operator[](i); }
  void __setitem__ (int i, float v) { self->operator[](i) = v; }
  bool __nonzero__ () const { return !self->IsZero(); }
  %pythoncode %{
    def __str__(self):
      return "%f,%f,%f,%f"%(self.x,self.y,self.z,self.w)
    def __repr__(self):
      return "cspace.csVector4(%s)"%(self)
  %}
}


%extend csQuaternion
{
  %pythoncode %{
    def __str__(self):
      return "%s,%f"%(self.v,self.w)
    def __repr__(self):
      return "cspace.csQuaternion(%s)"%(self)
  %}
}

%extend csTriangle
{
  int __getitem__ (int i) const { return self->operator[](i); }
  void __setitem__ (int i, int v) { self->operator[](i) = v; }
  bool __nonzero__ () const { return !(self->a||self->b||self->c); }
  %pythoncode %{
    def __str__(self):
      return "%d,%d,%d"%(self.a,self.b,self.c)
    def __repr__(self):
      return "cspace.csTriangle(%s)"%(self)
  %}
}

%extend csMatrix2
{
  %pythoncode %{
    def __str__(s):
      return "%f,%f\n%f,%f"%(s.m11,s.m12,s.m21,s.m22)
    def __repr__(s):
      return "cspace.csMatrix2(%f,%f,%f,%f)"%(s.m11,s.m12,s.m21,s.m22)
  %}
}
%extend csMatrix3
{
  csMatrix3 __rmul__ (float f) { return f * *self; }
  %pythoncode %{
    def __str__(s):
      return "%f,%f,%f\n%f,%f,%f\n%f,%f,%f"%(s.m11,s.m12,s.m13,
                s.m21,s.m22,s.m23,s.m31,s.m32,s.m33)
    def __repr__(s):
      return "cspace.csMatrix3(%f,%f,%f,%f,%f,%f,%f,%f,%f)"%(s.m11,s.m12,
                s.m13,s.m21,s.m22,s.m23,s.m31,s.m32,s.m33)
  %}
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
  %pythoncode %{
    def __str__(self):
      return str(self.GetO2T())+"\n"+str(self.GetOrigin())
    def __repr__(self):
      return "cspace.csTransform(%s,%s)"%(repr(self.GetO2T()),
                repr(self.GetOrigin()))
  %}
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

