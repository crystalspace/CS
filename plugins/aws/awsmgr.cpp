#include "cssysdef.h"
#include "aws.h"

awsManager::awsManager(iBase *p)
{
  CONSTRUCT_IBASE (p);
  CONSTRUCT_EMBEDDED_IBASE(scfiPlugIn);
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
