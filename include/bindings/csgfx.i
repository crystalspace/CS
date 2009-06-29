%module csgfx
%import "bindings/core.i"
%{
#include "igraphic.h"
#include "itexture.h"
#include "csgeom.h"
#include "csgfx.h"
#include "csutil.h"
#include "cstool/initapp.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/csgfx.i"

