/**** public header ****/

namespace CEL
{

struct iAnimationNode : public iBase
{
  // Has this node changed since it was last checked?
  // Is needed otherwise we could modify the tree and reset the playing
  // animation!
  virtual bool HasChanged () const = 0;

  /// @@@ GENJIX - BlendPerBone is forced to update PER BONE! = Overwrite
  /// @@@ GENJIX - Non flattenable weights
  // Can be ANIMATION, BLEND or OVERWRITE (maybe an ADD in the future)
  virtual NodeType GetNodeType () const = 0;

  // only works when NodeType == BLEND
  virtual const csArray< csTuple2< csRef<iAnimationNode>, float> > &GetChildBlendNodes () = 0;
  // only works when NodeType == OVERWRITE
  virtual const csRefArray<iAnimationNode> &GetChildOverwriteNodes () = 0;

  virtual const csRefArray<iAnimation> &GetChildAnimations () = 0;

  // methods for setting and getting properties go here
  // maybe use celData instead?

  virtual const csRefArray<iAnimationCondition> &GetConditions () = 0;

  // calls Evaluate on all its conditions
  virtual bool EvaluateConditions () = 0;
};

struct iAnimation : public iAnimationNode
{
  // always returns ANIMATION
  virtual NodeType GetNodeType () const = 0;
  // these always return nothing and shouldn't even get called
  virtual const csArray< csTuple2< csRef<iAnimationNode>, float> > &GetChildBlendNodes () = 0;
  virtual const csRefArray<iAnimationNode> &GetChildOverwriteNodes () = 0;

  // These should actually be read from the iAnimationNode properties
  // ... maybe these methods do not need to even exist.
  virtual void SetName (const char *name) = 0;
  virtual const char *GetName () const = 0;
  virtual void SetLoop (bool loop) = 0;
  virtual bool IsLoop () const = 0;
  // thats all a cel animation is!
};

struct iAnimationCondition : public iBase
{
  virtual const csRefArray<iAnimationCondition> &GetChildConditions () = 0;

  virtual const csRefArray<iAnimationResult> &GetResults () = 0;

  // evaluates this condition as well as calling Evaluate on all its children.
  // if the condition is true, this method also calls Activate on its results.
  virtual bool Evaluate () = 0;

  // methods for setting and getting properties go here
  // maybe use celData instead?
};

struct iAnimationResult : public iBase
{
  virtual void Activate () = 0;

  // methods for setting and getting properties go here
  // maybe use celData instead?
};

struct iAnimationNodeFactory : public iBase
{
  virtual csPtr<iAnimationNode> CreateNode () = 0;
};

struct iAnimationConditionFactory : public iBase
{
  virtual csPtr<iAnimationCondition> CreateCondition () = 0;
};

struct iAnimationResultFactory : public iBase
{
  virtual csPtr<iAnimationResult> CreateResult () = 0;
};

struct iAnimationSystem : public iBase
{
  virtual void RegisterNodeType (csStringID name, iAnimationNodeFactory *factory) = 0;

  virtual iAnimationNodeFactory* FindNodeType (csStringID name) const = 0;

  virtual void RegisterConditionType (csStringID name, iAnimationConditionFactory *factory) = 0;

  virtual iAnimationConditionFactory* FindNodeType (csStringID name) const = 0;

  virtual void RegisterResultType (csStringID name, iAnimationResultFactory *factory) = 0;

  virtual iAnimationResultFactory* FindResultType (csStringID name) const = 0;

  virtual iStringSet* GetStringSet () const = 0;
};

}

/**** plugin code ****/

CEL_PLUGIN_NAMESPACE_BEGIN(Animation)
{

using namespace CEL;

class StateSwitcherFactory : public scfImplementation1<StateSwitcherFactory,
  iAnimationNodeFactory>
{
  //...
};
class SpeedBlendFactory : public scfImplementation1<SpeedBlendFactory,
  iAnimationNodeFactory>
{
  //...
};
class DirectionBlendFactory : public scfImplementation1<DirectionBlendFactory,
  iAnimationNodeFactory>
{
  //...
};
class AnimationFactory : public scfImplementation1<AnimationFactory,
  iAnimationNodeFactory>
{
  //...
};

class StandardAnimNodes : public scfImplementation1<StandardAnimNodes,
  iComponent>
{
  struct
  {
    csRef<iAnimationNodeFactory> stateswitcher;
    csRef<iAnimationNodeFactory> speedblend;
    csRef<iAnimationNodeFactory> directionblend;
    csRef<iAnimationNodeFactory> animation;
  } nodefacts;

  struct
  {
    csRef<iAnimationConditionFactory> property;
    csRef<iAnimationConditionFactory> animplayed;
  } condfacts;

  struct
  {
    csRef<iAnimationResultFactory> transition;
    csRef<iAnimationResultFactory> playsound;
    csRef<iAnimationResultFactory> action;
    csRef<iAnimationResultFactory> message;
  } resultfacts;

  //...
};

SCF_IMPLEMENT_FACTORY(celStandardAnimNodes);

StandardAnimNodes::StandardAnimNodes(iBase *parent)
: scfImplementationType(parent),
{
  nodefacts.stateswitcher.AttachNew (new StateSwitcherNodeFactory (this));
  nodefacts.speedblend.AttachNew (new SpeedBlendNodeFactory (this));
  nodefacts.directionblend.AttachNew (new DirectionBlendNodeFactory (this));
  nodefacts.animation.AttachNew (new AnimationNodeFactory (this));

  condfacts.property.AttachNew (new PropertyConditionFactory (this));
  condfacts.animplayed.AttachNew (new AnimationPlayedConditionFactory (this));

  resultfacts.transition.AttachNew (new TransitionResultFactory (this));
  resultfacts.playsound.AttachNew (new PlaySoundResultFactory (this));
  resultfacts.action.AttachNew (new ActionResultFactory (this));
  resultfacts.message.AttachNew (new MessageResultFactory (this));
}

bool StandardAnimNodes::Initialize(iObjectRegistry *objreg)
{
  csRef<iAnimationSystem> animsys = csQueryRegistry<iAnimationSystem> (objreg);
  if (!animsys) return false;

  iStringSet *strset = animsys->GetStringSet();

  animsys->RegisterNodeType (strset->Request ("StateSwitcher"), nodefacts.stateswitcher);
  animsys->RegisterNodeType (strset->Request ("SpeedBlend"), nodefacts.speedblend);
  animsys->RegisterNodeType (strset->Request ("DirectionBlend"), nodefacts.directionblend);
  animsys->RegisterNodeType (strset->Request ("Animation"), nodefacts.animation);

  animsys->RegisterConditionType (strset->Request ("Property"), condfacts.property);
  animsys->RegisterConditionType (strset->Request ("AnimationPlayed"), condfacts.animplayed);

  animsys->RegisterResultType (strset->Request ("Transition"), resultfacts.transition);
  animsys->RegisterResultType (strset->Request ("PlaySound"), resultfacts.playsound);
  animsys->RegisterResultType (strset->Request ("Action"), resultfacts.action);
  animsys->RegisterResultType (strset->Request ("Message"), resultfacts.message);
}

}
CEL_PLUGIN_NAMESPACE_END(Animation)
