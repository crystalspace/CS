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

%include "imesh/animesh.h"
TYPEMAP_ARGOUT_PTR(csQuaternion)
TYPEMAP_ARGOUT_PTR(csVector3)
APPLY_TYPEMAP_ARGOUT_PTR(csQuaternion,csQuaternion& rot)
APPLY_TYPEMAP_ARGOUT_PTR(csVector3,csVector3& offset)
%clear csVector3& offset;
%clear csQuaternion& rot;

struct csSprite2DVertex;
%ignore iSprite2DState::GetVertices;
ARRAY_CHANGE_ALL_TEMPLATE(csSprite2DVertex)

%include "imesh/sprite2d.h"
%include "imesh/sprite3d.h"
%include "imesh/spritecal3d.h"
// Swig 1.3.24 doesn't handle pointer default args well unless we tell it
// to use an alternate way for that function
%feature("compactdefaultargs") HitBeamObject;
%include "imesh/object.h"
%template (csCharArrayArray) csArray<csArray<char> >;
%include "imesh/terrain.h"
%include "imesh/terrain2.h"

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

BUFFER_RW_FUNCTIONS(iGeneralFactoryState,GetVertices,GetVertexCount,
                csVector3,GetVerticesAsBuffer)
BUFFER_RW_FUNCTIONS(iGeneralFactoryState,GetNormals,GetVertexCount,
                csVector3,GetNormalsAsBuffer)
BUFFER_RW_FUNCTIONS(iGeneralFactoryState,GetColors,GetVertexCount,
                csColor4,GetColorsAsBuffer)
BUFFER_RW_FUNCTIONS(iGeneralFactoryState,GetTriangles,GetTriangleCount,
                csTriangle,GetTrianglesAsBuffer)

// imesh/terrain2.h
%extend csLockedHeightData
{
  float Get(int x,int y)
  { return self->data[y*self->pitch+x]; }

  void Set(int x,int y,float val)
  { self->data[y*self->pitch+x] = val; }
}

%extend csLockedMaterialMap
{
  unsigned char Get(int x,int y)
  { return self->data[y*self->pitch+x]; }

  void Set(int x,int y,unsigned char val)
  { self->data[y*self->pitch+x] = val; }
}


/* POST */
#ifndef SWIGIMPORTED
#undef APPLY_FOR_ALL_INTERFACES_POST
#define APPLY_FOR_ALL_INTERFACES_POST IMESH_APPLY_FOR_EACH_INTERFACE
#endif

%include "bindings/common/basepost.i"

#ifndef SWIGIMPORTED
cs_apply_all_interfaces
#endif

cs_lang_include(imeshpost.i)



