/* cstool module swig directives */

%ignore csColliderHelper::TraceBeam (iCollideSystem*, iSector*,
  const csVector3&, const csVector3&, bool, csIntersectingTriangle&,
  csVector3&, iMeshWrapper**);

/* ignore scfFakeInterface warning */
%template (scfFakecsColliderWrapper) scfFakeInterface<csColliderWrapper >;
%template(scfColliderWrapper) scfImplementationExt1<csColliderWrapper,csObject,scfFakeInterface<csColliderWrapper> >;
%include "cstool/collider.h"
%template (scfView) scfImplementation1<csView, iView >;
%include "cstool/csview.h"
%include "cstool/csfxscr.h"

%include "cstool/cspixmap.h"
%include "cstool/enginetools.h"

%ignore CS::Geometry::Primitives::boxTable;
%ignore CS::Geometry::Primitives::quadTable;
%include "cstool/primitives.h"

%include "cstool/genmeshbuilder.h"

%ignore iPen::Rotate;

%include "cstool/pen.h"

%template(scfProcTexture) scfImplementationExt3<csProcTexture, csObject, iTextureWrapper, iProcTexture, iSelfDestruct>;
// Needed to resolve THREADED_CALLABLE_DECL macros
%include "csutil/threadmanager.h"
%include "cstool/proctex.h"
%include "cstool/proctxtanim.h"

%include "cstool/simplestaticlighter.h"

/* POST */
#ifndef SWIGIMPORTED
#undef APPLY_FOR_ALL_INTERFACES_POST
#define APPLY_FOR_ALL_INTERFACES_POST CSTOOL_APPLY_FOR_EACH_INTERFACE
#endif

%include "bindings/common/basepost.i"

#ifndef SWIGIMPORTED
cs_apply_all_interfaces
#endif

cs_lang_include(cstoolpost.i)




