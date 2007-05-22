%module imesh
%import "bindings/cspace.i"
%{
#include "crystalspace.h"
%}
LANG_FUNCTIONS

#ifndef CS_SWIG_PUBLISH_IGENERAL_FACTORY_STATE_ARRAYS
%ignore iGeneralFactoryState::GetVertices;
%ignore iGeneralFactoryState::GetTexels;
%ignore iGeneralFactoryState::GetNormals;
%ignore iGeneralFactoryState::GetTriangles;
%ignore iGeneralFactoryState::GetColors;
#endif
%include "imesh/objmodel.h"
%include "imesh/genmesh.h"
%include "imesh/skeleton.h"
%include "imesh/gmeshskel2.h"
struct csSprite2DVertex;
%ignore iSprite2DState::GetVertices;
%template(csSprite2DVertexArrayReadOnly) iArrayReadOnly<csSprite2DVertex>;
%template(csSprite2DVertexArrayChangeElements) 
iArrayChangeElements<csSprite2DVertex>;
%template(csSprite2DVertexArrayChangeAll) iArrayChangeAll<csSprite2DVertex>;

%include "imesh/sprite2d.h"
%include "imesh/sprite3d.h"
%include "imesh/spritecal3d.h"
// Swig 1.3.24 doesn't handle pointer default args well unless we tell it
// to use an alternate way for that function
%feature("compactdefaultargs") HitBeamObject;
%include "imesh/object.h"
%include "imesh/thing.h"
%template (csCharArrayArray) csArray<csArray<char> >;
%include "imesh/terrain.h"

%include "imesh/particles.h"

// imesh/sprite2d.h
%extend iSprite2DState
{
  csSprite2DVertex* GetVertexByIndex(int index)
  { return &self->GetVertices()->Get(index); }

  int GetVertexCount()
  { return self->GetVertices()->GetSize(); }
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

#undef INTERFACE_APPLY
#define INTERFACE_APPLY(x) INTERFACE_POST(x)
IMESH_APPLY_FOR_EACH_INTERFACE

#if defined(SWIGPYTHON)
%pythoncode %{
  CS_POLYRANGE_LAST = csPolygonRange (-1, -1)
%}
#endif

