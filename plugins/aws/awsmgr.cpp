#include "cssysdef.h"
#include "isys/plugin.h"
#include "iutil/string.h"
#include "csgeom/csrect.h"
#include "csutil/csdllist.h"
#include "ivaria/iawsprefs.h"
#include "awsprefs.h"
#include "ivaria/iaws.h"
#include "aws.h"

IMPLEMENT_IBASE (awsManager)
  IMPLEMENTS_INTERFACE (iAws)
  IMPLEMENTS_EMBEDDED_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (awsManager::eiPlugIn)                                                               
   IMPLEMENTS_INTERFACE (iPlugIn)                                                                              
IMPLEMENT_EMBEDDED_IBASE_END                                                                                  
                                                                                                               
IMPLEMENT_FACTORY (awsManager)                                                                                
                                                                                                               
EXPORT_CLASS_TABLE (aws)                                                                                      
  EXPORT_CLASS (awsManager, "crystalspace.window.alternatemanager", "Crystal Space alternate window manager") 
EXPORT_CLASS_TABLE_END                                                                                        

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
awsManager::Load(char *defs_file)
{

}

