%module ivideo
%import "bindings/core.i"
%import "bindings/csgfx.i"
%{
#include "csgeom.h"
#include "csgfx.h"
#include "csutil.h"
#include "cstool/initapp.h"
#include "igraphic.h"
#include "imesh.h"
#include "itexture.h"
#include "ivideo.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/ivideo.i"
