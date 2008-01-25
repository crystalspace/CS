#ifndef _SW_CSGFX_I_
#define _SW_CSGFX_I_

%module csgfx
%import "bindings/core.i"
%{
#include "crystalspace.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/csgfx.i"
#ifndef SWIGIMPORTED
%import "bindings/iengine.i" /* due to at least iTextureWrapper */
%import "bindings/ivideo.i" /* due to at least iTextureHandle */
#endif

#endif _SW_CSGFX_I_

