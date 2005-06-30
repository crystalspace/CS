#include "cssysdef.h"
#include "manager.h"

#include "csutil/csevent.h"
#include "iengine/engine.h"
#include "iutil/comp.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/virtclk.h"
#include "ivaria/reporter.h"
#include "ivideo/txtmgr.h"


awsManager::awsManager(iBase *the_base)
{
  SCF_CONSTRUCT_IBASE (the_base);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  scfiEventHandler = 0;
}

awsManager::~awsManager()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q)
      q->RemoveListener (scfiEventHandler);

    scfiEventHandler->DecRef ();
  }

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool 
awsManager::Initialize (iObjectRegistry *_object_reg)
{
  object_reg = _object_reg;

  return true;
}

/*********************************************************************
 ***************** Event Handling ************************************
 ********************************************************************/

bool awsManager::HandleEvent (iEvent &Event)
{  
  // Find out what kind of event it is
  switch (Event.Type)
  {
  case csevMouseMove:
  case csevMouseUp:
  case csevMouseClick:
  case csevMouseDown:
  case csevKeyboard:
  case csevBroadcast:
    break;
  }

  return false;
}