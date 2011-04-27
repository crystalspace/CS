%module imesh
%import "bindings/core.i"
%import "bindings/ivaria.i"
%import "bindings/csgeom.i"
%import "bindings/ivideo.i"
%{
#include "csgeom.h"
#include "csgfx.h"
#include "cstool/initapp.h"
#include "csutil.h"
#include "igraphic.h"
#include "imesh.h"
#include "itexture.h"
#include "iutil.h"
#include "ivaria.h"
#include "ivideo.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/imesh.i"
