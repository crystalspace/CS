#include "cssysdef.h"
#include "aws.h"

awsManager::awsManager(iBase *p):canvas(this)
{
  CONSTRUCT_IBASE (p);
  CONSTRUCT_EMBEDDED_IBASE(scfiPlugIn);

  canvas.DisableAutoUpdate();
}

awsManager::~awsManager()
{
}

bool 
awsManager::Initialize(iSystem *sys)
{
  return true;
}

iAwsPrefs *
awsManager::GetPrefMgr()
{
   return prefmgr;
}
 
void
awsManager::SetPrefMgr(iAwsPrefs *pmgr)
{
   if (prefmgr && pmgr)
   {
      prefmgr->DecRef();
      prefmgr=pmgr;
      prefmgr->IncRef();
   }
   else if (pmgr)
   {
      prefmgr=pmgr;
      prefmgr->IncRef();
   }
}

awsWindow *
awsManager::GetTopWindow()
{ return top; }
    

void 
awsManager::SetTopWindow(awsWindow *_top)
{ top = _top; }


 //// Canvas stuff  //////////////////////////////////////////////////////////////////////////////////


awsManager::awsCanvas::awsCanvas (awsManager *_wmgr):wmgr(_wmgr)
{
}
 
awsManager::awsCanvas::~awsCanvas ()
{
}


void 
awsManager::awsCanvas::Animate (cs_time current_time)
{
}

