%include "imesh/skeleton2.h"
%include "imesh/bodymesh.h"
%include "imesh/animnode/skeleton2anim.h"
%template (SkeletonDebugNodeManager)
	  CS::Animation::iSkeletonAnimNodeManager<CS::Animation::iSkeletonDebugNodeFactory>;
%include "imesh/animnode/debug.h"
%template (SkeletonIKNodeManager)
	  CS::Animation::iSkeletonAnimNodeManager<CS::Animation::iSkeletonIKNodeFactory>;
%include "imesh/animnode/ik.h"
%template (SkeletonLookAtNodeManager)
	  CS::Animation::iSkeletonAnimNodeManager<CS::Animation::iSkeletonLookAtNodeFactory>;
%include "imesh/animnode/lookat.h"
%template (SkeletonRagdollNodeManager)
	  CS::Animation::iSkeletonAnimNodeManager<CS::Animation::iSkeletonRagdollNodeFactory>;
%include "imesh/animnode/ragdoll.h"
%template (SkeletonRetargetNodeManager)
	  CS::Animation::iSkeletonAnimNodeManager<CS::Animation::iSkeletonRetargetNodeFactory>;
%include "imesh/animnode/retarget.h"
%template (SkeletonSpeedNodeManager)
	  CS::Animation::iSkeletonAnimNodeManager<CS::Animation::iSkeletonSpeedNodeFactory>;
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
