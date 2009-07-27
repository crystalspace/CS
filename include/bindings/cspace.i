%module cspace
%{
#include "crystalspace.h"
%}
%include "bindings/common/core.i"
%include "bindings/common/imap.i"
%include "bindings/common/isndsys.i"
%include "bindings/common/ivaria.i"
%include "bindings/common/csgfx.i"
%include "bindings/common/ivideo.i"
%include "bindings/common/csgeom.i"
%include "bindings/common/imesh.i"
%include "bindings/common/iengine.i"
%include "bindings/common/cstool.i"

/*#undef APPLY_FOR_ALL_INTERFACES_POST
#define APPLY_FOR_ALL_INTERFACES_POST APPLY_FOR_ALL_INTERFACES
%include "bindings/common/basepost.i"
cs_lang_include(corepost.i)
*/
