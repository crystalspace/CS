%include "iengine/light.h"
%include "iengine/sector.h"
%include "iengine/engine.h"
%include "iengine/lod.h"

%ignore iCamera::GetTransform (); // Non-const.
%ignore iCamera::Perspective (const csVector3&, csVector2&) const;
%ignore iCamera::InvPerspective (const csVector2&, float, csVector3&) const;
%include "iengine/camera.h"

%include "iengine/campos.h"
%include "iengine/texture.h"
%include "iengine/material.h"
%template(iSceneNodeArrayReadOnly) iArrayReadOnly<iSceneNode * >;
%include "iengine/scenenode.h"
// Swig 1.3.24 doesn't handle pointer default args well unless we tell it
// to use an alternate way for that function
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
// Swig 1.3.24 doesn't handle pointer default args well unless we tell it
// to use an alternate way for that function
%feature("compactdefaultargs") IntersectSegment;
%include "iengine/viscull.h"
%include "iengine/portal.h"
%include "iengine/portalcontainer.h"
%include "iengine/rendersteps/icontainer.h"
%include "iengine/renderloop.h"
%include "iengine/rendermanager.h"
%template(iSwigCollectionArray) iArrayReadOnly<iCollection* >;
%include "iengine/collection.h"

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
/* List Methods */
LIST_OBJECT_FUNCTIONS(iMeshList,iMeshWrapper)
LIST_OBJECT_FUNCTIONS(iMeshFactoryList,iMeshFactoryWrapper)
LIST_OBJECT_FUNCTIONS(iMaterialList,iMaterialWrapper)
LIST_OBJECT_FUNCTIONS(iLightList,iLight)
LIST_OBJECT_FUNCTIONS(iCameraPositionList,iCameraPosition)
LIST_OBJECT_FUNCTIONS(iSectorList,iSector)
LIST_OBJECT_FUNCTIONS(iTextureList,iTextureWrapper)

/* POST */
#ifndef SWIGIMPORTED
#undef APPLY_FOR_ALL_INTERFACES_POST
#define APPLY_FOR_ALL_INTERFACES_POST IENGINE_APPLY_FOR_EACH_INTERFACE
#endif

%include "bindings/common/basepost.i"

#ifndef SWIGIMPORTED
cs_apply_all_interfaces
#endif

cs_lang_include(ienginepost.i)

