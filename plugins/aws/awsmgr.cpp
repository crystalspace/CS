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

void
awsManager::Mark(csRect &rect)
{
   for(int i=0; i<dirty_lid; ++i)
   {
       if (dirty[i].Intersects(rect))
	 dirty[i].AddAdjanced(rect);
   }

   //  If we get here it's because the rectangle didn't fit anywhere. So,
   // add in a new one.
   if (dirty_lid>15)
     dirty[15].AddAdjanced(rect);

   else
     dirty[dirty_lid++].Set(rect);
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
