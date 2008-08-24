#ifdef FILE_FOR_CLASS
%module ienginemod
#else
%module iengine
#endif
%import "bindings/core.i"
%import "bindings/ivideo.i" /* due to at least csZBufMode graph3d.h */
%import "bindings/csgeom.i"
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

#if defined(SWIGCSHARP) && !defined(SWIGIMPORTED)
%import "bindings/imesh.i" /* due to at least iMeshObject */
#endif
%include "bindings/common/iengine.i"

