%module iengine
%import "bindings/core.i"
%import "bindings/ivideo.i" /* due to at least csZBufMode graph3d.h */
%{
#include "csgeom.h"
#include "csgfx.h"
#include "csutil.h"
#include "cstool.h"
#include "iengine.h"
#include "igraphic.h"
#include "imap.h"
#include "ivideo.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/iengine.i"

