#ifdef FILE_FOR_CLASS
%module ienginemod
#else
%module iengine
#endif
%import "bindings/core.i"
%import "bindings/ivideo.i" /* due to at least csZBufMode graph3d.h */
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
%import "bindings/imesh.i" /* due to at least iMeshObject */
INLINE_FUNCTIONS
#endif

%include "bindings/common/iengine.i"

