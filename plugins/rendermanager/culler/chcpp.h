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
  csVisibilityObjectHistory (iGraphics3D* g3d, uint32 uTimeStamp)
    : scfImplementationType (this), uQueryTimestamp (uTimeStamp), g3d (g3d), eResult(INVALID)
  {
    g3d->OQInitQueries(&uOQuery, 1);
  }

  virtual ~csVisibilityObjectHistory()
  {
    g3d->OQDelQueries (&uOQuery, 1);
  }

  OcclusionVisibility WasVisible (unsigned int uTimeStamp)
  {
    if (eResult == INVALID || uTimeStamp != uQueryTimestamp + 1)
    {
      return VISIBLE;
    }

    if (eResult != UNKNOWN)
    {
      return eResult;
    }

    if (g3d->OQueryFinished (uOQuery))
    {
      eResult = g3d->OQIsVisible (uOQuery, VISIBILITY_THRESHOLD) ? VISIBLE : INVISIBLE;
    }

    return eResult;
  }

  void BeginQuery (uint32 uTimeStamp)
  {
    eResult = UNKNOWN;
    uQueryTimestamp = uTimeStamp;

    g3d->OQBeginQuery (uOQuery);
  }

  void EndQuery ()
  {
    g3d->OQEndQuery ();
  }

private:
  iGraphics3D* g3d;
  unsigned int uOQuery;
  uint32 uQueryTimestamp;
  OcclusionVisibility eResult;
};

#endif
