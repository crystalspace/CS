#include "cssysdef.h"
#include "isotest.h"
#include "imesh/skelanim.h"
#include "ivaria/reporter.h"

bool IsoTest::LoadKwartzAnim ()
{
  csRef<Skeleton::Animation::iAnimationFactoryLayer> animfactlay = myskel->GetFactory ()->GetAnimationFactoryLayer ();
  csRef<Skeleton::Animation::iAnimationFactory> animfact;
  int boneid;
  csRef<Skeleton::Animation::iChannel> animchan;
  #include "gencode_kwartz"
  return true;
}
