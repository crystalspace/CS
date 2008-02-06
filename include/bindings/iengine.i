#ifndef _SW_IENGINE_I_
#define _SW_IENGINE_I_

#ifdef FILE_FOR_CLASS
%module ienginemod
#else
%module iengine
#endif
%import "bindings/core.i"
%import "bindings/imesh.i" /* due to at least iMeshObject */
%import "bindings/ivideo.i" /* due to at least csZBufMode graph3d.h */
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/iengine.i"

#endif //_SW_IENGINE_I_
