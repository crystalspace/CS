%module imap
%import "bindings/core.i"
%import "bindings/iengine.i"
%import "bindings/isndsys.i"
%{
#include "csgeom.h"
#include "csgfx.h"
#include "csutil.h"
#include "cstool/initapp.h"
#include "iengine.h"
#include "igraphic.h"
#include "imap.h"
#include "isndsys.h"
#include "itexture.h"
#include "ivideo.h"
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/imap.i"

