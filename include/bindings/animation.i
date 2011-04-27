%module animation
%import "bindings/core.i"
%{
#include "cssysdef.h"
// Not including imesh.h since that results in ambiguous identifiers
#include "iutil.h"
#include "csutil.h"
#include "igeom.h"
#include "csgeom.h"
#include "imesh/skeleton2.h"
#include "imesh/animnode/skeleton2anim.h"
#include "imesh/animnode/debug.h"
#include "imesh/animnode/ik.h"
#include "imesh/animnode/lookat.h"
#include "imesh/animnode/ragdoll.h"
#include "imesh/animnode/retarget.h"
#include "imesh/animnode/speed.h"
using namespace CS::Animation;
%}

#ifndef SWIGIMPORTED
INLINE_FUNCTIONS
#endif

%include "bindings/common/animation.i"
