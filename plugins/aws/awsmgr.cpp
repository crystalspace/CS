#include "cssysdef.h"
#include "aws.h"

awsManager::awsManager(iBase *p)
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
  canvas.Initialize(sys);

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

void 
awsManager::SetContext(iGraphics2D *g2d, iGraphics3D *g3d)
{
   if (g2d && g3d)
   {
       ptG2D = g2d;
       ptG3D = g3d;
   }
}

void 
awsManager::SetDefaultContext()
{
  ptG2D = canvas.G2D();
  ptG3D = canvas.G3D();
}



 //// Canvas stuff  //////////////////////////////////////////////////////////////////////////////////


awsManager::awsCanvas::awsCanvas ()
{
   
}

awsManager::awsCanvas::~awsCanvas ()
{
}
 
void 
awsManager::awsCanvas::Animate (cs_time current_time)
{
}
