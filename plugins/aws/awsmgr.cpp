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

void 
awsManager::Load(const char *defs_file)
{
}
