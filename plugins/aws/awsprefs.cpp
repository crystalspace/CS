#include "cssysdef.h"
#include "isys/plugin.h"
#include "iutil/string.h"
#include "csgeom/csrect.h"
#include "csutil/csdllist.h"
#include "ivaria/iawsprefs.h"
#include "awsprefs.h"



////////////////////////////////////////////////////////

IMPLEMENT_IBASE (awsPrefManager)
  IMPLEMENTS_INTERFACE (iAwsPrefs)
IMPLEMENT_IBASE_END


awsPrefManager::awsPrefManager(iBase *iParent)
{
  CONSTRUCT_IBASE (iParent);
}

awsPrefManager::~awsPrefManager()
{

}

void 
awsPrefManager::Load(char *def_file)
{

}
