%include "imesh/skeleton2.h"
%include "imesh/bodymesh.h"
%include "imesh/animnode/skeleton2anim.h"
%include "imesh/animnode/ik.h"
%include "imesh/animnode/lookat.h"
%include "imesh/animnode/ragdoll.h"
%include "imesh/animnode/speed.h"

/* POST */
#ifndef SWIGIMPORTED
#undef APPLY_FOR_ALL_INTERFACES_POST
#define APPLY_FOR_ALL_INTERFACES_POST ANIMATION_APPLY_FOR_EACH_INTERFACE
#endif

%include "bindings/common/basepost.i"

#ifndef SWIGIMPORTED
cs_apply_all_interfaces
#endif

cs_lang_include(animationpost.i)
