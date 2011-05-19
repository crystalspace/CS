#include <cssysdef.h>
#include "vplLoader.h"
#include <iutil/objreg.h>
#include <iutil/plugin.h>

#include <iostream>
using namespace std;

SCF_IMPLEMENT_FACTORY (vplLoader)

vplLoader::vplLoader (iBase* parent) :
	scfImplementationType (this, parent),
	object_reg(0)
{
}

vplLoader::~vplLoader ()
{
}

bool vplLoader::Initialize (iObjectRegistry* r)
{
  object_reg = r;
  
  csRef<iPluginManager> mgr=csQueryRegistry<iPluginManager> (object_reg);
  m_pThOggLoader=csLoadPlugin<iVPLLoader> (mgr,
    "crystalspace.vpl.element.thogg");

  return true;
}

csPtr<iVPLData> vplLoader::LoadSound (const char * pFileName, const char *pDescription)
{
  csRef<iVPLData> data;
  if(m_pThOggLoader)
  {
	data=m_pThOggLoader->LoadSound(pFileName,pDescription);
	if(data.IsValid())
	  return csPtr<iVPLData> (data);
  }

  return 0;
}

