#ifndef _CHCPP_H_
#define _CHCPP_H_

#include "csplugincommon/rendermanager/operations.h"
#include "csplugincommon/rendermanager/posteffects.h"
#include "csplugincommon/rendermanager/render.h"
#include "csplugincommon/rendermanager/renderlayers.h"
#include "csplugincommon/rendermanager/shadersetup.h"
#include "csplugincommon/rendermanager/standardsorter.h"
#include "csplugincommon/rendermanager/svsetup.h"
#include <csutil/list.h>

using namespace CS::RenderManager;

typedef CS::RenderManager::RenderTree<
CS::RenderManager::RenderTreeStandardTraits> RenderTreeType;

// Visibility threshold parameter
#define VISIBILITY_THRESHOLD 0

// Frame skip parameter
#define VISIBILITY_SKIP_FRAMES 10

enum OcclusionVisibility
{
  VISIBLE,
  UNKNOWN,
  INVISIBLE,
  INVALID
};

/**
 * Class to hold the visibility information of a kdtree node.
 * The implementation is one that facilitates the use of the  
 * iKDTreeUserData mechanism for storing this information.
 */
class csVisibilityObjectHistory :
    public scfImplementation1<csVisibilityObjectHistory, iKDTreeUserData>
{
public:
  csVisibilityObjectHistory (iGraphics3D* g3d, uint32 uFrame, csKDTree* node)
    : scfImplementationType (this), uQueryFrame (uFrame), uNextCheck (uFrame),
      g3d (g3d), eResult(INVALID), child1 (node->GetChild1 ()), child2 (node->GetChild2 ())
  {
    g3d->OQInitQueries(&uOQuery, 1);
  }

  virtual ~csVisibilityObjectHistory()
  {
    g3d->OQDelQueries (&uOQuery, 1);
  }

  OcclusionVisibility WasVisible (unsigned int uFrame)
  {
    if (uFrame >= uNextCheck)
    {
      uNextCheck = uFrame;
    }

    if (eResult == INVALID || uFrame != uQueryFrame + 1)
    {
      return VISIBLE;
    }

    if (eResult != UNKNOWN)
    {
      return eResult;
    }

    eResult = g3d->OQIsVisible (uOQuery, VISIBILITY_THRESHOLD) ? VISIBLE : INVISIBLE;

    if (eResult == VISIBLE)
    {
      uNextCheck += VISIBILITY_SKIP_FRAMES;
    }

    return eResult;
  }

  void BeginQuery (uint32 uFrame)
  {
    eResult = UNKNOWN;
    uQueryFrame = uFrame;

    g3d->OQBeginQuery (uOQuery);
  }

  void EndQuery ()
  {
    g3d->OQEndQuery ();
  }

  bool CheckVisibility (uint32 uFrame)
  {
    if (uFrame >= uNextCheck)
    {
      return true;
    }

    return false;
  }

  bool MatchChildren (csKDTree* thisNode)
  {
    csKDTree* newChild1 = thisNode->GetChild1 ();
    csKDTree* newChild2 = thisNode->GetChild2 ();

    bool matching = (newChild1 == child1 && newChild2 == child2) ||
                    (newChild1 == child2 && newChild2 == child1);

    child1 = newChild1;
    child2 = newChild2;

    return matching;
  }

  void ResetVisibility ()
  {
    eResult = INVALID;
  }

private:
  iGraphics3D* g3d;
  unsigned int uOQuery;
  uint32 uQueryFrame;
  uint32 uNextCheck;
  OcclusionVisibility eResult;
  csKDTree* child1;
  csKDTree* child2;
};

#endif
